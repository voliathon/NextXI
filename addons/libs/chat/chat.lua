local core_chat = require('core.chat')
local chat = {}

function chat.print(text, color)
    color = color or 207
    core_chat.add_text(tostring(text), color, false)
end
function chat.error(text)
    core_chat.add_text(tostring(text), 167, false) 
end

function chat.warning(text)
    core_chat.add_text(tostring(text), 166, false) 
end

function chat.success(text)
    core_chat.add_text(tostring(text), 204, false) 
end

function chat.on_text_added(callback)
    core_chat.text_added:register(callback)
end

return chat