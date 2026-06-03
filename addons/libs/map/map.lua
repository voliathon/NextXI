local ffi = require('ffi')
local key_items = require('key_items')
local math = require('math')
local memory = require('memory')
local scanner = require('core.scanner')
local string = require('string')
local target = require('target')
local world = require('world')

ffi.cdef[[
    typedef int (__fastcall *map_fn)(void*, int, float, float, float);
]]

local fn = ffi.cast('map_fn', scanner.scan('&8B542408568D4424108BF18B4C2410508B44240C'))
local this = ffi.cast('void**', scanner.scan('8B7424148B4424108B7C240C8B0D'))[0]

local math_floor = math.floor
local string_char = string.char

local zones = {}
do
    local key_item_offsets = {
        [0] = 384,
        [1] = 1855,
        [2] = 2301,
    }

    local ptr = memory.map_table.ptr
    local i = 0
    while true do
        local entry = ptr[i]
        if entry.zone_id <= 0 then
            break
        end

        local zone = zones[entry.zone_id]
        if not zone then
            zone = {}
            zones[entry.zone_id] = zone
        end
        zone[entry.map_id] = entry
        zone.key_item_offset = key_item_offsets[entry.key_item_offset]
        zone.key_item_index = entry.key_item_index

        i = i + 1
    end
end

return {
    available = function(zone_id)
        local zone = zones[zone_id or world.zone_id]
        return key_items[zone.key_item_offset + zone.key_item_index].available
    end,
    coordinates = function(x, y, z)
        if y == nil then
            local entity = x or target.me
            local pos = entity.position
            x, y, z = pos.x, pos.y, pos.z
        end

        local map_id = fn(this, 0, x, z, y)
        local maps = zones[world.zone_id]
        local entry = maps[map_id]

        return
            x * entry.scale / 1200 - entry.offset_x / 240 - 16 / 15,
            -y * entry.scale / 1200 - entry.offset_y / 240 - 16 / 15,
            map_id
    end,
    position = function(x, y, z)
        if y == nil then
            local entity = x or target.me
            local pos = entity.position
            x, y, z = pos.x, pos.y, pos.z
        end

        local map_id = fn(this, 0, x, z, y)
        local maps = zones[world.zone_id]
        local entry = maps[map_id]

        local x_pos = x * entry.scale / 160 - entry.offset_x / 32 + 0.5
        local y_pos = -y * entry.scale / 160 - entry.offset_y / 32 + 0.5

        return '(' .. string_char(x_pos + 0x40) .. '-' .. tostring(math_floor(y_pos)) .. ')'
    end,
}
