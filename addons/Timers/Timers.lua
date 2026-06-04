local ui = require('ui')
local packets = require('packets')
local os = require('os')

local timers = {}
local active_timers = {}

local timers_window = ui.window_state()
timers_window.title = "Timers"
timers_window.size = {width = 250, height = 300}
timers_window.resizable = true
timers_window.visible = true
timers_window.style = "chromeless"
timers_window.color = ui.color(0, 0, 0, 150)

-- Example: Listen for Action Packets (0x028)
-- In a real implementation, we would use the resources library to get the duration.
packets.on('incoming', 0x028, function(action)
    -- Just a dummy check to create a timer when any action occurs
    -- We assume action.param contains the spell id.
    local spell_name = "Spell " .. tostring(action.param)
    local duration = 30 -- 30 seconds dummy duration
    
    table.insert(active_timers, {
        name = spell_name,
        start_time = os.clock(),
        duration = duration,
        color = ui.color.dodgerblue
    })
end)

ui.display(function()
    if #active_timers > 0 then
        ui.window(timers_window, function(layout)
            layout:label("[Active Timers]{color:skin_accent weight:bold}")
            layout:space(5)
            
            local current_time = os.clock()
            local i = 1
            
            while i <= #active_timers do
                local timer = active_timers[i]
                local elapsed = current_time - timer.start_time
                local remaining = timer.duration - elapsed
                
                if remaining <= 0 then
                    table.remove(active_timers, i)
                else
                    layout:label(timer.name .. " (" .. string.format("%.1f", remaining) .. "s)")
                    local progress = remaining / timer.duration
                    
                    -- Progress bar drawing
                    -- ImGui wrapper requires size, we will use percentage.
                    layout:progress(progress, timer.color, false)
                    layout:space(2)
                    i = i + 1
                end
            end
        end)
    end
end)
