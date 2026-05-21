-- Recursively export a directory from a partition to the host filesystem.
-- The source directory structure is preserved under the output directory.
--
-- Usage:
--   disk-analysis <image> script scripts/export_directory.lua "part=1,src=/etc,out=/tmp/export"
--   disk-analysis <image> script scripts/export_directory.lua "part=2,src=/home/user,out=/tmp/user_home"
--
-- Settings keys:
--   part — partition ID to open (required)
--   src  — absolute path of the directory to export (required)
--   out  — output directory on the host (default: /tmp/export)

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

local cfg      = parse_settings(da.settings)
local part_id  = tonumber(cfg.part) or error("settings must include part=<id>")
local src_dir  = cfg.src            or error("settings must include src=<path>")
local out      = cfg.out            or "/tmp/export"

local ok, fs = pcall(da.open_fs, part_id)
if not ok then error("could not open partition " .. part_id .. ": " .. tostring(fs)) end

-- Walk the root to reach the target directory entry step by step.
local function navigate(root, path)
    if path == "/" or path == "" then return root end
    local entry = root
    for seg in path:gmatch("[^/]+") do
        local children = entry:children()
        if not children[seg] then
            return nil, "path not found in filesystem: " .. path
        end
        entry = children[seg]
    end
    return entry
end

local root = fs:root()
local target, err = navigate(root, src_dir)
if not target then error(err) end
if not target:is_directory() then error(src_dir .. " is not a directory") end

target:load_all_descendants()
mkdir(out)

-- Strip the source prefix so exported paths are relative to out.
local prefix = src_dir:gsub("/$", "")

local exported, failed = 0, 0

local function walk(entry)
    if entry:is_link() or not entry:is_valid() then return end
    if entry:is_directory() then
        for _, child in pairs(entry:children()) do walk(child) end
    else
        local rel  = entry:full_path():sub(#prefix + 1)
        local dest = out .. rel
        local dir  = parent_dir(dest)
        if dir then mkdir(dir) end
        if fs:extract(entry:full_path(), dest) then
            da.log_info("exported  " .. entry:full_path())
            exported = exported + 1
        else
            da.log_warn("failed    " .. entry:full_path())
            failed = failed + 1
        end
    end
end

walk(target)
print(string.format("done — exported %d, failed %d", exported, failed))
