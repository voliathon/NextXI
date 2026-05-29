-- LuaFormatter off
local -- params
    version,
    version_major,
    version_minor,
    version_build,
    build_tag,
    client_path,
    scripts_path,
    client_width,
    client_height,
    ui_width,
    ui_height,
    client_hwnd,
    settings_path,
    user_path,
    package_path,
    package_name,
    get_package_list_ptr,
    get_package_readme_ptr,
    read_market_file_ptr,    
    write_market_file_ptr, 
    get_ffxi_player_ptr,
    get_ffxi_items_ptr,
    get_ffxi_spells_ptr = ...
-- LuaFormatter on

local ffi = require('ffi')

-- FFI Signatures
local get_package_list_c = ffi.typeof('char const*(*)()')(get_package_list_ptr)
local get_package_readme_c = ffi.typeof('char const*(*)(char const*)')(get_package_readme_ptr)
local read_market_file_c = ffi.typeof('char const*(*)(char const*)')(read_market_file_ptr)
local write_market_file_c = ffi.typeof('void(*)(char const*, char const*)')(write_market_file_ptr)

-- Clean Lua Wrappers
local function get_package_list()
    local ptr = get_package_list_c()
    if ptr ~= nil then return ffi.string(ptr) end
    return ""
end

local function get_package_readme(name)
    local ptr = get_package_readme_c(name)
    if ptr ~= nil then return ffi.string(ptr) end
    return ""
end

local function read_file(name)
    local ptr = read_market_file_c(name)
    if ptr ~= nil then return ffi.string(ptr) end
    return ""
end

local function write_file(name, data)
    write_market_file_c(name, data)
end

local get_ffxi_player_c = ffi.typeof('char const*(*)()')(get_ffxi_player_ptr)
local get_ffxi_items_c = ffi.typeof('char const*(*)()')(get_ffxi_items_ptr)
local get_ffxi_spells_c = ffi.typeof('char const*(*)()')(get_ffxi_spells_ptr)

local function get_player()
    local ptr = get_ffxi_player_c()
    if ptr ~= nil then return ffi.string(ptr) end
    return "{}"
end

local function get_items()
    local ptr = get_ffxi_items_c()
    if ptr ~= nil then return ffi.string(ptr) end
    return "{}"
end

local function get_spells()
    local ptr = get_ffxi_spells_c()
    if ptr ~= nil then return ffi.string(ptr) end
    return "[]"
end

-- Expose to the Engine
local windower = {
    version = version,
    version_major = version_major,
    version_minor = version_minor,
    version_build = version_build,
    build_tag = build_tag,
    client_path = client_path,
    scripts_path = scripts_path,
    settings_path = settings_path,
    user_path = user_path,
    package_path = package_path,
    package_name = package_name,
    
    -- Expose all 4 bridges to your NextXISDK!
    get_package_list = get_package_list, 
    get_package_readme = get_package_readme,
    read_file = read_file,   
    write_file = write_file, 
    
    settings = {
        client_size = {width = client_width, height = client_height},
        ui_size = {width = ui_width, height = ui_height}
    },
    client_hwnd = client_hwnd,
    
    ffxi = {
        get_player = get_player,
        get_items = get_items,
        get_spells = get_spells
    }
}

-- Windower 4 Compatibility Layer
local event_registry = {
    ['load'] = {},
    ['unload'] = {},
    ['addon command'] = {},
    ['incoming chunk'] = {},
    ['outgoing chunk'] = {},
    ['status change'] = {},
    ['login'] = {}
}

windower.register_event = function(event_name, callback)
    if event_registry[event_name] then
        table.insert(event_registry[event_name], callback)
    end
end

-- Used by the NextXI engine internally to trigger these legacy events
windower.trigger_event = function(event_name, ...)
    local blocked = false
    local modified_str = nil
    if event_registry[event_name] then
        for _, cb in ipairs(event_registry[event_name]) do
            local success, result = pcall(cb, ...)
            if not success then
                print("Error in event '" .. event_name .. "': " .. tostring(result))
            else
                if result == true then
                    blocked = true
                elseif type(result) == "string" then
                    modified_str = result
                end
            end
        end
    end
    return blocked, modified_str
end

windower.add_to_chat = function(mode, text)
    print(text)
end

windower.file_exists = function(path)
    local f = io.open(path, "r")
    if f ~= nil then
        io.close(f)
        return true
    else
        return false
    end
end

windower.debug = function(...)
    -- Stubbed out to avoid log spam
end

windower.send_command = function(cmd)
    -- Stubbed: In a full implementation, this routes back to command_manager.hpp
    print("Command Sent: " .. tostring(cmd))
end

windower.from_shift_jis = function(str) return str end
windower.convert_auto_trans = function(str) return str end

return windower
