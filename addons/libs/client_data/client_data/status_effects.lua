local memory = require('memory')
local d_msg = require('client_data.types.d_msg')

return d_msg.new(memory.status_effect_strings.d_msg, {
    name = 0,
    adjective = 1,
})
