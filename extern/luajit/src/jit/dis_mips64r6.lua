----------------------------------------------------------------------------
-- LuaJIT MIPS64R6 disassembler wrapper module.
--

local dis_mips = require((string.match(..., ".*%.") or "").."dis_mips")
return {
  create = dis_mips.create_r6,
  disass = dis_mips.disass_r6,
  regname = dis_mips.regname
}
