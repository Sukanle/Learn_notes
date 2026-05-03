#pragma once

#import "skl/memory/handle.hpp"

NAMESPACE_MEMORY_BEGIN

template<typename T, typename Tag = T>
class ResourceManager {
public:
    using HandleType = Handle<Tag>;
    using InitCallback = function_ref<void(T&)>;

    // 注入运行时内存资源（默认使用 new/delete）
    explicit ResourceManager(std::pmr::memory_resource *mem_res = std::pmr::new_delete_resource())
        : _pool(mem_res)
        , _free(mem_res) {
        static_assert(std::is_default_constructible_v<T>, "T must support a default constructor!");
        _pool.reserve(1024);
        // Index 0 永久保留为 Null Handle
        _pool.push_back({{}, 0});
    }
    ~ResourceManager() noexcept = default;

    SKL_DELETE_COPYABLE(ResourceManager);
    SKL_DEFAULT_MOVEABLE(ResourceManager);

    HandleType acquire(InitCallback init_cb = nullptr) {
        uint32_t idx = _pool.size();
        if (!_free.empty()) {
            idx = _free.back();
            _free.pop_back();
        } else
            _pool.push_back({{}, 0});

        auto &slot = _pool[idx];
        // Avoid adding 1 to UINT32_MAX overflow to become 0, and 0 is used to represent NULL.
        if (++slot.generation == 0) ++slot.generation;

        slot.pres = std::make_shared<T>();
        if (init_cb) init_cb(*slot.pres);

        return {idx, slot.generation};
    }

    // 释放资源
    void release(HandleType h, std::error_code &ec) noexcept {
        if (!h || h.index >= _pool.size()) {
            ec = make_error_code(std::errc::not_enough_memory);
            return;
        }
        auto &slot = _pool[h.index];
        if (slot.generation != h.generation || !slot.pres) {
            ec = make_error_code(std::errc::invalid_argument);
            return;
        }

        slot.pres.reset();   // 强引用 -1，若归零则触发析构
        if (++slot.generation == 0) ++slot.generation;
        _free.push_back(h.index);   // LIFO 回收
    }

    // 安全访问
    [[nodiscard]] std::shared_ptr<T> get(HandleType h) noexcept { return is_valid(h) ? _pool[h.index].pres : nullptr; }

    [[nodiscard]] std::weak_ptr<T>get(HandleType h) const noexcept {
        return is_valid(h) ? std::weak_ptr<T>(_pool[h.index].pres) : std::weak_ptr<T>{};
    }

    [[nodiscard]] T* get_raw(HandleType h) noexcept {
        return is_valid(h) ? _pool[h.index].get() : nullptr;
    }

    [[nodiscard]] const T* get_raw(HandleType h) const noexcept {
        return is_valid(h) ? _pool[h.index].get() : nullptr;
    }

    [[nodiscard]] bool is_valid(HandleType h) const noexcept {
        return h && h.index < _pool.size() && _pool[h.index].generation == h.generation && _pool[h.index].pres;
    }

    // 调试/维护接口
    [[nodiscard]] size_t capacity() const noexcept { return _pool.size(); }
    [[nodiscard]] size_t active() const noexcept { return capacity() - 1 - _free.size(); }

private:
    struct slot {
        std::shared_ptr<T> pres;
        uint32_t generation;
    };

    std::pmr::vector<slot> _pool;
    std::pmr::vector<uint32_t> _free;
};
NAMESPACE_MEMORY_END
