--[[
    Windower 4 / NextXI compatibility shim for the 'chat' library.

    Windower 4 addons use _libs.chat.controls.reset to reset color in
    windower.add_to_chat() calls. This shim provides that interface while
    also bridging to NextXI's native core.chat module for actual output.
]]

_libs = _libs or {}

-- Bridge to NextXI's native chat implementation where available
local core_chat_ok, core_chat = pcall(require, 'core.chat')

-- Windower 4 chat color control sequences for FFXI's chat system
-- 0x1F (31) = color byte prefix, 0x01 = reset to default
local controls = {
    reset       = '\31\1',
    color_start = '\31',
}

local chat = {controls = controls}

-- NextXI-compatible API (used by addons expecting require('chat') to return
-- a module with .print / .error / .warning / .success)
function chat.print(text, color)
    color = color or 207
    if core_chat_ok then
        core_chat.add_text(tostring(text), color, false)
    end
end

function chat.error(text)
    if core_chat_ok then core_chat.add_text(tostring(text), 167, false) end
end

function chat.warning(text)
    if core_chat_ok then core_chat.add_text(tostring(text), 166, false) end
end

function chat.success(text)
    if core_chat_ok then core_chat.add_text(tostring(text), 204, false) end
end

function chat.on_text_added(callback)
    if core_chat_ok then core_chat.text_added:register(callback) end
end

-- Stub windower.to_shift_jis / from_shift_jis if not available.
-- NextXI handles encoding internally; returning the string as-is is
-- sufficient for logging purposes.
if windower then
    if not windower.to_shift_jis then
        windower.to_shift_jis = function(str) return tostring(str or '') end
    end
    if not windower.from_shift_jis then
        windower.from_shift_jis = function(str) return tostring(str or '') end
    end
end

_libs.chat = chat
return chat
