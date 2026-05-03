#pragma once

#include "skl/utils/base.hpp"


NAMESPACE_UTILS_BEGIN
    using hash32_t = XXH32_hash_t;
    using hash64_t = XXH64_hash_t;
    using hash128_t = XXH128_hash_t;
    using hash_t = hash64_t;

    template<typename T>
    concept is_hash = std::is_same_v<T, hash32_t> || std::is_same_v<T, hash64_t> || std::is_same_v<T, hash128_t>;

    template<typename Hash = hash64_t>
        requires is_hash<Hash>
    inline auto hash_combine(Hash major, Hash minor, bool high64 = true) -> Hash {
        if constexpr (std::is_same_v<hash32_t, Hash>)
            return XXH32(&major, sizeof(major), minor);
        else if constexpr (std::is_same_v<hash64_t, Hash>)
            return XXH3_64bits_withSeed(&major, sizeof(major), minor);
        else
            return XXH3_128bits_withSeed(&major, sizeof(major), high64 ? minor.high64 : minor.low64);
    }

    inline hash32_t hash32_combine(hash32_t major, hash32_t minor) {
        return XXH32(&major, sizeof(major), minor);
    }
    inline hash64_t hash64_combine(hash64_t major, hash64_t minor) {
        return XXH3_64bits_withSeed(&major, sizeof(major), minor);
    }
    inline hash128_t hash128_combine(hash128_t major, hash128_t minor, bool high64 = true) {
        return XXH3_128bits_withSeed(&major, sizeof(major), high64 ? minor.high64 : minor.low64);
    }

    struct Hash32 {
        hash32_t operator()(const String &key) const { return XXH32(key.c_str(), key.length(), 0); }
        hash32_t operator()(String_view key) const { return XXH32(key.data(), key.length(), 0); }
    };
    struct Hash64 {
        hash64_t operator()(const String &key) const { return XXH3_64bits(key.c_str(), key.length()); }
        hash64_t operator()(String_view key) const { return XXH3_64bits(key.data(), key.length()); }
    };
    struct Hash128 {
        hash128_t operator()(const String &key) const { return XXH3_128bits(key.c_str(), key.length()); }
        hash128_t operator()(String_view key) const { return XXH3_128bits(key.data(), key.length()); }
    };
NAMESPACE_UTILS_END
