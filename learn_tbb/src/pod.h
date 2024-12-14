#pragma once
#ifndef POD_H
#define POD_H

#include <new>
#include <utility>

template <typename T> class pod {
private:
    T m_t;

public:
    pod() = default;

    pod(pod&& p) noexcept
        : m_t(std::move(p.m_t))
    {
    }

    pod(pod const& p) noexcept
        : m_t(p.m_t)
    {
    }

    pod& operator=(pod&& p) noexcept
    {
        m_t = std::move(p.m_t);
        return *this;
    }

    pod& operator=(pod const& p) noexcept
    {
        m_t = p.m_t;
        return *this;
    }

    pod(T&& t) noexcept
        : m_t(std::move(t))
    {
    }

    pod(T const& t) noexcept
        : m_t(t)
    {
    }

    pod& operator=(T&& t) noexcept
    {
        m_t = std::move(t);
        return *this;
    }

    pod& operator=(T const& t) noexcept
    {
        m_t = t;
        return *this;
    }

    operator T const&() const { return m_t; }

    operator T&() { return m_t; }

    T const& get() const { return m_t; }

    T& get() { return m_t; }

    template <class... Ts> pod& emplace(Ts&&... ts)
    {
        ::new (&m_t) T(std::forward<Ts>(ts)...);
        return *this;
    }

    void destroy() { m_t.~T(); }
};

#endif // POD_H
