local ui = require('ui')
local chat = require('chat')
local command = require('command')
local packet = require('packet')
local settings = require('settings')
local file = require('file')
local ffi = require('ffi')
local core_windower = require('core.windower')
local core_command = require('core.command')

local table = require('table')
local string = require('string')
local math = require('math')

local addon = {
    name = 'AddonManager',
    author = 'Voliathon of Bahamut',
    version = '1.3'
}

-- ============================================================================
-- 1. NATIVE SETTINGS & PERSISTENCE 
-- ============================================================================
local defaults = {
    addons = {} -- Stores boolean states for the active character
}

-- The settings library automatically intercepts this and saves to: 
-- addons/AddonManager/data/<CharName_Server>/profiles.lua
local profile_settings = settings.load(defaults, 'profiles')

local state = {
    show_ui = false,
    packages = {},
    scanned = false,
    show_readme = false,
    readme_title = "Readme",
    readme_content = "",
    current_character = "Global", 
    show_official_addons = true,    -- Opens automatically
    show_third_party_addons = false, 
    show_dev_tools = false, 
    show_dependencies = false 
}

local version_cache = {}

-- ============================================================================
-- 2. NETWORK LISTENER (Autoloads on Login)
-- ============================================================================
coroutine.schedule(function()
    coroutine.sleep_frame()
    
    packet.incoming:register_init({
        [{0x00A}] = function(p)
            if p and p.player_name then
                local name = p.player_name
                if type(name) == 'cdata' then name = ffi.string(name) else name = tostring(name) end
                
                if name ~= "" and name:lower() ~= "global" and state.current_character ~= name then
                    state.current_character = name
                    
                    -- Wait 1 second for the account_service to register the server ID.
                    -- This triggers settings.lua to dynamically swap your profile_settings 
                    -- object to the newly logged-in character's file.
                    coroutine.schedule(function()
                        coroutine.sleep(1)
                        
                        -- Execute the user's saved loadout
                        if profile_settings.addons then
                            for addon_id, is_active in pairs(profile_settings.addons) do
                                if is_active then
                                    core_command.input('/load ' .. addon_id)
                                end
                            end
                        end
                        
                        state.scanned = false 
                        
                        -- COUNTER-MEASURE: Defeat the account_service forced load
                        if not profile_settings.addons['config'] then
                            core_command.input('/unload config')
                        end
                    end)
                end
            end
        end
    })
end)

-- ============================================================================
-- 3. MARKDOWN TRANSLATOR
-- ============================================================================
local function parse_markdown_to_windower(text)
    if not text or text == "" then return "No content found." end
    text = text:gsub("^# (.-)\n", "[%1]{size:xx-large weight:bold color:skin_accent}\n")
    text = text:gsub("\n# (.-)\n", "\n[%1]{size:xx-large weight:bold color:skin_accent}\n")
    text = text:gsub("^## (.-)\n", "[%1]{size:x-large weight:bold}\n")
    text = text:gsub("\n## (.-)\n", "\n[%1]{size:x-large weight:bold}\n")
    text = text:gsub("^### (.-)\n", "[%1]{size:large weight:bold color:system_gray}\n")
    text = text:gsub("\n### (.-)\n", "\n[%1]{size:large weight:bold color:system_gray}\n")
    text = text:gsub("%*%*(.-)%*%*", "[%1]{weight:bold}")
    text = text:gsub("%*(.-)%*", "[%1]{style:italic}")
    text = text:gsub("`(.-)`", "[%1]{color:system_white}")
    return text
end

-- ============================================================================
-- 4. DYNAMIC SCROLL BAR & PACKAGE SCANNER
-- ============================================================================
local scroll_view_content_height = 360
local scroll_view = ui.scroll_panel_state(430, scroll_view_content_height)

local function update_scroll_canvas()
    local required_height = 10 

    -- Accordion Headers take ~30px space each (4 headers = 120px)
    
    required_height = required_height + 30
    if state.show_official_addons then
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "official" then required_height = required_height + 65 end
        end
    end
    required_height = required_height + 20 -- Spacer
    
    required_height = required_height + 30
    if state.show_third_party_addons then
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "third_party" then required_height = required_height + 65 end
        end
    end
    required_height = required_height + 20 

    required_height = required_height + 30
    if state.show_dev_tools then
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "dev" then required_height = required_height + 65 end
        end
    end
    required_height = required_height + 20 

    required_height = required_height + 30
    if state.show_dependencies then
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "core" or pkg.group == "dependency" then required_height = required_height + 60 end
        end
    end

    local final_height = math.max(360, required_height)
    if final_height ~= scroll_view_content_height then
        scroll_view_content_height = final_height
        scroll_view = ui.scroll_panel_state(430, scroll_view_content_height)
    end
end

local function calculate_readme_height(text)
    if not text or text == "" then return 460 end
    local total_height = 0
    for line in text:gmatch("([^\n]*)\n?") do
        if line == "" then
            total_height = total_height + 15 
        elseif line:match("^# ") then
            total_height = total_height + 45 
        elseif line:match("^## ") then
            total_height = total_height + 35 
        elseif line:match("^### ") then
            total_height = total_height + 25 
        else
            local chars = #line
            local wraps = math.max(1, math.ceil(chars / 85))
            total_height = total_height + (wraps * 20)
        end
    end
    return math.max(460, total_height + 50)
