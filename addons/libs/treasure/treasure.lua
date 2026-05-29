local client = require('shared.client')
local items = require('client_data.items')

local data, ftype = client.new('treasure_service')

ftype.fields.pool.type.base.fields.item = {
    get = function(data)
        return items[data.item_id]
    end,
}

return data
