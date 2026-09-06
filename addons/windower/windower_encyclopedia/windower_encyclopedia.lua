_addon = _addon or {}
_addon.name = 'windower_encyclopedia'
_addon.author = 'NextXI'
_addon.version = '1.0'
_addon.commands = {'win_enc'}

-- We can safely require ImGui even in a legacy script!
local imgui = require('imgui')

-- Set to true so it pops open immediately when loaded!
local show_ui = true

windower.register_event('addon command', function(...)
    local args = {...}
    local cmd = args[1] and args[1]:lower() or 'help'
    
    if cmd == 'ui' then
        show_ui = not show_ui
        windower.add_to_chat(8, "[W4 Encyclopedia] ImGui UI toggled.")
    elseif cmd == 'player' then
        local p = windower.ffxi.get_player()
        if p and p.vitals then
            windower.add_to_chat(8, string.format("[W4 Encyclopedia] HP: %d | MP: %d | TP: %d", p.vitals.hp, p.vitals.mp, p.vitals.tp))
        else
            windower.add_to_chat(167, "[W4 Encyclopedia] Vitals unavailable.")
        end
    else
        windower.add_to_chat(8, "=== Windower 4 Encyclopedia Index ===")
        windower.add_to_chat(8, " //win_enc player  : Prints vitals to chat")
        windower.add_to_chat(8, " //win_enc ui      : Toggles modern ImGui Demo window")
    end
end)

-- The engine will automatically detect and call this, granting legacy W4 addons modern UIs!
function imgui_render()
    if not show_ui then return end
    
    imgui.set_next_window_size(350, 220)
    local open = imgui.begin_window("W4 Legacy UI Bridge", 0)
    if not open then
        show_ui = false
    else
        imgui.text_colored(1.0, 0.5, 1.0, 1.0, "Legacy Addon, Modern UI!")
        imgui.separator()
        imgui.text("Welcome to the Windower 4 Encyclopedia.")
        imgui.text("This window spawned automatically on load.")
        
        imgui.spacing()
        imgui.text_colored(0.7, 0.7, 0.7, 1.0, "Instructions:")
        imgui.text("1. You can close this window with the 'X'.")
        imgui.text("2. Type //win_enc ui to bring it back.")
        imgui.text("3. Type //win_enc help for more commands.")
        
        imgui.spacing()
        if imgui.button("Print Chat from UI") then
            windower.add_to_chat(8, "[W4 Encyclopedia] Triggered via ImGui Button!")
        end
    end
    imgui.end_window()
end

-- === Auto-Execute on Load ===
-- Because this is in the main chunk, it runs exactly when you click 'Load'
windower.add_to_chat(206, "=== Windower 4 Encyclopedia Loaded ===")
windower.add_to_chat(206, "The UI window has been opened automatically.")
windower.add_to_chat(206, "Type //win_enc help for chat commands.")