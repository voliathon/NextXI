local math = require('math')
local table = require('table')

table.keys = function(t)
    local res = {}
    local count = 0

    for key in pairs(t) do
        count = count + 1
        res[count] = key
    end

    return res
end

do
    local prefix_search
    do
        local math_floor = math.floor

        local find_lower_bound = function(entries, prefix, index, from)
            for i = index - 1, from, -1 do
                if not entries[i]:starts_with(prefix) then
                    return i + 1
                end
            end

            return from
        end

        local find_upper_bound = function(entries, prefix, index, to)
            for i = index + 1, to, 1 do
                if not entries[i]:starts_with(prefix) then
                    return i - 1
                end
            end

            return to
        end

		prefix_search = function(entries, prefix, from, to)
            -- MOVED BOUNDS CHECK TO THE TOP TO PREVENT NIL CRASHES
            if from > to then
                return
            end

            local index = math_floor((to + from) / 2)
            local entry = entries[index]
            
            if entry:starts_with(prefix) then
                return unpack(entries, find_lower_bound(entries, prefix, index, from), find_upper_bound(entries, prefix, index, to))
            end

            if entry < prefix then
                return prefix_search(entries, prefix, index + 1, to)
            else
                return prefix_search(entries, prefix, from, index - 1)
            end
        end
    end

    table.prefix_search = function(t, k)
        return prefix_search(t, k, 1, #t)
    end
end

return table
