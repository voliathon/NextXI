local memory = require('memory')

local key_fns = {
    alliance = function() return memory.party.members[0].alliance_info end
}

return setmetatable({}, {
    __index = function(_, key)
        if type(key) ~= 'number' then
            local fn = key_fns[key]
            return fn and fn()
        elseif key < 1 or key > 18 then
            return nil
        end

        local member = memory.party.members[key - 1]
        return member.active and member or nil
    end,
    __newindex = function()
        error('Cannot assign to the \'party\' library.')
    end,
    __pairs = function(_)
        return function(t, k)
            if type(k) ~= 'number' then
                local key, fn = next(key_fns, k)
                if key then
                    local res = fn()
                    return key, res ~= nil and res or nil
                end
                k = 0
            end

            k = k + 1
            for i = k, 18 do
                local member = memory.party.members[i - 1]
                if member.active then
                    return i, member
                end
            end

            return nil, nil
        end, key_fns, nil
    end,
})
