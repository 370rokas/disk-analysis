-- Report the N largest files found across partitions (or a single partition).
--
-- Usage:
--   disk-analysis <image> script scripts/largest_files.lua
--   disk-analysis <image> script scripts/largest_files.lua "n=50"
--   disk-analysis <image> script scripts/largest_files.lua "part=1,n=20"
--
-- Settings keys:
--   n    — number of files to show (default: 20)
--   part — restrict to a single partition ID (default: all partitions)
--
-- Output goes to stdout; redirect to a file with shell redirection if needed.

local function parse_settings(s)
    local cfg = {}
    if not s or s == "" then return cfg end
    for pair in s:gmatch("[^,]+") do
        local k, v = pair:match("^([^=]+)=(.*)$")
        if k then cfg[k] = v end
    end
    return cfg
end

local function human(bytes)
    local units = { "B", "KiB", "MiB", "GiB", "TiB" }
    local v, i  = bytes, 1
    while v >= 1024 and i < #units do
        v = v / 1024
        i = i + 1
    end
    if i == 1 then
        return string.format("%d B", v)
    end
    return string.format("%.1f %s", v, units[i])
end

local cfg         = parse_settings(da.settings)
local n           = tonumber(cfg.n) or 20
local part_filter = cfg.part and tonumber(cfg.part)

local files       = {}
local total_files = 0

local function walk(entry, part_id)
    if entry:is_link() or not entry:is_valid() then return end
    if entry:is_directory() then
        for _, child in pairs(entry:children()) do walk(child, part_id) end
    else
        total_files = total_files + 1
        files[#files + 1] = {
            path = string.format("[part%d]%s", part_id, entry:full_path()),
            size = entry:size(),
        }
    end
end

for _, p in ipairs(da.list_partitions()) do
    if not p.has_filesystem or p.fs_type == "Unknown" then goto continue end
    if part_filter and p.id ~= part_filter then goto continue end

    da.log_info(string.format("scanning partition %d (%s)", p.id, p.fs_type))
    local ok, fs = pcall(da.open_fs, p.id)
    if not ok then
        da.log_warn("could not open partition " .. p.id .. ": " .. tostring(fs))
        goto continue
    end

    local root = fs:root()
    local ok2, err2 = pcall(function() root:load_all_descendants() end)
    if not ok2 then
        print(string.format("  warning: partition %d failed to load: %s", p.id, tostring(err2)))
        goto continue
    end
    walk(root, p.id)
    print(string.format("  partition %d: %d file(s) so far", p.id, total_files))
    ::continue::
end

table.sort(files, function(a, b) return a.size > b.size end)

local show = math.min(n, #files)
local col1 = 10

print(string.format("%-" .. col1 .. "s  %s", "SIZE", "PATH"))
print(string.rep("-", 80))
for i = 1, show do
    local f = files[i]
    print(string.format("%-" .. col1 .. "s  %s", human(f.size), f.path))
end
print(string.format("\n%d file(s) scanned, showing top %d", total_files, show))
