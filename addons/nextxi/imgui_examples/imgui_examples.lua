local imgui = require('imgui')
local command = require('core.command')
local chat = require('core.chat') -- Guaranteed working native chat API

-- Load our modular tabs
local tab_player = require('tabs.player_state')
local tab_grids = require('tabs.data_grids')
local tab_rich = require('tabs.rich_text')
local tab_sandbox = require('tabs.sandbox')

local WindowFlags = {
    None = 0, NoTitleBar = 1, NoResize = 2, NoMove = 4,
    NoScrollbar = 8, AlwaysAutoResize = 64, NoBackground = 128
}

local addon_state = {
    is_visible = true,
    hud_mode = false
}

local example_cmd = command.new('imgui_examples')

example_cmd:register('show', function()
    addon_state.is_visible = true
    chat.add_text("[imgui_examples] UI is now VISIBLE!", 206)
end)

example_cmd:register('hide', function()
    addon_state.is_visible = false
    chat.add_text("[imgui_examples] UI is now HIDDEN!", 206)
end)

-- Pass commands and chat down to the player tab module
tab_player.register_commands(example_cmd, chat)

function imgui_render()
    if not addon_state.is_visible then return end
    
    local flags = WindowFlags.None
    if addon_state.hud_mode then
        flags = WindowFlags.NoTitleBar + WindowFlags.AlwaysAutoResize + WindowFlags.NoBackground
    end

    imgui.set_next_window_size(550, 420)
    local window_open = imgui.begin_window("NextXI Features Showcase", flags)
    if not window_open then
        addon_state.is_visible = false
        imgui.end_window()
        return
    end

    if addon_state.hud_mode then
        tab_player.render_hud(imgui)
        if imgui.button("Exit HUD Mode") then addon_state.hud_mode = false end
    else
        if imgui.begin_tab_bar("ShowcaseTabs") then
            tab_player.render_tab(imgui, addon_state, chat)
            tab_grids.render_tab(imgui)
            tab_rich.render_tab(imgui)
            tab_sandbox.render_tab(imgui)
            imgui.end_tab_bar()
        end
    end
    
    imgui.end_window()
end