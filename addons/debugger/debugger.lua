local ui = require('ui')
local chat = require('chat')
local command = require('command')
local string = require('string')

local addon = {
    name = 'Debugger',
    author = 'Voliathon',
    version = '1.2'
}

local state = {
    visible = false,
    peak_memory = 0
}

local debug_window = ui.window_state()
debug_window.title = "Fenestra Live Debugger"
debug_window.size = {width = 460, height = 480}
debug_window.resizable = false
debug_window.visible = false

ui.display(function()
    if not state.visible then
        debug_window.visible = false
        return
    end
    
    debug_window.visible = true
    
    local success, err = pcall(function()
        -- FIX: We capture 'still_open'. If you click the 'X', this becomes false.
        local still_open = ui.window(debug_window, function(layout)
            
            local current_memory = collectgarbage("count")
            if current_memory > state.peak_memory then
                state.peak_memory = current_memory
            end
            
            layout:label("[FENESTRA ENGINE MONITOR]{size:large weight:bold color:skin_accent}")
            layout:label("─────────────────────────────────────────", ui.color.system_gray)
            layout:space(5)
            
            layout:label("[LUA MEMORY FOOTPRINT]{weight:bold}")
            
            local mem_color = current_memory > 50000 and ui.color.system_error or ui.color.system_white
            layout:label(string.format("Current Usage: %.2f KB", current_memory), mem_color)
            layout:label(string.format("Peak Usage:    %.2f KB", state.peak_memory), ui.color.system_gray)
            
            layout:space(5)
            
            if layout:button("🗑️ Force GC") then
                collectgarbage("collect")
                chat.success("Debugger: Forced Lua Garbage Collection.")
            end
            
            layout:space(15)

            layout:label("[HOW TO TEST FOR MEMORY LEAKS]{weight:bold color:skin_accent}")
            layout:label("1. Note the 'Current Usage' baseline above.", ui.color.system_gray)
            layout:label("2. Load an addon. Watch the memory rise.", ui.color.system_gray)
            layout:label("3. Unload the addon.", ui.color.system_gray)
            layout:label("4. Click 'Force Garbage Collection'.", ui.color.system_gray)
            layout:label("5. If usage does not return to baseline, it is leaking!", ui.color.system_white)
            
            layout:space(15)
            
            layout:label("[WHY DO WE NEED SAFE UNLOAD?]{weight:bold color:skin_accent}")
            layout:label("Clicking UI buttons to unload addons causes 0xC0000005", ui.color.system_gray)
            layout:label("crashes because memory is destroyed while the UI is drawing.", ui.color.system_gray)
            layout:space(5)
            layout:label("Fix: Wrap commands in [coroutine.schedule(function()]{color:system_white}")
            layout:label("This waits until the NEXT frame to safely destroy memory.", ui.color.system_white)
            
            layout:space(5)
            
            if layout:button("🛑 Safe Unload AddonMgr") then
                coroutine.schedule(function() 
                    require('core.command').input('/unload AddonManager')
                    chat.warning("Debugger: AddonManager safely terminated.")
                end)
            end
            
        end)
        
        -- FIX: Hide the window if the user clicked the top-right X
        if not still_open then
            state.visible = false
        end
    end)
    
    if not success then
        state.visible = false
        chat.error("Debugger Overlay Error: " .. tostring(err))
    end
end)

command.register('debug', function()
    state.visible = not state.visible
    if state.visible then
        chat.success("Debugger Overlay Active. Type /debug to hide.")
    end
end)

return addon