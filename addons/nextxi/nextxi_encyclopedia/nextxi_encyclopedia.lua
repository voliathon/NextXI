local imgui = require('imgui')
local command = require('core.command')
local chat = require('core.chat')
local windower = require('core.windower') -- Grab this to access ffxi.get_player()
local string = require('string')

local state = {
    show_main = true,
    show_demo = false,
    selected_index = 1
}

-- We store widget values here so they don't reset every frame!
local widget_state = {
    toggled = true,
    opacity = 100,
    r = 0.2, g = 0.8, b = 1.0
}

local ency_cmd = command.new('nxi_enc')
ency_cmd:register('show', function() state.show_main = true end)
ency_cmd:register('hide', function() state.show_main = false end)

local topics = {
    {
        title = "Player Vitals Tracker",
        desc = "Reads memory to display the character's current HP, MP, and TP.",
        code = "local p = windower.ffxi.get_player()\nprint(p.vitals.hp)",
        demo = function()
            imgui.text_colored(0.2, 0.8, 1.0, 1.0, "[Live Data Display]")
            imgui.separator()
            
            local p = windower.ffxi.get_player()
            if p and p.vitals then
                imgui.text(string.format("HP: %d / %d", p.vitals.hp, p.max_hp))
                imgui.text(string.format("MP: %d / %d", p.vitals.mp, p.max_mp))
                imgui.text(string.format("TP: %d", p.vitals.tp))
            else
                imgui.text_colored(1.0, 0.3, 0.3, 1.0, "Vitals currently unavailable.")
            end

            imgui.spacing()
            if imgui.button("Print Status to Chat") then
                if p and p.vitals then
                    local msg = string.format("[NextXI Encyclopedia] HP: %d | MP: %d | TP: %d", p.vitals.hp, p.vitals.mp, p.vitals.tp)
                    chat.add_text(msg, 206)
                else
                    chat.add_text("[NextXI Encyclopedia] Cannot print. Vitals unavailable.", 167)
                end
            end
        end
    },
    {
        title = "Interactive Widgets",
        desc = "Demonstrates core user inputs, sliders, and color editing.",
        code = "val = imgui.slider_int('Opacity', val, 0, 255)\nr,g,b = imgui.color_edit3('Theme', r, g, b)",
        demo = function()
            imgui.text_colored(0.4, 1.0, 0.4, 1.0, "Widget Sandbox")
            imgui.separator()
            
            if imgui.button("Click Me!") then
                chat.add_text("[NextXI] Action Button Clicked!", 206)
            end
            
            imgui.same_line()
            
            -- We overwrite the state table with the return values so they update visually
            widget_state.toggled = imgui.checkbox("Toggle Feature", widget_state.toggled)
            widget_state.opacity = imgui.slider_int("Opacity", widget_state.opacity, 0, 255)
            widget_state.r, widget_state.g, widget_state.b = imgui.color_edit3("Highlight Color", widget_state.r, widget_state.g, widget_state.b)
            
            imgui.spacing()
            imgui.progress_bar(widget_state.opacity / 255.0, string.format("%d%% Opacity", (widget_state.opacity / 255.0) * 100))
        end
    },
    {
        title = "Data Grids & Tables",
        desc = "Advanced table layouts for rendering inventories, stat blocks, or mob data.",
        code = "if imgui.begin_table('ID', 3) then\n  imgui.table_setup_column('Name')\n  imgui.table_headers_row()\nimgui.end_table()",
        demo = function()
            if imgui.begin_table("DataGridDemo", 3) then
                imgui.table_setup_column("Item Name")
                imgui.table_setup_column("Quantity")
                imgui.table_setup_column("Value")
                imgui.table_headers_row()
                
                imgui.table_next_row()
                imgui.table_next_column() imgui.text("Potions")
                imgui.table_next_column() imgui.text("99")
                imgui.table_next_column() imgui.text_colored(1.0, 0.8, 0.2, 1.0, "1,500g")
                
                imgui.table_next_row()
                imgui.table_next_column() imgui.text("Echo Drops")
                imgui.table_next_column() imgui.text("12")
                imgui.table_next_column() imgui.text_colored(1.0, 0.8, 0.2, 1.0, "300g")
                imgui.end_table()
            end
        end
    }
}

function imgui_render()
    if state.show_demo and topics[state.selected_index] then
        imgui.set_next_window_size(400, 300)
        local demo_open = imgui.begin_window("Live Demo: " .. topics[state.selected_index].title, 0)
        if not demo_open then
            state.show_demo = false
        else
            topics[state.selected_index].demo()
        end
        imgui.end_window()
    end

    if not state.show_main then return end
    imgui.set_next_window_size(750, 450)
    local main_open = imgui.begin_window("NextXI API Encyclopedia", 0)
    if not main_open then
        state.show_main = false
        imgui.end_window()
        return
    end

    if imgui.begin_table("DirectoryLayout", 2) then
        imgui.table_setup_column("Index", 220)
        imgui.table_setup_column("Documentation", 530)
        imgui.table_next_row()

        imgui.table_next_column()
        imgui.text_colored(1.0, 0.8, 0.2, 1.0, "Table of Contents")
        imgui.separator()
        for idx, t in ipairs(topics) do
            if imgui.selectable(t.title, state.selected_index == idx) then
                state.selected_index = idx
                state.show_demo = false
            end
        end

        imgui.table_next_column()
        local current = topics[state.selected_index]
        if current then
            imgui.text_colored(0.4, 0.8, 1.0, 1.0, current.title)
            imgui.text(current.desc)
            imgui.spacing()
            
            imgui.text_colored(0.7, 0.7, 0.7, 1.0, "Example Code:")
            imgui.text(current.code)
            imgui.spacing()
            imgui.separator()
            imgui.spacing()

            if imgui.button("Launch Live Demo Window") then
                state.show_demo = true
            end
        end
        imgui.end_table()
    end
    imgui.end_window()
end