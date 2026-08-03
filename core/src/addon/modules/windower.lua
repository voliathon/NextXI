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
    get_ffxi_spells_ptr,
    get_ffxi_entities_ptr,
    project_ptr = ...
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
local get_ffxi_entities_c = ffi.typeof('char const*(*)()')(get_ffxi_entities_ptr)
local project_c = ffi.typeof('void(*)(float const*, float*)')(project_ptr)


-- Lightweight inline JSON decoder (pure Lua, no external module required)
-- Handles objects {}, arrays [], strings, numbers, booleans, null
local json_decode
do
    local function skip_ws(s, i)
        while i <= #s do
            local c = s:sub(i,i)
            if c == ' ' or c == '\t' or c == '\n' or c == '\r' then i = i + 1
            else break end
        end
        return i
    end
    local parse_value  -- forward decl
    local function parse_string(s, i)
        i = i + 1  -- skip opening "
        local t = {}
        while i <= #s do
            local c = s:sub(i,i)
            if c == '"' then return table.concat(t), i + 1 end
            if c == '\\' then
                local n = s:sub(i+1,i+1)
                if     n == '"' then table.insert(t, '"');  i = i + 2
                elseif n == '\\' then table.insert(t, '\\'); i = i + 2
                elseif n == '/' then table.insert(t, '/');  i = i + 2
                elseif n == 'n' then table.insert(t, '\n'); i = i + 2
                elseif n == 'r' then table.insert(t, '\r'); i = i + 2
                elseif n == 't' then table.insert(t, '\t'); i = i + 2
                else table.insert(t, n); i = i + 2 end
            else
                table.insert(t, c); i = i + 1
            end
        end
        error('unterminated string')
    end
    local function parse_number(s, i)
        local j = i
        if s:sub(j,j) == '-' then j = j + 1 end
        while j <= #s and s:sub(j,j):match('%d') do j = j + 1 end
        if j <= #s and s:sub(j,j) == '.' then
            j = j + 1
            while j <= #s and s:sub(j,j):match('%d') do j = j + 1 end
        end
        if j <= #s and (s:sub(j,j) == 'e' or s:sub(j,j) == 'E') then
            j = j + 1
            if j <= #s and (s:sub(j,j) == '+' or s:sub(j,j) == '-') then j = j + 1 end
            while j <= #s and s:sub(j,j):match('%d') do j = j + 1 end
        end
        return tonumber(s:sub(i, j-1)), j
    end
    local function parse_array(s, i)
        i = i + 1  -- skip [
        local t = {}
        i = skip_ws(s, i)
        if s:sub(i,i) == ']' then return t, i + 1 end
        while true do
            local v; v, i = parse_value(s, i)
            table.insert(t, v)
            i = skip_ws(s, i)
            local c = s:sub(i,i)
            if c == ']' then return t, i + 1 end
            if c ~= ',' then error('expected , or ]') end
            i = i + 1
            i = skip_ws(s, i)
        end
    end
    local function parse_object(s, i)
        i = i + 1  -- skip {
        local t = {}
        i = skip_ws(s, i)
        if s:sub(i,i) == '}' then return t, i + 1 end
        while true do
            i = skip_ws(s, i)
            if s:sub(i,i) ~= '"' then error('expected key string') end
            local k; k, i = parse_string(s, i)
            i = skip_ws(s, i)
            if s:sub(i,i) ~= ':' then error('expected :') end
            i = i + 1
            i = skip_ws(s, i)
            local v; v, i = parse_value(s, i)
            t[k] = v
            i = skip_ws(s, i)
            local c = s:sub(i,i)
            if c == '}' then return t, i + 1 end
            if c ~= ',' then error('expected , or }') end
            i = i + 1
        end
    end
    parse_value = function(s, i)
        i = skip_ws(s, i)
        local c = s:sub(i,i)
        if c == '{' then return parse_object(s, i)
        elseif c == '[' then return parse_array(s, i)
        elseif c == '"' then return parse_string(s, i)
        elseif c == 't' then return true,  i + 4
        elseif c == 'f' then return false, i + 5
        elseif c == 'n' then return nil,   i + 4
        else return parse_number(s, i) end
    end
    json_decode = function(s)
        local ok, result = pcall(function()
            local v, _ = parse_value(s, 1)
            return v
        end)
        if ok then return result else return nil end
    end
end

local function get_player()
    local ptr = get_ffxi_player_c()
    if ptr ~= nil then
        return json_decode(ffi.string(ptr)) or {}
    end
    return {}
end

local function get_items()
    local ptr = get_ffxi_items_c()
    if ptr ~= nil then
        return json_decode(ffi.string(ptr)) or {}
    end
    return {}
end

local function get_spells()
    local ptr = get_ffxi_spells_c()
    if ptr ~= nil then
        return json_decode(ffi.string(ptr)) or {}
    end
    return {}
end

local function get_entities()
    local ptr = get_ffxi_entities_c()
    if ptr ~= nil then
        return json_decode(ffi.string(ptr)) or {}
    end
    return {}
end

local in_vec = ffi.new('float[3]')
local out_vec = ffi.new('float[2]')
local function project(x, y, z)
    in_vec[0] = x; in_vec[1] = y; in_vec[2] = z;
    project_c(in_vec, out_vec)
    if out_vec[0] < 0 and out_vec[1] < 0 then return nil, nil end
    return out_vec[0], out_vec[1]
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

    -- Windower 4 compat: addon_path points to this addon's directory.
    -- In NextXI, package_path is the addon root (e.g. .../addons/GearSwap/).
    -- We ensure it has a trailing separator so string concatenation works.
    addon_path = (package_path or ''):gsub('[/\\]+$', '') .. '/',

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
    
    math = {
        project = project
    },

    ffxi = {
        get_player = get_player,
        get_items = get_items,
        get_spells = get_spells,
        get_entities = get_entities,
        get_bag_info = function() return {} end
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

pcall(function()
    ffi.cdef[[
        uint32_t GetFileAttributesA(const char* lpFileName);
        bool CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes);
    ]]
end)
local bit = require('bit')

windower.file_exists = function(path)
    if type(path) ~= 'string' or path == '' then return false end
    local success, attrs = pcall(function() return ffi.C.GetFileAttributesA(path) end)
    if not success or attrs == 0xFFFFFFFF then return false end
    -- Check if it's NOT a directory (FILE_ATTRIBUTE_DIRECTORY is 16)
    return bit.band(attrs, 16) == 0
end

-- Windower 4 compat: check if a directory exists.
windower.dir_exists = function(path)
    if type(path) ~= 'string' or path == '' then return false end
    local success, attrs = pcall(function() return ffi.C.GetFileAttributesA(path) end)
    if not success or attrs == 0xFFFFFFFF then return false end
    return bit.band(attrs, 16) == 16
end

-- Windower 4 compat: create a directory.
windower.create_dir = function(path)
    if type(path) ~= 'string' or path == '' then return false end
    local success, res = pcall(function() return ffi.C.CreateDirectoryA(path, nil) end)
    return success and res
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
