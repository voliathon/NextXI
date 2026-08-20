local imgui = require('imgui')
local command = require('core.command')
local string = require('string') 
local math = require('math')

local WindowFlags = {
    None = 0,
    NoTitleBar = 1,
    NoResize = 2,
    NoMove = 4,
    NoScrollbar = 8,
    AlwaysAutoResize = 64,
    NoBackground = 128
}

local addon_state = {
    is_visible = true,
    hud_mode = false,      
    cast_timer = 0.0,      
    selected_job = "WAR",  
    jobs = {"WAR", "MNK", "WHM", "BLM", "RDM", "THF"},
    
    search_text = "Type here...",
    
    -- Classic Sandbox state
    smash_count = 0,
    auto_smash = false,
    smash_power = 0.1,
    smash_level = 1,
    theme_r = 1.0,
    theme_g = 0.4,
    theme_b = 0.4,
    
    -- Mock combat log data
    combat_log = {
        {text = "You hit the Goblin Thug for 45 points of damage.", r=1.0, g=1.0, b=1.0},
        {text = "The Goblin Thug hits you for 12 points of damage.", r=1.0, g=0.3, b=0.3},
        {text = "You cast Cure. You recover 30 HP.", r=0.3, g=1.0, b=0.3},
        {text = "Goblin Thug readies Bomb Toss.", r=1.0, g=0.8, b=0.2}
    }
}

local example_cmd = command.new('imgui_examples')

example_cmd:register('show', function()
    addon_state.is_visible = true
    print("[imgui_examples] UI is now VISIBLE!")
end)

example_cmd:register('hide', function()
    addon_state.is_visible = false
    print("[imgui_examples] UI is now HIDDEN!")
end)

function imgui_render()
    if not addon_state.is_visible then return end
    
    addon_state.cast_timer = addon_state.cast_timer + 0.01
    if addon_state.cast_timer > 1.0 then addon_state.cast_timer = 0.0 end
    
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
        imgui.text_colored(0.3, 1.0, 0.3, 1.0, "HUD OVERLAY ACTIVE - Borderless & Clean!")
        imgui.progress_bar(addon_state.cast_timer, string.format("Casting... %d%%", math.floor(addon_state.cast_timer * 100)))
        if imgui.button("Exit HUD Mode") then addon_state.hud_mode = false end
    else
        if imgui.begin_tab_bar("ShowcaseTabs") then
            
            if imgui.begin_tab_item("Controls") then
                imgui.text("Select Main Job:")
                if imgui.begin_combo("##jobcombo", addon_state.selected_job) then
                    for i, job in ipairs(addon_state.jobs) do
                        local is_selected = (addon_state.selected_job == job)
                        if imgui.selectable(job, is_selected) then
                            addon_state.selected_job = job
                        end
                    end
                    imgui.end_combo()
                end
                
                imgui.separator()
                imgui.text("Cast Bar Example:")
                imgui.progress_bar(addon_state.cast_timer, "Fire IV")
                
                imgui.separator()
                if imgui.button("Enable Transparent HUD Mode") then addon_state.hud_mode = true end
                imgui.end_tab_item()
            end

            if imgui.begin_tab_item("Data Grids") then
                if imgui.begin_table("PartyTable", 3) then
                    imgui.table_setup_column("Name")
                    imgui.table_setup_column("Job")
                    imgui.table_setup_column("HP %")
                    imgui.table_headers_row()

                    imgui.table_next_row()
                    imgui.table_next_column() imgui.text_colored(0.5, 0.8, 1.0, 1.0, "PlayerOne")
                    imgui.table_next_column() imgui.text(addon_state.selected_job .. "75")
                    imgui.table_next_column() imgui.progress_bar(0.85, "85%")

                    imgui.table_next_row()
                    imgui.table_next_column() imgui.text_colored(0.5, 0.8, 1.0, 1.0, "HealerGuy")
                    imgui.table_next_column() imgui.text("WHM75")
                    imgui.table_next_column() imgui.progress_bar(0.40, "40%")
                    
                    imgui.end_table()
                end
                imgui.end_tab_item()
            end
            
            if imgui.begin_tab_item("Trees & Inputs") then
                local changed, new_text = imgui.input_text("Search", addon_state.search_text)
                if changed then addon_state.search_text = new_text end
                
                imgui.separator()
                if imgui.tree_node("Inventory") then
                    if imgui.tree_node("Weapons") then
                        imgui.text_colored(0.8, 0.2, 0.8, 1.0, "- Excalibur")
                        imgui.text("- Mandau")
                        imgui.tree_pop()
                    end
                    imgui.tree_pop()
                end
                imgui.end_tab_item()
            end
            
            -- NEW: Rich Text & Logs
            if imgui.begin_tab_item("Rich Text & Logs") then
                
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
                
                -- Begin a child window (ID, Width, Height, Borders)
                if imgui.begin_child("CombatLog", 0, 120, true) then
                    for i, log in ipairs(addon_state.combat_log) do
                        imgui.text_colored(log.r, log.g, log.b, 1.0, log.text)
                    end
                    imgui.end_child()
                end
                
                if imgui.button("Spam Log") then
                    table.insert(addon_state.combat_log, {text = "You swing and miss.", r=0.7, g=0.7, b=0.7})
                end

                imgui.end_tab_item()
            end

            if imgui.begin_tab_item("Classic Sandbox") then
                imgui.text(string.format("Smash count: %.1f", addon_state.smash_count))
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
                
                imgui.separator()
                addon_state.theme_r, addon_state.theme_g, addon_state.theme_b = 
                    imgui.color_edit3("Theme Color", addon_state.theme_r, addon_state.theme_g, addon_state.theme_b)
                
                imgui.end_tab_item()
            end

            imgui.end_tab_bar()
        end
    end
    
    imgui.end_window()
end