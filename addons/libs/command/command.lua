local core_command = require('core.command')
local core_pin = require('core.pin')

local command = {}
local active_commands = {}

-- Helper to handle the actual C++ binding
local function register_single(cmd_name, callback)
    if type(cmd_name) ~= 'string' then return end
    
    if not active_commands[cmd_name] then
        local cmd_obj = core_command.new(cmd_name)
        core_pin(cmd_obj)
        
        cmd_obj:register(function(...)
            local args = {...}
            if active_commands[cmd_name] then
                active_commands[cmd_name](args)
            end
        end, '[string]*') 
    end
    
    active_commands[cmd_name] = callback
end

-- Upgraded to support both Strings and Tables (Aliases)
function command.register(command_name, callback)
    if type(command_name) == 'table' then
        for _, name in ipairs(command_name) do
            register_single(name, callback)
        end
    else
        register_single(command_name, callback)
    end
end

return command