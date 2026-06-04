local ui = require('ui')
local windower = require('core.windower')
local resources = require('resources')

local equip_window = ui.window_state()
equip_window.title = "EquipViewer"
equip_window.size = {width = 250, height = 450}
equip_window.resizable = false
equip_window.visible = true
equip_window.style = "chromeless"
equip_window.color = ui.color(0, 0, 0, 180)

local slots = {
    {name = "Main",  key = "main",  bag_key = "main_bag"},
    {name = "Sub",   key = "sub",   bag_key = "sub_bag"},
    {name = "Range", key = "range", bag_key = "range_bag"},
    {name = "Ammo",  key = "ammo",  bag_key = "ammo_bag"},
    {name = "Head",  key = "head",  bag_key = "head_bag"},
    {name = "Neck",  key = "neck",  bag_key = "neck_bag"},
    {name = "Ear1",  key = "left_ear", bag_key = "left_ear_bag"},
    {name = "Ear2",  key = "right_ear", bag_key = "right_ear_bag"},
    {name = "Body",  key = "body",  bag_key = "body_bag"},
    {name = "Hands", key = "hands", bag_key = "hands_bag"},
    {name = "Ring1", key = "left_ring", bag_key = "left_ring_bag"},
    {name = "Ring2", key = "right_ring", bag_key = "right_ring_bag"},
    {name = "Back",  key = "back",  bag_key = "back_bag"},
    {name = "Waist", key = "waist", bag_key = "waist_bag"},
    {name = "Legs",  key = "legs",  bag_key = "legs_bag"},
    {name = "Feet",  key = "feet",  bag_key = "feet_bag"}
}

ui.display(function()
    ui.window(equip_window, function(layout)
        layout:label("[Equipped Gear]{color:dodgerblue weight:bold}")
        layout:space(5)
        
        local all_items = windower.ffxi.get_items()
        local eq = all_items and all_items.equipment or nil

        for _, slot in ipairs(slots) do
            local item_name = "[Empty]"
            local item_color = "system_gray"
            
            if eq then
                local bag_id = eq[slot.bag_key]
                local inv_idx = eq[slot.key]
                
                if bag_id ~= nil and inv_idx ~= nil and inv_idx > 0 then
                    local bag = all_items[bag_id]
                    if bag and bag[inv_idx] then
                        local item_id = bag[inv_idx].id
                        if item_id and item_id > 0 then
                            local res_item = resources.items[item_id]
                            if res_item then
                                item_name = res_item.name
                                item_color = "system_white"
                            end
                        end
                    end
                end
            end
            
            -- Right-align the item name for a cleaner look
            layout:label(string.format("[%-6s]{color:skin_accent} : [%s]{color:%s}", slot.name, item_name, item_color))
            layout:space(2)
        end
    end)
end)
