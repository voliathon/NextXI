
local dis_mips = require((string.match(..., ".*%.") or "").."dis_mips")
return {
  create = dis_mips.create_r6_el,
  disass = dis_mips.disass_r6_el,
  regname = dis_mips.regname
}
