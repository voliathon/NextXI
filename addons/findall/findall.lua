local ui = require('ui')
local chat = require('chat')
local command = require('command')
local packet = require('packet')
local settings = require('settings')
local windower = require('core.windower')
local resources = require('resources')
local ffi = require('ffi')

local addon = {
    name = 'findall',
    author = 'Voliathon of Bahamut (Port)',
    version = '1.0'
}

-- Shared global cache for all characters
local cache = settings.load({}, 'findall_cache', true) 
local current_name = "Global"

coroutine.schedule(function()
    coroutine.sleep_frame()
    packet.incoming:register_init({
        [{0x00A}] = function(p)
            if p and p.player_name then
                local name = p.player_name
                if type(name) == 'cdata' then name = ffi.string(name) else name = tostring(name) end
                current_name = name
            end
        end
    })
end)

-- Cache updating routine
local function update_cache()
    if current_name == "Global" then return end
    
    local all_items = windower.ffxi.get_items()
    if not all_items then return end
    
    local char_cache = {}
    for bag_id, bag in pairs(all_items) do
        if type(bag) == 'table' and bag_id ~= 'equipment' then
            for i, item in ipairs(bag) do
                if item and item.id and item.id > 0 then
                    local res_item = resources.items[item.id]
                    if res_item then
                        local name = res_item.name:lower()
                        char_cache[name] = (char_cache[name] or 0) + (item.count or 1)
                    end
                end
            end
        end
    end
    
    cache[current_name] = char_cache
    settings.save('findall_cache', true)
end

-- Update cache every 60 seconds
coroutine.schedule(function()
    while true do
        coroutine.sleep(60)
        update_cache()
    end
end)

command.register({'findall', 'fa'}, function(args)
    if not args[1] then
        chat.error("FindAll: Missing item query. Usage: /findall <item name>")
        return
    end
    
    -- Force update our own cache before searching
    update_cache()
    
    local query = table.concat(args, " "):lower()
    local found = false
    
    chat.success("FindAll: Searching across all characters for '" .. query .. "'...")
    
    for char_name, inv in pairs(cache) do
        for item_name, count in pairs(inv) do
            if item_name:find(query) then
                chat.print(string.format("  [%s] %s : %d", char_name, item_name, count), ui.color.dodgerblue)
                found = true
            end
        end
    end
    
    if not found then
        chat.warning("FindAll: No items matching '" .. query .. "' found.")
    end
end)

return addon
