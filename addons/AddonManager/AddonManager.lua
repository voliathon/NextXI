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
    version = '2.1'
}

-- ============================================================================
-- 1. NATIVE SETTINGS & PERSISTENCE
-- ============================================================================
local defaults = {
    -- We now store settings per-character. 'Global' is the fallback.
    Global = { addons = {} }
}

local profile_settings = settings.load(defaults, 'profiles')

local state = {
    show_ui = false,
    packages = {},
    scanned = false,
    show_readme = false,
    readme_title = "Readme",
    readme_content = "",
    current_character = "Global",
    show_official_addons = true,
    show_third_party_addons = true,
    show_dev_tools = false,
    show_dependencies = false
}

local version_cache = {}

-- Elite UI Colors
local COLOR_ACCENT = ui.color.rgb(100, 200, 255)
local COLOR_ON = ui.color.rgb(40, 200, 100)
local COLOR_OFF = ui.color.rgb(220, 80, 80)
local COLOR_TEXT = ui.color.rgb(230, 230, 230)
local COLOR_MUTED = ui.color.rgb(140, 140, 140)
local COLOR_WARN = ui.color.rgb(255, 180, 50)
local COLOR_BG = ui.color.fade(ui.color.system_black, 230)
local COLOR_BORDER = ui.color.rgb(80, 120, 160)

local ELITE_STYLE = {
    text_color = COLOR_TEXT,
    background_color = COLOR_BG,
    border_color = COLOR_BORDER,
    opacity = 0.95
}

-- Helper to get the active character's addons table
local function get_char_addons()
    local c = state.current_character
    if not profile_settings[c] then profile_settings[c] = { addons = {} } end
    return profile_settings[c].addons
end

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
                    
                    coroutine.schedule(function()
                        coroutine.sleep(1)
                        
                        -- Print Banner on Zone In
                        chat.success("====================================================")
                        chat.success(" [ NextXI AddonManager 2.1 ] System Online")
                        chat.success(" Profile Loaded: " .. name)
                        chat.success(" Type /addon to configure your dashboard.")
                        chat.success("====================================================")
                        
                        local addons = get_char_addons()
                        for k, v in pairs(addons) do
                            if type(v) == "boolean" then
                                addons[k] = v and "login" or "off"
                            end
                        end

                        for addon_id, state_val in pairs(addons) do
                            if state_val == "login" then
                                core_command.input('/load ' .. addon_id)
                            end
                        end

                        coroutine.schedule(function()
                            local expected_character = name
                            coroutine.sleep(5)
                            if state.current_character == expected_character then
                                for addon_id, state_val in pairs(addons) do
                                    if state_val == "delayed" then
                                        core_command.input('/load ' .. addon_id)
                                    end
                                end
                            end
                        end)
                        
                        state.scanned = false 
                        
                        local config_state = addons['config'] or "off"
                        if config_state == "off" then
                            core_command.input('/unload config')
                        end
                    end)
                end
            end
        end
    })
end)

-- ============================================================================
-- 3. README MARKDOWN PARSER
-- ============================================================================
local function parse_markdown_to_windower(md_text)
    if not md_text or md_text == "" then return "No documentation available." end
    local parsed = md_text:gsub("\r\n", "\n")
    parsed = parsed:gsub("^#+ ", ""):gsub("\n#+ ", "\n")
    parsed = parsed:gsub("%*%*(.-)%*%*", "%1")
    parsed = parsed:gsub("%*(.-)%*", "%1")
    parsed = parsed:gsub("%[(.-)%]%((.-)%)", "%1 (%2)")
    return parsed
end

local function calculate_readme_height(text)
    if not text then return 460 end
    local lines = 1
    for i in text:gmatch("\n") do lines = lines + 1 end
    local total_height = lines * 22
    local code_blocks = 0
    for block in text:gmatch("`") do
        code_blocks = code_blocks + 1
        if code_blocks % 2 == 1 then
            total_height = total_height + 20 
        end
    end
    return math.max(610, total_height + 50)
