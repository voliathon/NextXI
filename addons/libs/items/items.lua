local channel = require('core.channel')
local client = require('shared.client')
local resources = require('resources')
local string = require('string.ext')

local string_normalize = string.normalize

local search_client = channel.get('items_service', 'items_service_search')
local search_call = search_client.call
local search_read = search_client.read

local resources_items = resources.items

local search_prefix = function(_, prefix)
    return search_prefix(prefix)
end

local data, ftype = client.new('items_service', 'items')

ftype.fields.bags.type.base.base.fields.item = {
    get = function(item_data)
        return resources_items[item_data.id]
    end,
}

ftype.fields.find_ids = {
    data = function(_, item_name)
        return search_read(search_client, 'id_map', string_normalize(item_name)) or {}
    end,
}

ftype.fields.search_inventories = {
    data = function(_, item_name)
        return search_read(search_client, 'search_map', string_normalize(item_name)) or {}
    end,
}

ftype.fields.search_prefix = {
    data = function(_, prefix)
        return search_call(search_client, search_prefix, string_normalize(prefix))
    end,
}

return data
