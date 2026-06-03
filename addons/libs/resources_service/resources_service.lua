local channel = require('core.channel')

resources = channel.new('resources')

resources.data = setmetatable({}, {
    __index = function(t, k)
        if type(k) ~= 'string' then
            return nil
        end

        local status, resource = pcall(require, 'resources_data:' .. k)
        if not status then
            return nil
        end

        t[k] = resource
        return resource
    end,
})

resources.env = {
    next = next,
}
