local memory = require('memory')
local d_msg = require('client_data.types.d_msg')

return d_msg.new(memory.action_strings.mounts, {
    name = 0,
})
