local success, err = pcall(function()
    local command = require('core.command')
    local windower = require('core.windower')
    _G.windower = windower
    coroutine.schedule(function()
        for i = 1, 30 do coroutine.sleep_frame() end
        command.input('/load AddonManager')
        windower.add_to_chat(207, "NextXI Core: AddonManager Boot Request Sent.")
    end)
    coroutine.schedule(function()
        local was_logged_in = false
        while true do
            coroutine.sleep_frame()
            local player_json = windower.ffxi.get_player()
            local is_logged_in = (player_json ~= "{}" and player_json ~= nil)
            
            if is_logged_in and not was_logged_in then
                was_logged_in = true
                for i = 1, 30 do coroutine.sleep_frame() end
                windower.trigger_event('login', "Player") 
            elseif not is_logged_in and was_logged_in then
                was_logged_in = false
                windower.trigger_event('logout')
            end
        end
    end)
end)

if not success then
    local windower = require('core.windower')
    windower.add_to_chat(167, "init.lua Core Boot Error: " .. tostring(err))
end