require('core.event') -- Required for the serializer
local channel = require('core.channel')
local serializer = require('core.serializer')
local struct = require('struct')

local channel_get = channel.get
local serializer_deserialize = serializer.deserialize
local struct_name = struct.name
local struct_metatype = struct.metatype
local struct_from_ptr = struct.from_ptr

local prepared = {}

local prepare_struct
local prepare_array

local configure_ftype = function(ftype)
    local count = ftype.count
    local fields = ftype.fields

    if count then
        prepare_array(ftype)
    elseif fields then
        prepare_struct(ftype)
    end

    local name = ftype.name
    if not name or prepared[name] then
        return
    end

    if count or fields then
        struct_name(ftype, name)
        struct_metatype(ftype)
    end

    prepared[name] = true
end

prepare_struct = function(struct)
    for _, field in pairs(struct.fields) do
        local ftype = field.type
        if ftype then
            configure_ftype(ftype)
        end
    end
end

prepare_array = function(array)
    local ftype = array.base
    if ftype then
        configure_ftype(ftype)
    end
end

return {
    new = function(package_name, data_name)
        data_name = data_name or 'data'

        local data_client = channel_get(package_name, package_name .. '_' .. data_name)
        local data = data_client:read()

        local ftype = serializer_deserialize(data.ftype)
        configure_ftype(ftype)

        return struct_from_ptr(ftype, data.address), ftype
    end,
    configure = function(ftype)
        configure_ftype(ftype)
    end,
}
