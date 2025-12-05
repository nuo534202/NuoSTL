#ifndef NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_
#define NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_

#include <compare>
#include <memory>
#include <initializer_list>
#include <iostream>
#include <iterator>

/* support <ranges> */
#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
    #include <ranges>
#endif

#include "nuo_typedefs.hpp"
#include "core/algorithms/nuo_min.hpp"

namespace nuostl
{

template <typename T, typename Allocator = std::allocator<T>>
class nuo_vector
{
public:
    using value_type                = T;
    using allocator_type            = Allocator;
    using pointer                   = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer             = typename std::allocator_traits<Allocator>::const_pointer;
    using reference                 = value_type &;
    using const_reference           = const value_type &;
    using difference_type           = size_t;
    using iterator                  = T *;
    using const_iterator            = const T *;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

private:
    pointer         _data;
    size_t       _size;
    size_t       _capacity;
    allocator_type  _alloc;

public:
    /* construct / copy / destroy */
    /* TODO: replace to_string with nuo_to_string */
    /* TODO: Use reserve() to reallocate memory */
    constexpr nuo_vector() noexcept(noexcept(Allocator()))
        : nuo_vector(Allocator()) {}

    constexpr explicit nuo_vector(const Allocator &alloc) noexcept
        : _data(nullptr), _size(0), _capacity(0), _alloc(alloc) {}

