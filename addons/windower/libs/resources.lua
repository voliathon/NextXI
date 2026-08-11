--[[
    A library to handle ingame resources, as provided by ResourceExtractor. It will look for the files in Windower/res/.

	This library provides a set of functions to aid in debugging.

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

require('functions')
require('tables')
require('sets')
require('strings')
require('bit')

local functions, table, string = _libs.functions, _libs.tables, _libs.strings

-- Failsafe to prevent recursive loading crashes!
if not functions or not table then
    return {}
end

local fns = {}
local slots = setmetatable({}, {__mode = 'k'})

local language_string = _addon and _addon.language and _addon.language:lower() or windower.ffxi.get_info().language:lower()
local language_string_log = language_string .. '_log'
local language_string_short = language_string .. '_short'

local resource_mt = {}

local resources = setmetatable({}, {__index = function(t, k)
    if k == nil then return nil end
    local fn = assert(fns[k], ('Cannot find resource file "%s".'):format(tostring(k)))
    t[k] = setmetatable(fn(), resource_mt)
    return t[k]
end})

_libs.resources = resources

local redict = {
    name = language_string,
    name_log = language_string_log,
    name_short = language_string_short,
    english = 'en',
    japanese = 'ja',
    english_log = 'enl',
    japanese_log = 'ja',
    english_short = 'ens',
    japanese_short = 'jas',
}

local resource_entry_mt = {
    __index = function(t, k)
        return redict[k] and t[redict[k]] or table[k]
    end,
}

local bit_slots = {
    jobs = true,
    slots = true,
    races = true,
    targets = {
        combination = 'replace',
        values = {
            [0x01] = 'Self', [0x02] = 'Player', [0x04] = 'Party', [0x08] = 'Ally',
            [0x10] = 'NPC', [0x20] = 'Enemy', [0x60] = 'Object', [0x9D] = 'Corpse',
        },
    },
    flags = {
        values = {
            [0x0001] = 'Wall-mounted', [0x0002] = 'Flag01', [0x0004] = 'Obtainable from Goblin Box',
            [0x0008] = 'Usable inside Mog Garden', [0x0010] = 'Can Send POL', [0x0020] = 'Inscribable',
            [0x0040] = 'No Auction', [0x0080] = 'Scroll', [0x0100] = 'Linkshell', [0x0200] = 'Usable',
            [0x0400] = 'NPC Tradeable', [0x0800] = 'Equippable', [0x1000] = 'No NPC Sale',
            [0x2000] = 'No Delivery', [0x4000] = 'No PC Trade', [0x8000] = 'Rare', [0x6040] = 'Exclusive',
        },
    },
}

local resource_group = function(r, fn, attr)
    attr = redict[attr] or attr
    local filter_fn
    
    if type(fn) == 'function' then
        filter_fn = fn
    elseif bit_slots[attr] and class(fn) == 'Set' then
        filter_fn = function(val) return set.subset(val, fn) end
    elseif bit_slots[attr] then
        filter_fn = function(val) return set.contains(val, fn) end
    else
        filter_fn = function(val) return val == fn end
    end

    local res = {}
    for value, id in table.it(r) do
        if value[attr] ~= nil and filter_fn(value[attr]) then
            res[id] = value
        end
    end

    slots[res] = slots[r]
    return setmetatable(res, resource_mt)
end

resource_mt.__class = 'Resource'

resource_mt.__index = function(t, k)
    if slots[t] and slots[t]:contains(k) then
        return function(fn) return resource_group(t, fn, k) end
    end
    return table[k]
end

resource_mt.__tostring = function(t)
    local names = {}
    for _, v in pairs(t) do table.insert(names, v.name) end
    return '{' .. table.concat(names, ', ') .. '}'
end

local resources_path = windower.windower_path .. 'addons/shared_libs/'
local flag_cache = {}

local check_flags = function(bits, lookup)
    if lookup.combination == 'replace' and lookup.values[bits] ~= nil then
        return S{lookup.values[bits]}
    end
    local flags = S{}
    for flag, value in pairs(lookup.values) do
        if bit.band(flag, bits) >= flag then flags:add(value) end
    end
    return flags
end

local parse_flags = function(bits)
    local flags = S{}
    local count = 0
    repeat
        local flag = 2 ^ count
        if bit.band(flag, bits) >= flag then flags:add(count) end
        count = count + 1
    until flag > bits
    return flags
end

local get_flags = function(bits, attribute, lookup)
    flag_cache[attribute] = flag_cache[attribute] or {}
    flag_cache[attribute][bits] = flag_cache[attribute][bits] or (lookup and check_flags(bits, lookup) or parse_flags(bits))
    return flag_cache[attribute][bits]
end

local post_process
local res_names = {}
for _, file in ipairs(windower.get_dir(resources_path)) do
    if string.sub(file, -4) == '.lua' then
        table.insert(res_names, string.sub(file, 1, -5))
    end
end

for _, res_name in ipairs(res_names) do
    fns[res_name] = function()
        local res, slot_table = dofile(resources_path .. res_name .. '.lua')
        for _, v in pairs(res) do
            if type(v) == 'table' then setmetatable(v, resource_entry_mt) end
        end
        slots[res] = S(slot_table)
        post_process(res)
        return res
    end
end

local fn_cache = {}

post_process = function(t)
    local slot_set = slots[t]
    for key in slot_set:it() do
        if bit_slots[key] then
            fn_cache[key] = function(bits)
                return get_flags(bits, key, bit_slots[key] ~= true and bit_slots[key] or nil)
            end
        end
    end

    for _, entry in pairs(t) do
        for key, fn in pairs(fn_cache) do
            if entry[key] ~= nil then
                entry[key] = fn(entry[key])
            end
        end
    end

    for key in pairs(redict) do
        slot_set:add(key)
    end
end

return resources