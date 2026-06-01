--[[
    Windower 4 -> NextXI compatibility shim for the 'config' library.

    Windower 4 addons use config.load(path, defaults) to read XML settings
    files and config.save(settings, path) to persist them.

    This shim returns the defaults table as-is. A full XML implementation
    can be layered in later using NextXI's settings package if needed.
]]

_libs = _libs or {}

local config = {}
_libs.config = config

-- Loads settings from an XML file, merging with defaults.
-- In this shim the file is ignored and defaults are returned directly.
function config.load(path, defaults)
    local settings = {}
    if defaults then
        for k, v in pairs(defaults) do
            settings[k] = v
        end
    end
    return settings
end

-- Saves settings to an XML file (no-op in this shim).
function config.save(settings, path)
    -- not implemented
end

-- Alias for creating a new settings table from defaults
function config.new(defaults)
    return config.load(nil, defaults)
end

return config
