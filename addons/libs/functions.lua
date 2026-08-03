    Adds some tools for functional programming. Amends various other namespaces by functions used in a functional context, when they don't make sense on their own.
]]
local debug = debug or require('debug')


_libs = _libs or {}

local string, math, table, coroutine = require('string'), require('math'), require('table'), require('coroutine')

functions = {}
boolean = {}

_libs.functions = functions

local functions, boolean = functions, boolean
functions.empty = function() end

debug.setmetatable(false, {__index = function(_, k)
    return boolean[k] or (_raw and _raw.error or error)(string.format('"%s" is not defined for booleans', tostring(k)), 2)
end})

for _, t in pairs({functions, boolean, math, string, table}) do
    t.fn = function(val)
        return function()
            return val
        end
    end
end
function functions.identity(...)
    return ...
end
function functions.const(val)
    return function()
        return val
    end
end
function functions.call(fn, ...)
    return fn(...)
end
function functions.cond(fn, check)
    return function(...)
        return check(...) and fn(...) or nil
    end
end
function functions.args(fn, ...)
    local args = {...}
    return function(...)
        local res = {}
        for key, arg in ipairs(args) do
            if type(arg) == 'number' then
                rawset(res, key, select(arg, ...))
            else
                rawset(res, key, arg(...))
            end
        end
        return fn(unpack(res))
    end
end
function functions.prepare(fn, ...)
    local args = {...}
    return function()
        return fn(unpack(args))
    end
end
function functions.apply(fn, ...)
    local args = {...}
    return function(...)
        local res = {}
        for key, arg in ipairs(args) do
            res[key] = arg
        end
        local key = #args
        for _, arg in ipairs({...}) do
            key = key + 1
            res[key] = arg
        end
        return fn(unpack(res))
    end
end
function functions.endapply(fn, ...)
    local args = {...}
    return function(...)
        local res = {...}
        local key = #res
        for _, arg in ipairs(args) do
            key = key + 1
            res[key] = arg
        end
        return fn(unpack(res))
    end
end
function functions.pipe(fn1, fn2)
    return function(...)
        return fn1(fn2(...))
    end
end
function functions.equals(el)
    return function(cmp)
        return el == cmp
    end
end
function functions.negate(fn)
    return function(...)
        return not (true == fn(...))
    end
end
function functions.select(fn, i)
    return function(...)
        return select(i, fn(...))
    end
end
function functions.it(fn, ...)
    local res = {fn(...)}
    local key = 0
    return function()
        key = key + 1
        return res[key]
    end
end
function functions.pack(fn)
    local res = {}
    local value = fn()
    local count = 0
    while value ~= nil do
        count = count + 1
        res[count] = value
        value = fn()
    end
    return res
end
function functions.schedule(fn, time, ...)
    return coroutine.schedule(functions.prepare(fn, ...), time)
end
function functions.delay(fn, time, ...)
    local args = {...}

    return function()
        functions.schedule(fn, time, unpack(args))
    end
end
function functions.loop(fn, interval, cond)
    if interval <= 0 then
        return
    end

    if type(cond) == 'number' then
        cond = (function()
            local i = 0
            local lim = cond
            return function()
                i = i + 1
                return i <= lim
            end
        end)()
    end
    cond = cond or function() return true end

    return coroutine.schedule(function()
        while cond() do
            fn()
            coroutine.sleep(interval)
        end
    end, 0)
end
    Various built-in wrappers
]]
function functions.string(fn)
    return tostring(fn)
end
function functions.type(fn)
    return type(fn)
end
function functions.class(fn)
    return class(fn)
end

local function index(fn, key)
    if type(key) == 'number' then
        return fn:select(key)
    elseif rawget(functions, key) then
        return function(...)
            return functions[key](...)
        end
    end

    (_raw and _raw.error or error)(string.format('"%s" is not defined for functions', tostring(key)), 2)
end

local function add(fn, args)
    return fn:apply(unpack(args))
end

