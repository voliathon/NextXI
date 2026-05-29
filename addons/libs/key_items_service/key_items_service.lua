local bit = require('bit')
local packet = require('packet')
local server = require('shared.server')
local string = require('string')
local struct = require('struct')

local type_size = 0x200
local type_count = 8

local key_item = struct.struct({
    id                  = {struct.uint32},
    available           = {struct.bool},
    examined            = {struct.bool},
})

local key_items = server.new('key_items', key_item[type_count * type_size])

for i = 0, type_count * type_size - 1 do
    key_items[i].id = i
end

local bit_band = bit.band
local bit_lshift = bit.lshift
local string_byte = string.byte

packet.incoming:register_init({
    [{0x055}] = function(p)
        local offset = p.type * type_size
        local available = p.key_items_available
        local examined = p.key_items_examined

        for i = 0, 0x1FF do
            local ki = key_items[i + offset]

            local index = (i / 8) + 1
            local mask = bit_lshift(1, i % 8)
            ki.available = bit_band(string_byte(available, index), mask) ~= 0
            ki.examined = bit_band(string_byte(examined, index), mask) ~= 0
        end
    end,
})
