local ffi = require('ffi')

local serializer = require('core.serializer')

local error = error
local type = type

local args = {...}

-- LuaFormatter off
local scan_native = ffi.new(
    'void*(*)(char const*,size_t,char const*,size_t)',
    args[1])
-- LuaFormatter on

local scan = function(signature, module)
    if type(signature) ~= 'string' then
        error('bad argument #1 to \'scan\' (string expected, got ' ..
                  type(signature) .. ')', 2)
    end
    if module == nil then
        module = 'ffximain.dll'
    elseif type(module) ~= 'string' then
        error('bad argument #2 to \'scan\' (string expected, got ' ..
                  type(module) .. ')', 2)
    end

    local result = scan_native(module, #module, signature, #signature)
    if result == nil then -- coalesce nullptr to nil
        result = nil
    end
    return result
end

local scanner = {scan = scan}

serializer.register('__scanner', scanner, false)
serializer.register('__scanner.scan', scanner.scan)

return scanner
