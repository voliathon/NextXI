    Windower 4 -> NextXI compatibility shim for the 'config' library.

    Windower 4 addons use config.load(path, defaults) to read XML settings
    files and config.save(settings, path) to persist them.

    This shim returns the defaults table as-is. A full XML implementation
    can be layered in later using NextXI's settings package if needed.
]]

_libs = _libs or {}

local config = {}
_libs.config = config
function config.load(path, defaults)
    local settings = {}
    if defaults then
        for k, v in pairs(defaults) do
            settings[k] = v
        end
    end
    return settings
end
function config.save(settings, path)
end
function config.new(defaults)
    return config.load(nil, defaults)
end

return config
