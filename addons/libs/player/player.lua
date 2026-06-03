local client = require('shared.client')
local resources = require('resources')

local data, ftype = client.new('player_service')

ftype.fields.state = {
    get = function(data)
        return resources.statuses[data.state_id]
    end
}

ftype.fields.main_job = {
    get = function(data)
        return resources.jobs[data.main_job_id]
    end
}

ftype.fields.sub_job = {
    get = function(data)
        return resources.jobs[data.sub_job_id]
    end
}

ftype.fields.title = {
    get = function(data)
        return resources.titles[data.title_id]
    end
}

ftype.fields.race = {
    get = function(data)
        return resources.races[data.race_id]
    end
}

ftype.fields.home_point_zone = {
    get = function(data)
        return resources.zones[data.home_point_zone_id]
    end
}

return data
