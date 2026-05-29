local client = require('shared.client')
local entities = require('entities')
local event = require('core.event')
local os = require('os')
local math = require('math')

local data, ftype = client.new('action_service')

ftype.fields.action.type.fields.targets = {
    get = function(data)
        local target_ids = data.target_ids
        local target_ids_length = #target_ids
        return setmetatable({}, {
            __len = function(_)
                for i = 0, target_ids_length - 1 do
                    if target_ids[i] == 0 then
                        return i
                    end
                end
                return target_ids_length
            end,
            __index = function(_, k)
                return entities:by_id(target_ids[k])
            end,
            __pairs = function(t)
                return function(t, k)
                    k = k + 1
                    if k == target_ids_length then
                        return nil, nil
                    end

                    return k, t[k]
                end, t, -1
            end,
            __ipairs = pairs,
            __newindex = error,
            __metatable = false,
        })
    end,
}

local math_max = math.max
local os_clock = os.clock

ftype.fields.spells.type.base.fields.recast = {
    get = function(data)
        return math_max(data._recast_end - os_clock(), 0)
    end,
}

ftype.fields.spells.type.base.fields.available = {
    get = function(data)
        return data.learned and data._level_requirements
    end,
}

ftype.fields.spells.type.base.fields.ready = {
    get = function(data)
        return data.learned and data.available and data.recast == 0
    end,
}

local job_ability_recasts = data.job_ability_recasts
ftype.fields.job_abilities.type.base.fields.recast = {
    get = function(data)
        return math_max(job_ability_recasts[data._recast_id].recast_end - os_clock(), 0)
    end,
}

ftype.fields.job_abilities.type.base.fields.ready = {
    get = function(data)
        return data.available and data.recast == 0
    end,
}

ftype.fields.weapon_skills.type.base.fields.ready = {
    get = function(data)
        return data.available
    end,
}

ftype.fields.mounts.type.base.fields.ready = {
    get = function(data)
        return data.available
    end,
}

local get_event = function(service_event)
    local ev = event.new()
    service_event:register(function()
        ev:trigger(data.action)
    end)
    return ev
end

return {
    filter_action = get_event(data.filter_action),
    pre_action = get_event(data.pre_action),
    mid_action = get_event(data.mid_action),
    post_action = get_event(data.post_action),
    category = data.category,
    spells = data.spells,
    job_abilities = data.job_abilities,
    weapon_skills = data.weapon_skills,
    mounts = data.mounts,
}