local function sub(fn, args)
    return fn:endapply(unpack(args))
end

debug.setmetatable(functions.empty, {
    __index = index,
    __add = add,
    __sub = sub,
    __concat = functions.pipe,
    __unm = functions.negate,
    __class = 'Function'
})
    Logic functions
Mainly used to pass as arguments.
]]
function boolean._true(val)
    return val == true
end
function boolean._false(val)
    return val == false
end
function boolean._not(val)
    return not val
end
function boolean._and(val1, val2)
    return val1 and val2
end
function boolean._or(val1, val2)
    return val1 or val2
end
function boolean._exists(val)
    return val ~= nil
end
function boolean._is(val1, val2)
    return val1 == val2
end
    Math functions
]]
function math.even(num)
    return num % 2 == 0
end
function math.odd(num)
    return num % 2 == 1
end
function math.add(val1, val2)
    return val1 + val2
end
function math.mult(val1, val2)
    return val1 * val2
end
function math.sub(val1, val2)
    return val1 - val2
end
function math.div(val1, val2)
    return val1 / val2
end
    Table functions
]]
function table.get(t, ...)
    local res = {}
    for i = 1, select('#', ...) do
        rawset(res, i, t[select(i, ...)])
    end
    return unpack(res)
end
function table.rawget(t, ...)
    local res = {}
    for i = 1, select('#', ...) do
        rawset(res, i, rawget(t, select(i, ...)))
    end
    return unpack(res)
end
function table.set(t, ...)
    for i = 1, select('#', ...), 2 do
        t[select(i, ...)] = select(i + 1, ...)
    end
    return t
end
function table.rawset(t, ...)
    for i = 1, select('#', ...), 2 do
        rawset(t, select(i, ...), select(i + 1, ...))
    end
    return t
end
function table.lookup(t, ref, key)
    return ref[t[key]]
end

table.it = (function()
    local it = function(t)
        local key

        return function()
            key = next(t, key)
            return t[key], key
        end
    end

    return function(t)
        local meta = getmetatable(t)
        if not meta then
            return it(t)
        end

        local index = meta.__index
        if index == table then
            return it(t)
        end

        local fn = type(index) == 'table' and index.it or index(t, 'it') or it
        return (fn == table.it and it or fn)(t)
    end
end)()
function table.map(t, fn)
    local res = {}
    for value, key in table.it(t) do
        res[key] = fn(value)
    end

    return setmetatable(res, getmetatable(t))
end
function table.key_map(t, fn)
    local res = {}
    for value, key in table.it(t) do
        res[fn(key)] = value
    end

    return setmetatable(res, getmetatable(t))
end
function table.filter(t, fn)
    if type(fn) ~= 'function' then
        fn = functions.equals(fn)
    end

    local res = {}
    for value, key in table.it(t) do
        if fn(value) then
            res[key] = value
        end
    end

    return setmetatable(res, getmetatable(t))
end
function table.key_filter(t, fn)
    if type(fn) ~= 'function' then
        fn = functions.equals(fn)
    end

    local res = {}
    for value, key in table.it(t) do
        if fn(key) then
            res[key] = value
        end
    end

    return setmetatable(res, getmetatable(t))
end
function table.reduce(t, fn, init)
    local acc = init
    for value in table.it(t) do
        if init then
            acc = fn(acc, value)
        else
            acc = value
            init = true
        end
    end

    return acc
end
function table.any(t, fn)
    for value in table.it(t) do
        if fn(value) then
            return true
        end
    end

    return false
end
function table.all(t, fn)
    for value in table.it(t) do
        if not fn(value) then
            return false
        end
    end

    return true
end
    String functions.
]]
function string.eq(str, strcmp)
    return str == strcmp
end
function string.ieq(str, strcmp)
    return str:lower() == strcmp:lower()
end
function string.map(str, fn)
    return (str:gsub('.', fn))
end
Copyright © 2013-2015, Windower
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Windower nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Windower BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]