end

-- ============================================================================
-- 4. MANIFEST SCANNER
-- ============================================================================
local function scan_packages()
    state.packages = {}
    
    local raw_data = core_windower.get_package_list()
    if not raw_data then return end

    local char_addons = get_char_addons()

    for pkg_str in raw_data:gmatch("([^;]+)") do
        local raw_name, pkg_version, pkg_path, has_readme_str = pkg_str:match("([^|]+)|([^|]+)|([^|]+)|([^|]+)")
        if raw_name then
            local pkg_name = raw_name:match("^%s*(.-)%s*$")
            local exact_version = pkg_version:match("^%s*([%d%.]+)") or pkg_version 
            
            local is_dev = false
            local is_official = false
            local engine_support = "DX8/DX11"
            local author = "Unknown"
            local description = "No description provided."
            
            if pkg_name == "config" then is_official = true end 
            
            if version_cache[pkg_path] then
                exact_version = version_cache[pkg_path]
                is_dev = version_cache[pkg_path .. "_dev"] or false
                is_official = version_cache[pkg_path .. "_off"] or is_official
                engine_support = version_cache[pkg_path .. "_eng"] or "DX8/DX11"
                author = version_cache[pkg_path .. "_auth"] or "Unknown"
                description = version_cache[pkg_path .. "_desc"] or "No description provided."
            else
                local manifest_file = file.new(pkg_path .. "\\manifest.xml")
                if manifest_file:exists() then
                    local content = manifest_file:read()
                    if content then
                        local raw_v = content:match("<version>%s*(.-)%s*</version>")
                        if raw_v then exact_version = raw_v end
                        
                        local raw_author = content:match("<author>%s*(.-)%s*</author>")
                        if raw_author then author = raw_author end
                        
                        local raw_desc = content:match("<description>%s*(.-)%s*</description>")
                        if raw_desc then
                            if raw_desc:match("^%[DEV%]") then is_dev = true end
                            if raw_desc:match("^%[OFFICIAL%]") then is_official = true end
                            description = raw_desc:gsub("^%[DEV%]%s*", ""):gsub("^%[OFFICIAL%]%s*", "")
                        end

                        local raw_engine = content:match("<engine>%s*(.-)%s*</engine>")
                        if raw_engine then engine_support = raw_engine end
                    end
                end
                version_cache[pkg_path] = exact_version
                version_cache[pkg_path .. "_dev"] = is_dev
                version_cache[pkg_path .. "_off"] = is_official
                version_cache[pkg_path .. "_eng"] = engine_support
                version_cache[pkg_path .. "_auth"] = author
                version_cache[pkg_path .. "_desc"] = description
            end
            
            local cat = "addon"
            local grp = "third_party"

            if pkg_name == "NextXISDK" or pkg_name == "AddonManager" then
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
                if is_dev then grp = "dev"
                elseif is_official then grp = "official" end
            end
            
            local current_state = "off"
            if char_addons[pkg_name] ~= nil then
                local s = char_addons[pkg_name]
                if type(s) == "boolean" then
                    current_state = s and "login" or "off"
                else
                    current_state = s
                end
            end

            table.insert(state.packages, {
                id = pkg_name, 
                name = pkg_name,
                version = exact_version, 
                path = pkg_path,
                has_readme = (has_readme_str == "1"), 
                loaded = (current_state ~= "off"),
                lifecycle = current_state,
                category = cat,
                group = grp,
                engine = engine_support or "DX8/DX11",
                author = author,
                description = description
            })
        end
    end
    
    table.sort(state.packages, function(a, b) 
        if a.loaded ~= b.loaded then return a.loaded end
        return a.name:lower() < b.name:lower() 
    end)
    state.scanned = true
end

-- ============================================================================
-- 5. ELITE UI RENDERING
-- ============================================================================
local market_window = ui.window_state()
market_window.title = " NEXTXI ADDON MANAGER"
market_window.size = {width = 760, height = 750}
market_window.visible = false

