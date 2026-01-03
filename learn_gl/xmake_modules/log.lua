---@diagnostic disable: lowercase-global
import("core.base.option")

-- 调试级别控制
local DEBUG_LEVEL = option.get("debug-deps") and tonumber(option.get("debug-deps")) or 0
local LOG_INDENT = option.get("log-indent") and tonumber(option.get("log-indent")) or 2
-- 调试颜色
local COLORS = {
	error = "${bright red}",
	warn = "${bright yellow}",
	info = "${bright green}",
	debug = "${bright cyan}",
	trace = "${bright blue}",
	reset = "${reset}",
}

local LEVEL_TO_COLOR = {
	[1] = "error", -- 错误
	[2] = "warn", -- 警告
	[3] = "info", -- 信息
	[4] = "debug", -- 调试
	[5] = "trace", -- 跟踪
}
-- 默认模块名称
local module_name = "unknown"

--- 设置当前模块名称
--- @param name string 模块名称
function set_debug_module(name)
	module_name = name or "unknown"
	Info(0, "Module name set to: %s", module_name)
end

--- 获取当前模块名称
--- @return string 当前模块名称
function get_debug_module()
	return module_name
end

function get_colors()
	return COLORS
end

--- 调试日志输出（C风格格式化）
--- @param level integer 日志级别 (1-5)
--- @param prefix string 日志前缀说明(1-5)
--- @param format string 格式字符串（使用 %s, %d 等）
--- @param ... any 格式参数
function Vprint(level, prefix, format, ...)
	if DEBUG_LEVEL > 0 then
		if level <= (2 + DEBUG_LEVEL) then
			cprint(COLORS[LEVEL_TO_COLOR[level]] .. prefix .. COLORS.reset .. format, ...)
		end
	end
end

--- @param level integer 缩进级别
function indent_level(level)
	return string.rep(" ", LOG_INDENT * tonumber(level))
end

--- 跟踪日志（C风格格式化）
--- @param level integer 缩进级别
--- @param format string 格式字符串
--- @param ... any 格式参数
function Trace(level, format, ...)
	local str = "[" .. module_name .. "]"
	Vprint(5, "TRACE: " .. string.format("%-15s", str) .. indent_level(level), format, ...)
end

--- 调试日志（C风格格式化）
--- @param level integer 缩进级别
--- @param format string 格式字符串
--- @param ... any 格式参数
function Debug(level, format, ...)
	local str = "[" .. module_name .. "]"
	Vprint(4, "DEBUG: " .. string.format("%-15s", str) .. indent_level(level), format, ...)
end
--- 关键日志（C风格格式化）
--- @param level integer 缩进级别
--- @param format string 格式字符串
--- @param ... any 格式参数
function Info(level, format, ...)
	local str = "[" .. module_name .. "]"
	Vprint(3, "INFO: " .. string.format("%-15s", str) .. indent_level(level), format, ...)
end

--- 警告日志（C风格格式化）
--- @param level integer 缩进级别
--- @param format string 格式字符串
--- @param ... any 格式参数
function Warn(level, format, ...)
	local str = "[" .. module_name .. "]"
	Vprint(2, "WARN: " .. string.format("%-15s", str) .. indent_level(level), format, ...)
end

--- 错误日志（C风格格式化）
--- @param level integer 缩进级别
--- @param format string 格式字符串
--- @param ... any 格式参数
function Error(level, format, ...)
	local str = "[" .. module_name .. "]"
	Vprint(1, "ERROR: " .. string.format("%-15s", str) .. indent_level(level), format, ...)
end

-- 其他辅助函数保持不变...
function set_debug_level(level)
	DEBUG_LEVEL = level or 0
	if option then
		option.set("debug-deps", tostring(DEBUG_LEVEL))
	end
	Info(0, "Debug level set to: %d", level)
end

function get_debug_level()
	return DEBUG_LEVEL
end

function set_log_indent(level)
	LOG_INDENT = level or 2
	if option then
		option.set("log-indent", tostring(LOG_INDENT))
	end
	Info(0, "Log indent set to: %d", level)
end

function set_log_indent()
	return LOG_INDENT
end

function dep_tree(deps, indent)
	indent = indent or 0
	local result = ""
	for i, dep in ipairs(deps) do
		local prefix = string.rep("\t", indent) .. "├── "
		if i == #deps then
			prefix = string.rep("\t", indent) .. "└── "
		end
		result = result .. prefix .. path.basename(dep) .. "\n"
	end
	return result
end

--- 将 table 格式化为字符串
--- @param t any 要格式化的表
--- @param indent_level? integer 缩进级别（可选，默认0）
--- @param prefix_desc? string 前缀说明
--- @return string 格式化后的字符串
function table.format(t, indent_level, prefix_desc)
	indent_level = indent_level or 0
	prefix_desc = prefix_desc or 0
	local indent_str = prefix_desc .. string.rep(" ", LOG_INDENT * indent_level)
	local format_cache = {}

	local function format_value(value, current_indent)
		if type(value) == "table" then
			if format_cache[tostring(value)] then
				return current_indent .. "*" .. tostring(value)
			end

			format_cache[tostring(value)] = true
			local result = current_indent .. tostring(value) .. " {"

			for pos, val in pairs(value) do
				if type(val) == "table" then
					result = result
						.. "\n"
						.. current_indent
						.. "  ["
						.. tostring(pos)
						.. "] => "
						.. tostring(val)
						.. " {"
					result = result .. "\n" .. format_value(val, current_indent .. "    ")
					result = result .. "\n" .. current_indent .. "  }"
				elseif type(val) == "string" then
					result = result
						.. "\n"
						.. current_indent
						.. "  ["
						.. tostring(pos)
						.. '] => "'
						.. val:gsub("\n", "\\n"):gsub("\r", "\\r")
						.. '"'
				else
					result = result .. "\n" .. current_indent .. "  [" .. tostring(pos) .. "] => " .. tostring(val)
				end
			end

			result = result .. "\n" .. current_indent .. "}"
			return result
		elseif type(value) == "string" then
			return current_indent .. '"' .. value:gsub("\n", "\\n"):gsub("\r", "\\r") .. '"'
		else
			return current_indent .. tostring(value)
		end
	end

	if type(t) == "table" then
		return format_value(t, indent_str)
	else
		return tostring(t)
	end
end

--- 打印 table（调用 table.format 生成字符串后打印）
--- @param t any 要打印的表
--- @param indent_level? integer 缩进级别（可选，默认0）
function table.print(t, indent_level, prefix_desc)
	indent_level = indent_level or 0
	prefix_desc = prefix_desc or 0
	local formatted = table.format(t, indent_level, prefix_desc)
	cprint(formatted)
end

function string.prefix_lines(str, prefix)
	if not str or str == "" then
		return str
	end
	-- 处理行首和换行符后的位置
	return prefix .. str:gsub("\n", "\n" .. prefix)
end
