local client = require('shared.client')
local resources = require('resources')

local data, ftype = client.new('items_service', 'equipment')

ftype.base.fields.item.type.fields.item = {
    get = function(data)
        return resources.items[data.id]
    end,
}

return data
