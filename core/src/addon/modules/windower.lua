--[[
Copyright © Windower Dev Team

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"),to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

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
    read_market_file_ptr,    -- Must be here to catch arg 19!
    write_market_file_ptr = ... -- Must be here to catch arg 20!
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
    
    -- Expose all 4 bridges to your FenestraSDK!
    get_package_list = get_package_list, 
    get_package_readme = get_package_readme,
    read_file = read_file,   
    write_file = write_file, 
    
    settings = {
        client_size = {width = client_width, height = client_height},
        ui_size = {width = ui_width, height = ui_height}
    },
    client_hwnd = client_hwnd
}

return windower