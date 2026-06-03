local client = require('shared.client')
local resources = require('resources')

local data, ftype = client.new('world_service')

ftype.fields.zone = {
    get = function(data)
        return resources.zones[data.zone_id]
    end,
}

ftype.fields.weather = {
    get = function(data)
        return resources.weather[data.weather_id]
    end,
}

return data
