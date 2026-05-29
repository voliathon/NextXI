local class
do
    local getmetatable = getmetatable
    local rawget = rawget
    local tostring = tostring

    class = function(value)
        local class = getmetatable(value)
        if type(class) == 'table' then class = rawget(class, '__class') end
        if class == nil then return nil end
        return tostring(class)
    end
end

return class
