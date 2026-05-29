require('core.event') -- Required for the serializer
local ffi = require('ffi')
local channel = require('core.channel')
local pin = require('core.pin')
local serializer = require('core.serializer')
local struct = require('struct')
local windower = require('core.windower')

ffi.cdef[[
    void* HeapAlloc(void*, uint32_t, size_t);
    bool HeapFree(void*, uint32_t, void*);
    void* HeapCreate(uint32_t, size_t, size_t);
    bool HeapDestroy(void*);
]]

local C = ffi.C
local ffi_cast = ffi.cast
local ffi_gc = ffi.gc
local channel_new = channel.new
local serializer_serialize = serializer.serialize
local struct_name = struct.name

local destroyed = false
local heap = ffi_gc(C.HeapCreate(0, 0, 0), function(heap)
    destroyed = true
    C.HeapDestroy(heap)
end)

local ptr_cache = setmetatable({}, {
    __mode = 'k',
})

local destroy = function(cdata)
    if destroyed then
        return
    end

    C.HeapFree(heap, 0, ptr_cache[cdata])
end

local attach_gc = function(cdata, ptr)
    ptr_cache[cdata] = tonumber(ffi_cast('intptr_t', ptr))
    return ffi_gc(cdata, destroy)
end

local new_ptr = function(ftype)
    struct_name(ftype)
    return ffi_cast(ftype.name .. '*', C.HeapAlloc(heap, 8, ftype.size))
end

local service_name = windower.package_path:gsub('(.+\\)', '')

return {
    new = function(name, ftype)
        name, ftype = ftype and name or 'data', ftype or name

        local server = channel_new(service_name .. '_' .. name)
        pin(server)

        local ptr = new_ptr(ftype)
        server.data = {
            address = tonumber(ffi_cast('intptr_t', ptr)),
            ftype = serializer_serialize(ftype, true),
        }

        return attach_gc(ptr[0], ptr), ftype
    end,
    new_ptr = function(ftype)
        local ptr = new_ptr(ftype)
        return attach_gc(ptr, ptr), ftype
    end,
}
