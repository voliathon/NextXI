local math = require('math')
local string = require('string')
local table = require('table')

local memory = require('memory')
local settings = require('settings')
local windower = require('core.windower')
local command = require('command')
local chat = require('chat')
local ui = require('ui')

local defaults = {
    graphics = {
        aspect_ratio = { auto = true, value = 16 / 9 },
        gamma = { red = 1.5, green = 1.5, blue = 1.5 },
        framerate = 30,           -- Changed to 30 FPS Default
        animation_framerate = 30, -- Changed to 30 FPS Default
        clipping_plane = 10,
        footstep_effects = true,
    },
    audio = {
        footstep_effects = true,
    },
}

local options = settings.load(defaults)
local state = { show_ui = false }
local function apply_settings()
    local graphics = memory.graphics
    local graphics_options = options.graphics
    graphics.gamma.red = graphics_options.gamma.red
    graphics.gamma.green = graphics_options.gamma.green
    graphics.gamma.blue = graphics_options.gamma.blue
    local window_aspect_ratio = (4 / 3) / (windower.settings.client_size.width / windower.settings.client_size.height)
    graphics.render.aspect_ratio = graphics_options.aspect_ratio.auto and window_aspect_ratio or ((4 / 3) / graphics_options.aspect_ratio.value)
    if graphics_options.framerate <= 30 then
        graphics.render.framerate_divisor = 2
    else
        graphics.render.framerate_divisor = 1
    end

    if graphics_options.animation_framerate <= 30 then
        graphics.animation_framerate = 2
    else
        graphics.animation_framerate = 1
    end
    graphics.clipping_plane_entity = graphics_options.clipping_plane
    graphics.clipping_plane_map = graphics_options.clipping_plane
    graphics.footstep_effects = options.graphics.footstep_effects
    memory.volumes.footsteps = options.audio.footstep_effects and 1.0 or 0.0
end
coroutine.schedule(function()
    coroutine.sleep_frame()
    local success, err = pcall(apply_settings)
    if not success then
        chat.print("Config Addon Init Error: " .. tostring(err), ui.color.system_error)
    else
        chat.print("Config Loaded: Type /config or /cfg to open.", ui.color.skin_accent)
    end
end)
local config_window = ui.window_state()
config_window.title = " NextXI Client Configuration"
config_window.size = {width = 420, height = 520}
config_window.visible = false

local cfg_scroll = ui.scroll_panel_state(400, 900)
local COLOR_HEADER = ui.color.rgb(100, 200, 255)
local COLOR_MUTED  = ui.color.rgb(140, 140, 140)

ui.display(function()
    local success, err = pcall(function()
        if state.show_ui then
            config_window.visible = true

            local window_still_open = ui.window(config_window, function(layout)

                layout:height(480):scroll_panel(cfg_scroll, function(c)
                    c:space(6)
                    c:label("  GRAPHICS & PERFORMANCE", COLOR_HEADER)
                    c:label("  ──────────────────────────────────────────────", COLOR_MUTED)
                    c:space(4)
                    c:label("  Camera FPS  (current: " .. tostring(options.graphics.framerate) .. ")")
                    local c30 = c:width(100):button("btn_fps_30", "  30 FPS  ", false)
                    c:same_line()
                    local c60 = c:width(100):button("btn_fps_60", "  60 FPS  ", false)
                    if c30 then options.graphics.framerate = 30; apply_settings(); settings.save() end
                    if c60 then options.graphics.framerate = 60; apply_settings(); settings.save() end

                    c:space(6)
                    c:label("  Animation FPS  (current: " .. tostring(options.graphics.animation_framerate) .. ")")
                    local a30 = c:width(100):button("btn_afps_30", "  30 FPS  ", false)
                    c:same_line()
                    local a60 = c:width(100):button("btn_afps_60", "  60 FPS  ", false)
                    if a30 then options.graphics.animation_framerate = 30; apply_settings(); settings.save() end
                    if a60 then options.graphics.animation_framerate = 60; apply_settings(); settings.save() end

                    c:space(10)
                    c:label("  Draw Distance / Clipping Plane  (" .. string.format("%.1f", options.graphics.clipping_plane) .. ")")
                    local new_clip = c:slider("sld_clip", options.graphics.clipping_plane, 1.0, 25.0)
                    if new_clip ~= options.graphics.clipping_plane then
                        options.graphics.clipping_plane = new_clip; apply_settings(); settings.save()
                    end

                    c:space(10)
                    c:label("  Gamma - Red  (" .. string.format("%.2f", options.graphics.gamma.red) .. ")", ui.color.rgb(255, 100, 100))
                    local new_r = c:slider("sld_gamma_r", options.graphics.gamma.red, 0.5, 3.0)
                    if new_r ~= options.graphics.gamma.red then
                        options.graphics.gamma.red = new_r; apply_settings(); settings.save()
                    end

                    c:label("  Gamma - Green  (" .. string.format("%.2f", options.graphics.gamma.green) .. ")", ui.color.rgb(100, 220, 100))
                    local new_g = c:slider("sld_gamma_g", options.graphics.gamma.green, 0.5, 3.0)
                    if new_g ~= options.graphics.gamma.green then
                        options.graphics.gamma.green = new_g; apply_settings(); settings.save()
                    end

                    c:label("  Gamma - Blue  (" .. string.format("%.2f", options.graphics.gamma.blue) .. ")", ui.color.rgb(80, 160, 255))
                    local new_b = c:slider("sld_gamma_b", options.graphics.gamma.blue, 0.5, 3.0)
                    if new_b ~= options.graphics.gamma.blue then
                        options.graphics.gamma.blue = new_b; apply_settings(); settings.save()
                    end

                    c:space(14)
                    c:label("  AUDIO & EFFECTS", COLOR_HEADER)
                    c:label("  ──────────────────────────────────────────────", COLOR_MUTED)
                    c:space(6)

                    local pc, _ = c:check("chk_particles", "  Enable Visual Footstep Dust Particles", options.graphics.footstep_effects)
                    if pc then
                        options.graphics.footstep_effects = not options.graphics.footstep_effects
                        apply_settings(); settings.save()
                    end

                    c:space(4)

                    local fc, _ = c:check("chk_foot", "  Enable Footstep Sound Effects", options.audio.footstep_effects)
                    if fc then
                        options.audio.footstep_effects = not options.audio.footstep_effects
                        apply_settings(); settings.save()
                    end

                    c:space(20)
                end)
            end)

            if not window_still_open then
                state.show_ui = false
                chat.print("Config UI Closed.", ui.color.system_gray)
            end
        else
            config_window.visible = false
        end
    end)

    if not success then
        state.show_ui = false
        chat.print("Config UI Crash: " .. tostring(err), ui.color.system_error)
    end
end)
command.register({'config', 'cfg'}, function(args)
    state.show_ui = not state.show_ui
    if state.show_ui then
        chat.success("Config UI Opened.")
    end
end)