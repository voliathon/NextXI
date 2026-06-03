----------------------------------------------------------------------------
-- LuaJIT MIPS64EL disassembler wrapper module.
--

local dis_mips = require((string.match(..., ".*%.") or "").."dis_mips")
return {
  create = dis_mips.create_el,
  disass = dis_mips.disass_el,
  regname = dis_mips.regname
}
