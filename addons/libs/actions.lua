    Windower 4 -> NextXI stub for the 'actions' library.

    In Windower 4, actions.lua parsed FFXI action packets (0x028) and fired
    a 'action' windower event with structured data so addons could react to
    spells, weapon skills, and job abilities.

    In NextXI, packet parsing is handled via the packet module differently.
    This stub allows addons that require 'actions' to load without crashing.
    The 'action' event will not fire until proper NextXI packet bridging is
    implemented.
]]

_libs = _libs or {}
_libs.actions = {}

return _libs.actions
