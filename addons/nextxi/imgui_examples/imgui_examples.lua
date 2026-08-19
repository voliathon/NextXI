local imgui = require('imgui')
local command = require('core.command')
local string = require('string') 

local addon_state = {
    is_visible = true,
    smash_count = 0,
    auto_smash = false,
    smash_power = 1.0,
    smash_level = 1,
    theme_r = 1.0,
    theme_g = 0.4,
    theme_b = 0.4
}

local caveman_cmd = command.new('imgui_examples')

caveman_cmd:register('show', function()
    addon_state.is_visible = true
    print("[imgui_examples] UI is now VISIBLE! Ooga Booga!")
end)

caveman_cmd:register('hide', function()
    addon_state.is_visible = false
    print("[imgui_examples] UI is now HIDDEN!")
end)

function imgui_render()
    if not addon_state.is_visible then return end
    
    local window_open = imgui.begin_window("NextXI imgui Examples")
    if not window_open then
        addon_state.is_visible = false
        imgui.end_window()
        return
    end

    imgui.text("Welcome to the NextXI ImGui Sandbox!")
    if imgui.is_item_hovered() then
        imgui.set_tooltip("Hover tooltips work perfectly!")
    end

    imgui.separator()
    
    if imgui.collapsing_header("Smash Controls") then
        imgui.text("Smash count: " .. tostring(addon_state.smash_count))
        
        if imgui.button("Smash Button") then
            addon_state.smash_count = addon_state.smash_count + addon_state.smash_level
        end

        imgui.same_line()
        addon_state.auto_smash = imgui.checkbox("Auto-Smash Mode", addon_state.auto_smash)
        
        if addon_state.auto_smash then
            addon_state.smash_count = addon_state.smash_count + (addon_state.smash_level * addon_state.smash_power)
        end

        imgui.separator()
        addon_state.smash_power = imgui.slider_float("Smash Multiplier", addon_state.smash_power, 0.1, 10.0)
        addon_state.smash_level = imgui.slider_int("Smash Level", addon_state.smash_level, 1, 100)
    end

    if imgui.collapsing_header("Style Options") then
        addon_state.theme_r, addon_state.theme_g, addon_state.theme_b = 
            imgui.color_edit3("Theme Color", addon_state.theme_r, addon_state.theme_g, addon_state.theme_b)
        
        imgui.text(string.format("RGB Values: %.2f, %.2f, %.2f", addon_state.theme_r, addon_state.theme_g, addon_state.theme_b))
    end
    
    imgui.end_window()
end