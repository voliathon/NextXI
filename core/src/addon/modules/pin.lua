---@diagnostic disable: unused-local
local serializer = require('core.serializer')

local pin
do
    local pinned_objects = {}
    pin = function(value)
        pinned_objects[value] = value
        return value
    end
end

serializer.register('__pin', pin, false)

return pin