end

local function scan_packages()
    state.packages = {}
    
    local raw_data = core_windower.get_package_list()
    if not raw_data then return end

    for pkg_str in raw_data:gmatch("([^;]+)") do
        local raw_name, pkg_version, pkg_path, has_readme_str = pkg_str:match("([^|]+)|([^|]+)|([^|]+)|([^|]+)")
        if raw_name then
            local pkg_name = raw_name:match("^%s*(.-)%s*$")
            local exact_version = pkg_version:match("^%s*([%d%.]+)") or pkg_version 
            
            local is_dev = false
            local is_official = false
            if pkg_name == "config" then is_official = true end 

            if version_cache[pkg_path] then
                exact_version = version_cache[pkg_path]
                is_dev = version_cache[pkg_path .. "_dev"] or false
                is_official = version_cache[pkg_path .. "_off"] or is_official
            else
                local manifest_file = file.new(pkg_path .. "\\manifest.xml")
                if manifest_file:exists() then
                    local content = manifest_file:read()
                    if content then
                        local raw_v = content:match("<version>%s*(.-)%s*</version>")
                        if raw_v then exact_version = raw_v end
                        
                        -- Parse description for tags
                        local raw_desc = content:match("<description>%s*(.-)%s*</description>")
                        if raw_desc then
                            if raw_desc:match("^%[DEV%]") then is_dev = true end
                            if raw_desc:match("^%[OFFICIAL%]") then is_official = true end
                        end
                    end
                end
                version_cache[pkg_path] = exact_version
                version_cache[pkg_path .. "_dev"] = is_dev
                version_cache[pkg_path .. "_off"] = is_official
            end
            
            local cat = "addon"
            local grp = "third_party"

            if pkg_name == "FenestraSDK" or pkg_name == "AddonManager" then
                cat = "core"
                grp = "core"
            elseif pkg_path:find("[\\/]libs[\\/]") or 
                   pkg_name:match("_service$") or 
                   pkg_name:match("_data$") or 
                   pkg_name == "mime" or 
                   pkg_name == "socket" then
                cat = "dependency"
                grp = "dependency"
            else
                if is_dev then
                    grp = "dev"
                elseif is_official then
                    grp = "official"
                else
                    grp = "third_party"
                end
            end

            local is_loaded = (profile_settings.addons and profile_settings.addons[pkg_name] == true)
            
            table.insert(state.packages, {
                id = pkg_name, 
                name = pkg_name,
                version = exact_version, 
                path = pkg_path,
                has_readme = (has_readme_str == "1"), 
                loaded = is_loaded,
                category = cat,
                group = grp
            })
        end
    end
    
    table.sort(state.packages, function(a, b) return a.name < b.name end)
    state.scanned = true
    
    update_scroll_canvas()
end

-- ============================================================================
-- 5. UI RENDERING 
-- ============================================================================
local market_window = ui.window_state()
market_window.title = "Addon Manager    |    ACTIVE PROFILE: " .. state.current_character:upper()
market_window.size = {width = 450, height = 550}
market_window.resizable = false
market_window.visible = false

local readme_window = ui.window_state()
readme_window.title = "Readme Viewer"
readme_window.size = {width = 600, height = 500}
readme_window.resizable = false
readme_window.visible = false
local readme_scroll = ui.scroll_panel_state(580, 460)

-- Centralized rendering logic to keep the scroll loop clean
local function draw_package(canvas, pkg)
    if pkg.group == "core" or pkg.group == "dependency" then
        canvas:label(pkg.name .. "  v" .. pkg.version, ui.color.system_gray)
        
        if pkg.group == "core" then
            canvas:label("  🔒 CORE SYSTEM", ui.color.system_white)
        else
            canvas:label("  [ LIBRARY ]", ui.color.system_disabled)
        end
        
        if pkg.has_readme then
            local clicked_readme = canvas:button("btn_rm_" .. pkg.id, "📄 ReadMe", false)
            if clicked_readme then
                state.readme_title = pkg.name .. " Readme"
                local raw_text = core_windower.get_package_readme(pkg.id) or ""
                state.readme_content = parse_markdown_to_windower(raw_text)
                
                local dynamic_h = calculate_readme_height(raw_text)
                readme_scroll = ui.scroll_panel_state(580, dynamic_h)
                
                state.show_readme = true
            end
        end
        canvas:space(20) 
    else
        canvas:label(pkg.name .. "  v" .. pkg.version)
        
        local tgl_label = pkg.loaded and "  [Active]{color:limegreen weight:bold}" or "  Offline"
        local clicked, _ = canvas:check("tgl_" .. pkg.id, tgl_label, pkg.loaded)
        
        if clicked and not pkg.locked then
            pkg.locked = true 
            pkg.loaded = not pkg.loaded
            
            if not profile_settings.addons then profile_settings.addons = {} end
            profile_settings.addons[pkg.id] = pkg.loaded
            
            if pkg.loaded then
                coroutine.schedule(function()
                    core_command.input('/load ' .. pkg.id) 
                    chat.success("AddonManager: Loaded '" .. pkg.name .. "'")
                    pkg.locked = false 
                end)
            else
                coroutine.schedule(function()
                    core_command.input('/unload ' .. pkg.id)
                    chat.warning("AddonManager: Unloaded '" .. pkg.name .. "'")
                    pkg.locked = false 
                end)
            end
            settings.save('profiles') 
        end

        if pkg.has_readme then
            local clicked_readme = canvas:button("btn_rm_" .. pkg.id, "📄 ReadMe", false)
            if clicked_readme then
                state.readme_title = pkg.name .. " Readme"
                local raw_text = core_windower.get_package_readme(pkg.id) or ""
                state.readme_content = parse_markdown_to_windower(raw_text)
                
                local dynamic_h = calculate_readme_height(raw_text)
                readme_scroll = ui.scroll_panel_state(580, dynamic_h)
                
                state.show_readme = true
            end
        end
        canvas:space(25) 
    end
