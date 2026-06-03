local client = require('shared.client')
local math = require('math')
local os = require('os')

local data, ftype = client.new('status_effects_service')

ftype.fields.player = {
    get = function(data)
        return data.party[0]
    end,
}

local math_max = math.max
local os_clock = os.clock

ftype.fields.array.type.base.fields.duration = {
    get = function(data)
        return math_max(data._duration_end - os_clock(), 0)
    end,
}

return data
