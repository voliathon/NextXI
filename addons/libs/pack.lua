
local ffi = require('ffi')

local pack = {}
function string.unpack(str, fmt, pos)
    pos = pos or 1
    if fmt == 'H' or fmt == 'S' then
        if pos + 1 <= #str then
            local b1 = string.byte(str, pos)
            local b2 = string.byte(str, pos + 1)
            return (b2 * 256) + b1, pos + 2
        end
    elseif fmt == 'I' or fmt == 'i' then
        if pos + 3 <= #str then
            local b1 = string.byte(str, pos)
            local b2 = string.byte(str, pos + 1)
            local b3 = string.byte(str, pos + 2)
            local b4 = string.byte(str, pos + 3)
            local val = b1 + (b2 * 256) + (b3 * 65536) + (b4 * 16777216)
            if fmt == 'i' and val >= 2147483648 then
                val = val - 4294967296
            end
            return val, pos + 4
        end
    end
    return 0, pos
end

return pack
