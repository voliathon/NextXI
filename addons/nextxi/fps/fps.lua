_addon.name = 'fps'
_addon.author = 'NextXI'
_addon.version = '1.0.0'
_addon.commands = {'fps'}

local os = require('os')
local string = require('string')
local imgui = require('imgui')
local windower = require('core.windower')

local show_fps = true
local last_time = os.clock()
local frames = 0
local current_fps = 0

function imgui_render()
    local current_time = os.clock()
    frames = frames + 1
    if current_time - last_time >= 1.0 then
        current_fps = frames
        frames = 0
        last_time = current_time
    end

    if not show_fps then return end

    -- Flag 235 removes borders, titlebar, backgrounds, and auto-resizes
    imgui.begin_window("FPS Monitor", 235)
    
    if current_fps >= 29 then
        imgui.text_colored(0.2, 1.0, 0.2, 1.0, string.format("FPS: %d", current_fps))
    elseif current_fps >= 20 then
        imgui.text_colored(1.0, 1.0, 0.2, 1.0, string.format("FPS: %d", current_fps))
    else
        imgui.text_colored(1.0, 0.2, 0.2, 1.0, string.format("FPS: %d", current_fps))
    end

    imgui.end_window()
end

windower.register_event('addon command', function(...)
    show_fps = not show_fps
    print('FPS Monitor display: ' .. tostring(show_fps))
end)

-- Toggle via F12 Key (DIK Code 88)
windower.register_event('keyboard', function(dik, down, flags, blocked)
    if dik == 88 and down then
        show_fps = not show_fps
    end
end)