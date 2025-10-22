#ifndef NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_
#define NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_

#include <bit>
#include <compare>
#include <memory>
#include <initializer_list>
#include <iterator>

/* support <ranges> */
#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    #include <ranges>
    #ifndef NUOSTL_HAS_RANGES
        #define NUOSTL_HAS_RANGES 1
    #endif
#else
    #ifndef NUOSTL_HAS_RANGES
        #define NUOSTL_HAS_RANGES 0
    #endif
#endif

#include "nuo_typedefs.hpp"

namespace nuostl
{

template <typename T, typename Allocator = std::allocator<T>>
class nuo_vector
{

public:
    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = size_t;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    pointer _data;
    size_type _size;
    size_type _capacity;
    allocator_type _alloc;

public:
    /* construct / copy / destroy */
    constexpr nuo_vector() noexcept(noexcept(Allocator()))
        : nuo_vector(Allocator()) {}

    constexpr explicit nuo_vector(const Allocator &alloc) noexcept
        : _size(0), _capacity(0), _data(nullptr), _alloc(alloc) {}

    constexpr explicit nuo_vector(size_type n, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (n == 0)
        {
            nuo_vector(alloc);
            return;
        }

        _size = n;
        _capacity = std::bit_ceil(n);
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr nuo_vector(size_type n, const T &value, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (n == 0)
        {
            nuo_vector(alloc);
            return;
        }

        _size = n;
        _capacity = std::bit_ceil(_size);
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, value);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    template <typename InputIter>
    constexpr nuo_vector(InputIter first, InputIter last, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (first > last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        if (first == last)
        {
            nuo_vector(alloc);
            return;
        }

        _size = last - first;
        _capacity = std::bit_ceil(_size);
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i;
        InputIter it;
        try
        {
            for (it = first; it < last; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

#if defined(NUOSTL_HAS_RANGES) && NUOSTL_HAS_RANGES == 1
    template <std::ranges::container - compatible - range<T> R>
    constexpr nuo_vector(std::from_range_t, R &&rg, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if constexpr (std::ranges::sized_range<R>)
        {
            _size = std::ranges::size(rg);
            if (_size == 0)
            {
                nuo_vector(alloc);
                return;
            }

            _capacity = std::bit_ceil(_size);
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                for (auto &&elem : rg)
                {
                    std::allocator_traits<Allocator>::construct(
                        _alloc,
                        _data + i,
                        std::forward<decltype(elem)>(elem)
                    );
                    i++;
                }
            }
            catch
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destory(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            _size = _capacity = 0;
            _data = nullptr;
            for (auto &&elem : rg)
                push_back(std::forward<decltype(elem)>(elem));
        }
    }
#endif

    constexpr nuo_vector(const nuo_vector &nv)
        : _size(nv._size), _capacity(nv._capacity), _alloc(nv._alloc)
    {
        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr nuo_vector(nuo_vector &&nv) noexcept
        : _data(nv._data),
            _size(nv._size),
            _capacity(nv._capacity),
            _alloc(std::move(nv._alloc))
    {
        nv._data = nullptr, nv._size = nv._capacity = 0;
    }

    constexpr nuo_vector(const nuo_vector &nv, const type_identity_t<Allocator> &alloc)
        : _size(nv._size), _capacity(nv._capacity), _alloc(alloc)
    {
        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i,
                                                            std::move(nv._data[i]));
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr nuo_vector(nuo_vector &&nv, const type_identity_t<Allocator> &alloc)
        : _size(nv._size), _capacity(nv._capacity), _alloc(alloc)
    {
        if (_alloc == nv._alloc)
        {
            _data = _data;
            nv._data = nullptr;
            nv._size = nv._capacity = 0;
        }
        else
        {
            if (_size == 0)
            {
                _size = _capacity = 0;
                _data = nullptr;
                return;
            }

            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i;
            try
            {
                for (i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i,
                                                                std::move(nv._data[i]));
            }
            catch
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destory(_alloc, _data + i);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
    }

    constexpr nuo_vector(std::initializer_list<T> il, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        _size = il.size();
        _capacity = std::bit_ceil(_size);

        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception &e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr ~nuo_vector()
    {
        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        if (_data != nullptr)
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
    }

    constexpr nuo_vector &operator=(const nuo_vector &nv)
        : _size(nv._size), _capacity(nv._capacity), _alloc(nv._alloc)
    {
        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return *this;
        }

        _data = nv._data;

        return *this;
    }

    constexpr nuo_vector &operator=(nuo_vector &&nv) noexcept(
        std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
        std::allocator_traits<Allocator>::is_always_equal::value)
    {
        if (this == &nv)
            return *this;

        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        constexpr bool propagate =
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value;

        if constexpr (propagate)
        {
            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _data = nv._data;
            _size = nv._size;
            _capacity = nv._capacity;
            _alloc = std::move(nv._alloc);

            nv._size = nv._capacity = 0;
            nv._data = nullptr;
        }
        else if (_alloc == nv._alloc)
        {
            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _data = nv._data;
            _size = nv._size;
            _capacity = nv._capacity;

            nv._size = nv._capacity = 0;
            nv._data = nullptr;
        }
        else
        {
            if (_capacity < nv._size)
            {
                if (_data != nullptr)
                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

                _capacity = nv._capacity;
                _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
            }

            _size = nv._size;

            size_type i = 0;
            try
            {
                for (i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc,
                                                                _data + i,
                                                                std::move(nv._data[i]));
            }
            catch
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
    }

    constexpr nuo_vector &operator=(std::initializer_list<T> il)
        : _alloc(Allocator())
    {
        _size = il.size();
        _capacity = std::bit_ceil(_size);

        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return *this;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception &e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }

        return *this;
    }

    template <class InputIter>
    constexpr void assign(InputIter first, InputIter last)
    {
        if (first > last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        _size = last - first;

        if (_capacity < last - first)
        {
            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = std::bit_ceil(_size);
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
        }

        size_type i = 0;
        try
        {
            for (InputIter it = first; it != last; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    template <container - compatible - range<T> R>
    constexpr void assign_range(R &&rg)
    {
        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        if constexpr (std::ranges::sized_range<R>)
        {
            _size = std::ranges::size(rg);

            if (_capacity < _size)
            {
                if (_data != nullptr)
                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

                _capacity = std::bit_ceil(_size);
                _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
            }

            size_type i = 0;
            try
            {
                for (auto &&elem : rg)
                {
                    std::allocator_traits<Allocator>::construct(
                        _alloc,
                        _data + i,
                        std::forward<decltype(elem)>(elem)
                    );
                    i++;
                }
            }
            catch
            {
                for (size_type j = 0; j < i; ++j)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            _size = 0;
            try
            {
                for (auto &&elem : rg)
                {
                    if (_capacity < _size)
                    {
                        size_type new_capacity = _capacity == 0 ? 1 : capacity << 1;
                        pointer new_data =
                            std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

                        for (size_type i = 0; i < _size; i++)
                        {
                            std::allocator_traits<Allocator>::construct(_alloc, new_data,
                                                                        std::move(_data[i]));
                            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                        }

                        if (_data != nullptr)
                            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

                        _data = new_data;
                        _capacity = new_capacity;
                    }

                    std::allocator_traits<Allocator>::construct(_alloc, _data + _size,
                                                            std::forward<decltype(elem)>(elem));

                    _size++;
                }
            }
            catch
            {
                for (size_type j = 0; j < _size; ++j)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocte(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
    }

    constexpr void assign(size_type n, const T &u)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        _size = n;

        if (_capacity < _size)
        {
            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocte(_alloc, _data, _capacity);

            _capacity = std::bit_ceil(_size);
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _data, _capacity);
        }

        size_type i = 0;
        try
        {
            for (i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, std::move(u));
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr void assign(std::initializer_list<T> il)
    {
        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        _size = il.size();

        if (_capacity < _size)
        {
            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = std::bit_ceil(_size);
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);
        }

        size_type i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return _alloc;
    }


    /* iterators */
    constexpr iterator begin() noexcept
    {
        return _data;
    }

    constexpr const_iterator begin() const noexcept
    {
        return _data;
    }

    constexpr iterator end() noexcept
    {
        return _data + _size;
    }

    constexpr const_iterator end() const noexcept
    {
        return _data + _size;
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return std::reverse_iterator(_data + _size);
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return std::reverse_iterator(_data + _size);
    }

    constexpr reverse_iterator rend() noexcept
    {
        return std::reverse_iterator(_data);
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return std::reverse_iterator(_data);
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return _data;
    }

    constexpr const_iterator cend() const noexcept
    {
        return _data + _size;
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return std::reverse_iterator(_data + _size);
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return std::reverse_iterator(_data);
    }


    /* capacity */
    constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    constexpr size_type size() const noexcept
    {
        return _size;
    }

    constexpr size_type max_size() const noexcept
    {
        return std::allocator<Allocator>::max_size(_alloc);
    }

    constexpr size_type capacity() const noexcept
    {
        return _capacity;
    }

    constexpr void resize(size_type sz)
    {

        if (sz < _size)
        {
            for (size_type i = sz; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            _size = sz;
        }
        else if (sz > _size)
        {
            size_type new_capacity = std::bit_ceil(sz);
            if (new_capacity > _capacity)
            {
                pointer new_data =
                    std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

                size_type i;
                try
                {
                    for (i = 0; i < _size; i++)
                    {
                        std::allocator_traits<Allocator>::construct(_alloc,
                                                                    new_data + i,
                                                                    _data[i]);
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                    }
                }
                catch
                {
                    for (size_type j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, new_data + j);

                    for (size_type j = i; j < _size; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    std::allocator_traits<Allocator>::deallocate(_alloc, new_data, new_capacity);
                    _data = new_data = nullptr;
                    _capacity = _size = 0;
                    throw;
                }

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _data = new_data, new_data = nullptr;
                _capacity = new_capacity;
            }

            for (size_type i = _size; i < sz; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i);

            _size = sz;
        }
    }

    constexpr void resize(size_type sz, const T &c)
    {
        if (sz > _size)
        {
            size_type new_capacity = std::bit_ceil(sz);
            if (_capacity < new_capacity)
            {
                pointer new_data =
                    std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _data = new_data, new_data = nullptr;
                _capacity = new_capacity;
            }
        }

        _size = sz;

        size_type i;
        try
        {
            for (i = 0; i < sz; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, c);
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = nullptr;
            _capacity = _size = 0;
            throw;
        }
    }

    constexpr void reserve(size_type n)
    {
        if (_capacity >= n)
            return;
        
        size_type new_capacity = std::bit_ceil(n);

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
            {
                std::allocator_traits<Allocator>::construct(_alloc, new_data + i, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch
        {
            for (int j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, new_data + j);
            
            std::allocator_traits<Allocator>::deallocate(new_data, new_capacity);
            new_data = nullptr;
            throw;
        }

        std::allocator_traits<Allocator>::deallocte(_alloc, _data, _capacity);
        _data = new_data, new_data = nullptr;
        _capacity = new_capacity;
    }

    constexpr void shrink_to_fit()
    {
        if (_size == 0)
        {
            std::allocator_traits<Allocator>::deallocate(_alloc, _capacity);
            _data = nullptr;
            _size = _capacit = 0;
            return;
        }

        size_type new_capacity = std::bit_ceil(_size);
        if (_capacity == new_capacity)
            return;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        size_type i;
        try
        {
            for (i = 0; i < _size; i++)
            {
                std::allocator_traits<Allocator>::construct(_alloc, new_data + i, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destory(_alloc, new_data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, new_data, new_capacity);
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = new_data = nullptr;
            _size = _capacity = 0;
            throw;
        }

        _capacity = new_capacity;
    }


    /* element access */
    constexpr reference operator[](size_type n)
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + n + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                n + ") >= this->size() (which is " + n + ")"
            );
        
        return _data[n];
    }

    constexpr const_reference operator[](size_type n) const
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + n + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                n + ") >= this->size() (which is " + n + ")"
            );
        
        return _data[n];
    }

    constexpr const_reference at(size_type n) const
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + n + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                n + ") >= this->size() (which is " + n + ")"
            );
        
        return _data[n];
    }

    constexpr reference at(size_type n)
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + n + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                n + ") >= this->size() (which is " + n + ")"
            );
        
        return _data[n];
    }

    constexpr reference front()
    {
        return *_data;
    }

    constexpr const_reference front() const
    {
        return *_data;
    }

    constexpr reference back()
    {
        return *(_data + _size - 1);
    }

    constexpr const_reference back() const
    {
        return *prev(_data + _size - 1);
    }


    /* data access */
    constexpr T *data() noexcept;
    constexpr const T *data() const noexcept;


    /* modifiers */
    template <class... Args>
    constexpr reference emplace_back(Args &&...args);
    constexpr void push_back(const T &x);
    constexpr void push_back(T &&x);
    template <container - compatible - range<T> R>
    constexpr void append_range(R &&rg);
    constexpr void pop_back();

    template <class... Args>
    constexpr iterator emplace(const_iterator position, Args &&...args);
    constexpr iterator insert(const_iterator position, const T &x);
    constexpr iterator insert(const_iterator position, T &&x);
    constexpr iterator insert(const_iterator position, size_type n, const T &x);
    template <class InputIter>
    constexpr iterator insert(const_iterator position, InputIter first, InputIter last);
    template <container - compatible - range<T> R>
    constexpr iterator insert_range(const_iterator position, R &&rg);
    constexpr iterator insert(const_iterator position, std::initializer_list<T> il);
    constexpr iterator erase(const_iterator position);
    constexpr iterator erase(const_iterator first, const_iterator last);
    constexpr void swap(nuo_vector &) noexcept(
        allocator_traits<Allocator>::propagate_on_container_swap::value ||
        allocator_traits<Allocator>::is_always_equal::value);
    constexpr void clear() noexcept;
};

// template<class InputIter, class Allocator = std::allocator</*iter-value-type*/<InputIter>>>
//   nuo_vector(InputIter, InputIter, Allocator = Allocator())
//     -> nuo_vector</*iter-value-type*/<InputIter>, Allocator>;

// template<std::ranges::input_range R, class Allocator = std::allocator<ranges::range_value_t<R>>>
//   nuo_vector(from_range_t, R&&, Allocator = Allocator())
//     -> nuo_vector<ranges::range_value_t<R>, Allocator>;

} /* namespace nuostl */

#endif