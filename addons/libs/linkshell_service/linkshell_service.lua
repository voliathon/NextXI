local packet = require('packet')
local server = require('shared.server')
local struct = require('struct')

local ls_struct = struct.struct({
    name                = {struct.string(0x14)},
    permissions         = {struct.data(0x04)},
    color               = {struct.struct({
        red                 = {struct.uint8},
        green               = {struct.uint8},
        blue                = {struct.uint8},
    })},
    lsmes               = {struct.struct({
        message             = {struct.string(0x80)},
        player_name         = {struct.string(0x10)},
        timestamp           = {struct.uint32},
    })},
    bag_index           = {struct.uint8},
})

local data = server.new(struct.struct({
    [1]                 = {ls_struct},
    [2]                 = {ls_struct},
}))

packet.incoming:register_init({
    [{0x0CC}] = function(p)
        local ls_data = data[p.linkshell_index + 1]

        ls_data.name = p.linkshell_name
        ls_data.permissions = p.permissions
        ls_data.lsmes.message = p.message
        ls_data.lsmes.timestamp = p.timestamp
        ls_data.lsmes.player_name = p.player_name
    end,
    [{0x037}] = function(p)
        local ls_data = data[1]

        ls_data.color.red = p.linkshell1_red
        ls_data.color.green = p.linkshell1_green
        ls_data.color.blue = p.linkshell1_blue
    end,
    [{0x0E0}] = function(p)
        local ls_data = data[p.linkshell_number]

        ls_data.bag_index = p.bag_index
        if ls_data.bag_index ~= 0 then
            return
        end

        ls_data.name = ''
        ls_data.permissions = '\x00\x00\x00\x00'
        ls_data.color.red = 0
        ls_data.color.green = 0
        ls_data.color.blue = 0
        ls_data.lsmes.message = ''
        ls_data.lsmes.player_name = ''
        ls_data.lsmes.timestamp = 0
    end,
})
