local string = require('string')

local state = {
    smash_count = 0, auto_smash = false, smash_power = 0.1, smash_level = 1,
    theme_r = 1.0, theme_g = 0.4, theme_b = 0.4
}

local M = {}

function M.render_tab(imgui)
    if imgui.begin_tab_item("Classic Sandbox") then
        imgui.text(string.format("Smash count: %.1f", state.smash_count))
        if imgui.button("Smash Button") then
            state.smash_count = state.smash_count + state.smash_level
        end
        imgui.same_line()
        state.auto_smash = imgui.checkbox("Auto-Smash Mode", state.auto_smash)
        
        if state.auto_smash then
            state.smash_count = state.smash_count + (state.smash_level * state.smash_power)
        end

        imgui.separator()
        state.smash_power = imgui.slider_float("Smash Multiplier", state.smash_power, 0.1, 10.0)
        state.smash_level = imgui.slider_int("Smash Level", state.smash_level, 1, 100)
        
        imgui.separator()
        state.theme_r, state.theme_g, state.theme_b = 
            imgui.color_edit3("Theme Color", state.theme_r, state.theme_g, state.theme_b)
        
        imgui.end_tab_item()
    end
end

return M