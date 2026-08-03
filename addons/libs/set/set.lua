local meta = {}

meta.__index = function(_, _)
    error('Cannot access set index.')
end

meta.__newindex = function(_, _, _)
    error('Cannot assign to set indices.')
end

meta.__eq = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            return false
        end
    end

    for el in pairs(data2) do
        if data1[el] == nil then
            return false
        end
    end

    return true
end

meta.__le = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            return false
        end
    end

    return true
end

meta.__lt = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            return false
        end
    end

    for el in pairs(data2) do
        if data1[el] == nil then
            return true
        end
    end

    return false
end

meta.__add = function(s1, s2)
    local data = {}

    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        data[el] = el
    end

    for el in pairs(data2) do
        data[el] = el
    end

    return setmetatable({ data = data }, meta)
end

meta.__mul = function(s1, s2)
    local data = {}

    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] ~= nil then
            data[el] = el
        end
    end

    return setmetatable({ data = data }, meta)
end

meta.__sub = function(s1, s2)
    local data = {}

    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            data[el] = el
        end
    end

    return setmetatable({ data = data }, meta)
end

meta.__pow = function(s1, s2)
    local data = {}

    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            data[el] = el
        end
    end

    for el in pairs(data2) do
        if data1[el] == nil then
            data[el] = el
        end
    end

    return setmetatable({ data = data }, meta)
end

meta.__tostring = function(s)
    local data = s.data
    local el = next(data)
    if el == nil then
        return '{}'
    end

    local res = tostring(el)

    el = next(data, el)
    while el ~= nil do
        res = res .. ', ' .. tostring(el)
        el = next(data, el)
    end

    return '{' .. res .. '}'
end

meta.__ipairs = function(s)
    error('ipairs not defined for sets.')
end

meta.__pairs = function(s)
    return next, s.data, nil
end

meta.__create = function(...)
    local data = {}
    for i = 1, select('#', ...) do
        local el = select(i, ...)
        data[el] = el
    end
    return setmetatable({ data = data }, meta)
end

meta.__convert = function(t)
    local data = {}
    for _, el in pairs(t) do
        data[el] = el
    end
    return setmetatable({ data = data }, meta)
end

meta.__add_element = function(s, el)
    s.data[el] = el
end

meta.__remove_key = function(s, el)
    s.data[el] = nil
end

local set = {}

set.contains = function(s, el)
    return s.data[el] == el
end

set.union = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data2) do
        data1[el] = el
    end
end

set.intersection = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data1) do
        if data2[el] == nil then
            data1[el] = nil
        end
    end
end

set.difference = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data2) do
        data1[el] = nil
    end
end

set.symmetric_difference = function(s1, s2)
    local data1 = s1.data
    local data2 = s2.data

    for el in pairs(data2) do
        data1[el] = data1[el] == nil and el or nil
    end
end

local enumerable = require('enumerable')
return enumerable.init_type(meta, set, 'set')
