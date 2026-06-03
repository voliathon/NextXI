local ffi = require('ffi')
local scanner = require('core.scanner')
local types = require('memory:types')
local unicode = require('core.unicode')
local win32 = require('win32')

local byte_ptr = ffi.typeof('char*')
local void_ptr_ptr = ffi.typeof('void**')

local rawset = rawset
local next = next
local ffi_cast = ffi.cast
local scanner_scan = scanner.scan

local get_module_handle = win32.def({
    name = 'GetModuleHandleW',
    returns = 'HMODULE',
    parameters = {
        'LPCWSTR',
    },
})

local modules = setmetatable({
    ['polcore.dll'] = get_module_handle((unicode.to_utf16('polcore.dll'))) == nil and 'polcoreEU.dll' or 'polcore.dll',
}, {
    __index = function(t, k)
        t[k] = k
        return k
    end,
})

local get_scan_ptr = function(ftype)
    local ptr = scanner_scan(ftype.signature, modules[ftype.module])
    if ptr == nil then
        error('Invalid signature for ' .. ftype.name .. ': ' .. ftype.signature)
    end

    return ptr
end

local setup_name = function(name)
    local ftype = types[name]

    local ptr = get_scan_ptr(ftype)

    local offsets = ftype.offsets
    for i = 1, #offsets do
        ptr = ffi_cast(void_ptr_ptr, ffi_cast(byte_ptr, ptr) + offsets[i])[0]
    end

    return ftype, ptr
end

local get_instance = function(ftype, ptr)
    return ffi_cast(ftype.name .. '*', ptr)[0]
end

return setmetatable({}, {
    __index = function(t, k)
        local ftype, ptr = setup_name(k)

        local cdata = get_instance(ftype, ptr)
        rawset(t, k, cdata)
        return cdata
    end,
    __newindex = function(_, k, value)
        types[k] = value
    end,
    __pairs = function(_)
        return function(t, k)
            local key = next(t, k)
            return key, key and t[key]
        end, types, nil
    end,
})
