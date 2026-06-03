local player = require('player')
local packet = require('packet')
local struct = require('struct')
local account = require('account')
local server = require('shared.server')

local assembly_offset, attachment_offset = 0x2000, 0x2100

local data, ftype = server.new(struct.struct({
    name = {struct.string(0x10)};

    head_id = {struct.int32},
    frame_id = {struct.int32},
    attachments = {struct.struct({
        id = {struct.int32},
        slot = {struct.int32},
    })[0x0C]},

    skills = {struct.struct({
        melee = {struct.int32},
        melee_max = {struct.int32},
        ranged = {struct.int32},
        ranged_max = {struct.int32},
        magic = {struct.int32},
        magic_max = {struct.int32},
    })},

    stats = {struct.struct({
        str             = {struct.int32},
        str_modifier    = {struct.int32},
        dex             = {struct.int32},
        dex_modifier    = {struct.int32},
        vit             = {struct.int32},
        vit_modifier    = {struct.int32},
        agi             = {struct.int32},
        agi_modifier    = {struct.int32},
        int             = {struct.int32},
        int_modifier    = {struct.int32},
        mnd             = {struct.int32},
        mnd_modifier    = {struct.int32},
        chr             = {struct.int32},
        chr_modifier    = {struct.int32},
    })},

    available_heads = {struct.bool[32]},
    available_frames= {struct.bool[32]},
    available_attachments = {struct.bool[256]},
}))

struct.reset_on(account.logout, data, ftype)
struct.reset_on(player.job_change, data, ftype)

packet.incoming:register_init({
    [{0x044, 0x12}] = function(p)
        data.name = p.pet_name
        data.head_id = p.automaton_head + assembly_offset
        data.frame_id = p.automaton_frame + assembly_offset

        data.skills.melee = p.melee
        data.skills.magic = p.magic
        data.skills.ranged = p.ranged
        data.skills.melee_max = p.melee_max
        data.skills.magic_max = p.magic_max
        data.skills.ranged_max = p.ranged_max

        for k, v in pairs(p.attachments) do
             data.attachments[k].id = v + attachment_offset
             data.attachments[k].slot = k + 1
        end

        for i = 0, 31 do
            data.available_heads[i] = p.available_heads[i]
            data.available_frames[i] = p.available_frames[i]
        end

        for i = 0, 255 do
            data.available_attachments[i] = p.available_attach[i]
        end

        data.stats.chr = p.chr
        data.stats.str = p.str
        data.stats.agi = p.agi
        data.stats.mnd = p.mnd
        data.stats.vit = p.vit
        data.stats.dex = p.dex
        data.stats.chr_modifier = p.chr_modifier
        data.stats.str_modifier = p.str_modifier
        data.stats.agi_modifier = p.agi_modifier
        data.stats.mnd_modifier = p.mnd_modifier
        data.stats.vit_modifier = p.vit_modifier
        data.stats.dex_modifier = p.dex_modifier
    end,
})
