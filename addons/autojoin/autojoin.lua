local ui = require('ui')
local chat = require('chat')
local command = require('command')
local packet = require('packet')
local settings = require('settings')

local addon = {
    name = 'autojoin',
    author = 'Voliathon of Bahamut (Port)',
    version = '1.0'
}

local defaults = {
    whitelist = {},
    mode = 'whitelist' -- 'whitelist', 'all', 'none'
}
local options = settings.load(defaults)

local autojoin_window = ui.window_state()
autojoin_window.title = "AutoJoin"
autojoin_window.size = {width = 300, height = 400}
autojoin_window.resizable = false
autojoin_window.visible = false
local show_ui = false

packet.incoming:register({
    [{0x074}] = function(p)
        -- Packet 0x074 is party invite
        if not p.name then return end
        local inviter = tostring(p.name):lower()
        
        local should_join = false
        if options.mode == 'all' then
            should_join = true
        elseif options.mode == 'whitelist' then
            for _, v in ipairs(options.whitelist) do
                if tostring(v):lower() == inviter then
                    should_join = true
                    break
                end
            end
        end
        
        if should_join then
            chat.success("AutoJoin: Accepting party invite from " .. inviter:upper())
            coroutine.schedule(function()
                coroutine.sleep(1)
                -- Send party accept command
                command.input('/join')
            end)
        end
    end
})

ui.display(function()
    if not show_ui then return end
    
    autojoin_window.visible = true
    local still_open = ui.window(autojoin_window, function(layout)
        layout:label("[AutoJoin Configuration]{color:skin_accent weight:bold}")
        layout:space(10)
        
        layout:label("Mode: " .. options.mode:upper())
        layout:same_line()
        if layout:button("btn_mode", " Toggle Mode ", false) then
            if options.mode == 'whitelist' then options.mode = 'all'
            elseif options.mode == 'all' then options.mode = 'none'
            else options.mode = 'whitelist' end
            settings.save()
        end
        
        layout:space(10)
        layout:label("Whitelist:")
        for i, name in ipairs(options.whitelist) do
            layout:label(" - " .. name)
        end
        
        layout:space(10)
        layout:label("Use '/autojoin add <name>' to whitelist.")
    end)
    
    if not still_open then show_ui = false end
end)

command.register({'autojoin', 'aj'}, function(args)
    if args[1] == 'add' and args[2] then
        table.insert(options.whitelist, args[2])
        settings.save()
        chat.success("AutoJoin: Added " .. args[2] .. " to whitelist.")
    elseif args[1] == 'remove' and args[2] then
        for i, v in ipairs(options.whitelist) do
            if v:lower() == args[2]:lower() then
                table.remove(options.whitelist, i)
                settings.save()
                chat.success("AutoJoin: Removed " .. args[2] .. " from whitelist.")
                return
            end
        end
    else
        show_ui = not show_ui
    end
end)

return addon
