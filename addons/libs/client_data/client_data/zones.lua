local memory = require('memory')
local d_msg = require('client_data.types.d_msg')

local names = d_msg.new(memory.d_msg_table.zones[0])
local at = d_msg.new(memory.d_msg_table.zone_autotranslates[0])
local search = d_msg.new(memory.d_msg_table.zone_search_names[0])
local size = #names

return setmetatable({}, {
    __index = function(_, id)
        return {
            name = names[id][1],
            auto_translate = at[id][1],
            search = search[id][1],
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
