# HARSH - hardly a real shell

A misguided try to replace common shell wrapper scripts with lua.

Nowhere near complete shell functionality.

Status: DO NOT USE

## Installation

What this needs: a C++ 20x compiler and some lua dev package.
Probably `lua53` on Arch, probably `liblua5.3-0` on Ubuntu 20.04.

```
make
cp bin/harsh /usr/local/bin/
cp lib/harshlib.lua /usr/local/share/harshlib.lua
```

Anywhere in `PATH` is fine of course and you can change the harsh "stdlib"
path via a define in `main.cpp`.

It doesn't have interactive mode, so the only way to use it is via shebang or
calling it with a file as the first argument.

## Implemented shell "stubs"

  * `dirname()`
  * `basename()`
  * `sleep()` + `msleep()`
  * `mkdir()` (implicit `-p`)

## Implemented functions as replacements

  * `abspath()` for `readlink -f`
  * `E()` for `getenv()` / env vars like `$HOME`
  * `argv(i)` for `$0 $1 ...`
  * `empty(s)` for `"$0" = ""` checks
  * `argv_at()` for `$@`
  * `execv()` for "exec /path/binary" (buggy, can't do quoted arguments)

## Some globals with special meaning

  * `env.PORT = 3000` instead of `export PORT=3000`
  * `V.VAR = "hello"` instead of `VAR="hello"` in `run()`

## Other useful stuff

  * `run(cmd)` to execute with dumb variable expansion
  * `run_raw(cmd)` to execute
  * `slurp(filename)` reads a file
  * `spit(filename, "data")` writes to a file

## Examples

### variable expansion in `run()`

```
#!/usr/bin/env harsh
V.IMAGE = image
local cmd = [[
docker run --rm ${IMAGE} date
]]
print(run(cmd))
```

### pipes

```
V.tokenFile = "/tmp/token"
local curl = run([[
    echo '{"token":"foo","b":3}'
]])
local jq = curl:pipe([[ jq -r .token ]])
spit(V.tokenFile, jq)
-- $ cat /tmp/token yields "foo"
```

### Known limitations

  * `execv()` just splits on whitespace, so `execv([[ echo "hello world" ]])` won't work
  * `run()` might also break on quoting, due to lack of a proper parser
  * arguments with `%` in it have a funky workaround, e.g. `date +%s`
  * all the debug output is still in