#include "skl/fs.hpp"
#include "skl/utils/error_category.hpp"
#include "skl/utils/lang.hpp"
#define SIMDJSON_AVX512_ALLOWED 0
#define NDEBUG 1
#include <simdjson.h>
#undef NDEBUG

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <vector>

NAMESPACE_UTILS_BEGIN
struct I18NLoader::package {
    simdjson::dom::parser parser;
    simdjson::dom::element root;
};

namespace {

thread_local std::string g_last_i18n_error;

void set_last_i18n_error(const char *msg) {
    if (!msg) {
        g_last_i18n_error.clear();
        return;
    }
    g_last_i18n_error.assign(msg);
}

bool parse_hex_code(std::string_view key, uint32_t &out) {
    if (key.size() < 3 || key[0] != '0' || (key[1] != 'x' && key[1] != 'X')) return false;
    uint32_t value = 0;
    for (size_t i = 2; i < key.size(); ++i) {
        const char c = key[i];
        uint32_t nibble = 0;
        if (c >= '0' && c <= '9')
            nibble = static_cast<uint32_t>(c - '0');
        else if (c >= 'a' && c <= 'f')
            nibble = static_cast<uint32_t>(c - 'a' + 10U);
        else if (c >= 'A' && c <= 'F')
            nibble = static_cast<uint32_t>(c - 'A' + 10U);
        else
            return false;
        value = (value << 4U) | nibble;
    }
    out = value;
    return true;
}

std::string normalize_lang(std::string_view lang) {
    if (lang.empty()) return "zh-CN";

    std::string value(lang);
    for (char &c : value) {
        if (c == '_') c = '-';
    }

    std::string lower = value;
    std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return (char)(std::tolower(c)); });
    if (lower.starts_with("zh")) return "zh-CN";
    return value;
}

std::string runtime_lang() {
    if (const char *explicit_lang = std::getenv("SKL_LANG"); explicit_lang && explicit_lang[0] != '\0') {
        return normalize_lang(explicit_lang);
    }

    if (const char *env_lang = std::getenv("LANG"); env_lang && env_lang[0] != '\0') {
        std::string raw(env_lang);
        const auto dot = raw.find('.');
        if (dot != std::string::npos) raw = raw.substr(0, dot);
        return normalize_lang(raw);
    }

    return "zh-CN";
}

std::vector<std::filesystem::path> search_roots() {
    std::vector<std::filesystem::path> roots;
    roots.emplace_back(std::filesystem::current_path());

    char exe_dir[skl::fs::SKL_PATH_MAX] = {0};
    if (skl::fs::exedir(exe_dir, sizeof(exe_dir)) != nullptr)
        roots.emplace_back(exe_dir);

    std::vector<std::filesystem::path> expanded;
    expanded.reserve(roots.size() * 8);
    for (const auto &root : roots) {
        std::filesystem::path p = root;
        for (int i = 0; i < 8; ++i) {
            if (p.empty()) break;
            expanded.push_back(p);
            if (!p.has_parent_path()) break;
            const std::filesystem::path parent = p.parent_path();
            if (parent == p) break;
            p = parent;
        }
    }

    std::ranges::sort(expanded.begin(), expanded.end());
    expanded.erase(std::ranges::unique(expanded).begin(), expanded.end());
    return expanded;
}

std::vector<std::filesystem::path> candidate_language_files() {
    const std::string lang = runtime_lang();
    const std::vector<std::filesystem::path> roots = search_roots();

    std::vector<std::filesystem::path> candidates;
    for (const auto &r : roots) {
        candidates.push_back(r / "resources/language/debug" / ("error_" + lang + ".json"));
        candidates.push_back(r / "resources/language/debug/error.json");
    }
    return candidates;
}

}   // namespace

I18NLoader::I18NLoader(std::error_code &ec, const std::string &file_path, std::string_view lang) {
    impl = std::make_unique<package>();
    (void)lang;
    ec.clear();

    if (file_path.empty() || !std::filesystem::exists(file_path)) {
        ec = make_error_code(errc::lang_no_such_file);
        return;
    }

    auto error = impl->parser.load(file_path).get(impl->root);
    if (error) {
        ec = make_error_code((errc)(error | 0x0200U));
        return;
    }

    simdjson::dom::object doc;
    if (impl->root.get(doc)) {
        ec = make_error_code(errc::invalid_argument);
        return;
    }

    for (auto field : doc) {
        std::string_view module = field.key;
        if (module == "meta" || module == "submodules") continue;

        simdjson::dom::object module_obj;
        if (field.value.get(module_obj)) continue;

        auto &module_index = index_map[std::string(module)];
        for (auto item : module_obj) {
            uint32_t code = 0;
            if (!parse_hex_code(item.key, code)) continue;

            std::string_view msg;
            if (item.value.get_string().get(msg)) continue;
            module_index[code] = std::string(msg);
        }
    }

    if (index_map.empty()) {
        ec = make_error_code(errc::invalid_argument);
        return;
    }

    loaded = true;
}
std::string_view I18NLoader::find(std::error_code &ec, std::string_view submodule, uint32_t code) const noexcept {
    ec.clear();
    if (!loaded || !impl) return {};

    const auto module_it = index_map.find(std::string(submodule));
    if (module_it == index_map.end()) return {};

    const auto &module_map = module_it->second;
    auto it = module_map.find(code);
    if (it != module_map.end()) return it->second;
    return {};
}

I18NLoader::~I18NLoader() = default;
I18NLoader::I18NLoader(I18NLoader &&) noexcept = default;
I18NLoader &I18NLoader::operator=(I18NLoader &&) noexcept = default;

const I18NLoader &global_i18n_loader() noexcept {
    static const I18NLoader loader = []() -> I18NLoader {
        std::error_code ec;
        const std::string lang = runtime_lang();
        for (const auto &candidate : candidate_language_files()) {
            if (!std::filesystem::exists(candidate)) continue;
            I18NLoader try_loader(ec, candidate.string(), lang);
            if (try_loader.is_loaded()) return try_loader;
        }

        I18NLoader fallback(ec, "", lang);
        return fallback;
    }();
    return loader;
}

NAMESPACE_UTILS_END

extern "C" {

int skl_i18n_find_ex(const char *submodule, uint32_t code, const char **out_message, size_t *out_size) {
    if (out_message) *out_message = nullptr;
    if (out_size) *out_size = 0;

    if (!submodule || !out_message) {
        skl::utils::set_last_i18n_error("invalid argument");
        return -1;
    }

    const auto &loader = skl::utils::global_i18n_loader();
    if (!loader.is_loaded()) {
        skl::utils::set_last_i18n_error("i18n loader not loaded");
        return -2;
    }

    std::error_code ec;
    const std::string_view message = loader.find(ec, submodule, code);
    if (ec) {
        const std::string text = ec.message();
        skl::utils::set_last_i18n_error(text.c_str());
        return -3;
    }

    if (message.empty()) {
        skl::utils::set_last_i18n_error("message not found");
        return 1;
    }

    *out_message = message.data();
    if (out_size) *out_size = message.size();
    skl::utils::set_last_i18n_error(nullptr);
    return 0;
}

const char *skl_i18n_last_error_message(void) {
    return skl::utils::g_last_i18n_error.c_str();
}

}
