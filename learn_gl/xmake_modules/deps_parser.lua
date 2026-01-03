--! @diagnostic disable: undefined-global, lowercase-global
--! @module deps_parser
--- 解析源文件中的 xmake-deps 注释，支持键值对语法和多类型依赖
--! @author YourName
--! @license MIT
import("core.base.option")
import("core.project.config")
import("deps_cache") -- 导入缓存模块
import("log")

--- 修剪字符串两端空格
--! @param str string 输入字符串
--! @return string 修剪后的字符串
function _trim(str)
	if not str then
		return ""
	end
	return str:gsub("^%s+", ""):gsub("%s+$", "")
end

--- 从字符串中提取带引号的内容
--! @param str string 输入字符串
--! @return string 提取的内容（不带引号）
function _extract_quoted_content(str)
	if not str then
		return ""
	end
	str = _trim(str)
	if str:sub(1, 1) == '"' and str:sub(-1) == '"' then
		return str:sub(2, -2)
	end
	if str:sub(1, 1) == "'" and str:sub(-1) == "'" then
		return str:sub(2, -2)
	end
	return str
end

--- 解析键值对格式的块内容
--! @param block_content string 块内容
--! @return table 解析结果
function _parse_kv_block(block_content)
	local result = {
		type = "file", -- default type
		paths = {},
		files = {},
		libs = {},
		lib_paths = {},
	}

	log.Trace(0, "Parsing KV block content: '%s'", block_content)

	-- 尝试提取所有键值对
	for key, value in block_content:gmatch("(%w+)%s*=%s*({[^}]+})") do
		key = _trim(key):lower()
		log.Trace(0, "Found KV pair: %s = %s", key, value)

		if key == "type" then
			-- 提取类型值
			local type_value = value:match('"([^"]+)"') or value:match("'([^']+)'") or value
			type_value = _trim(type_value):lower()
			if type_value == "file" or type_value == "static" or type_value == "dynamic" then
				result.type = type_value
				log.Trace(0, "✓ Set type to: %s", type_value)
			else
				log.Warn(0, "Invalid type value: %s, defaulting to 'file'", type_value)
			end
		elseif key == "path" then
			-- 提取路径列表
			for path in value:gmatch('"([^"]+)"') do
				table.insert(result.paths, _trim(path))
				log.Trace(0, "✓ Added path: %s", path)
			end
		elseif key == "file" then
			-- 提取文件列表
			local file_groups = {}
			-- 处理多维数组：{{"file1"}, {"file2"}}
			for inner_array in value:gmatch("{([^}]*)}") do
				local files = {}
				for file in inner_array:gmatch('"([^"]+)"') do
					table.insert(files, _trim(file))
					log.Trace(0, "✓ Added file: %s", file)
				end
				if #files > 0 then
					table.insert(file_groups, files)
				end
			end
			result.files = file_groups
		elseif key == "lib" then
			-- 提取库列表
			for lib_name in value:gmatch('"([^"]+)"') do
				table.insert(result.libs, _trim(lib_name))
				log.Trace(0, "✓ Added library: %s", lib_name)
			end
		elseif key == "lib_path" then
			-- 提取库路径
			for lib_path in value:gmatch('"([^"]+)"') do
				table.insert(result.lib_paths, _trim(lib_path))
				log.Trace(0, "✓ Added lib_path: %s", lib_path)
			end
		end
	end

	-- 处理默认值
	if result.type == "file" and #result.paths == 0 then
		table.insert(result.paths, "src")
		log.Trace(0, "✓ Set default path to 'src' for file type")
	end

	if result.type ~= "file" and #result.lib_paths == 0 then
		-- 默认库路径
		if #result.paths > 0 and result.paths[1] == "default" then
			table.insert(result.lib_paths, "build/lib")
			log.Trace(0, "✓ Set default lib_path to 'build/lib' for library type")
		else
			-- 保留用户指定的路径
			for _, path in ipairs(result.paths) do
				table.insert(result.lib_paths, path)
			end
		end
	end

	return result
end