    constexpr explicit nuo_vector(size_t n, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (n == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _capacity = _size = n;
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);        

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

    constexpr nuo_vector(size_t n, const T &value, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (n == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _capacity = _size = n;
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, value);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

    template <
        typename InputIter,
        typename = typename std::enable_if<!std::is_integral<InputIter>::value>::type
    >
    constexpr nuo_vector(InputIter first, InputIter last, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if (first > last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        if (first == last)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _size = _capacity = std::distance(first, last);
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        idx_t i = 0;
        try
        {
            for (InputIter it = first; it != last; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

#if _LIBCPP_STD_VER >= 23
    template <std::ranges::input_range R>
    constexpr nuo_vector(std::from_range_t, R &&rg, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        if constexpr (std::ranges::sized_range<R>)
        {
            _capacity = _size = std::ranges::size(rg);
            if (_size == 0)
            {
                _size = _capacity = 0;
                _data = nullptr;
                return;
            }
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            idx_t i = 0;
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
            catch (const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destory(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
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

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
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

    constexpr nuo_vector(const nuo_vector &nv, const std::type_identity_t<Allocator> &alloc)
        : _size(nv._size), _capacity(nv._capacity), _alloc(alloc)
    {
        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

    constexpr nuo_vector(nuo_vector &&nv, const std::type_identity_t<Allocator> &alloc)
        : _size(nv._size), _capacity(nv._capacity), _alloc(alloc)
    {
        if (_alloc == nv._alloc)
        {
            _data = nv._data;
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

            idx_t i = 0;
            try
            {
                for (; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i,
                                                                std::move(nv._data[i]));
            }
            catch (const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destory(_alloc, _data + i);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
    }

    constexpr nuo_vector(std::initializer_list<T> il, const Allocator &alloc = Allocator())
        : _alloc(alloc)
    {
        _size = _capacity = il.size();

        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        idx_t i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
    }

    constexpr ~nuo_vector()
    {
        if (_data != nullptr)
        {
            for (idx_t i = 0; i < _size; i++)
            {
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
            
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
        }

        _size = _capacity = 0;
        _data = nullptr;
    }

    constexpr nuo_vector &operator=(const nuo_vector &nv)
    {
        if (this == &nv)
            return *this;

        for (idx_t i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        _alloc = nv.get_allocator();
        reserve(nv.capacity());
        _size = nv.size();

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv[i]);
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        return *this;
    }

    constexpr nuo_vector &operator=(nuo_vector &&nv) noexcept(
        std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
        std::allocator_traits<Allocator>::is_always_equal::value)
    {
        if (this == &nv)
            return *this;

        for (idx_t i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

        constexpr bool propagate =
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value;
        
        constexpr bool always_equal = 
            std::allocator_traits<Allocator>::is_always_equal::value;

        if (propagate || always_equal || _alloc == nv._alloc)
        {
            if (_data != nullptr)
            {
                for (idx_t i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _data = nullptr;
            }

            _alloc = std::move(nv._alloc);
            _data = nv._data, _size = nv._size, _capacity = nv._capacity;
            nv._data = nullptr, nv._size = nv._capacity = 0;
        }
        else
        {
            if (_capacity >= nv._size)
            {
                for (idx_t i = 0; i < nuo_min(_size, nv._size); i++)
                    _data[i] = std::move(nv._data[i]);
                
                if (_size > nv._size)
                {
                    for (idx_t i = nv._size; i < _size; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < nv._size)
                {
                    idx_t i = _size;
                    try
                    {
                        for (; i < nv._size; i++)
                            std::allocator_traits<Allocator>::construct(
                                _alloc, _data + i, std::move(nv._data[i]));
                    }
                    catch(const std::exception& e)
                    {
                        for (idx_t j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        std::cerr << e.what() << std::endl;
                    }
                }
                _size = nv._size;
            }
            else
            {
                if (_data != nullptr)
                {
                    for (idx_t i = 0; i < _size; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                    
                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _data = nullptr;
                }

                _size = nv._size, _capacity = nv._capacity;

                _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

                idx_t i = 0;
                try
                {
                    for (; i < _size; i++)
                        std::allocator_traits<Allocator>::construct(
                            _alloc, _data + i, std::move(nv._data[i]));
                }
                catch (const std::exception& e)
                {
                    for (idx_t j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    std::cerr << e.what() << std::endl;
                }   
            }
        }

        return *this;
    }

    constexpr nuo_vector &operator=(std::initializer_list<T> il)
    {
        _size = _capacity = il.size();
        _alloc = Allocator();

        if (_size == 0)
        {
            _size = _capacity = 0;
            _data = nullptr;
            return *this;
        }

        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        idx_t i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        return *this;
    }

    template <
        typename InputIter,
        typename = typename std::enable_if<!std::is_integral<InputIter>::value>::type
    >
    constexpr void assign(InputIter first, InputIter last)
    {
        if (first > last)
            throw std::length_error("nuo_vector: first is smaller than or equal to last");

        difference_type n = std::distance(first, last);

        if (n > _capacity)
        {
            size_t new_capacity =
                _capacity ? (n + _capacity - 1)  / _capacity * _capacity : n;
            reserve(new_capacity);
        }

        for (idx_t i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
        
        idx_t i = 0;
        try
        {
            for (auto it = first; it != last && i < n; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0, _data = nullptr;
            std::cerr << e.what() << std::endl;
        }

        _size = n;        
    }

    template <std::ranges::input_range R>
    constexpr void assign_range(R &&rg)
    {
        /* get the size of range */
        difference_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        for (idx_t i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
        _size = 0;

        size_t new_capacity =
            _capacity ? (n + _capacity - 1) / _capacity * _capacity : n;
        reserve(new_capacity);

        idx_t i = _size;
        try
        {
            auto first = std::ranges::begin(rg), last = std::ranges::end(rg);
            for (auto it = first; it != last; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = nullptr;
            _size = _capacity = 0;
            std::cerr << e.what() << std::endl;
            throw;
        }
        
        _size += n;
    }

    constexpr void assign(size_t n, const T &u)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (_capacity < n)
        {
            for (idx_t i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = _size = n;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            idx_t i = 0;
            try
            {
                for (; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
            }
            catch (const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
        else
        {
            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (idx_t i = 0; i < nuo_min(_size, n); i++)
                    _data[i] = u;

                if (_size > n)
                {
                    for (idx_t i = _size; i < n; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < n)
                {
                    idx_t i = _size;
                    try
                    {
                        for (; i < n; i++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
                    }
                    catch (const std::exception& e)
                    {
                        for (idx_t j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        std::cerr << e.what() << std::endl;
                        throw;
                    }
                }
            }
            else
            {
                for (idx_t i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                idx_t i = 0;
                try
                {
                    for (; i < n; i++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
                }
                catch (const std::exception& e)
                {
                    for (idx_t j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    std::cerr << e.what() << std::endl;
                    throw;
                }
            }

            _size = n;
        }
    }

    constexpr void assign(std::initializer_list<T> il)
    {
        size_t n = il.size();

        if (_capacity < n)
        {
            for (idx_t i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = _size = n;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            idx_t i = 0;
            try
            {
                for (auto it = il.begin(); i < _size && it != il.end(); i++, it++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
            }
            catch (const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
        else
        {
            auto it = il.begin();

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (idx_t i = 0; i < nuo_min(_size, n) && it != il.end(); i++, it++)
                    _data[i] = *it;

                if (_size > n)
                {
                    for (idx_t i = _size; i < n; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < n)
                {
                    idx_t i = _size;
                    try
                    {
                        for (; i < n && it != il.end(); i++, it++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                    }
                    catch (const std::exception& e)
                    {
                        for (idx_t j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        std::cerr << e.what() << std::endl;
                        throw;
                    }
                }
            }
            else
            {
                for (idx_t i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                idx_t i = 0;
                try
                {
                    for (; i < n && it != il.end(); i++, it++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                }
                catch (const std::exception& e)
                {
                    for (idx_t j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    std::cerr << e.what() << std::endl;
                    throw;
                }
            }

            _size = n;
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
        if (this->_data == nullptr)
            return true;

        return _size == 0;
    }

    constexpr size_t size() const noexcept
    {
        return _size;
    }

    constexpr size_t max_size() const noexcept
    {
        return std::allocator_traits<Allocator>::max_size(_alloc);
    }

    constexpr size_t capacity() const noexcept
    {
        return _capacity;
    }

    constexpr void resize(size_t sz)
    {
        reserve(sz);
        
        idx_t i = _size;
        try
        {
            for (; i < sz; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = _size; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = nullptr;
            _size = _capacity = 0;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _size = sz;
    }

    constexpr void resize(size_t sz, const T &c)
    {
        reserve(sz);

        idx_t i = _size;
        try
        {
            for (; i < sz; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, c);
        }
        catch (const std::exception& e)
        {
            for (idx_t j = _size; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = nullptr;
            _size = _capacity = 0;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _size = sz;
    }

    constexpr void reserve(size_t n)
    {
        if (_capacity >= n)
            return;
        
        size_t new_capacity = n;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
            {
                std::allocator_traits<Allocator>::construct(_alloc, new_data + i, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, new_data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, new_data, new_capacity);
            new_data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
        _data = new_data, new_data = nullptr;
        _capacity = new_capacity;
    }

    constexpr void shrink_to_fit()
    {
        if (_size == 0)
        {
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = nullptr;
            _size = _capacity = 0;
            return;
        }

        size_t new_capacity = _size;
        if (_capacity == new_capacity)
            return;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        idx_t i = 0;
        try
        {
            for (; i < _size; i++)
            {
                std::allocator_traits<Allocator>::construct(_alloc, new_data + i, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch (const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, new_data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, new_data, new_capacity);
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _data = new_data = nullptr;
            _size = _capacity = 0;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _capacity = new_capacity;
    }


    /* element access */
    constexpr reference operator[](size_t n)
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + std::to_string(n) + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                std::to_string(n) + ") >= this->size() (which is " + std::to_string(n) + ")"
            );
        
        return _data[n];
    }

    constexpr const_reference operator[](size_t n) const
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + std::to_string(n) + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                std::to_string(n) + ") >= this->size() (which is " + std::to_string(n) + ")"
            );
        
        return _data[n];
    }

    constexpr const_reference at(size_t n) const
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + std::to_string(n) + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                std::to_string(n) + ") >= this->size() (which is " + std::to_string(n) + ")"
            );
        
        return _data[n];
    }

    constexpr reference at(size_t n)
    {
        if (n < 0)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " + std::to_string(n) + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                std::to_string(n) + ") >= this->size() (which is " + std::to_string(n) + ")"
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
        return *(_data + _size - 1);
    }


    /* data access */
    constexpr T *data() noexcept
    {
        if (empty())
            return nullptr;

        return _data;
    }

    constexpr const T *data() const noexcept
    {
        if (empty())
            return nullptr;

        return _data;
    }


    /* modifiers */
    template <typename... Args>
    constexpr reference emplace_back(Args &&...args)
    {
        if (_size >= _capacity)
        {
            size_t new_capacity = _capacity ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(
                _alloc, _data + _size, std::forward<Args>(args)...);
        }
        catch (const std::exception& e)
        {
            for (idx_t i = 0; i <= _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
        
        _size++;

        return _data[_size - 1];
    }

    constexpr void push_back(const T &x)
    {
        if (_size >= _capacity)
        {
            size_t new_capacity = _capacity > 0 ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(_alloc, _data + _size, x);
        }
        catch (const std::exception& e)
        {
            for (idx_t i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _size++;
    }

    constexpr void push_back(T &&x)
    {
        if (_size >= _capacity)
        {
            size_t new_capacity = _capacity ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(_alloc, _data + _size, std::move(x));
        }
        catch (const std::exception& e)
        {
            for (idx_t i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _size++;
    }

    template <std::ranges::input_range R>
    constexpr void append_range(R &&rg)
    {
        difference_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        if (_size >= _capacity)
        {
            size_t new_capacity =
                _capacity ? ((_size + _capacity + n - 1) / _capacity * _capacity) : n;
            reserve(new_capacity);
        }

        idx_t i = _size;
        try
        {
            auto first = std::ranges::begin(rg), last = std::ranges::end(rg);
            for (auto it = first; i < _size + n && it != last; i++, it++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        _size += n;
    }

    constexpr void pop_back()
    {
        if (_size == 0)
            throw std::length_error("nuo_vector: nuo_vector is empty!");
        
        std::allocator_traits<Allocator>::destroy(_alloc, _data + _size - 1);
        _size--;
    }

    template <class... Args>
    constexpr iterator emplace(const_iterator position, Args &&...args)
    {
        difference_type pos = position - _data;

        if (_size >= _capacity)
        {
            size_t new_capacity = _capacity ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>)
        {
            try
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + _size);
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (idx_t i = _size; i > pos; i--)
                    _data[i] = _data[i - 1];
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (idx_t i = _size; i > pos; i--)
                    _data[i] = std::move(_data[i - 1]);
            }

            try
            {
                std::allocator_traits<Allocator>::destroy(_alloc, _data + pos);
                std::allocator_traits<Allocator>::construct(_alloc, _data + pos, std::forward<Args>(args)...);
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }
        }
        else
        {
            idx_t i = _size;
            try
            {
                for (; i > pos; i--)
                {
                    std::allocator_traits<Allocator>::construct(
                        _alloc, _data + i, std::move(_data[i - 1]));
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i - 1);
                }
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                for (idx_t j = i + 1; j < _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }
        }

        return _data + pos;
    }

    constexpr iterator insert(const_iterator position, const T &x)
    {
        difference_type pos = position - _data;

        if (_size >= _capacity)
        {
            size_t new_capacity = _capacity ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>)
        {
            try
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + _size);
                _size++;
            }
            catch(const std::exception& e)
            {
                for (idx_t i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (idx_t i = _size - 1; i > pos; i--)
                    _data[i] = _data[i - 1];
                _data[pos] = x;
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (idx_t i = _size - 1; i > pos; i--)
                    _data[i] = std::move(_data[i - 1]);
                _data[pos] = x;
            }
            
        }
        else
        {
            idx_t i = _size;
            try
            {
                for (; i > pos; i--)
                {
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, _data[i - 1]);
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i - 1);
                }
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (idx_t j = i + 1; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }

            try
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + pos, x);
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }            
            
            _size++;
        }

        return _data + pos;
    }

    constexpr iterator insert(const_iterator position, T &&x)
    {
        const T& nx = std::move(x);
        return insert(position, nx);
    }

    constexpr iterator insert(const_iterator position, size_t n, const T &x)
    {
        difference_type pos = position - _data;

        if (_size + n >= _capacity)
        {
            size_t new_capacity =
                _capacity > 0 ? (_size + _capacity + n - 1) / _capacity * _capacity : n;
            reserve(new_capacity);
        }

        if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>)
        {
            idx_t i = _size;
            try
            {
                for (; i < _size + n; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i);
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (idx_t i = _size; i > pos; i--)
                    _data[i + n - 1] = _data[i - 1];
                for (idx_t i = pos; i < pos + n; i++)
                    _data[i] = x;
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (idx_t i = _size; i > pos; i--)
                    _data[i + n - 1] = std::move(_data[i - 1]);
                for (idx_t i = pos; i < pos + n; i++)
                    _data[i] = x;
            }
            
            _size += n;
        }
        else
        {
            idx_t i = _size - 1;
            try
            {
                for (; i > pos; i--)
                {
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i + n, _data[i]);
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j <= i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (idx_t j = i + n + 1; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }

            i = pos;
            try
            {
                for (; i < pos + n; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, x);
            }
            catch(const std::exception& e)
            {
                for (idx_t j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (idx_t j = pos + n - 1; j < _size + n; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                std::cerr << e.what() << std::endl;
                throw;
            }  
            
            _size += n;
        }

        return _data + pos;
    }
    
    template <
        typename InputIter,
        typename = typename std::enable_if<!std::is_integral<InputIter>::value>::type
    >
    constexpr iterator insert(const_iterator position, InputIter first, InputIter last)
    {
        if (first >= last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        difference_type n = std::distance(first, last), pos = position - _data;

        if (_size >= _capacity)
        {
            size_t new_capacity =
                _capacity ? ((_size + _capacity + n - 1) / _capacity * _capacity) : n;
            reserve(new_capacity);
        }

        idx_t i = _size;
        try
        {
            for (;i > pos; i--)
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + i + n - 1, _data[i - 1]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i - 1);
            }
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i - 1; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (idx_t j = _size + n - 1; j > _size + i - 1; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        i = pos;
        try
        {
            for (InputIter it = first; it != last && i < pos + n; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (idx_t j = _size + n - 1; j >= _size + pos; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
        
        _size += n;

        return _data + pos;
    }

    template <std::ranges::input_range R>
    constexpr iterator insert_range(const_iterator position, R &&rg)
    {
        difference_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        difference_type pos = position - _data;

        if (_size + n > _capacity)
        {
            size_t new_capacity =
                _capacity ? ((_size + _capacity + n - 1) / _capacity * _capacity) : n;
            reserve(new_capacity);
        }

        idx_t i = _size;
        try
        {
            for (;i > pos; i--)
            {
                std::allocator_traits<Allocator>::construct(_alloc,
                                                            _data + i + n - 1,
                                                            _data[i - 1]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i - 1);
            }
        }
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (idx_t j = _size + n - 1; j > _size + i; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }

        i = pos;
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
        catch(const std::exception& e)
        {
            for (idx_t j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (idx_t j = _size + n - 1; j >= _size + pos; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            std::cerr << e.what() << std::endl;
            throw;
        }
        
        _size += n;

        return _data + pos;
    }

    constexpr iterator insert(const_iterator position, std::initializer_list<T> il)
    {
        return insert(position, il.begin(), il.end());
    }

    constexpr iterator erase(const_iterator position)
    {
        return erase(position, position + 1);
    }

    constexpr iterator erase(const_iterator first, const_iterator last)
    {
        if (first > last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        difference_type start_idx = first - _data;
        difference_type n = last - first;

        iterator first_iter = _data + start_idx;
        iterator last_iter = _data + start_idx + n;

        iterator i = first_iter;
        try
        {
            for (; i != last_iter; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, i);

            for (i = last_iter; i != _data + _size; i++)
            {
                std::allocator_traits<Allocator>::construct(_alloc, i - n, *i);
                std::allocator_traits<Allocator>::destroy(_alloc, i);
            }
        }
        catch(const std::exception& e)
        {
            for (iterator j = _data; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, j);

            for (iterator j = last_iter; j < first_iter + n; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, j);

            for (iterator j = i + n; j < _data + _size; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, j);

            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }

        _size -= n;

        return _data + start_idx;
    }

    constexpr void swap(nuo_vector &nv) noexcept(
        std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
        std::allocator_traits<Allocator>::is_always_equal::value)
    {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
        {
            std::swap(_alloc, nv._alloc);
        }
        else if constexpr (!std::allocator_traits<Allocator>::is_always_equal::value)
        {
            throw std::logic_error(
                "nuo_vector::swap: allocators can not be propagated and they are not equal"
            );
        }

        std::swap(_data, nv._data);
        std::swap(_size, nv._size);
        std::swap(_capacity, nv._capacity);
    }

    friend constexpr void swap(nuo_vector &lhs, nuo_vector &rhs) noexcept(
        std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
        std::allocator_traits<Allocator>::is_always_equal::value)
    {
        lhs.swap(rhs);
    }

    constexpr void clear() noexcept
    {
        for (idx_t i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
        _size = 0;
    }
};

#if _LIBCPP_STD_VER >= 17
template<
    typename InputIter,
    typename Allocator = std::allocator<typename std::iterator_traits<InputIter>::value_type>
>
nuo_vector(InputIter, InputIter, Allocator = Allocator())
    -> nuo_vector<typename std::iterator_traits<InputIter>::value_type, Allocator>;
#endif

#if _LIBCPP_STD_VER >= 23
template<
    std::ranges::input_range R,
    typename Allocator = std::allocator<std::ranges::range_value_t<R>>
>
nuo_vector(std::from_range_t, R&&, Allocator = Allocator())
    -> nuo_vector<std::ranges::range_value_t<R>, Allocator>;
#endif

} /* namespace nuostl */

#endif