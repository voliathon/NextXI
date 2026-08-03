    A few table helper functions, in addition to a new T-table interface, which enables method indexing on tables.

    To define a T-table with explicit values use T{...}, to convert an existing table t, use T(t). To access table methods of a T-table t, use t:methodname(args).

    For lists, tables with sequential integral indices, use the lists library and the respective L{...} constructor. For sets, tables with unique elements and irrelevant order, use the sets library and the respective S{...} constructor.
]]

_libs = _libs or {}

require('maths')
require('functions')

local math, functions = _libs.maths, _libs.functions

local table = require('table')

_libs.tables = table

_raw = _raw or {}
_raw.table = setmetatable(_raw.table or {}, {__index = table})
    Signatures
]]

_meta = _meta or {}
_meta.T = _meta.T or {}
_meta.T.__index = table
_meta.T.__class = 'Table'

_meta.N = {}
_meta.N.__class = 'nil'
function T(t)
    local res
    if class(t) == 'Set' then
        res = T{}

        local key = 1
        for el in pairs(t) do
            if type(el) == 'table' then
                res[key] = table.copy(el)
            else
                res[key] = el
            end
            key = key + 1
        end
    elseif class(t) == 'List' then
        res = T{}

        local key = 1
        for _, el in ipairs(t) do
            if type(el) == 'table' then
                res[key] = table.copy(el)
            else
                res[key] = el
            end
            key = key + 1
        end
    else
        res = t or {}
    end
    return setmetatable(res, _meta.T)
end

N = (function()
    local nt = setmetatable({}, _meta.N)
    return function()
        return nt
    end
end)()

function class(o)
    local mt = getmetatable(o)

    return mt and mt.__class or type(o)
end
function table.fn(t)
    return functions.const(t)
end
function table.isarray(t)
    return table.length(t) == #t
end
function table.length(t)
    local count = 0
    for _ in table.it(t) do
        count = count + 1
    end

    return count
end
function table.first(t, n)
    n = n or 1
    return t[n]
