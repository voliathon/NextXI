_addon.name = 'addon_encyclopedia_w4'
_addon.author = 'NextXI'
_addon.version = '1.0'
_addon.commands = {'ency_w4'}

-- NextXI requires us to load the windower module explicitly
-- We assign it globally so the rest of the W4 script functions normally
windower = require('core.windower')

-- Legacy Windower 4 uses the global event registry
windower.register_event('addon command', function(...)
    local args = {...}
    if #args > 0 and args[1] == 'player' then
        local p = windower.ffxi.get_player()
        if p and p.vitals then
            local msg = string.format("[Legacy W4] HP: %d/%d | MP: %d/%d | TP: %d", 
                p.vitals.hp, p.max_hp, p.vitals.mp, p.max_mp, p.vitals.tp)
            
            windower.add_to_chat(8, msg)
        else
            local err = "[Legacy W4] Error: Tracker empty. Zone to refresh."
            windower.add_to_chat(167, err)
        end
    elseif #args > 0 and args[1] == 'help' then
        windower.add_to_chat(8, "Commands: //ency_w4 player")
    end
end)