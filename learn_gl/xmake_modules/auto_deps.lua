--! @diagnostic disable: undefined-global, lowercase-global
--! @module auto_deps
--- 自动扫描和分析项目中的自定义头文件
--! @author YourName
--! @license MIT
import("core.base.option")
import("log")

--- 标准化路径分隔符
--! @param path string 路径
--! @return string 标准化后的路径
function _normalize_path(path)
	return path:gsub("\\", "/")
end

--- 匹配自定义头文件模式
--! @param header_path string 头文件路径
--! @param pattern any 模式（字符串、正则表达式或函数）
--! @return boolean 是否匹配
function _match_custom_header(header_path, pattern)
	-- 标准化路径分隔符
	local normalized_path = header_path:gsub("\\", "/")

	log.Debug(1, "Matching header: %s against pattern: %s", normalized_path, tostring(pattern))

	-- 处理 table 类型的模式（支持多个模式）
	if type(pattern) == "table" then
		log.Debug(2, "Pattern type: table (multiple patterns)")
		for i, sub_pattern in ipairs(pattern) do
			log.Debug(3, "Sub-pattern %d/%d: %s", i, #pattern, tostring(sub_pattern))
			if _match_custom_header(header_path, sub_pattern) then
				log.Debug(3, "✓ Table pattern matched at index %d", i)
				return true
			end
		end
		log.Debug(3, "✗ No sub-pattern matched")
		return false
	end

	-- 模式可以是字符串或函数
	if type(pattern) == "string" then
		log.Debug(3, "Pattern type: string")

		-- 支持多种匹配方式：
		-- 1. 完整路径匹配
		-- 2. 目录名匹配（包含在路径中）
		-- 3. 正则表达式匹配（以^开头）

		if pattern:sub(1, 1) == "^" then
			-- 正则表达式匹配
			log.Debug(3, "Using regex pattern: %s", pattern)
			local match_result = normalized_path:match(pattern)
			if match_result then
				log.Debug(3, "✓ Regex match successful: %s matches %s", normalized_path, pattern)
				return true
			else
				log.Debug(3, "✗ Regex match failed: %s does not match %s", normalized_path, pattern)
				return false
			end
		else
			-- 目录名匹配（检查路径中是否包含该目录名）
			log.Debug(3, "Using directory pattern: %s", pattern)
			local dir_name = pattern:gsub("/", "")
			log.Debug(3, "Normalized directory name: %s", dir_name)

			-- 检查所有可能的匹配情况
			local match_found = false
			local match_type = ""

			if normalized_path:find("/" .. dir_name .. "/", 1, true) ~= nil then
				match_found = true
				match_type = "directory in middle"
			elseif normalized_path:find("^" .. dir_name .. "/", 1, true) ~= nil then
				match_found = true
				match_type = "directory at start"
			elseif normalized_path:find("/" .. dir_name .. "$", 1, true) ~= nil then
				match_found = true
				match_type = "directory at end"
			elseif normalized_path == dir_name then
				match_found = true
				match_type = "exact match"
			end

			if match_found then
				log.Debug(
					3,
					"✓ Directory pattern matched (%s): %s contains '%s'",
					match_type,
					normalized_path,
					dir_name
				)
				return true
			else
				log.Debug(3, "✗ Directory pattern failed: %s does not contain '%s'", normalized_path, dir_name)
				log.Trace(4, "Checked patterns:")
				log.Trace(
					5,
					"- '/%s/' -> %s",
					dir_name,
					tostring(normalized_path:find("/" .. dir_name .. "/", 1, true) ~= nil)
				)
				log.Trace(
					5,
					"- '^%s/' -> %s",
					dir_name,
					tostring(normalized_path:find("^" .. dir_name .. "/", 1, true) ~= nil)
				)
				log.Trace(
					5,
					"- '/%s$' -> %s",
					dir_name,
					tostring(normalized_path:find("/" .. dir_name .. "$", 1, true) ~= nil)
				)
				log.Trace(5, "- exact match -> %s", tostring(normalized_path == dir_name))
				return false
			end
		end
	elseif type(pattern) == "function" then
		log.Debug(2, "Pattern type: function")
		log.Debug(3, "Calling custom match function for: %s", normalized_path)
		local result = pattern(header_path)
		if result then
			log.Debug(3, "✓ Function pattern returned true for: %s", normalized_path)
		else
			log.Debug(3, "✗ Function pattern returned false for: %s", normalized_path)
		end
		return result
	else
		log.Warn(1, "Unsupported pattern type: %s (expected string, function, or table)", type(pattern))
		return false
	end
end

--- 解析 #include 语句
--! @param content string 文件内容
--! @return table<string> 包含的头文件列表
function _extract_includes(content)
	local includes = {}
	for line in content:gmatch("[^\r\n]+") do
		-- 匹配 #include "header.h"
		local match = line:match('#%s*include%s*["]%s*([^"]+)')
		if match then
			table.insert(includes, match)
		end
	end
	return includes
end

--- 解析头文件路径
--! @param header string 头文件名
--! @param srcfile string 源文件路径
--! @param options table 选项
--! @return string|nil 解析后的头文件路径
function _resolve_header_path(header, srcfile, options)
	options = options or {}
	local search_paths = options.search_paths or {}
	local ignore_paths = options.ignore_paths or {}

	-- 添加源文件所在目录到搜索路径
	table.insert(search_paths, 1, path.directory(srcfile))

	-- 标准化忽略路径
	local ignore_patterns = {}
	for _, ignore in ipairs(ignore_paths) do
		if type(ignore) == "string" then
			table.insert(ignore_patterns, ignore:gsub("\\", "/"))
		end
	end

	-- 尝试在每个搜索路径中查找
	for _, dir in ipairs(search_paths) do
		local fullpath = path.join(dir, header)
		local normalized_path = _normalize_path(fullpath)

		-- 检查是否在忽略路径中
		local should_ignore = false
		for _, ignore_pattern in ipairs(ignore_patterns) do
			if normalized_path:find(ignore_pattern, 1, true) then
				should_ignore = true
				break
			end
		end

		if should_ignore then
			log.Debug(1, "✗ Ignored (matches ignore pattern): %s", normalized_path)
			goto continue
		end

		if os.isfile(fullpath) then
			log.Debug(1, "✓ Found header: %s -> %s", header, normalized_path)
			return path.absolute(fullpath)
		end

		::continue::
	end

	-- 尝试项目根目录
	local project_header = path.join(os.projectdir(), header)
	if os.isfile(project_header) then
		return path.absolute(project_header)
	end

	return nil
end

--- 递归扫描自定义头文件
--! @param srcfile string 源文件路径
--! @param rules table 扫描规则
--! @param options table 选项
--! @param visited table 已访问的头文件（避免循环依赖）
--! @param current_depth integer 当前递归深度
--! @return table<string> 自定义头文件路径列表
function _scan_recursively(srcfile, rules, options, visited, current_depth)
	current_depth = current_depth or 1
	local max_depth = options.max_depth or 5
	visited = visited or {}

	-- 检查递归深度
	if current_depth > max_depth then
		log.Info(0, "✓ Maximum recursion depth reached (%d) for %s", max_depth, srcfile)
		return {}
	end

	-- 检查是否已访问
	local abs_srcfile = path.absolute(srcfile)
	if visited[abs_srcfile] then
		log.Info(0, "✓ Already visited: %s (skipping to avoid cycle)", abs_srcfile)
		return {}
	end
	visited[abs_srcfile] = true

	local content = io.readfile(srcfile)
	if not content then
		log.Warn(0, "Failed to read file: %s", srcfile)
		return {}
	end

	local custom_headers = {}
	local includes = _extract_includes(content)
	log.Info(0, "Found %d include statements in %s", #includes, srcfile)

	for i, header in ipairs(includes) do
		log.Info(0, "Processing include %d/%d: %s", i, #includes, header)

		-- 尝试解析头文件路径
		local header_path = _resolve_header_path(header, srcfile, options)
		if not header_path or not os.isfile(header_path) then
			log.Info(0, "✗ Header not found: %s", header)
			goto continue
		end

		-- 检查是否匹配任何自定义规则
		local is_custom = false
		for rule_idx, rule in ipairs(rules) do
			log.Debug(1, "Checking rule %d/%d", rule_idx, #rules)

			for pattern_idx, pattern in ipairs(rule.custom_headers) do
				log.Debug(2, "Pattern %d/%d: %s", pattern_idx, #rule.custom_headers, tostring(pattern))

				if _match_custom_header(header_path, pattern) then
					log.Debug(2, "✓ Pattern matched for: %s", header_path)
					is_custom = true
					break
				end
			end

			if is_custom then
				break
			end
		end

		if is_custom then
			-- 添加到自定义头文件列表
			table.insert(custom_headers, header_path)
			log.Info(0, "✓ Found custom header: %s", header_path)

			-- 递归扫描该头文件
			local sub_headers = _scan_recursively(header_path, rules, options, visited, current_depth + 1)
			for _, sub_header in ipairs(sub_headers) do
				if not table.contains(custom_headers, sub_header) then
					table.insert(custom_headers, sub_header)
				end
			end
		else
			log.Info(0, "✗ Header does not match custom patterns: %s", header_path)
		end

		::continue::
	end

	return custom_headers
end

--- 扫描源文件中的自定义头文件
--! @param srcfile string 源文件路径
--! @param rules table 扫描规则，格式：{
--!     { custom_headers = { "pattern1", "pattern2", function() ... end }, search_paths = { "path1", "path2" }, ignore_paths = { "ignore1" } }
--! }
--! @param options table 选项：{recursive=true, max_depth=5, search_paths={}, ignore_paths={}}
--! @return table<string> 匹配的自定义头文件路径列表
function scan(srcfile, rules, options)
	log.set_debug_module("auto_deps")
	options = options or {}
	local recursive = options.recursive ~= false

	log.Info(0, "=== Starting header scan for: %s ===", srcfile)
	log.Debug(0, "Recursive mode: %s", tostring(recursive))

	-- 验证参数
	if not srcfile or not os.isfile(srcfile) then
		log.Error(0, "Invalid source file: %s", tostring(srcfile))
		return {}
	end

	if not rules or #rules == 0 then
		log.Warn(0, "No rules provided for scanning %s", srcfile)
		return {}
	end

	-- 扫描自定义头文件
	local custom_headers = {}

	if recursive then
		custom_headers = _scan_recursively(srcfile, rules, options, {})
	else
		-- 非递归模式：只扫描直接包含的头文件
		local content = io.readfile(srcfile)
		if not content then
			log.Warn(0, "Failed to read file: %s", srcfile)
			return {}
		end

		local includes = _extract_includes(content)
		log.Info(0, "Found %d local includes in %s", #includes, srcfile)

		for i, header in ipairs(includes) do
			log.Info(0, "Processing include %d/%d: %s", i, #includes, header)

			local header_path = _resolve_header_path(header, srcfile, options)
			if not header_path or not os.isfile(header_path) then
				log.Info(0, "✗ Header not found: %s", header)
				goto continue
			end

			-- 检查是否匹配任何自定义规则
			local is_custom = false
			for _, rule in ipairs(rules) do
				for _, pattern in ipairs(rule.custom_headers) do
					if _match_custom_header(header_path, pattern) then
						is_custom = true
						break
					end
				end
				if is_custom then
					break
				end
			end

			if is_custom then
				table.insert(custom_headers, header_path)
				log.Info(0, "✓ Found custom header: %s", header_path)
			end

			::continue::
		end
	end

	-- 去重
	local unique_headers = {}
	local seen = {}
	for _, header in ipairs(custom_headers) do
		if not seen[header] then
			table.insert(unique_headers, header)
			seen[header] = true
		end
	end

	log.Info(0, "=== Scan completed (%d custom headers) ===", #unique_headers)
	return unique_headers
end
--- Scan source file and collect both dependencies and type information
--! @param srcfile string Source file path
--! @param rules table Scan rules
--! @param options table Options
--! @return table<string>, string Custom headers list and file type
function scan_with_type(srcfile, rules, options)
	log.set_debug_module("auto_deps")
	options = options or {}

	log.Info(0, "=== Starting header scan with type detection for: %s ===", srcfile)

	-- First scan for dependencies
	local custom_headers = scan(srcfile, rules, options)

	-- Then parse the type
	import("type_parser")
	local file_type = type_parser.parse_type(srcfile, {
		start_marker = "xmake-type:",
		end_marker = "xmake-type-end",
	})

	log.Info(0, "=== Scan completed with type '%s' (%d custom headers) ===", file_type, #custom_headers)

	return custom_headers, file_type
end
