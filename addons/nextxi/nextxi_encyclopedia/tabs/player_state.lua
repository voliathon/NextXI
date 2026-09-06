local math = require('math')
local string = require('string')

local state = {
    cast_timer = 0.0,
    selected_job = "WAR",
    jobs = {"WAR", "MNK", "WHM", "BLM", "RDM", "THF"}
}

local M = {}

function M.register_commands(cmd, chat)
    cmd:register('player', function()
        local p = windower.ffxi.get_player()
        if p and p.vitals then
            local msg = string.format("[PLAYER] Zone: %d | HP: %d/%d | MP: %d/%d | TP: %d", 
                p.zone_id, p.vitals.hp, p.max_hp, p.vitals.mp, p.max_mp, p.vitals.tp)
            
            -- Dump to in-game chat (Type 206 is standard echo)
            chat.add_text(msg, 206)
            -- Dump to NextXI Control Center Console
            print(msg)
        else
            local err = "[PLAYER] Error: Tracker empty. Wait for next server packet."
            chat.add_text(err, 167)
            print(err)
        end
    end)
end

function M.render_hud(imgui)
    state.cast_timer = state.cast_timer + 0.01
    if state.cast_timer > 1.0 then state.cast_timer = 0.0 end
    imgui.text_colored(0.3, 1.0, 0.3, 1.0, "HUD OVERLAY ACTIVE - Borderless & Clean!")
    imgui.progress_bar(state.cast_timer, string.format("Casting... %d%%", math.floor(state.cast_timer * 100)))
end

function M.render_tab(imgui, main_state, chat)
    state.cast_timer = state.cast_timer + 0.01
    if state.cast_timer > 1.0 then state.cast_timer = 0.0 end

    if imgui.begin_tab_item("Controls & Player") then
        imgui.text("Select Main Job:")
        if imgui.begin_combo("##jobcombo", state.selected_job) then
            for _, job in ipairs(state.jobs) do
                if imgui.selectable(job, state.selected_job == job) then
                    state.selected_job = job
                end
            end
            imgui.end_combo()
        end
        
        imgui.separator()
        imgui.text("Cast Bar Example:")
        imgui.progress_bar(state.cast_timer, "Fire IV")
        
        imgui.separator()
        if imgui.button("Enable Transparent HUD Mode") then main_state.hud_mode = true end
        
        imgui.separator()
        if imgui.button("Dump Player Data to Chat & Console") then
            local p = windower.ffxi.get_player()
            if p and p.vitals then
                local msg = string.format("[PLAYER] HP: %d/%d | MP: %d/%d | TP: %d", 
                    p.vitals.hp, p.max_hp, p.vitals.mp, p.max_mp, p.vitals.tp)
                
                chat.add_text(msg, 206)
                print(msg)
            else
                local err = "[PLAYER] Data empty. Wait for update packet."
                chat.add_text(err, 167)
                print(err)
            end
        end
        imgui.end_tab_item()
    end
end

return M