local event = require('core.event')
local memory = require('memory')
local packet = require('packet')
local server = require('shared.server')
local struct = require('struct')

local data = server.new(struct.struct({
    logged_in           = {struct.bool},
    name                = {struct.string(0x10)},
    id                  = {struct.int32},
    server_id           = {struct.int32},
    login               = {data = event.new()},
    logout              = {data = event.new()},
}))

local login_event = data.login
local logout_event = data.logout

packet.incoming:register_init({
    [{0x00A}] = function(p)
        local login = not data.logged_in
        if not login then
            return
        end

        coroutine.schedule(function()
            local info = memory.account_info
            while info.server_id == -1 do
                coroutine.sleep_frame()
            end

            data.name = info.name
            data.id = info.id
            data.server_id = info.server_id % 0x20
            data.logged_in = true

            login_event:trigger()
        end)
    end,
    [{0x00B, 0x01}] = function(p)

        data.logged_in = false
        data.server_id = 0
        data.name = ''
        data.id = 0

        logout_event:trigger()
    end,
})
