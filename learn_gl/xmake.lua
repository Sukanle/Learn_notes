-- xmake.lua - 使用依赖机制确保构建顺序：头文件扫描 -> 库生成 -> 目标文件生成
set_project("graphics-demos")
set_version("0.1.0")
set_defaultmode("release")

-- 公共配置
add_includedirs("include")
set_languages("c++20")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "./" })

-- 编译器配置
if is_plat("macosx") then
	set_toolchains("llvm")
	set_config("cc", "/opt/homebrew/opt/llvm/bin/clang")
	set_config("cxx", "/opt/homebrew/opt/llvm/bin/clang++")
elseif is_plat("windows") then
	set_toolchains("msvc")
end

add_requires("glfw3", { system = true })

-- 存储库信息
libraries = {}

-- 创建头文件扫描规则
rule("scan_headers")
set_extensions(".h", ".hpp", ".cpp")
on_load(function(target)
	import("xmake_modules.type_parser")
	import("xmake_modules.deps_parser")
	import("xmake_modules.log")
	log.set_debug_module("scan_headers")
	log.set_debug_level(1)

	-- 仅在第一次加载时执行扫描
	if target:values("scan_headers") then
		return
	end

	log.Info(0, "=== Starting header scan phase ===")

	-- 收集所有头文件 (.h 和 .hpp)
	local all_headers = {}
	-- 收集 .h 文件
	for _, header_path in ipairs(os.files("include/skl/**.h")) do
		if os.isfile(header_path) then
			table.insert(all_headers, header_path)
			log.Debug(2, "Found .h file: %s", header_path)
		end
	end
	-- 收集 .hpp 文件
	for _, header_path in ipairs(os.files("include/skl/**.hpp")) do
		if os.isfile(header_path) then
			table.insert(all_headers, header_path)
			log.Debug(2, "Found .hpp file: %s", header_path)
		end
	end
	log.Info(0, "Total header files found: %d", #all_headers)

	-- 分析每个头文件，识别库
	local new_g_libraries = {}
	local found_count = 0 -- 专门用于计数

	for i, header_path in ipairs(all_headers) do
		log.Info(1, "Analyzing header %d/%d: %s", i, #all_headers, header_path)
		local header_type = type_parser.parse_type(header_path)
		log.Debug(1, "Header %s has type: %s", path.filename(header_path), header_type)

		if header_type == "Static" or header_type == "Dynamic" then
			local lib_name = path.basename(header_path):gsub("%.h$", ""):gsub("%.hpp$", "")
			if not new_g_libraries[lib_name] then
				new_g_libraries[lib_name] = {
					kind = header_type,
					header = header_path,
				}
				found_count = found_count + 1 -- 增加计数
				log.Info(0, "✓ Found library header: %s (%s) - %s", lib_name, header_type, header_path)
			end
		end
	end

	-- 更新全局库信息
	for lib_name, lib_info in pairs(new_g_libraries) do
		target:data_set("libraries[".. lib_name .."].kind" , lib_info.kind)
		target:data_set("libraries[".. lib_name .."].header", lib_info.header)
	end

	-- 存储库数量到全局变量
    target:data_set("libraries.count", tostring(found_count)) -- 将count存储在libraries表中
	log.Info(0, "✓ header scan completed. found %d g_libraries", found_count)
    table.print(libraries)

	-- 标记已完成扫描
	target:values("scan_headers", true)
end)
rule_end()

-- 创建头文件扫描器伪目标（确保最先执行）
target("header_scanner")
set_kind("phony")
add_rules("scan_headers")
on_build(function(target)
	import("xmake_modules.log")
	log.set_debug_module("scan_headers")
	log.Info(0, "✅ header scanning completed successfully")
    table.print(target:data("libraries"))
end)
target_end()

-- 创建库构建器（确保在头文件扫描后执行）
for _, lib_file in pairs(os.files("./src/*.cpp")) do
	local target_name = path.basename(lib_file):gsub("%.cpp$", "")
	target(target_name)
	add_deps("header_scanner") -- 确保在头文件扫描后执行
	-- 跳过count字段
    if libraries[target_name] == nil then
        set_enabled(false)
    else
	on_build(function(target)
		import("xmake_modules.deps_parser")
		import("xmake_modules.log")
		log.set_debug_module("build_g_libraries")
		log.set_debug_level(1)

		-- 仅在第一次加载时执行
		-- if target:values("g_libraries_built") then
		-- 	return
		-- end

		log.Info(0, "=== starting library build phase ===")

		-- 确保头文件已扫描 - 使用正确的计数方式
		if libraries.count == nil or libraries.count == 0 then
			log.warn(0, "no g_libraries found. skipping library build phase.")
			target:values("g_libraries_built", true)
			return
		end

		target:values("g_libraries_built", true)
		log.Info(0, "✓ library build phase completed. created %d g_libraries", libraries.count)
		if lib_name ~= "count" then
			log.Info(0, "creating library target: %s (%s)", lib_name, lib_info.kind)
			-- local lib_target = target(lib_name)
			if lib_info.kind == "static" then
				target:set("kind", "static")
				-- target:set("targetdir", "build/lib")
			else
				target:set("kind", "shared")
				-- target:set("targetdir", "build/lib")
			end

			-- 添加实现文件
			local header_deps = deps_parser.parse_deps(lib_info.header)
			log.Info(1, "library %s has %d dependency files", lib_name, #header_deps)
			for _, dep_file in ipairs(header_deps) do
				if os.isfile(dep_file) then
					lib_target:add("files", dep_file)
					log.Info(2, "✓ added file to %s: %s", lib_name, dep_file)
				else
					log.warn(1, "dependency file not found: %s", dep_file)
				end
			end

			-- 库的公共配置
			target:set("languages", "c++20")
			target:add("includedirs", "include")
			target:add("packages", "glfw3")
			if is_plat("macosx") then
				target:add("frameworks", "opengl")
			elseif is_plat("windows") then
				target:add("syslinks", "opengl32")
			elseif is_plat("linux") then
				target:add("syslinks", "gl", "x11")
			end
		end
		-- set_kind("phony")

		log.Info(0, "✅ library building completed successfully")
	end)
	target_end()
    end
end

-- 构建每个 demo，确保在库构建后执行
for _, srcfile in ipairs(os.files("demo/*.cpp")) do
	local target_name = path.basename(srcfile):gsub("%.cpp$", "")

	target(target_name)
	add_deps("header_scanner") -- 确保在头文件扫描后执行
	-- 确保在库构建后执行
	set_kind("binary")
	-- set_targetdir("build/bin")
	add_files(srcfile)

	-- 添加 glad.c.o（如果存在）
	local glad_obj = "tmp/obj/glad.c.o"
	if os.isfile(glad_obj) then
		add_files(glad_obj)
		-- 	log.Info(0, "✓ added glad object file: %s", glad_obj)
	end

	on_build(function(target_obj)
		import("xmake_modules.auto_deps")
		import("xmake_modules.type_parser")
		import("xmake_modules.deps_parser")
		import("xmake_modules.log")

		log.set_debug_module("demo_builder")
		log.set_debug_level(1)

		-- 使用 auto_deps 扫描依赖
		local custom_rules = {
			{
				custom_headers = { "skl" }, -- 匹配 skl/ 目录
				search_paths = { "include" },
			},
		}

		local custom_headers = auto_deps.scan(srcfile, custom_rules, {
			recursive = true,
			max_depth = 3,
			search_paths = { "include" },
		})

		log.Info(0, "Found %d custom headers for %s", #custom_headers, target_name)

		-- 处理依赖
		local direct_deps = {}
		local lib_deps = {}
		local seen = {}

		for _, header_path in ipairs(custom_headers) do
			local header_type = type_parser.parse_type(header_path)
			local lib_name = path.basename(header_path):gsub("%.h$", ""):gsub("%.hpp$", "")

			if libraries[lib_name] then
				-- 库依赖
				table.insert(lib_deps, lib_name)
				log.Info(0, "✓ Adding library dependency: %s", lib_name)
			elseif header_type == "Target" then
				-- 直接依赖
				local header_deps = deps_parser.parse_deps(header_path)
				for _, dep in ipairs(header_deps) do
					if not seen[dep] then
						table.insert(direct_deps, dep)
						seen[dep] = true
						log.Info(1, "✓ Adding direct dependency: %s", dep)
					end
				end
			end
		end

		-- 添加直接依赖
		if #direct_deps > 0 then
			target_obj:add("files", direct_deps)
		end

		-- 链接库
		for _, lib_name in ipairs(lib_deps) do
			target_obj:add("deps", lib_name)
            target_obj:add("link", lib_name)
			log.Info(0, "✓ Linked library: %s", lib_name)
		end

		log.Info(
			0,
			"✓ Configured target %s with %d direct deps and %d library deps",
			target_name,
			#direct_deps,
			#lib_deps
		)
	end)

	-- 系统依赖
	add_packages("glfw3")
	if is_plat("macosx") then
		add_frameworks("OpenGL")
	elseif is_plat("windows") then
		add_syslinks("opengl32")
	elseif is_plat("linux") then
		add_syslinks("GL", "X11")
	end
	target_end()
end

-- Release 优化
if is_mode("release") then
	set_optimize("fastest")
	add_defines("NDEBUG")
	if is_plat("macosx", "linux") then
		add_cxflags("-flto", "-s")
		add_ldflags("-flto", "-s")
	elseif is_plat("windows") then
		add_cxflags("/GL")
		add_ldflags("/LTCG", "/OPT:REF", "/OPT:ICF")
	end
end
