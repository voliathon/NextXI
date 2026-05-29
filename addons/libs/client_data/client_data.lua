return setmetatable({}, {
    __index = function(client_data, name)
        local data = require('client_data.' .. name)
        rawset(client_data, name, data)
        return data
    end,
    __newindex = error,
    __metatable = false,
})
