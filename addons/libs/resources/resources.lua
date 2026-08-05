local enumerable = require('enumerable')
local channel = require('core.channel')

local fetch = channel.get('resources_service', 'resources')

local iterate = function(data, resource_name, index)
    return (next(data[resource_name], index))
end

local constructors = setmetatable({}, {
    __index = function(mts, resource_name)
        local data = fetch:call(function(data, resource_name)
            return data[resource_name] ~= nil
        end, resource_name)

        if not data then
            return nil
        end

        local meta = {}

        meta.__index = function(t, index)
            local data = fetch:read(resource_name, index)
            if data == nil then
                return nil
            end
            data.name = data.en
            return data
        end

        meta.__pairs = function(t)
            return function(t, k)
                local key = fetch:call(iterate, resource_name, k)
                return key, t[key]
            end, t, nil
        end

        meta.__add_element = function(t, el)
            rawset(t, el.id, el)
        end

        local constructor = enumerable.init_type(meta, {})
        mts[resource_name] = constructor
        return constructor
    end,
})

return setmetatable({}, {
    __index = function(_, resource_name)
        local constructor = constructors[resource_name]
        return constructor and constructor()
    end
})
