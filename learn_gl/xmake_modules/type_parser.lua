-- xmake_modules/type_parser.lua
import("core.base.option")
import("log")

local TYPE_VALUES = {
	Target = "binary",
	Static = "static",
	Dynamic = "shared",
	Null = "null",
}

function parse_type(srcfile, options)
	log.set_debug_module("type_parser")
	options = options or {}
	local start_marker = options.start_marker or "xmake-type:"

	log.Info(1, "Parsing type from: %s", srcfile)

	if not os.isfile(srcfile) then
		log.Warn(0, "File not found: %s", srcfile)
		return "Target"
	end

	local content = io.readfile(srcfile)
	if not content then
		log.Warn(0, "Failed to read file: %s", srcfile)
		return "Target"
	end

	-- 支持注释中的标记
	local pattern = "[%s///*]*%s*" .. start_marker:gsub("%p", "%%%0") .. "%s*([%w_]+)"
	local type_value = content:match(pattern)

	if not type_value then
		log.Debug(1, "No type marker found in %s", srcfile)
		return "Target"
	end

	type_value = type_value:gsub("^%s+", ""):gsub("%s+$", "")
	if not TYPE_VALUES[type_value] then
		log.Warn(0, "Invalid type '%s' in %s, defaulting to Target", type_value, srcfile)
		return "Target"
	end

	log.Info(0, "✓ Found type '%s' in %s", type_value, srcfile)
	return type_value
end
