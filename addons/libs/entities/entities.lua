local memory = require('memory')
local bit = require('bit')

local array = memory.entities

local by_id
do
    local bit_band = bit.band

    by_id = function(id, min, max)
        if bit_band(id, 0xFF000000) ~= 0 then
            local sub_mask = bit_band(id, 0x7FF)
            local index = sub_mask + (bit_band(id, 0x800) ~= 0 and 0x700 or 0)
            if index < min or index > max then
                return nil
            end

            local entity = array[index]
            if entity == nil or entity.id ~= id then
                return nil
            end

            return entity
        end

        for i = min, max - 1 do
            local entity = array[i]
            if entity ~= nil and entity.id == id then
                return entity
            end
        end

        return nil
    end
end

local by_name = function(name, min, max)
    for i = min, max - 1 do
        local entity = array[i]
        if entity ~= nil and entity.name == name then
            return entity
        end
    end

    return nil
end

local index = function(key, min, max)
    if type(key) ~= 'number' or key < min or key > max - 1 then
        return nil
    end

    local entity = array[key]
    return entity ~= nil and entity or nil
end

local iterator = function(min, max)
    return function(t, k)
        k = k + 1
        if k > max - 1 then
            return nil, nil
        end

        local entity = t[k]
        return k, entity ~= nil and entity or nil
    end, array, min - 1
end

local build_table = function(min, max, t)
    t = t or {}

    t.by_id = function(_, id)
        return by_id(id, min, max)
    end

    t.by_name = function(_, name)
        return by_name(name, min, max)
    end

    return setmetatable(t, {
        __index = function(_, k)
            return index(k, min, max)
        end,
        __pairs = function(_)
            return iterator(min, max)
        end,
        __ipairs = pairs,
        __newindex = error,
        __metatable = false,
    })
end

return build_table(0x000, 0x900, {
    npcs = build_table(0x000, 0x400),
    pcs = build_table(0x400, 0x700),
    allies = build_table(0x700, 0x900),
})
