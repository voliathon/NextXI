-- addons/control_center/theme_manager.lua
local imgui = require('imgui')
local presets = require('presets')

local manager = {}

-- ImGui Color Indices
manager.Col = {
    Text = 0, WindowBg = 2, ChildBg = 3, PopupBg = 4, FrameBg = 7, FrameBgHovered = 8, 
    FrameBgActive = 9, TitleBg = 10, TitleBgActive = 11, TitleBgCollapsed = 12, 
    Button = 21, ButtonHovered = 22, ButtonActive = 23, Header = 24, HeaderHovered = 25, 
    HeaderActive = 26, Tab = 33, TabHovered = 34, TabActive = 35, TabUnfocused = 36, 
    TabUnfocusedActive = 37, CheckMark = 18, SliderGrab = 19, SliderGrabActive = 20
}

manager.state = {
    is_visible = true,
    window_rounding = 6.0, frame_rounding = 4.0, popup_rounding = 4.0, tab_rounding = 4.0,
    text_col = {0.95, 0.95, 0.95}, win_bg = {0.10, 0.10, 0.13}, frame_bg = {0.16, 0.16, 0.21},
    title_bg = {0.18, 0.20, 0.28}, title_act = {0.28, 0.35, 0.52}, btn_col = {0.24, 0.32, 0.48},
    btn_hov = {0.35, 0.45, 0.65}, header_col = {0.20, 0.28, 0.42}, tab_col = {0.16, 0.20, 0.28},
    tab_act = {0.28, 0.40, 0.60}, accent = {0.40, 0.65, 1.00}
}

function manager.apply()
    local s = manager.state
    local C = manager.Col

    imgui.set_rounding(s.window_rounding, s.frame_rounding, s.popup_rounding, s.tab_rounding)
    imgui.set_color(C.Text, s.text_col[1], s.text_col[2], s.text_col[3], 1.00)
    imgui.set_color(C.WindowBg, s.win_bg[1], s.win_bg[2], s.win_bg[3], 0.95)
    imgui.set_color(C.ChildBg, s.win_bg[1] * 0.9, s.win_bg[2] * 0.9, s.win_bg[3] * 0.9, 0.90)
    imgui.set_color(C.PopupBg, s.win_bg[1], s.win_bg[2], s.win_bg[3], 0.98)
    imgui.set_color(C.FrameBg, s.frame_bg[1], s.frame_bg[2], s.frame_bg[3], 1.00)
    imgui.set_color(C.FrameBgHovered, s.frame_bg[1] * 1.2, s.frame_bg[2] * 1.2, s.frame_bg[3] * 1.2, 1.00)
    imgui.set_color(C.FrameBgActive, s.accent[1], s.accent[2], s.accent[3], 0.60)
    imgui.set_color(C.TitleBg, s.title_bg[1], s.title_bg[2], s.title_bg[3], 1.00)
    imgui.set_color(C.TitleBgActive, s.title_act[1], s.title_act[2], s.title_act[3], 1.00)
    imgui.set_color(C.TitleBgCollapsed, s.title_bg[1] * 0.5, s.title_bg[2] * 0.5, s.title_bg[3] * 0.5, 1.00)
    imgui.set_color(C.Button, s.btn_col[1], s.btn_col[2], s.btn_col[3], 0.80)
    imgui.set_color(C.ButtonHovered, s.btn_hov[1], s.btn_hov[2], s.btn_hov[3], 1.00)
    imgui.set_color(C.ButtonActive, s.accent[1], s.accent[2], s.accent[3], 1.00)
    imgui.set_color(C.Header, s.header_col[1], s.header_col[2], s.header_col[3], 0.80)
    imgui.set_color(C.HeaderHovered, s.btn_hov[1], s.btn_hov[2], s.btn_hov[3], 0.90)
    imgui.set_color(C.HeaderActive, s.accent[1], s.accent[2], s.accent[3], 1.00)
    imgui.set_color(C.Tab, s.tab_col[1], s.tab_col[2], s.tab_col[3], 0.86)
    imgui.set_color(C.TabUnfocused, s.tab_col[1] * 0.8, s.tab_col[2] * 0.8, s.tab_col[3] * 0.8, 1.00)
    imgui.set_color(C.TabUnfocusedActive, s.tab_act[1] * 0.7, s.tab_act[2] * 0.7, s.tab_act[3] * 0.7, 1.00)
    imgui.set_color(C.TabHovered, s.btn_hov[1], s.btn_hov[2], s.btn_hov[3], 0.90)
    imgui.set_color(C.TabActive, s.tab_act[1], s.tab_act[2], s.tab_act[3], 1.00)
    imgui.set_color(C.CheckMark, s.accent[1], s.accent[2], s.accent[3], 1.00)
    imgui.set_color(C.SliderGrab, s.accent[1], s.accent[2], s.accent[3], 0.80)
    imgui.set_color(C.SliderGrabActive, s.accent[1], s.accent[2], s.accent[3], 1.00)
end

function manager.load_preset(name)
    local p = presets[name]
    if p then
        for k, v in pairs(p) do
            manager.state[k] = v
        end
        manager.apply()
    end
end

return manager