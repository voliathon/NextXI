local memory = require('memory')
local d_msg = require('client_data.types.d_msg')

local names = d_msg.new(memory.action_strings.spells)
local size = #names

return setmetatable({}, {
    __index = function(_, id)
        return {
            name = names[id],
        }
    end,
    __pairs = function(t)
        return function(t, i)
            i = i + 1
            if i == size then
                return nil, nil
            end
            return i, t[i]
        end, t, -1
    end,
    __ipairs = pairs,
    __len = function(_)
        return size
    end,
    __newindex = error,
    __metatable = false,
})
