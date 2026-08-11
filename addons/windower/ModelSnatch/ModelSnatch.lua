_addon.name = 'ModelSnatch'
_addon.author = 'Antigravity'
_addon.version = '1.0'
_addon.commands = {'modelsnatch', 'snatch'}

require('tables')
require('logger')
require('strings')
local packets = require('packets')
local texts = require('texts')
local files = require('files')
local res = require('resources')

-- Current snatched entity data
local current_snatch = nil
-- Cache for item names extracted from examine packet 0x0C9
local item_names_cache = {}
-- UI State
local showing_help = false

-- Create UI box
local display_box = texts.new({
    pos = { x = 200, y = 200 },
    bg = { alpha = 200, red = 0, green = 0, blue = 0, visible = true },
    flags = { draggable = true },
    text = { size = 11, font = 'Consolas', alpha = 255, red = 255, green = 255, blue = 255 },
    padding = 6
})

local slot_name_map = {
    [4] = 'head',   -- Head
    [5] = 'body',   -- Body
    [6] = 'hands',  -- Hands
    [7] = 'legs',   -- Legs
    [8] = 'feet',   -- Feet
    [0] = 'main',   -- Main
    [1] = 'sub',    -- Sub
    [2] = 'ranged'  -- Ranged
}

local function get_item_name_ui(names_tbl, slot_id, mode)
    if mode == 'npc' then
        return ""
    end
    if names_tbl and names_tbl[slot_id] then
        return "| " .. names_tbl[slot_id]
    end
    return "| (Unknown/Not Examined)"
end

