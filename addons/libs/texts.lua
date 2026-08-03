
local texts = {}

local text_obj_mt = {
    __index = {
        text = function(self, str)
            self._text = str
        end,
        show = function(self)
            self._visible = true
        end,
        hide = function(self)
            self._visible = false
        end,
        destroy = function(self)
            self._visible = false
        end,
        pos = function(self, x, y)
            self._x = x
            self._y = y
        end
    }
}

function texts.new(settings)
    local obj = {
        _name = "text_" .. tostring(math.random(10000)),
        _text = "",
        _visible = false,
        _x = 0,
        _y = 0
    }
    setmetatable(obj, text_obj_mt)
    return obj
end

return texts
