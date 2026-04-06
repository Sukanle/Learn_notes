set_project("opengl")
set_version("0.1.5")
includes("@builtin/check")

option("module_dynamic_load")
	set_default(false)
	set_showmenu(true)
	set_description("Enable module dynamic loading (use SKL_MODULE_LANG and link skl-dl in executables)")
option_end()

add_includedirs(os.scriptdir() .. "/include", os.scriptdir() .. "/extern")
-- add_cxflags(os.getenv("CPPFLAGS"), os.getenv("LDFLAGS"))
set_languages("c99", "cxx20")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")
if is_mode("debug") then
	add_defines("__DEBUG__", "_DEBUG", "DEBUG")
end
local libs = {
	{
		path = {
			os.scriptdir() .. "/src/fs.cpp",
		},
		name = "skl-fs",
		type = "share",
		package = {},
		deps = {},
		lazy_undefined = false,
	},
	{
		path = {
			os.scriptdir() .. "/extern/glad/glad.c",
			os.scriptdir() .. "/src/graphics/gl/shader.cpp",
			os.scriptdir() .. "/src/graphics/gl/camera.cpp",
			os.scriptdir() .. "/src/graphics/gl/texture.cpp",
			os.scriptdir() .. "/src/graphics/error_category.cpp",
		},
		name = "skl-gl",
		type = "share",
		package = {
			"glfw3",
			"glm",
		},
		deps = { "skl-utils" },
		lazy_undefined = true,
	},
	{
		path = {
			os.scriptdir() .. "/src/utils/error_category.cpp",
		},
		name = "skl-utils",
		type = "share",
		package = {},
		deps = {},
		lazy_undefined = true,
	},
	{
		path = {
			os.scriptdir() .. "/src/utils/dl.cpp",
		},
		name = "skl-dl",
		type = "share",
		package = {},
		deps = {},
		lazy_undefined = true,
	},
	{
		path = {
			os.scriptdir() .. "/src/utils/lang.cpp",
		},
		name = "skl-lang",
		type = "share",
		package = {
			"simdjson",
		},
		deps = {
            "skl-fs"
        },
		lazy_undefined = true,
	},
}

add_requires("pkgconfig::glfw3", { alias = "glfw3" })
add_requires("pkgconfig::glm", { alias = "glm" })
add_requires("pkgconfig::simdjson", { alias = "simdjson" })
add_requires("opengl", { optional = true })

for i = 1, #libs do
	target(libs[i].name)
	set_kind("shared")
	add_files(libs[i].path)
	add_defines("SHARED")
	add_cxflags("-Wno-parentheses-equality")
	if libs[i].lazy_undefined then
		-- Allow unresolved symbols in shared libs and resolve at final load/link stage.
		if is_plat("macosx") then
			add_shflags("-undefined", "dynamic_lookup", {force = true})
		elseif is_plat("linux") then
			add_shflags("-Wl,--allow-shlib-undefined", {force = true})
		elseif is_plat("windows") then
			add_shflags("/FORCE:UNRESOLVED", {tools = {"link", "clang_cl"}, force = true})
			add_shflags("-Wl,--allow-shlib-undefined", {tools = {"gcc", "g++", "clang", "clang++"}, force = true})
		end
	end
	for j = 1, #libs[i].package do
		add_packages(libs[i].package[j])
	end
	for j = 1, #libs[i].deps do
		add_deps(libs[i].deps[j])
        add_links(libs[i].deps[j])
	end
	target_end()
end

local function to_table(str, tab, fn, null)
	local res = {}
	local midres = {}
	for i = 1, #tab do
		-- {{type = "share", name = "skl-fs"}}
		-- (?<=type\s?=\s?)[^,|}|\s]+
		-- "share"
		local tmp = {}
		local pattern = tab[i] .. "%s-=%s-([^,|}%s]+)"
		for word in string.gmatch(str, pattern) do
			local num = tonumber(word)
			if word == "true" then
				table.insert(tmp, true)
			elseif word == "false" then
				table.insert(tmp, false)
			elseif word == "null" then
				table.insert(tmp, null)
			elseif num then
				table.insert(tmp, num)
			else
				word = string.gsub(word, '"', "")
				table.insert(tmp, word)
			end
		end
		table.insert(midres, tmp)
	end
	-- return fn(res)
	for j = 1, #midres[1] do
		local tmp = fn(midres, j)
		if tmp ~= nil then
			table.insert(res, tmp)
		end
	end
	return res
