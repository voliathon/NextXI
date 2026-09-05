local table = require('table')

local state = {
    search_text = "Type here...",
    combat_log = {
        {text = "You hit the Goblin Thug for 45 points of damage.", r=1.0, g=1.0, b=1.0},
        {text = "The Goblin Thug hits you for 12 points of damage.", r=1.0, g=0.3, b=0.3},
        {text = "You cast Cure. You recover 30 HP.", r=0.3, g=1.0, b=0.3},
        {text = "Goblin Thug readies Bomb Toss.", r=1.0, g=0.8, b=0.2}
    }
}

local M = {}

function M.render_tab(imgui)
    if imgui.begin_tab_item("Trees & Logs") then
        local changed, new_text = imgui.input_text("Search", state.search_text)
        if changed then state.search_text = new_text end
        
        imgui.separator()
        if imgui.tree_node("Inventory") then
            if imgui.tree_node("Weapons") then
                imgui.text_colored(0.8, 0.2, 0.8, 1.0, "- Excalibur")
                imgui.text("- Mandau")
                imgui.tree_pop()
            end
            imgui.tree_pop()
        end

        imgui.separator()
        imgui.text("Hover over this item:")
        imgui.same_line()
        imgui.text_colored(0.8, 0.2, 0.8, 1.0, "[Kraken Club]")
        if imgui.is_item_hovered() then
            imgui.begin_tooltip()
            imgui.text_colored(0.8, 0.2, 0.8, 1.0, "Kraken Club")
            imgui.separator()
            imgui.text("DMG: 11 Delay: 264")
            imgui.text_colored(0.5, 1.0, 0.5, 1.0, "Occasionally attacks 2 to 8 times.")
            imgui.end_tooltip()
        end

        imgui.separator()
        imgui.text("Scrolling Combat Log (Child Window):")
        
        if imgui.begin_child("CombatLog", 0, 120, true) then
            for _, log in ipairs(state.combat_log) do
                imgui.text_colored(log.r, log.g, log.b, 1.0, log.text)
            end
            imgui.end_child()
        end
        
        if imgui.button("Spam Log") then
            table.insert(state.combat_log, {text = "You swing and miss.", r=0.7, g=0.7, b=0.7})
        end

        imgui.end_tab_item()
    end
end

return M