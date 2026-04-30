local core_unicode = require('core.unicode')
local unicode = {}

function unicode.to_shift_jis(utf8_string)
    return core_unicode.to_shift_jis(utf8_string)
end

function unicode.from_shift_jis(shift_jis_string)
    return core_unicode.from_shift_jis(shift_jis_string)
end

unicode.symbols = core_unicode.symbol

return unicode