end
function table.last(t, n)
    n = n or 1
    n = n - 1
    return t[#t-n]
end
function table.contains(t, searchval)
    for val in table.it(t) do
        if val == searchval then
            return true
        end
    end

    return false
end
function table.containskey(t, searchkey)
    return rawget(t, searchkey) ~= nil
end
function table.append(t, val)
    t[#t+1] = val
    return t;
end
function table.extend(t, t_extend)
    if type(t_extend) ~= 'table' then
        return table.append(t, t_extend)
    end

    for val in table.it(t_extend) do
        table.append(t, val)
    end

    return t
end

_meta.T.__add = table.extend
function table.count(t, fn)
    if type(fn) ~= 'function' then
        fn = functions.equals(fn)
    end

    local count = 0
    for val in table.it(t) do
        if fn(val) then
            count = count + 1
        end
    end

    return count
end
function table.clear(t)
    for key in pairs(t) do
        rawset(t, key, nil)
    end

    return t
end
function table.update(t, t_update, recursive, maxrec, rec)
    if t_update == nil then
        return t
    end

    recursive = recursive or false
    maxrec = maxrec or -1
    rec = rec or 0

    for key, val in pairs(t_update) do
        if t[key] ~= nil and recursive and rec ~= maxrec and type(t[key]) == 'table' and type(val) == 'table' and not table.isarray(val) then
            t[key] = table.update(t[key], val, true, maxrec, rec + 1)
        else
            t[key] = val
        end
    end

    return t
end
function table.amend(t, t_amend, recursive, maxrec, rec)
    if t_amend == nil then
        return t
    end

    recursive = recursive or false
    maxrec = maxrec or -1
    rec = rec or 0

    local cmp
    for key, val in pairs(t_amend) do
        if t[key] ~= nil and recursive and rec ~= maxrec and type(t[key]) == 'table' and type(val) == 'table' and class(val) ~= 'List' and class(val) ~= 'Set' and not table.isarray(val) then
            t[key] = table.amend(t[key], val, true, maxrec, rec + 1)
        elseif t[key] == nil then
            t[key] = val
        end
    end

    return t
end
function table.find(t, fn)
    fn = type(fn) ~= 'function' and functions.equals(fn) or fn

    for key, val in pairs(t) do
        if fn(val) then
            return key, val
        end
    end
end
function table.keyset(t)
    local res = {}
    if _libs.sets then
        for key in pairs(t) do
            res[key] = true
        end

        return setmetatable(res, _meta.S)
    end

    local i = 0
    for key in pairs(t) do
        i = i + 1
        res[i] = key
    end

    if _libs.lists then
        res.n = i
    end

    return setmetatable(res, _libs.lists and _meta.L or _meta.T)
end
function table.flatten(t, recursive)
    recursive = true and (recursive ~= false)

    local res = {}
    local key = 1
    local flat = {}
    for _, val in ipairs(t) do
        if type(val) == 'table' then
            if recursive then
                flat = table.flatten(val, recursive)
                table.extend(res, flat)
                key = key + #flat
            else
                table.extend(res, val)
                key = key + #val
            end
        else
            res[key] = val
            key = key + 1
        end
    end

    return T(res)
end
function table.equals(t, t_eq, depth)
    depth = depth or -1
    if depth == 0 then
        return t == t_eq
    end
    if class(t) ~= class(t_eq) then
        return false
    end

    local seen = {}

    for key, val in pairs(t) do
        local cval = rawget(t_eq, key)
        if val ~= cval then
            if type(val) == 'table' and class(val) == class(cval) then
                if not table.equals(val, cval, depth - 1) then
                    return false
                end
            else
                return false
            end
        end
        seen[key] = true
    end

    for key, val in pairs(t_eq) do
        if not seen[key] then
            return false
        end
    end

    return true
end
function table.delete(t, el)
    for key, val in pairs(t) do
        if val == el then
            if type(key) == 'number' then
                return table.remove(t, key)
            else
                local ret = t[key]
                t[key] = nil
                return ret
            end
        end
    end
end
function table.keyfind(t, fn)
    for key, val in pairs(t) do
        if fn(key) then
            return key, val
        end
    end
end
function table.slice(t, from, to)
    local n  = #t

    from = from or 1
    if from < 0 then
        from = (from % n) + 1
    end
    to = to or n
    if to < 0 then
        to = (to % n) + 1
    end
    local res = {}
    local key = 1
    for i = from, to do
        res[key] = t[i]
        key = key + 1
    end

    return setmetatable(res, getmetatable(t))
end
function table.splice(t, from, to, st)
    local n1 = #t
    local n2 = #st
    local tcpy = table.copy(t)

    for stkey = 1, n2 do
        tkey = from + stkey - 1
        t[tkey] = st[stkey]
    end

    for cpykey = to + 1, n1 do
        newkey = cpykey + n2 - (to - from) - 1
        t[newkey] = tcpy[cpykey]
    end

    local nn = #t
    for rmkey = nn - (to - from) + n2, nn do
        t[rmkey] = nil
    end

    t = res

    return t
end
function table.reverse(t)
    local res = {}

    local n = #t
    local rkey = n
    for key = 1, n do
        res[key] = t[rkey]
        rkey = rkey - 1
    end

    return setmetatable(res, getmetatable(t))
end
function table.dict(...)
    local res = type(...) == 'table' and ... or {}

    local start = type(...) == 'table' and 2 or 1
    for k = start, select('#', ...), 2 do
        res[select(k, ...)] = select(k + 1, ...)
    end

    return setmetatable(res, _meta.T)
end
function table.with(t, attr, val)
    val = type(val) ~= 'function' and functions.equals(val) or val
    for key, el in pairs(t) do
        if type(el) == 'table' and val(el[attr]) then
            return el, key
        end
    end

    return nil, nil
end
_raw.table.sort = _raw.table.sort or table.sort
function table.sort(t, ...)
    _raw.table.sort(t, ...)
    return t
end
function table.rekey(t, key)
    local res = {}

    for value in table.it(t) do
        res[value[key]] = value
    end

    return setmetatable(res, getmetatable(t))
end
function table.unpack(t, ...)
    local count = select('#', ...);
    if count == 0 then
        return unpack(t)
    end

    local temp = {}
    local args = {...}
    for i = 1, count do
        temp[i] = t[args[i]]
    end

    return unpack(temp)
end

function table.pack(...)
    return {...}
end
function table.extract(t)
    local res = {}
    local i = 1
    for value in table.it(t) do
        res[i] = value
        i = i + 1
    end

    return table.unpack(res)
end
function table.copy(t, deep)
    deep = deep ~= false and true
    local res = {}

    for value, key in table.it(t) do
        if type(value) == 'table' and deep then
            value = (not rawget(value, 'copy') and value.copy or table.copy)(value)
        end
        res[key] = value
    end

    return setmetatable(res, getmetatable(t))
end
function table.reassign(t, tn)
    return table.update(table.clear(t), tn)
end
function table.range(start, finish, step)
    if finish == nil then
        start, finish = 1, start
    end

    step = step or 1

    local res = {}
    for key = start, finish, step do
        res[key] = key
    end

    return setmetatable(res, _meta.T)
end
function table.chunks(t, size)
    return table.range(math.ceil(t:length()/size)):map(function(i) return t:slice(size*(i - 1) + 1, size*i) end)
end
_raw.table.concat = table.concat
function table.concat(t, delim, from, to)
    delim = delim or ''
    local res = ''

    if from or to then
        from = from or 1
        to = to or #t
        for key = from, to do
            local val = rawget(t, key)
            res = res .. tostring(val)
            if key < to then
                res = res .. delim
            end
        end
    else
        for value, key in table.it(t) do
            res = res .. tostring(value)
            if next(t, key) then
                res = res .. delim
            end
        end
    end

    return res
end
function table.sconcat(t, ...)
    return table.concat(t, ' ', ...)
end
function table.empty(t, rec)
    if not rec then
        return next(t) == nil
    end

    for val in table.it(t) do
        if type(val) ~= 'table' then
            return false;
        else
            if not table.empty(val, true) then
                return false;
            end
        end
    end

    return true
end
function table.sum(t)
    return table.reduce(t, math.add, 0)
end
function table.mult(t)
    return table.reduce(t, math.mult, 1)
end
function table.min(t)
    return table.reduce(t, math.min)
end
function table.max(t)
    return table.reduce(t, math.max)
end
Copyright © 2013-2015, Windower
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Windower nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Windower BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]
