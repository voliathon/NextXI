local client = require('shared.client')
local resources = require('resources')

local data, ftype = client.new('account_service')

ftype.fields.server = {
    get = function(data)
        return resources.servers[data.server_id]
    end,
}

return data