local readme_window = ui.window_state()
readme_window.title = " Documentation"
readme_window.size = {width = 750, height = 650}
readme_window.visible = false
local readme_scroll = ui.scroll_panel_state(730, 610)

local scroll_view_content_height = 620
local scroll_view = ui.scroll_panel_state(730, scroll_view_content_height)

local function update_scroll_canvas()
    local required_height = 20 
    
    if state.show_official_addons then
        required_height = required_height + 40
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "official" then required_height = required_height + 80 end
        end
    end
    if state.show_third_party_addons then
        required_height = required_height + 40
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "third_party" then required_height = required_height + 80 end
        end
    end
    if state.show_dev_tools then
        required_height = required_height + 40
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "dev" then required_height = required_height + 80 end
        end
    end
    if state.show_dependencies then
        required_height = required_height + 40
        for _, pkg in ipairs(state.packages) do
            if pkg.group == "core" or pkg.group == "dependency" then required_height = required_height + 80 end
        end
    end

    local final_height = math.max(620, required_height)
    if final_height ~= scroll_view_content_height then
        scroll_view_content_height = final_height
        scroll_view = ui.scroll_panel_state(730, scroll_view_content_height)
    end
end

-- Draw a single addon row
local function draw_package(canvas, pkg)
    canvas:space(8)

    local c_state = pkg.lifecycle or "off"
    local is_on = (c_state == "login" or c_state == "boot" or c_state == "delayed")

    -- ROW 1: Name colored green if ENABLED, muted gray if DISABLED
    local name_color = is_on and COLOR_ON or COLOR_MUTED
    canvas:label("  " .. pkg.name .. "  (v" .. pkg.version .. ")", name_color)
    canvas:same_line()

    if pkg.group == "core" or pkg.group == "dependency" then
        -- System packages: badge only, no toggle
        if pkg.group == "core" then
            canvas:width(160):label("[ CORE SYSTEM ]", COLOR_WARN)
        else
            canvas:width(160):label("[ DEPENDENCY  ]", COLOR_MUTED)
        end
    else
        -- Status badge: green ENABLED / red DISABLED
        if is_on then
            canvas:width(120):label("  [● ENABLED ]", COLOR_ON)
        else
            canvas:width(120):label("  [○ DISABLED]", COLOR_OFF)
        end

        canvas:same_line()

        -- Action button: clearly labeled, wide enough to always fit text
        local btn_label = is_on and "  Disable  " or "  Enable  "
        local btn_color = is_on and COLOR_OFF or COLOR_ON
        local clicked = canvas:width(110):button("act_" .. pkg.id, btn_label, false)

        -- Readme button
        if pkg.has_readme then
            canvas:same_line()
            if canvas:width(90):button("btn_rm_" .. pkg.id, "  Readme  ", false) then
                state.readme_title = " " .. pkg.name .. " - Documentation"
                local raw_text = core_windower.get_package_readme(pkg.id) or ""
                state.readme_content = parse_markdown_to_windower(raw_text)
                local dynamic_h = calculate_readme_height(raw_text)
                readme_scroll = ui.scroll_panel_state(730, dynamic_h)
                state.show_readme = true
            end
        end

        if clicked then
            -- Toggle state immediately in the data model
            local new_state = is_on and "off" or "login"
            pkg.lifecycle = new_state
            pkg.loaded = (new_state ~= "off")

            -- Persist to profile
            local char_addons = get_char_addons()
            char_addons[pkg.id] = new_state
            settings.save('profiles')

            -- Issue the engine command
            if new_state == "off" then
                core_command.input('/unload ' .. pkg.id)
                chat.warning("AddonManager: Unloaded [" .. pkg.name .. "]")
            else
                core_command.input('/load ' .. pkg.id)
                chat.success("AddonManager: Loaded [" .. pkg.name .. "]")
            end
        end
    end

    -- ROW 2: Author + description
    canvas:space(2)
    canvas:label("     Author: " .. (pkg.author or "Unknown") .. "  |  " .. pkg.group:upper(), COLOR_MUTED)
    canvas:space(2)
    canvas:label("     " .. (pkg.description or "No description provided."), COLOR_TEXT)

    canvas:space(6)
    canvas:label("-----------------------------------------------------------------------------------------------------------------------------------", COLOR_MUTED)
