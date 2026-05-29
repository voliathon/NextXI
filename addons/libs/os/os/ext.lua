local fn = require('expression')
local os = require('os')
local unicode = require('core.unicode')
local win32 = require('win32')
local windower = require('core.windower')

local execute = win32.def({
    name = 'ShellExecuteW',
    returns = 'int32_t',
    parameters = {
        'void*',
        'wchar_t const*',
        'wchar_t const*',
        'wchar_t const*',
        'wchar_t const*',
        'int32_t',
    },
    module = 'Shell32',
    failure = fn.between(0x00, 0x20),
})

do
    local unicode_to_utf16 = unicode.to_utf16

    local path_cache = setmetatable({}, {
        __index = function(t, k)
            local value = unicode_to_utf16(k)
            t[k] = value
            return value
        end,
    })

    local open_w = unicode_to_utf16('open')

    local hwnd = windower.client_hwnd

    os.open = function(path)
        execute(hwnd, open_w, path_cache[path], nil, nil, 1)
    end
end

return os
