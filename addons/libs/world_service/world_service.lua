local event = require('core.event')
local packet = require('packet')
local server = require('shared.server')
local struct = require('struct')

local data = server.new(struct.struct({
    zone_id             = {struct.int32},
    weather_id          = {struct.int32},
    mog_house           = {struct.bool},
    music               = {struct.struct({
        day                 = {struct.int32},
        night               = {struct.int32},
        solo_combat         = {struct.int32},
        party_combat        = {struct.int32},
        mount               = {struct.int32},
        knockout            = {struct.int32},
        mog_house           = {struct.int32},
        fishing             = {struct.int32},
    })},
    zone_change         = {data = event.new()},
    weather_change      = {data = event.new()},
}))

data.zone_id = -1
data.weather_id = -1

local music = data.music

local music_type_to_field = {
    [0] = 'day',
    [1] = 'night',
    [2] = 'solo_combat',
    [3] = 'party_combat',
    [4] = 'mount',
    [5] = 'knockout',
    [6] = 'mog_house',
    [7] = 'fishing',
}

local zone_change_event = data.zone_change
local weather_change_event = data.weather_change

packet.incoming:register_init({
    [{0x00A}] = function(p)
        data.zone_id = p.zone_id
        data.weather_id = p.weather_id
        data.mog_house = p.flags.mog_house
        music.day = p.day_music
        music.night = p.night_music
        music.solo_combat = p.solo_combat_music
        music.party_combat = p.party_combat_music
        music.mount = p.mount_music

        zone_change_event:trigger()
        weather_change_event:trigger()
    end,
    [{0x057}] = function(p)
        data.weather_id = p.weather_id

        weather_change_event:trigger()
    end,
    [{0x05F}] = function(p)
        music[music_type_to_field[p.music_type]] = p.song_id
    end,
    [{0x00B}] = function(p)
        data.zone_id = -1
        data.weather_id = -1
        music.day = 0
        music.night = 0
        music.solo_combat = 0
        music.party_combat = 0
        music.mount = 0
    end,
})
