local channel = require('core.channel')

local id_map = channel.get('client_data_service', 'id_map')

local dat_file = setmetatable({}, {
    __index = function(_, id)
        return id_map:read(id)
    end,
    __newindex = error,
    __metatable = false,
})

return dat_file
