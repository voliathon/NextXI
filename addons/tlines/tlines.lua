local ui = require('ui')
local windower = require('core.windower')
local json = require('json')

local addon = {
    name = 'tlines',
    author = 'Voliathon',
    version = '1.0'
}

local tlines_window = ui.window_state()
tlines_window.title = "Target Lines Overlay"
tlines_window.style = "chromeless"
tlines_window.visible = true
tlines_window.click_through = true

local color_red = ui.color(255, 50, 50, 200)

ui.display(function()
    ui.screen(function(layout)
        local entities_str = windower.ffxi.get_entities()
        local status, entities = pcall(json.decode, entities_str)
        if not status or not entities then return end

        local function get_entity(id)
            for _, e in ipairs(entities) do
                if e.id == id then return e end
            end
        end

        for _, e in ipairs(entities) do
            if e.target_id then
                local target = get_entity(e.target_id)
                if target then
                    local ex, ey = windower.math.project(e.x, e.y, e.z)
                    local tx, ty = windower.math.project(target.x, target.y, target.z)

                    if ex and ey and tx and ty then
                        -- Draw a dashed 'vector' line using interpolated dots!
                        local steps = 20
                        local dx = (tx - ex) / steps
                        local dy = (ty - ey) / steps
                        for i=0, steps do
                            local px = ex + (dx * i)
                            local py = ey + (dy * i)
                            layout:move(px, py):size(4, 4):fill(color_red)
                        end
                        
                        -- Draw labels
                        layout:move(ex - 20, ey - 20):label(e.name, ui.color.system_white)
                        layout:move(tx - 20, ty - 20):label(target.name, ui.color.system_white)
                    end
                end
            end
        end
    end)
end)

return addon
