set_project("opengl")
set_version("0.1.3")
includes("@builtin/check")

add_includedirs(os.scriptdir() .. "/include", os.scriptdir() .. "/extern")
add_cxflags(os.getenv("CPPFLAGS"), os.getenv("LDFLAGS"))
-- set_toolchains("llvm")
set_languages("c99", "cxx20")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

local libs = {
	{
		path = {
			os.scriptdir() .. "/src/fs.cpp",
		},
		name = "skl-fs",
		type = "share",
		package = {},
	},
	{
		path = {
			os.scriptdir() .. "/src/shader.cpp",
			os.scriptdir() .. "/src/glad.c",
		},
		name = "skl-gl",
		type = "share",
		package = {
			"glfw3",
		},
	},
}

add_requires("pkg_config::glfw3", { alias = "glfw3" })
add_requires("pkg_config::glm", { alias = "glm" })
add_requires("opengl", { optional = true })

for i = 1, #libs do
	target(libs[i].name)
	set_kind("shared")
	add_files(libs[i].path)
	add_defines("SHARED")
	for j = 1, #libs[i].package do
		add_packages(libs[i].package[j])
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

add_packages("glfw3")
add_packages("opengl")
local cache_deps = { mtime = 0, path = os.scriptdir() .. "/.xmake/deps.json" }

target("load_deps_json")
set_kind("phony")
if os.exists(cache_deps.path) then
	cache_deps.mtime = os.mtime(cache_deps.path)
end

local function auto_deps(path, tab, callback)
	on_load(function(target)
		import("core.base.json")
		local t = {}
		if os.mtime(path) > cache_deps.mtime then
			local data = table.to_array(io.lines(path, { continuation = "\\" }))[1]
			if string.find(data, "Deps:") == nil then
				return
			end
			data = string.gsub(data, "Deps: ", "")
			data = string.gsub(data, "// ", "")

			t = to_table(data, tab, callback, json.null)
			table.sort(t, function(a, b)
				return a.priority < b.priority
			end)
		else
			t = json.loadfile(cache_deps.path)
		end
		json.savefile(cache_deps.path, t)
		for i = 1, #t do
			target:add("deps", t[i].name)
			target:add("links", t[i].name)
		end
	end)
end


for _, filepath in pairs(os.files(os.scriptdir() .. "/demo/*.cpp")) do
	local basename = path.basename(filepath)
	target(basename)
	set_kind("binary")
	add_files(filepath)
    add_cxflags("-Wno-parentheses-equality")
	check_cincludes("STB_IMAGE_IMPLEMENTATION", "stb_image.h", { includedirs = os.scriptdir() .. "/include"})
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
end

for _, filepath in pairs(os.files(os.scriptdir() .. "/demo/*.c")) do
	local basename = path.basename(filepath)
	target(basename)
	set_kind("binary")
	add_files(filepath, os.scriptdir() .. "/src/glad.c")
end
