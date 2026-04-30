local ui = require('ui')
local chat = require('chat')
local command = require('command')

local addon = {
    name = 'LibTester',
    author = 'Voliathon of Bahamut',
    version = '1.0'
}

command.register('libtest', function(args)
    local target_lib = args[1] or "file"
    
    chat.print("LibTester: Attempting to load '" .. target_lib .. "'...", ui.color.system_white)
    
    local success, result = pcall(require, target_lib)
    
    if success then
        chat.print("SUCCESS: '" .. target_lib .. "' loaded correctly!", ui.color.green)
        
        -- If it's a table, count the keys to prove it's not an empty dummy table
        if type(result) == "table" then
            local count = 0
            for k, v in pairs(result) do count = count + 1 end
            chat.print(" -> Returned a table with " .. count .. " keys.", ui.color.green)
        end
    else
        chat.print("FAIL: Sandbox blocked '" .. target_lib .. "'.", ui.color.system_error)
        chat.print(" -> Error: " .. tostring(result), ui.color.system_error)
    end
end)

chat.print("LibTester Initialized. Type /libtest <library_name>", ui.color.skin_accent)

return addon