--- 从源文件内容中解析 xmake-deps 声明
--! @param content string 文件内容
--! @param srcfile string 源文件路径
--! @param options table 选项
--! @return table 依赖信息
function _parse_deps_content(content, srcfile, options)
	options = options or {}
	local start_marker = options.start_marker or "xmake-deps:"
	local end_marker = options.end_marker or "xmake-deps-end"
	local colors = log.get_colors()
	local prefix = colors.trace .. "[" .. log.get_debug_module() .. "]" .. log.indent_level(1) .. colors.reset

	log.Debug(0, "Parsing dependency block in: %s", srcfile)

	-- 直接在整个内容中查找 start_marker
	local start_pos, end_pos_start = content:find(start_marker, 1, true)
	if not start_pos then
		log.Debug(0, "✗ No '%s' marker found in %s", start_marker, srcfile)
		return { type = "file", paths = {}, files = {}, libs = {}, lib_paths = {} }
	end

	log.Debug(0, "✓ Found start marker at position %d-%d", start_pos, end_pos_start)

	-- 从 start_marker 之后开始查找 end_marker
	local block_end_pos, end_pos_end = content:find(end_marker, end_pos_start + 1, true)
	if not block_end_pos then
		log.Warn(0, "Found start marker but no end marker ('%s') in %s", end_marker, srcfile)
		return { type = "file", paths = {}, files = {}, libs = {}, lib_paths = {} }
	end

	log.Debug(0, "✓ Found end marker at position %d-%d", block_end_pos, end_pos_end)

	-- 提取标记块内容
	local block_content = content:sub(end_pos_start + 1, block_end_pos - 1)
	log.Debug(0, "✓ Found dependency block between positions %d and %d", end_pos_start + 1, block_end_pos - 1)
	log.Trace(0, "Block content (length %d):", #block_content)
	log.Trace(0, "'%s'", block_content)

	-- 解析键值对块
	local deps_info = _parse_kv_block(block_content)

	-- 解析文件路径（如果是文件类型）
	if deps_info.type == "file" then
		local resolved_files = {}
		local seen = {}

		for i, file_group in ipairs(deps_info.files) do
			local base_path = deps_info.paths[i] or deps_info.paths[1] or "src"

			for _, file in ipairs(file_group) do
				local fullpath = path.join(base_path, file)
				local resolved_path = nil

				-- 检查文件是否存在
				if os.isfile(fullpath) then
					resolved_path = path.absolute(fullpath)
				else
					-- 尝试在项目根目录下查找
					local alt_path = path.join(os.projectdir(), fullpath)
					if os.isfile(alt_path) then
						resolved_path = path.absolute(alt_path)
					else
						-- 尝试相对于源文件的路径
						local rel_path = path.join(path.directory(srcfile), fullpath)
						if os.isfile(rel_path) then
							resolved_path = path.absolute(rel_path)
						else
							log.Warn(0, "Dependency file not found: %s", file)
							goto continue_file
						end
					end
				end

				if not seen[resolved_path] then
					table.insert(resolved_files, resolved_path)
					seen[resolved_path] = true
					log.Info(0, "✓ Found dependency file: %s", resolved_path)
				end

				::continue_file::
			end
		end

		deps_info.resolved_files = resolved_files
	end

	return deps_info
end

--- 带缓存的依赖解析
--! @param srcfile string 源文件路径
--! @param options table 选项
--! @return table 依赖信息
function parse_deps(srcfile, options)
	log.set_debug_module("deps_parser")
	options = options or {}
	local force_rebuild = options.force_rebuild or false

	log.Info(0, "=== Parsing deps from: %s ===", srcfile)

	-- 1. 首先检查缓存
	if not force_rebuild then
		local cached_deps = deps_cache.get_file_deps(srcfile, options)
		if cached_deps then
			log.Info(0, "✓ Cache hit for %s", srcfile)
			return cached_deps
		else
			log.Info(0, "✗ Cache miss for %s", srcfile)
		end
	else
		log.Info(0, "Forcing rebuild (ignoring cache) for %s", srcfile)
	end

	-- 2. 读取文件内容
	local content = io.readfile(srcfile)
	if not content then
		log.Warn(0, "Failed to read file: %s", srcfile)
		return { type = "file", paths = {}, files = {}, libs = {}, lib_paths = {} }
	end

	-- 3. 解析依赖
	local deps_info = _parse_deps_content(content, srcfile, options)

	-- 4. 保存到缓存
	local cache_data = {
		type = deps_info.type,
		resolved_files = deps_info.resolved_files or {},
		libs = deps_info.libs,
		lib_paths = deps_info.lib_paths,
	}
	deps_cache.set_file_deps(srcfile, cache_data)

	log.Info(0, "=== Parsed dependency info from %s ===", srcfile)
	log.Info(0, "Type: %s", deps_info.type)
	if deps_info.type == "file" then
		log.Info(0, "Files found: %d", #deps_info.resolved_files)
	else
		log.Info(0, "Libraries to link: %d", #deps_info.libs)
		log.Info(0, "Library paths: %d", #deps_info.lib_paths)
	end

	return deps_info
end

--- 清理所有缓存
function clear_cache()
	log.Info(0, "=== Clearing dependency parser caches ===")
	deps_cache.clear_cache()
	log.Info(0, "✓ Dependency parser caches cleared successfully")
end

--- 获取缓存统计
function get_cache_stats()
	local stats = deps_cache.get_cache_stats()
	if stats then
		log.Info(0, "=== Cache Statistics ===")
		log.Info(0, "Cache type: %s", stats.cache_type)
		log.Info(0, "Cache file: %s", stats.cache_file or "N/A")
		log.Info(0, "Header deps count: %d", stats.head_deps_count or 0)
		log.Info(0, "Source deps count: %d", stats.src_deps_count or 0)
		log.Info(0, "Total entries: %d", stats.total_entries or 0)
		log.Info(0, "File exists: %s", stats.file_exists and "Yes" or "No")
		log.Info(0, "File size: %d bytes", stats.file_size or 0)
		return stats
	else
		log.Warn(0, "Failed to get cache statistics")
		return {}
	end
end