end

ui.display(function()
    local success, err = pcall(function()

        if state.show_ui then
            market_window.visible = true
            if not state.scanned then scan_packages() end
            
            local window_still_open = ui.window(market_window, function(layout)
                
                layout:space(5)

                layout:height(440):scroll_panel(scroll_view, function(canvas)
                    
                    -- Section 1: Official Addons
                    local clk_off, _ = canvas:check("chk_off", "Official Addons", state.show_official_addons)
                    if clk_off then state.show_official_addons = not state.show_official_addons; update_scroll_canvas() end
                    
                    if state.show_official_addons then
                        canvas:space(10)
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "official" then draw_package(canvas, pkg) end
                        end
                    end
                    
                    canvas:space(10)
                    canvas:label("──────────────────────────────────────────", ui.color.system_gray)
                    canvas:space(5)

                    -- Section 2: Third-Party Addons
                    local clk_3p, _ = canvas:check("chk_3p", "Third-Party Addons", state.show_third_party_addons)
                    if clk_3p then state.show_third_party_addons = not state.show_third_party_addons; update_scroll_canvas() end
                    
                    if state.show_third_party_addons then
                        canvas:space(10)
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "third_party" then draw_package(canvas, pkg) end
                        end
                    end

                    canvas:space(10)
                    canvas:label("──────────────────────────────────────────", ui.color.system_gray)
                    canvas:space(5)

                    -- Section 3: Developer Tools
                    local clk_dev, _ = canvas:check("chk_dev", "Developer Tools", state.show_dev_tools)
                    if clk_dev then state.show_dev_tools = not state.show_dev_tools; update_scroll_canvas() end
                    
                    if state.show_dev_tools then
                        canvas:space(10)
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "dev" then draw_package(canvas, pkg) end
                        end
                    end

                    canvas:space(10)
                    canvas:label("──────────────────────────────────────────", ui.color.system_gray)
                    canvas:space(5)

                    -- Section 4: Core Systems & Libraries
                    local clk_dep, _ = canvas:check("chk_dep", "Core Systems & Libraries", state.show_dependencies)
                    if clk_dep then state.show_dependencies = not state.show_dependencies; update_scroll_canvas() end
                    
                    if state.show_dependencies then
                        canvas:space(10)
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "core" or pkg.group == "dependency" then draw_package(canvas, pkg) end
                        end
                    end
                    
                end) 
            end)
            if not window_still_open then state.show_ui = false end
        else
            market_window.visible = false
        end

        if state.show_readme then
            readme_window.visible = true
            readme_window.title = state.readme_title
            
            local rm_still_open = ui.window(readme_window, function(layout)
                layout:height(460):scroll_panel(readme_scroll, function(canvas)
                    canvas:label(state.readme_content)
                end)
            end)
            if not rm_still_open then state.show_readme = false end
        else
            readme_window.visible = false
        end
    end)
    if not success then 
        state.show_ui = false 
        state.show_readme = false
        chat.error("AddonManager UI Crash: " .. tostring(err))
    end
end)

-- ============================================================================
-- 6. COMMAND ROUTER
-- ============================================================================
command.register({'addon', 'addons'}, function(args)
    local success, err = pcall(function()
        if args[1] == "rescan" then
            scan_packages()
            chat.success("AddonManager: Addons rescanned.")
        elseif args[1] == "reload" and args[2] then
            core_command.input('/reload ' .. args[2])
            chat.success("AddonManager: Reloading package '" .. args[2] .. "'...")
        else
            state.show_ui = not state.show_ui
            if state.show_ui then state.scanned = false end 
        end
    end)
    if not success then chat.error("Command Crash: " .. tostring(err)) end
end)

addon.unload = function()
    chat.warning("AddonManager has been unloaded.")
end

chat.success("----------------------------------------------------")
chat.success("AddonManager Initialized. Type /addon to continue...")
chat.success("----------------------------------------------------")
return addon