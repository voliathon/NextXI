local M = {}

function M.render_tab(imgui)
    if imgui.begin_tab_item("Data Grids") then
        if imgui.begin_table("PartyTable", 3) then
            imgui.table_setup_column("Name")
            imgui.table_setup_column("Job")
            imgui.table_setup_column("HP %")
            imgui.table_headers_row()

            imgui.table_next_row()
            imgui.table_next_column() imgui.text_colored(0.5, 0.8, 1.0, 1.0, "PlayerOne")
            imgui.table_next_column() imgui.text("WAR75")
            imgui.table_next_column() imgui.progress_bar(0.85, "85%")

            imgui.table_next_row()
            imgui.table_next_column() imgui.text_colored(0.5, 0.8, 1.0, 1.0, "HealerGuy")
            imgui.table_next_column() imgui.text("WHM75")
            imgui.table_next_column() imgui.progress_bar(0.40, "40%")
            
            imgui.end_table()
        end
        imgui.end_tab_item()
    end
end

return M