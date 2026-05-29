local memory = require('memory')
local d_msg = require('client_data.types.d_msg')

return d_msg.new(memory.d_msg_table.equipment_slots[0], {
    name = 0,
})
