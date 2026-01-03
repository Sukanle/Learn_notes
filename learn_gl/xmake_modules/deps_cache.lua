--! @diagnostic disable: undefined-global, lowercase-global
--! @module deps_cache
--- 依赖缓存管理模块（JSON 文件存储实现）
--! @author YourName
--! @license MIT
import("core.base.option")
import("core.base.json")
import("log")

-- 缓存文件路径
local CACHE_FILE = path.join(os.projectdir(), ".xmake", "skl-cache.json")

-- 全局缓存数据
local cache_data = {
	["head-deps"] = {},
	["src-deps"] = {},
}

-- 标记缓存是否已修改
local cache_status = {
	modified = false,
	load = false,
}

--- 加载缓存文件
function _load_cache()
	if os.isfile(CACHE_FILE) then
		local data = json.loadfile(CACHE_FILE)
		if data then
			cache_data = data
			-- 确保必要的字段存在
			cache_data["head-deps"] = cache_data["head-deps"] or {}
			cache_data["src-deps"] = cache_data["src-deps"] or {}
			log.Info(1, "✓ Loaded cache from %s", CACHE_FILE)
			return true
		else
			log.Warn(0, "Failed to decode cache file: %s", CACHE_FILE)
		end
	end
	-- 初始化默认结构
	cache_data = {
		["head-deps"] = {},
		["src-deps"] = {},
	}
	return false
end

--- 保存缓存文件
function _save_cache()
	if not cache_status.modified then
		return
	end

	-- 确保 .xmake 目录存在
	os.mkdir(path.directory(CACHE_FILE))

	json.savefile(CACHE_FILE, cache_data)
	cache_status.modified = false

	local head_count = #(cache_data["head-deps"] or {})
	local src_count = #(cache_data["src-deps"] or {})
	log.Info(0, "✓ Saved cache to %s (head-deps: %d, src-deps: %d)", CACHE_FILE, head_count, src_count)
end

--- 初始化缓存
function _init_cache()
	log.set_debug_module("deps_cache")
	if not cache_status.load then
		_load_cache()
        cache_status.load = true
	end
end

--- 查找头文件依赖缓存
--! @param header_path string 头文件路径
--! @return table|nil 缓存数据
function _find_head_dep(header_path)
	for _, item in ipairs(cache_data["head-deps"]) do
		if item.target == header_path then
			return item
		end
	end
	return nil
end

--- 查找源文件依赖缓存
--! @param srcfile string 源文件路径
--! @return table|nil 缓存数据
function _find_src_dep(srcfile)
	for _, item in ipairs(cache_data["src-deps"]) do
		if item.head == srcfile then
			return item
		end
	end
	return nil
end

