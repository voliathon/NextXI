local command = require('core.command')
local channel = require('core.channel')

do
    local handling = nil
    local channels = setmetatable({}, {
        __mode = 'v',
        __index = function(t, package)
            local st_channel = channel.get(package, '__sub_target_channel')
            rawset(t, package, st_channel)
            return st_channel
        end,
    })

    local search_handler = function(source, command_string)
        if source:match('^sub_target') then
            local package, counter, target_id =
                command_string:match('^/aim \u{FFFD}select_sub_target\u{FFFD} "(%w+)" (%d+) (%d+)$')
            if package and target_id then
                local tag = package .. ':' .. counter
                if handling == tag then
                    handling = nil
                    local package_channel = channels[package]
                    if package_channel then
                        package_channel:pcall(function(_, ...)
                            report_result(...)
                        end, tonumber(counter), tonumber(target_id))
                    end
                else
                    handling = tag
                end
                return
            end
        end
        command.input('/:' .. command_string:sub(2), source)
    end
    command.core.register('aim', search_handler, true)
end
