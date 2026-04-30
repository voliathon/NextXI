local chat = require('chat')

local addon = { name = 'CrashTest', author = 'Test', version = '1.0' }

coroutine.schedule(function()
    coroutine.sleep_frame()
    for i = 1, 1000 do
        -- Standard print writes to the Windower CLI console
        print("Console Stress Test Message " .. tostring(i))
    end
end)

return addon