local imgui = require('imgui')
local command = require('core.command')
local theme = require('theme_manager')

local dummy_preview_text = "Sample Input..."

local function HelpMarker(desc)
    imgui.same_line()
    imgui.text_colored(0.5, 0.5, 0.5, 1.0, "(?)")
    if imgui.is_item_hovered() then
        imgui.set_tooltip(desc)
    end
end

local cc_cmd = command.new('control_center')

cc_cmd:register('show', function() theme.state.is_visible = true end)
cc_cmd:register('hide', function() theme.state.is_visible = false end)
cc_cmd:register('preset', function(preset_name)
    theme.load_preset(preset_name)
    print("[control_center] Applied preset: " .. tostring(preset_name))
end)

function imgui_render()
    if not theme.state.is_visible then return end

    imgui.set_next_window_size(540, 640)
    local open = imgui.begin_window("NextXI Theme Studio")
    if not open then
        theme.state.is_visible = false
        imgui.end_window()
        return
    end

    local acc = theme.state.accent
    imgui.text_colored(acc[1], acc[2], acc[3], 1.0, "Control Center Visual Customizer")
    imgui.text("Changes apply instantly to the Control Center and all Addons.")
    imgui.spacing()
    imgui.separator()
    imgui.spacing()

    if imgui.begin_tab_bar("ThemeTabs") then
        
        -- TAB 1: PRESETS
        if imgui.begin_tab_item("Presets") then
            imgui.spacing()
            imgui.text("Select a curated theme:")
            imgui.spacing()

            if imgui.button("PlayOnline Viewer") then theme.load_preset("playonline") end
            imgui.same_line() imgui.text_colored(0.8, 0.7, 0.5, 1.0, "(Silver & Steel Blue)")

            if imgui.button("Warm Tans") then theme.load_preset("warm_tans") end
            imgui.same_line() imgui.text_colored(0.5, 0.6, 0.8, 1.0, "(Classic FFXI Tan & Brown)")

            if imgui.button("Nord Arctic") then theme.load_preset("nord_frost") end
            imgui.same_line() imgui.text_colored(0.6, 0.8, 0.9, 1.0, "(Modern Cool Grey)")
            
            if imgui.button("Navy Blue") then theme.load_preset("navy_blue") end
            imgui.same_line() imgui.text_colored(1.0, 0.4, 0.3, 1.0, "(Navy Blue & Black)")
            
            if imgui.button("Deep Purple") then theme.load_preset("deep_purple") end
            imgui.same_line() imgui.text_colored(0.8, 0.3, 0.9, 1.0, "(Dark Purples & Teal)")

            if imgui.button("Ember Crimson") then theme.load_preset("ember_red") end
            imgui.same_line() imgui.text_colored(0.6, 0.2, 0.8, 0.9, "(Red/Black)")
            
            if imgui.button("Jeuno Marble") then theme.load_preset("jeuno_marble") end
            imgui.same_line() imgui.text_colored(0.8, 0.8, 0.8, 1.0, "(Silver & Light Blue)")
            
            if imgui.button("Cyber Neon") then theme.load_preset("cyber_neon") end
            imgui.same_line() imgui.text_colored(0.9, 0.2, 0.8, 1.0, "(High Contrast Dark)")

            imgui.spacing()
            imgui.separator()
            imgui.spacing()
            
            if imgui.button("Reset to NextXI Default Dark") then
                imgui.reset_default_style()
                theme.state.text_col = {1.0, 1.0, 1.0}
                theme.state.win_bg   = {0.06, 0.06, 0.06}
            end
            HelpMarker("Reverts all UI elements to the engine's built-in dark theme.")

            imgui.end_tab_item()
        end

        -- TAB 2: PALETTE BUILDER
        if imgui.begin_tab_item("Color Palette") then
            imgui.spacing()
            
            imgui.text("Global Text & Backgrounds")
            theme.state.text_col[1], theme.state.text_col[2], theme.state.text_col[3] =
                imgui.color_edit3("Text Color", theme.state.text_col[1], theme.state.text_col[2], theme.state.text_col[3])

            theme.state.win_bg[1], theme.state.win_bg[2], theme.state.win_bg[3] =
                imgui.color_edit3("Window Base", theme.state.win_bg[1], theme.state.win_bg[2], theme.state.win_bg[3])

            theme.state.frame_bg[1], theme.state.frame_bg[2], theme.state.frame_bg[3] =
                imgui.color_edit3("Input Frames", theme.state.frame_bg[1], theme.state.frame_bg[2], theme.state.frame_bg[3])

            imgui.spacing()
            imgui.separator()
            imgui.spacing()
            
            imgui.text("Headers & Tabs")
            theme.state.title_act[1], theme.state.title_act[2], theme.state.title_act[3] =
                imgui.color_edit3("Active Title", theme.state.title_act[1], theme.state.title_act[2], theme.state.title_act[3])

            theme.state.tab_col[1], theme.state.tab_col[2], theme.state.tab_col[3] =
                imgui.color_edit3("Base Tab", theme.state.tab_col[1], theme.state.tab_col[2], theme.state.tab_col[3])
            
            theme.state.tab_act[1], theme.state.tab_act[2], theme.state.tab_act[3] =
                imgui.color_edit3("Active Tab", theme.state.tab_act[1], theme.state.tab_act[2], theme.state.tab_act[3])

            imgui.spacing()
            imgui.separator()
            imgui.spacing()

            imgui.text("Buttons & Accents")
            theme.state.btn_col[1], theme.state.btn_col[2], theme.state.btn_col[3] =
                imgui.color_edit3("Button Base", theme.state.btn_col[1], theme.state.btn_col[2], theme.state.btn_col[3])

            theme.state.accent[1], theme.state.accent[2], theme.state.accent[3] =
                imgui.color_edit3("Accents", theme.state.accent[1], theme.state.accent[2], theme.state.accent[3])

            imgui.spacing()
            if imgui.button("Apply Palette Changes") then
                theme.apply()
            end

            imgui.end_tab_item()
        end

        -- TAB 3: ROUNDING & CORNERS
        if imgui.begin_tab_item("Geometry") then
            imgui.spacing()
            
            theme.state.window_rounding = imgui.slider_float("Window Corners", theme.state.window_rounding, 0.0, 16.0)
            HelpMarker("Adjusts the outside borders of the main windows. (0 = Sharp)")
            
            theme.state.frame_rounding  = imgui.slider_float("Frame Corners", theme.state.frame_rounding, 0.0, 12.0)
            HelpMarker("Adjusts buttons, input boxes, and dropdown menus.")
            
            theme.state.tab_rounding    = imgui.slider_float("Tab Corners", theme.state.tab_rounding, 0.0, 12.0)
            HelpMarker("Adjusts the top corners of navigation tabs.")

            imgui.spacing()
            if imgui.button("Apply Geometry") then
                theme.apply()
            end

            imgui.spacing()
            imgui.separator()
            imgui.spacing()
            
            -- LIVE PREVIEW SECTION
            imgui.text("Live Geometry Preview:")
            imgui.spacing()
            
            if imgui.begin_tab_bar("PreviewTabs") then
                if imgui.begin_tab_item("Active Tab") then
                    imgui.spacing()
                    imgui.button("Sample Button")
                    imgui.same_line()
                    
                    local changed, new_text = imgui.input_text("##dummy", dummy_preview_text)
                    if changed then dummy_preview_text = new_text end
                    
                    imgui.end_tab_item()
                end
                if imgui.begin_tab_item("Inactive Tab") then
                    imgui.end_tab_item()
                end
                imgui.end_tab_bar()
            end

            imgui.end_tab_item()
        end

        imgui.end_tab_bar()
    end

    imgui.end_window()
end