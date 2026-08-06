local imgui = require('imgui')

local smash_count = 0

-- The C++ engine will call this function automatically every single frame!
function imgui_render()
    -- 1. Create the Window
    imgui.begin_window("Caveman Test UI")
    
    -- 2. Draw some text
    imgui.text("Fire burns bright! Bridge is open!")
    imgui.text("Smash count: " .. tostring(smash_count))
    
    -- 3. Draw a button and check if it was clicked
    if imgui.button("Smash Button") then
        smash_count = smash_count + 1
        print("Caveman smashed the button " .. tostring(smash_count) .. " times!")
    end
    
    -- 4. End the Window
    imgui.end_window()
end