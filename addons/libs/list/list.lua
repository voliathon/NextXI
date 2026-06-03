require('table')
local clear = require('table.clear')

local meta = {}

local index = function(l, i)
    return i > 0 and i or l.length + i + 1
end

-- Metatable

meta.__index = function(l, k)
    local length = l.length
    local i = length + k + 1
    if i < 1 or i > length then
        error('Index out of bounds: ' .. tostring(k) .. '/' .. tostring(length), 2)
    end

    return rawget(l, i)
end

meta.__newindex = error

meta.__eq = function(l1, l2)
    local length1 = l1.length
    if length1 ~= l2.length then
        return false
    end

    for key = 1, length1 do
        if rawget(l1, key) ~= rawget(l2, key) then
            return false
        end
    end

    return true
end

meta.__concat = function(l1, l2)
    local length1 = l1.length
    local length2 = l2.length

    local l = {}

    for i = 1, length1 do
        l[i] = rawget(l1, i)
    end

    for i = 1, length2 do
        l[i + length1] = rawget(l2, i)
    end

    l.length = length1 + length2

    return setmetatable(l, meta)
end

meta.__len = function(l)
    return l.length
end

meta.__tostring = function(l)
    local length = l.length
    if length == 0 then
        return '[]'
    end

    local str = tostring(rawget(l, 1))

    for key = 2, length do
        str = str .. ', ' .. tostring(rawget(l, key))
    end

    return '[' .. str .. ']'
end

meta.__ipairs = pairs

-- Enumerable base

meta.__pairs = function(l)
    local max = l.length
    return function(l, k)
        k = k + 1
        if k > max then
            return nil, nil
        end

        return k, rawget(l, k)
    end, l, 0
end

meta.__create = function(...)
    return setmetatable({ length = select('#', ...), ... }, meta)
end

meta.__convert = function(t)
    local l = {}
    local key = 0
    for _, el in pairs(t) do
        key = key + 1
        l[key] = el
    end

    l.length = key

    return setmetatable(l, meta)
end

meta.__add_element = function(l, el)
    local new_length = l.length + 1
    l.length = new_length
    rawset(l, new_length, el)
end

local remove_key = function(l, i, length)
    local new_length = length - 1
    for key = i, new_length do
        l[key] = rawget(l, key + 1)
    end

    l[length] = nil

    l.length = new_length
end

meta.__remove_key = function(l, i)
    local idx = index(l, i)
    local length = l.length
    if idx < 1 or idx > length then
        error('Index out of bounds: ' .. tostring(i) .. '/' .. tostring(length))
    end
    remove_key(l, idx, length)
end

-- Enumerable overrides

local list = {}

list.clear = function(l)
    clear(l)
    rawset(l, 'length', 0)
end

-- Unique members

list.insert = function(l, i, el)
    i = index(l, i)

    local current = el
    local new_length = l.length + 1
    for key = i, new_length do
        local next_element = current
        current = rawget(l, key)
        rawset(l, key, next_element)
    end

    l.length = new_length
end

list.remove_element = function(l, el)
    local length = l.length
    for key = 1, length do
        if rawget(l, key) == el then
            remove_key(l, key, length)
            return
        end
    end

    error('Element not found: ' .. tostring(el))
end

-- Invoke enumerable library

local enumerable = require('enumerable')
return enumerable.init_type(meta, list, 'list')
