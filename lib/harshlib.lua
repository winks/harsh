-- internal functions

local L = {}
local V = {}
local env = {}

L.MAX_ARGV_LENGTH = 100
L.ERR_PREFIX = '#O# '

L.err = function(s)
    io.stderr:write(s .. "\n")
end

L.err2 = function(s)
    L.err(L.ERR_PREFIX .. s)
end

L.lib_get_keys = function(t)
    local keys = {}
    for key,_ in pairs(t) do
        table.insert(keys, key)
    end
    return keys
end

L.trim = function(s)
    return s:match'^()%s*$' and '' or s:match'^%s*(.*%S)'
end

--

local function pp(s)
    if s == nil or s == '' then
        print(s)
        return
    end
    L.err2("[[")
    print(s)
    L.err2("]]")
end


local function empty(v)
    if type(v) == "table" then
        return false
    end
    if v == nil or v == '' then
        return true
    end
    return false
end

local function argv_at()
    local t = {}
    for i=1,L.MAX_ARGV_LENGTH do
        local v = argv(i)
        if v == nil then
            return table.concat(t, " ")
        end
        table.insert(t, v)
    end
    return table.concat(t, " ")
end

local function E(var)
    return os.getenv(var)
end

local function plusOne(i)
    return i + 1
end

local function run_raw(cmd)
    local env_keys = L.lib_get_keys(env)
    L.err2(#env_keys .. " " .. table.concat(env_keys, ","))
    local cc = cmd
    if #env_keys > 0 then
        for k,v in pairs(env) do
            local vv = type(v) == "number" and v or "\"" .. string.gsub(v, '"', '\\"') .. "\""
            vv = type(v) == "nil" and "" or vv
            cc = k .. "=" .. vv .. " " .. cc
        end
    end
    L.err2("running:\n" .. L.ERR_PREFIX .. cc .. "")
    local handle = io.popen(cc)
    local result = handle:read("*a")
    handle:close()
    return result
end

local function run(cmd)
    local v_keys = L.lib_get_keys(V)
    -- @FIXME
    local placeholder = "€__€__€"
    cmd = string.gsub(cmd, "%%", placeholder)
    --pp(cmd)
    allargs = string.gsub(argv_at(), "%%", placeholder)
    --pp(allargs)
    -- end @FIXME

    for k,_ in pairs(V) do
        cmd = string.gsub(cmd, "${"..k.."}", V[k])
    end
    cmd = string.gsub(cmd, "$@", allargs)
    -- @FIXME
    cmd = string.gsub(cmd, placeholder, "%%")
    L.err2(cmd)
    --return cmd
    return run_raw(cmd)
end

string.pipe = function(s, next)
    V.input = L.trim(s)
    V.next = next
    return run([[ echo '${input}' | ${next} ]])
end


env.TEST = 23
env.HOME = E("HOME")
L.err2(run("date"))
L.err2("##################################")