-- Format the text based on current_snatch
local function update_ui()
    if showing_help then
        local lines = {}
        lines[#lines+1] = "=== ModelSnatch Help ==="
        lines[#lines+1] = "  //snatch pc      - Snatch models & gear names (requires /examine)"
        lines[#lines+1] = "  //snatch npc     - Snatch models without names (for NPCs)"
        lines[#lines+1] = "  //snatch export  - Export snatched data to DressUp text file"
        lines[#lines+1] = "  //snatch hide    - Hide the UI overlay"
        lines[#lines+1] = "  //snatch         - Toggle this help menu"
        lines[#lines+1] = "========================"
        display_box:text(table.concat(lines, '\n'))
        display_box:show()
        return
    end

    if not current_snatch then
        display_box:hide()
        return
    end

    local n = current_snatch.names or {}
    local mode = current_snatch.mode

    local lines = {}
    lines[#lines+1] = string.format("=== ModelSnatch: %s ===", current_snatch.name)
    if mode == 'npc' then
        lines[#lines] = lines[#lines] .. " [NPC]"
    end

    lines[#lines+1] = string.format(" Race : %-5d", current_snatch.race)
    lines[#lines+1] = string.format(" Face : %-5d", current_snatch.models[1])
    lines[#lines+1] = string.format(" Head : %-5d %s", current_snatch.models[2], get_item_name_ui(n, 4, mode))
    lines[#lines+1] = string.format(" Body : %-5d %s", current_snatch.models[3], get_item_name_ui(n, 5, mode))
    lines[#lines+1] = string.format(" Hands: %-5d %s", current_snatch.models[4], get_item_name_ui(n, 6, mode))
    lines[#lines+1] = string.format(" Legs : %-5d %s", current_snatch.models[5], get_item_name_ui(n, 7, mode))
    lines[#lines+1] = string.format(" Feet : %-5d %s", current_snatch.models[6], get_item_name_ui(n, 8, mode))
    lines[#lines+1] = string.format(" Main : %-5d %s", current_snatch.models[7], get_item_name_ui(n, 0, mode))
    lines[#lines+1] = string.format(" Sub  : %-5d %s", current_snatch.models[8], get_item_name_ui(n, 1, mode))
    lines[#lines+1] = string.format(" Range: %-5d %s", current_snatch.models[9], get_item_name_ui(n, 2, mode))
    lines[#lines+1] = "=============================================="
    lines[#lines+1] = "Type //snatch export to save"

    display_box:text(table.concat(lines, '\n'))
    display_box:show()
end

local function snatch_target(target_id)
    local mob = nil
    if target_id then
        mob = windower.ffxi.get_mob_by_id(target_id)
    else
        mob = windower.ffxi.get_mob_by_target('t')
    end

    if mob and mob.models then
        current_snatch = {
            id = mob.id,
            name = mob.name,
            race = mob.race,
            names = item_names_cache[mob.id] or {},
            mode = 'pc',
            models = {
                [1] = mob.models[1] or 0,
                [2] = mob.models[2] or 0,
                [3] = mob.models[3] or 0,
                [4] = mob.models[4] or 0,
                [5] = mob.models[5] or 0,
                [6] = mob.models[6] or 0,
                [7] = mob.models[7] or 0,
                [8] = mob.models[8] or 0,
                [9] = mob.models[9] or 0,
            }
        }
        showing_help = false
        update_ui()
        notice('ModelSnatch: Snatched model data for ' .. mob.name)
    else
        notice('ModelSnatch: Could not find target or model data.')
    end
end

local function snatch_npc(target_id)
    local mob = nil
    if target_id then
        mob = windower.ffxi.get_mob_by_id(target_id)
    else
        mob = windower.ffxi.get_mob_by_target('t')
    end

    if mob and mob.models then
        current_snatch = {
            id = mob.id,
            name = mob.name,
            race = mob.race,
            names = {}, -- Explicitly empty for NPC
            mode = 'npc',
            models = {
                [1] = mob.models[1] or 0,
                [2] = mob.models[2] or 0,
                [3] = mob.models[3] or 0,
                [4] = mob.models[4] or 0,
                [5] = mob.models[5] or 0,
                [6] = mob.models[6] or 0,
                [7] = mob.models[7] or 0,
                [8] = mob.models[8] or 0,
                [9] = mob.models[9] or 0,
            }
        }
        showing_help = false
        update_ui()
        notice('ModelSnatch: Snatched NPC model data for ' .. mob.name)
    else
        notice('ModelSnatch: Could not find target or model data.')
    end
end

local function export_snatch()
    if not current_snatch then
        error('ModelSnatch: No model data to export. /examine or //snatch someone first.')
        return
    end

    local n = current_snatch.names or {}
    local mode = current_snatch.mode

    local function get_item_name_exp(names_tbl, slot_id, mode)
        if mode == 'npc' then return "" end
        if names_tbl and names_tbl[slot_id] then
            return "-- " .. names_tbl[slot_id]
        end
        return "-- (Unknown/Not Examined)"
    end

    -- Format for easy use in DressUp / custom scripts
    local out = {}
    out[#out+1] = string.format("-- Model Data for %s", current_snatch.name)
    out[#out+1] = string.format("Race: %d", current_snatch.race)
    out[#out+1] = string.format("Face: %d", current_snatch.models[1])
    out[#out+1] = string.format("Head: %-5d %s", current_snatch.models[2], get_item_name_exp(n, 4, mode))
    out[#out+1] = string.format("Body: %-5d %s", current_snatch.models[3], get_item_name_exp(n, 5, mode))
    out[#out+1] = string.format("Hands: %-5d %s", current_snatch.models[4], get_item_name_exp(n, 6, mode))
    out[#out+1] = string.format("Legs: %-5d %s", current_snatch.models[5], get_item_name_exp(n, 7, mode))
    out[#out+1] = string.format("Feet: %-5d %s", current_snatch.models[6], get_item_name_exp(n, 8, mode))
    out[#out+1] = string.format("Main: %-5d %s", current_snatch.models[7], get_item_name_exp(n, 0, mode))
    out[#out+1] = string.format("Sub: %-5d %s", current_snatch.models[8], get_item_name_exp(n, 1, mode))
    out[#out+1] = string.format("Ranged: %-5d %s", current_snatch.models[9], get_item_name_exp(n, 2, mode))

    local dir = windower.addon_path .. 'data/'
    if not windower.dir_exists(dir) then
        windower.create_dir(dir)
    end
    
    local path = dir .. current_snatch.name .. '_models.txt'
    local f = io.open(path, 'w')
    if f then
        f:write(table.concat(out, '\n'))
        f:close()
        notice('ModelSnatch: Exported to ' .. path)
    else
        error('ModelSnatch: Failed to write export file.')
    end
end

-- Command handling
windower.register_event('addon command', function(...)
    local args = {...}
    local cmd = args[1] and args[1]:lower()

    if cmd == 'export' then
        export_snatch()
    elseif cmd == 'hide' or cmd == 'close' then
        showing_help = false
        display_box:hide()
        current_snatch = nil
    elseif cmd == 'npc' then
        snatch_npc()
    elseif cmd == 'pc' then
        snatch_target()
    elseif not cmd or cmd == 'help' then
        if showing_help then
            showing_help = false
            update_ui() -- will either hide or show current_snatch
        else
            showing_help = true
            update_ui()
        end
    else
        notice('ModelSnatch: Unknown command. Type //snatch to toggle help UI.')
    end
end)

-- Catch incoming examine equipment packet to get exact item names
windower.register_event('incoming chunk', function(id, data, modified, injected, blocked)
    if id == 0x0C9 then
        local packet = packets.parse('incoming', data)
        if packet['Type'] == 3 then -- Equipment Type
            local target_id = packet['Target ID']
            item_names_cache[target_id] = item_names_cache[target_id] or {}
            
            local count = packet['Count'] or 0
            for i = 1, count do
                local item_id = packet[string.format('Item %d', i)]
                local slot_id = packet[string.format('Slot %d', i)]
                if item_id and slot_id and slot_name_map[slot_id] then
                    local item_data = res.items[item_id]
                    if item_data then
                        item_names_cache[target_id][slot_id] = item_data.en
                    end
                end
            end

            -- Update UI if currently examining this target
            if current_snatch and current_snatch.id == target_id then
                current_snatch.names = item_names_cache[target_id]
                update_ui()
            end
        end
    end
end)

-- Piggyback on /examine (Outgoing Action packet 0x01A, Category 16)
windower.register_event('outgoing chunk', function(id, data, modified, injected, blocked)
    if id == 0x01A then
        local packet = packets.parse('outgoing', data)
        if packet['Category'] == 16 then -- Examine
            local target_id = packet['Target']
            -- Use a slight delay so client registers target or packet processes
            coroutine.schedule(function()
                snatch_target(target_id)
            end, 0.5)
        end
    end
end)
