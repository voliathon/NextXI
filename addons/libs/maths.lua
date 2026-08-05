    A few math helper functions.
]]
local debug = debug or require('debug')


_libs = _libs or {}

require('functions')

local functions = _libs.functions
local string = require('string')

local math = require('math')

_libs.maths = math

_raw = _raw or {}
_raw.math = setmetatable(_raw.math or {}, {__index = math})

debug.setmetatable(0, {
    __index = function(_, k)
        return math[k] or (_raw and _raw.error or error)(string.format('"%s" is not defined for numbers', tostring(k)), 2)
    end
})
local digitorder = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}
math.e = math.exp(1)
math.tau = 2 * math.pi
math.phi = (1 + math.sqrt(5))/2
function math.round(num, prec)
    local mult = 10^(prec or 0)
    return (num * mult + 0.5):floor() / mult 
end
function math.sgn(num)
    return num > 0 and 1 or num < 0 and -1 or 0
end
_raw.math.log = math.log
function math.log(val, base)
    if not base then
        return _raw.math.log(val)
    end

    return _raw.math.log(val)/_raw.math.log(base)
end
function math.binary(val)
    return val:base(2)
end
function math.octal(val)
    return val:base(8)
end
function math.hex(val)
    return val:base(16)
end
function math.base(val, base)
    if base == nil or base == 10 or val == 0 then
        return val:string()
    elseif base == 1 then
        return string.rep('1', val)
    end

    local num = val:abs()

    local res = {}
    local key = 1
    local pos
    while num > 0 do
        pos = num % base + 1
        res[key] = digitorder[pos]
        num = (num / base):floor()
        key = key + 1
    end

    local str = ''
    local n = key - 1
    for key = 1, n do
        str = str..res[n - key + 1]
    end

    if val < 0 then
        str = '-'..str
    end

    return str
end
math.string = tostring
math.char = string.char

function math.degree(v)
    return 360 * v / math.tau
end

function math.radian(v)
    return math.tau * v / 360
end
Copyright © 2013-2014, Windower
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Windower nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Windower BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]
