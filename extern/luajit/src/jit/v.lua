----------------------------------------------------------------------------
-- Verbose mode of the LuaJIT compiler.
--

-- Cache some library functions and objects.
local jit = require("jit")
local jutil = require("jit.util")
local vmdef = require("jit.vmdef")
local funcinfo, traceinfo = jutil.funcinfo, jutil.traceinfo
local type, sub, format = type, string.sub, string.format
local stdout, stderr = io.stdout, io.stderr

-- Active flag and output file handle.
local active, out

------------------------------------------------------------------------------

local startloc, startex

local function fmtfunc(func, pc)
  local fi = funcinfo(func, pc)
  if fi.loc then
    return fi.loc
  elseif fi.ffid then
    return vmdef.ffnames[fi.ffid]
  elseif fi.addr then
    return format("C:%x", fi.addr)
  else
    return "(?)"
  end
end

-- Format trace error message.
local function fmterr(err, info)
  if type(err) == "number" then
    if type(info) == "function" then info = fmtfunc(info) end
    local fmt = vmdef.traceerr[err]
    if fmt == "NYI: bytecode %s" then
      local oidx = 6 * info
      info = sub(vmdef.bcnames, oidx+1, oidx+6)
    end
    err = format(fmt, info)
  end
  return err
end

-- Dump trace states.
local function dump_trace(what, tr, func, pc, otr, oex)
  if what == "start" then
    startloc = fmtfunc(func, pc)
    startex = otr and "("..otr.."/"..(oex == -1 and "stitch" or oex)..") " or ""
  else
    if what == "abort" then
      local loc = fmtfunc(func, pc)
      if loc ~= startloc then
	out:write(format("[TRACE --- %s%s -- %s at %s]\n",
	  startex, startloc, fmterr(otr, oex), loc))
      else
	out:write(format("[TRACE --- %s%s -- %s]\n",
	  startex, startloc, fmterr(otr, oex)))
      end
    elseif what == "stop" then
      local info = traceinfo(tr)
      local link, ltype = info.link, info.linktype
      if ltype == "interpreter" then
	out:write(format("[TRACE %3s %s%s -- fallback to interpreter]\n",
	  tr, startex, startloc))
      elseif ltype == "stitch" then
	out:write(format("[TRACE %3s %s%s %s %s]\n",
	  tr, startex, startloc, ltype, fmtfunc(func, pc)))
      elseif link == tr or link == 0 then
	out:write(format("[TRACE %3s %s%s %s]\n",
	  tr, startex, startloc, ltype))
      elseif ltype == "root" then
	out:write(format("[TRACE %3s %s%s -> %d]\n",
	  tr, startex, startloc, link))
      else
	out:write(format("[TRACE %3s %s%s -> %d %s]\n",
	  tr, startex, startloc, link, ltype))
      end
    else
      out:write(format("[TRACE %s]\n", what))
    end
    out:flush()
  end
end

------------------------------------------------------------------------------

-- Detach dump handlers.
local function dumpoff()
  if active then
    active = false
    jit.attach(dump_trace)
    if out and out ~= stdout and out ~= stderr then out:close() end
    out = nil
  end
end

-- Open the output file and attach dump handlers.
local function dumpon(outfile)
  if active then dumpoff() end
  if not outfile then outfile = os.getenv("LUAJIT_VERBOSEFILE") end
  if outfile then
    out = outfile == "-" and stdout or assert(io.open(outfile, "w"))
  else
    out = stderr
  end
  jit.attach(dump_trace, "trace")
  active = true
end

-- Public module functions.
return {
  on = dumpon,
  off = dumpoff,
  start = dumpon -- For -j command line option.
}
