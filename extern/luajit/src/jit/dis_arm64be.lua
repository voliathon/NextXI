----------------------------------------------------------------------------
-- LuaJIT ARM64BE disassembler wrapper module.
--

return require((string.match(..., ".*%.") or "").."dis_arm64")
