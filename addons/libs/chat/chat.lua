local core_chat = require('core.chat')
local chat = {}

function chat.print(text, color)
    color = color or 207
    core_chat.add_text(tostring(text), color, false)
end

-- ==========================================
-- QoL: Semantic Developer Logging
-- ==========================================
function chat.error(text)
    -- 167 is standard FFXI system error red
    core_chat.add_text(tostring(text), 167, false) 
end

function chat.warning(text)
    -- 166 is usually an alert orange/yellow
    core_chat.add_text(tostring(text), 166, false) 
end

function chat.success(text)
    -- 204 or 207 gives a nice positive/success color
    core_chat.add_text(tostring(text), 204, false) 
end

function chat.on_text_added(callback)
    core_chat.text_added:register(callback)
end

return chat