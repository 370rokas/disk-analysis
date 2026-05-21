-- Export all database files from a disk image.
-- Covers SQLite (including WAL/SHM journal files), Access, Realm, and dBASE.
--
-- Usage:
--   disk-analysis <image> script scripts/export_databases.lua "out=/tmp/databases"
--   disk-analysis <image> script scripts/export_databases.lua "out=/tmp/databases,part=27"
--
-- Settings keys:
--   out  — output directory on the host (default: /tmp/databases)
--   part — restrict to a single partition ID (default: all partitions)

local DB_EXTENSIONS = {
    -- SQLite and its journal/WAL files
    ["db"]      = "sqlite",
    ["sqlite"]  = "sqlite",
    ["sqlite3"] = "sqlite",
    ["db3"]     = "sqlite",
    ["db-wal"]  = "sqlite-wal",
    ["db-shm"]  = "sqlite-shm",
    -- Microsoft Access
    ["mdb"]     = "access",
    ["accdb"]   = "access",
    -- Realm
    ["realm"]   = "realm",
    -- dBASE / FoxPro
    ["dbf"]     = "dbase",
}

local function parse_settings(s)
    local cfg = {}
    if not s or s == "" then return cfg end
    for pair in s:gmatch("[^,]+") do
        local k, v = pair:match("^([^=]+)=(.*)$")
        if k then cfg[k] = v end
    end
    return cfg
end

local function mkdir(path)
    if package.config:sub(1, 1) == "\\" then
        os.execute('md "' .. path:gsub("/", "\\") .. '" 2>nul')
    else
        os.execute('mkdir -p "' .. path .. '"')
    end
end

local function parent_dir(path)
    return path:match("^(.+)/[^/]*$")
end

local function file_ext(name)
    -- handle double extensions like .db-wal, .db-shm
    local ext2 = name:match("%.([^%.]+%.[^%.]+)$")
    if ext2 and DB_EXTENSIONS[ext2:lower()] then return ext2:lower() end
    local ext1 = name:match("%.([^%.]+)$")
    return ext1 and ext1:lower() or nil
end

local cfg         = parse_settings(da.settings)
local out         = cfg.out or "/tmp/databases"
local part_filter = cfg.part and tonumber(cfg.part)

mkdir(out)

local exported  = 0
local failed    = 0
local by_type   = {}

local function walk(fs, entry, dest_base)
    if entry:is_link() or not entry:is_valid() then return end
    if entry:is_directory() then
        for _, child in pairs(entry:children()) do
            walk(fs, child, dest_base)
        end
    else
        local ext = file_ext(entry:name())
        local db_type = ext and DB_EXTENSIONS[ext]
        if db_type then
            local dest = dest_base .. entry:full_path()
            local dir  = parent_dir(dest)
            if dir then mkdir(dir) end
            if fs:extract(entry:full_path(), dest) then
                print(string.format("  [%-10s]  %s", db_type, entry:full_path()))
                exported = exported + 1
                by_type[db_type] = (by_type[db_type] or 0) + 1
            else
                print(string.format("  [failed   ]  %s", entry:full_path()))
                failed = failed + 1
            end
        end
    end
end

for _, p in ipairs(da.list_partitions()) do
    if not p.has_filesystem or p.fs_type == "Unknown" then goto continue end
    if part_filter and p.id ~= part_filter then goto continue end

    print(string.format("scanning partition %d (%s, %s)...", p.id, p.name, p.fs_type))
    local ok, fs = pcall(da.open_fs, p.id)
    if not ok then
        print(string.format("  could not open: %s", tostring(fs)))
        goto continue
    end

    local ok2, err2 = pcall(function() fs:root():load_all_descendants() end)
    if not ok2 then
        print(string.format("  failed to walk: %s", tostring(err2)))
        goto continue
    end

    walk(fs, fs:root(), string.format("%s/part%d", out, p.id))
    ::continue::
end

print(string.rep("-", 40))
print(string.format("exported %d database file(s), %d failed", exported, failed))
if exported > 0 then
    print("breakdown:")
    for t, n in pairs(by_type) do
        print(string.format("  %-14s %d", t, n))
    end
end
