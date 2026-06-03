local client = require('shared.client')
local resources = require('resources')

local key_items = resources.key_items

local data, ftype = client.new('key_items_service', 'key_items')

ftype.base.fields.key_item = {
    get = function(data)
        return key_items[data.id]
    end,
}

return data