end



local function draw_section_header(canvas, title, var_name)
    local is_open = state[var_name]
    local icon = is_open and " [-] " or " [+] "
    -- Use a wide fixed button so the full title text always fits
    if canvas:width(710):button("hdr_" .. var_name, icon .. title, false) then
        state[var_name] = not is_open
        update_scroll_canvas()
    end
    canvas:space(6)
    return state[var_name]
end

ui.display(function()
    local success, err = pcall(function()
        if state.show_ui then
            market_window.visible = true
            if not state.scanned then scan_packages() end
            
            ui.push_style(ELITE_STYLE)
            
            local window_still_open = ui.window(market_window, function(layout)
                
                layout:space(10)
                layout:label("    Profile Management", COLOR_MUTED)
                layout:space(5)
                layout:label("    Active Profile:  " .. state.current_character:upper(), COLOR_ACCENT)
                
                layout:same_line()
                layout:space(380)
                if layout:button("btn_rescan", "   SCAN FOR NEW ADDONS   ", false) then
                    scan_packages()
                    chat.success("AddonManager: Packages rescanned dynamically.")
                end
                
                layout:space(15)
                layout:label("=============================================================================================================", COLOR_BORDER)
                layout:space(5)

                layout:height(610):scroll_panel(scroll_view, function(canvas)
                    
                    -- Section 1: Official Addons
                    if draw_section_header(canvas, "NEXTXI OFFICIAL ADDONS", "show_official_addons") then
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "official" then draw_package(canvas, pkg) end
                        end
                        canvas:space(20)
                    end

                    -- Section 2: Third-Party Addons
                    if draw_section_header(canvas, "COMMUNITY ADDONS", "show_third_party_addons") then
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "third_party" then draw_package(canvas, pkg) end
                        end
                        canvas:space(20)
                    end

                    -- Section 3: Developer Tools
                    if draw_section_header(canvas, "DEVELOPER TOOLS", "show_dev_tools") then
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "dev" then draw_package(canvas, pkg) end
                        end
                        canvas:space(20)
                    end

                    -- Section 4: Core Systems & Libraries
                    if draw_section_header(canvas, "CORE SYSTEMS & DEPENDENCIES", "show_dependencies") then
                        for _, pkg in ipairs(state.packages) do
                            if pkg.group == "core" or pkg.group == "dependency" then draw_package(canvas, pkg) end
                        end
                        canvas:space(20)
                    end
                    
                end) 
            end)
            ui.pop_style()
            if not window_still_open then state.show_ui = false end
        else
            market_window.visible = false
        end

        if state.show_readme then
            readme_window.visible = true
            readme_window.title = state.readme_title
            
            ui.push_style(ELITE_STYLE)
            local rm_still_open = ui.window(readme_window, function(layout)
                layout:height(610):scroll_panel(readme_scroll, function(canvas)
                    canvas:space(10)
                    canvas:label("  " .. state.readme_content)
                end)
            end)
            ui.pop_style()
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
            chat.success("AddonManager: Packages rescanned dynamically.")
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

-- ============================================================================
-- 7. BOOT EXECUTION (Core Addons)
-- ============================================================================
coroutine.schedule(function()
    coroutine.sleep_frame()
    local global_addons = profile_settings["Global"] and profile_settings["Global"].addons or {}
    for addon_id, state_val in pairs(global_addons) do
        if state_val == "boot" then
            core_command.input('/load ' .. addon_id)
        end
    end
end)

return addon
