--[[
	Copyright © 2026, NextXI Contributors
	Copyright © 2013-2015, Windower
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
	  documentation and/or other materials provided with the distribution.
	* Neither the name of Windower nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
	BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
	SHALL Windower BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]


_libs = _libs or {}

require('lists')

local math = require('math')

local res = require('resources')

local slips = {}

_libs.slips = slips

slips.default_storages = {'inventory', 'safe', 'storage', 'locker', 'satchel', 'sack', 'case', 'wardrobe', 'safe2', 'wardrobe2', 'wardrobe3', 'wardrobe4', 'wardrobe5', 'wardrobe6', 'wardrobe7', 'wardrobe8'}
slips.storages = L(res.slips:map(function(slip) return slip.item_id end))
slips.items = res.slips:rekey('item_id'):map(function(slip) return L(slip.items) end)

function slips.get_slip_id(n)
    if n > slips.storages:length() then
        return nil
    end

    return slips.storages[n]
end

function slips.get_slip_by_id(id)
    return slips.items[id]
end

function slips.get_slip(n)
    local id = slips.get_slip_id(n)

    if id == nil then
        return nil
    end

    return slips.get_slip_by_id(id)
end

function slips.get_slip_number_by_id(id)
    if slips.items[id] == nil then
        return nil
    end

    return slips.storages:find(id)
end

function slips.get_slip_id_by_item_id(id)
    for _, slip_id in ipairs(slips.storages) do
        if slips.items[slip_id]:contains(id) then
            return slip_id
        end
    end

    return nil
end

function slips.get_slip_by_item_id(id)
    local slip_id = slips.get_slip_id_by_item_id(id)

    if slip_id == nil then
        return nil
    end

    return slips.get_slip_by_id(slip_id)
end

function slips.get_item_bit_position(id, slip_id)
    local slip

    if slip_id ~= nil then
        if slips.items[slip_id] == nil then
            return nil
        end

        slip = slips.items[slip_id]
    else
        slip = slips.get_slip_by_item_id(id)

        if slip == nil then
            return nil
        end
    end

    local bit_position = slip:find(id)

    return bit_position
end

function slips.get_slip_page_by_item_id(id, slip_id)
    local bitPosition = slips.get_item_bit_position(id, slip_id)

    if bitPosition == nil then
        return nil
    end

    return math.floor(bitPosition / 16) + 1
end

function slips.player_has_item(id)
    local slip_id = slips.get_slip_id_by_item_id(id)

    if slip_id == nil then
        return false
    end

    local items = windower.ffxi.get_items()
    -- Add failsafe
    if not items then return false end

    for _, storage in ipairs(slips.default_storages) do
        if items[storage] then
            for _, item in ipairs(items[storage]) do
                -- Restored the missing item.id check!
                if item.id == slip_id then
                    local bit_position = slips.get_item_bit_position(id, slip_id)
                    local bitmask      = item.extdata:byte(math.floor((bit_position - 1) / 8) + 1)

                    if bitmask < 0 then
                        bitmask = bitmask + 256
                    end

                    local bit = math.floor((bitmask / 2 ^ ((bit_position - 1) % 8)) % 2)

                    return bit ~= 0
                end
            end
        end
    end

    return false
end

function slips.get_player_items()
    local slips_items = T{}

    for _, slip_id in ipairs(slips.storages) do
        slips_items[slip_id] = L{}
    end

    local items = windower.ffxi.get_items()
    
    -- Added the missing failsafe here!
    if not items then return slips_items end

    for _, storage in ipairs(slips.default_storages) do
        -- Added the missing storage check here!
        if items[storage] then
            for _, item in ipairs(items[storage]) do
                if slips.storages:contains(item.id) then
                    for bit_position = 0, item.extdata:length() * 8 - 1 do
                        local bitmask = item.extdata:byte(math.floor(bit_position / 8) + 1)

                        if bitmask < 0 then
                            bitmask = bitmask + 256
                        end

                        local bit = math.floor((bitmask / 2 ^ (bit_position % 8)) % 2)

                        if bit ~= 0 and slips.items[item.id] then
                            slips_items[item.id]:append(slips.items[item.id][bit_position + 1])
                        end
                    end
                end
            end
        end
    end

    return slips_items
end

return slips