end

add_packages("glfw3", "opengl")
local cache_deps = { path = os.scriptdir() .. "/.xmake/deps.json" }
cache_deps.mtime = os.mtime(cache_deps.path)

local function auto_deps(srcpath, tab, callback)
	on_load(function(target)
		import("core.base.json")
		local fjson = {}
		local is_cache_exists = os.exists(cache_deps.path)
		if is_cache_exists then
			fjson = json.loadfile(cache_deps.path)
		end
		local t = { target = target:name(), config = {} }
		if os.mtime(srcpath) > cache_deps.mtime or not fjson[t.target] then
			local data = table.to_array(io.lines(srcpath, { continuation = "\\" }))
			data = data[1]
			if string.find(data, "Deps:") == nil then
				return
			end
			print("Create deps cache by json... %s", path.basename(srcpath))
			data = string.gsub(data, "Deps: ", "")
			data = string.gsub(data, "// ", "")

			t.config = to_table(data, tab, callback, json.null)
			table.sort(t.config, function(a, b)
				return a.priority < b.priority
			end)
			fjson[t.target] = t.config
		else
			print("Load deps cache by json... %s", path.basename(srcpath))
			t.config = fjson[t.target]
		end
		json.savefile(cache_deps.path, fjson)

		for i = 1, #t.config do
			target:add("deps", t.config[i].name)
			target:add("links", t.config[i].name)
		end
	end)
end

target("lang_linked")
    set_kind("binary")
    add_files(os.scriptdir() .. "/demo/lang.cpp")
    add_cxflags("-Wno-parentheses-equality")
    add_deps("skl-gl")
    add_links("skl-gl")
	add_deps("skl-lang")
	add_links("skl-lang")

target("lang_dynamic")
	set_kind("binary")
	add_files(os.scriptdir() .. "/demo/lang.cpp")
	add_cxflags("-Wno-parentheses-equality")
	add_defines("SKL_LANG_DEMO_DYNAMIC", "SKL_MODULE_LANG")
	add_deps("skl-utils")
	add_links("skl-utils")
	add_deps("skl-dl")
	add_links("skl-dl")

for _, filepath in ipairs(os.files(os.scriptdir() .. "/demo/**/*.cpp")) do
	local basename = path.basename(filepath)
	if basename == "lang" then
		goto continue_cpp_demo
	end
	target(basename)
	set_kind("binary")
	add_files(filepath)
	add_packages("glm")
	add_cxflags("-Wno-parentheses-equality")
	if has_config("module_dynamic_load") then
		add_defines("SKL_MODULE_LANG")
		-- When lang is loaded dynamically, executable side must link skl-dl for loader/lifetime management.
		add_deps("skl-dl")
		add_links("skl-dl")
	else
		-- Resolve skl-utils i18n symbols at final executable link stage.
		add_deps("skl-lang")
		add_links("skl-lang")
	end
	auto_deps(filepath, { "type", "name", "cache", "priority", "deps" }, function(tab, pos)
		local is_cache = tab[3][pos]
		if is_cache == false then
			return nil
		end
		return {
			type = tab[1][pos],
			name = tab[2][pos],
			priority = tab[4][pos],
			deps = tab[5][pos],
		}
	end)
	::continue_cpp_demo::
end

for _, filepath in ipairs(os.files(os.scriptdir() .. "/demo/**/*.c")) do
	target(path.basename(filepath))
	set_kind("binary")
	add_files(filepath, os.scriptdir() .. "/extern/glad/glad.c")
end