--- 获取文件依赖（带缓存）
--! @param srcfile string 源文件路径
--! @param options table 选项：{force_rebuild=false}
--! @return table<string>|nil 依赖列表或 nil
function get_file_deps(srcfile, options)
	_init_cache()

	options = options or {}
	local force_rebuild = options.force_rebuild or false
	local mtime = os.mtime(srcfile)

	if not mtime then
		log.Warn(0, "File not found or no modification time: %s", tostring(srcfile))
		return nil
	end

	log.Info(0, "Getting file deps cache for: %s", srcfile)

	if force_rebuild then
		log.Warn(0, "Forcing rebuild (ignoring cache) for %s", srcfile)
		return nil
	end

	local cached_item = _find_src_dep(srcfile)
	if cached_item then
		-- 验证所有依赖文件的 mtime
		local valid = true
		for _, dep in ipairs(cached_item.deps) do
			local dep_mtime = os.mtime(dep)
			if not dep_mtime or dep_mtime > cached_item.mtime then
				valid = false
				log.Info(
					2,
					"Dependency modified: %s (cached mtime=%d, current mtime=%d)",
					dep,
					cached_item.mtime,
					dep_mtime
				)
				break
			end
		end

		if valid and cached_item.mtime == mtime then
			log.Info(0, "✓ Cache hit for %s (%d dependencies)", srcfile, #(cached_item.deps or {}))
			return cached_item.deps
		else
			log.Info(0, "✗ Cache miss for %s (file modified or dependencies changed)", srcfile)
			if not valid then
				log.Info(2, "Dependency check failed for %s", srcfile)
			end
		end
	else
		log.Info(0, "✗ Cache miss for %s (not found in cache)", srcfile)
	end

	return nil
end

--- 设置文件依赖缓存
--! @param srcfile string 源文件路径
--! @param deps table<string> 依赖列表
function set_file_deps(srcfile, deps)
	_init_cache()

	local mtime = os.mtime(srcfile)
	if not mtime then
		log.Error(0, "Cannot cache file deps: file not found or no modification time: %s", srcfile)
		return
	end

	local cached_item = _find_src_dep(srcfile)
	if cached_item then
		-- 更新现有缓存
		cached_item.mtime = mtime
		cached_item.deps = deps
	else
		-- 添加新缓存
		table.insert(cache_data["src-deps"], {
			head = srcfile,
			mtime = mtime,
			deps = deps,
		})
	end

	cache_status.modified = true

	local str = "✓ Set cache for %s (%d dependencies)"
	if log.get_debug_level() < 5 then
		log.Info(0, str, srcfile, #deps)
	else
		log.Trace(0, str .. ":", srcfile, #deps)
		log.Trace(1, "Cache file: %s", CACHE_FILE)
		local colors = log.get_colors()
		local prefix = colors.trace .. "[" .. log.get_debug_module() .. "]" .. colors.reset
		table.print({ head = srcfile, mtime = mtime, deps = deps }, 2, prefix)
	end

	-- 立即保存缓存
	_save_cache()

	if #deps ~= 0 and log.get_debug_level() >= 4 then
		for i, dep in ipairs(deps) do
			log.Debug(1, "Dep %d/%d: %s", i, #deps, tostring(dep))
		end
	end
end

--- 获取头文件依赖缓存
--! @param header_path string 头文件路径
--! @param options table 选项：{force_rebuild=false}
--! @return table<string>|nil 依赖列表或 nil
function get_header_deps(header_path, options)
	_init_cache()

	options = options or {}
	local force_rebuild = options.force_rebuild or false
	local mtime = os.mtime(header_path)

	if not mtime then
		log.Warn(0, "Header file not found or no modification time: %s", tostring(header_path))
		return nil
	end

	log.Info(0, "Getting header deps cache for: %s", header_path)

	if force_rebuild then
		log.Warn(0, "Forcing rebuild (ignoring cache) for %s", header_path)
		return nil
	end

	local cached_item = _find_head_dep(header_path)
	if cached_item then
		-- 验证所有依赖文件的 mtime
		local valid = true
		for _, dep in ipairs(cached_item.deps) do
			local dep_mtime = os.mtime(dep)
			if not dep_mtime or dep_mtime > cached_item.mtime then
				valid = false
				log.Info(
					2,
					"Dependency modified: %s (cached mtime=%d, current mtime=%d)",
					dep,
					cached_item.mtime,
					dep_mtime
				)
				break
			end
		end

		if valid and cached_item.mtime == mtime then
			log.Info(0, "✓ Header cache hit for %s (%d dependencies)", header_path, #(cached_item.deps or {}))
			return cached_item.deps
		else
			log.Warn(0, "✗ Header cache miss for %s (file modified or dependencies changed)", header_path)
		end
	else
		log.Warn(0, "✗ Header cache miss for %s (not found in cache)", header_path)
	end

	return nil
end

--- 设置头文件依赖缓存
--! @param header_path string 头文件路径
--! @param deps table<string> 依赖列表
function set_header_deps(header_path, deps)
	_init_cache()

	local mtime = os.mtime(header_path)
	if not mtime then
		log.Warn(0, "Cannot cache header deps: file not found or no modification time: %s", header_path)
		return
	end

	local cached_item = _find_head_dep(header_path)
	if cached_item then
		-- 更新现有缓存
		cached_item.mtime = mtime
		cached_item.deps = deps
	else
		-- 添加新缓存
		table.insert(cache_data["head-deps"], {
			target = header_path,
			mtime = mtime,
			deps = deps,
		})
	end

	cache_status.modified = true

	log.Info(0, "✓ Set header cache for %s (%d dependencies)", header_path, #deps)

	-- 立即保存缓存
	_save_cache()
end

--- 清理所有缓存
function clear_cache()
	log.Info(0, "=== Clearing all dependency caches ===")

	-- 重置缓存数据
	cache_data = {
		["head-deps"] = {},
		["src-deps"] = {},
	}
	cache_status.modified = true

	-- 删除缓存文件
	if os.isfile(CACHE_FILE) then
		os.rm(CACHE_FILE)
		log.Info(0, "✓ Removed cache file: %s", CACHE_FILE)
	end

	log.Info(0, "✓ All caches cleared successfully")
end

--- 获取缓存统计
function get_cache_stats()
	_init_cache()

	local head_count = #(cache_data["head-deps"] or {})
	local src_count = #(cache_data["src-deps"] or {})

	return {
		cache_type = "json_file",
		cache_file = CACHE_FILE,
		head_deps_count = head_count,
		src_deps_count = src_count,
		total_entries = head_count + src_count,
		file_exists = os.isfile(CACHE_FILE),
		file_size = os.isfile(CACHE_FILE) and os.filesize(CACHE_FILE) or 0,
	}
end

-- 在模块加载时初始化缓存
_init_cache()
