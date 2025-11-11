#ifndef NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_
#define NUOSTL_CORE_SEQ_CONT_NUO_VECTOR_HPP_

#include <compare>
#include <memory>
#include <initializer_list>
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
    using size_type                 = size_t;
    using difference_type           = size_t;
    using iterator                  = T *;
    using const_iterator            = const T *;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

private:
    pointer         _data;
    size_type       _size;
    size_type       _capacity;
    allocator_type  _alloc;

public:
    /* construct / copy / destroy */
    /* TODO: replace to_string with nuo_to_string */
    /* TODO: Use reserve() to reallocate memory */
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
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _capacity = _size = n;
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);        

        size_type i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i);
        }
        catch (const std::exception& e)
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
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _capacity = _size = n;
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, value);
        }
        catch (const std::exception& e)
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
            _size = _capacity = 0;
            _data = nullptr;
            return;
        }

        _size = _capacity = last - first;
        _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

        size_type i = 0;
        try
        {
            for (InputIter it = first; it < last; it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception& e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
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
            catch (const std::exception& e)
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

        size_type i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
        }
        catch (const std::exception& e)
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
        : _size(nv._size),
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

        size_type i = 0;
        try
        {
            for (; i < _size; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
        }
        catch (const std::exception& e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
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

            size_type i = 0;
            try
            {
                for (; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i,
                                                                std::move(nv._data[i]));
            }
            catch (const std::exception& e)
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
        _size = _capacity = il.size();

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
        catch (const std::exception& e)
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
        if (_data != nullptr)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
        }

        _size = _capacity = 0;
        _data = nullptr;
    }

    constexpr nuo_vector &operator=(const nuo_vector &nv)
    {
        if (_capacity < nv._size)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = nv._capacity, _size = nv._size;
            
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                for (; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, nv._data[i]);
            }
            catch (const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                throw;
            }
        }
        else
        {
            for (size_type i = 0; i < nuo_min(_size, nv._size); i++)
                _data[i] = nv._data[i];
            
            if (_size > nv._size)
            {
                for (size_type i = nv._size; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
            else if (_size < nv._size)
            {
                size_type i = _size;
                try
                {
                    for (; i < nv._size; i++)
                        std::allocator_traits<Allocator>::construct(
                            _alloc, _data + i, nv._data[i]);
                }
                catch (const std::exception& e)
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
        
        constexpr bool always_equal = 
            std::allocator_traits<Allocator>::is_always_equal::value;

        if (propagate || always_equal || _alloc == nv._alloc)
        {
            if (_data != nullptr)
            {
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _data = nullptr;
            }

            _alloc = std::move(nv._alloc);
            _data = nv._data, _size = nv._size, _capacity = nv._capacity;
            _data = nullptr, nv._size = nv._capacity = 0;
        }
        else
        {
            if (_capacity >= nv._size)
            {
                for (size_type i = 0; i < nuo_min(_size, nv._size); i++)
                    _data[i] = std::move(nv._data[i]);
                
                if (_size > nv._size)
                {
                    for (size_type i = nv._size; i < _size; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < nv._size)
                {
                    size_type i = _size;
                    try
                    {
                        for (; i < nv._size; i++)
                            std::allocator_traits<Allocator>::construct(
                                _alloc, _data + i, std::move(nv._data[i]));
                    }
                    catch(const std::exception& e)
                    {
                        for (size_type j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        throw;
                    }
                }
                _size = nv._size;
            }
            else
            {
                if (_data != nullptr)
                {
                    for (size_type i = 0; i < _size; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                    
                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _data = nullptr;
                }

                _size = nv._size, _capacity = nv._capacity;

                _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

                size_type i = 0;
                try
                {
                    for (; i < _size; i++)
                        std::allocator_traits<Allocator>::construct(
                            _alloc, _data + i, std::move(nv._data[i]));
                }
                catch (const std::exception& e)
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

        size_type i = 0;
        try
        {
            for (auto it = il.begin(); it != il.end(); it++, i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch (const std::exception& e)
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
            throw std::length_error("nuo_vector: first is smaller than or equal to last");

        if (_size + last - first > _capacity)
        {
            size_type new_capacity =
                _size + last - first > 0 ?
                (_size + last - first + _capacity - 1)  / _capacity * _capacity :
                1;
            reserve(new_capacity);
        }

        if (_capacity < last - first)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = _size = last - first;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                for (InputIter it = first; it != last; it++, i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
            }
            catch (const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            using iter_value_type = typename std::iterator_traits<InputIter>::value_type;

            if constexpr (std::is_copy_assignable_v<iter_value_type>)
            {
                InputIter it = first;
                for (size_type i = 0; i < nuo_min(_size, last - first); i++, it++)
                    _data[i] = *it;

                if (_size > last - first)
                {
                    for (size_type i = _size; i < last - first; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < last - first)
                {
                    size_type i = _size;
                    try
                    {
                        for (; it != last && i < _capacity; it++, i++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                    }
                    catch (const std::exception& e)
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
            else
            {
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                
                size_type i = 0;
                try
                {
                    for (InputIter it = first; it != last && i < _capacity; it++, i++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                }
                catch (const std::exception& e)
                {
                    for (size_type j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    throw;
                }
            }

            _size = last - first;
        }
    }

    template <std::ranges::input_range R>
    constexpr void assign_range(R &&rg)
    {
        /* get the size of range */
        size_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        if (_capacity < n)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocte(_alloc, _data, _capacity);

            _size = _capacity = n;

            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                auto first = std::ranges::begin(rg), last = std::ranges::end(rg);
                for (auto it = first; it != last && i < _capacity; it++, i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
            }
            catch (const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            using range_value_type = std::ranges::range_value_t<R>;
            auto first = std::ranges::begin(rg), last = std::ranges::end(rg);

            if constexpr (std::is_copy_assignable_v<range_value_type>)
            {
                auto it = first;
                for (size_type i = 0; i < nuo_min(_size, n); i++, it++)
                    _data[i] = *it;

                if (_size > n)
                {
                    for (size_type i = n; i < _size; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < n)
                {
                    size_type i = _size;
                    try
                    {
                        for (; i < n; i++, it++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                    }
                    catch (const std::exception& e)
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
            else
            {
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                
                size_type i = 0;
                try
                {
                    for (auto it = first; i < n; i++, it++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                }
                catch (const std::exception& e)
                {
                    for (size_type j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    throw;
                }
            }

            _size = n;
        }
    }

    constexpr void assign(size_type n, const T &u)
    {
        if (n < 0)
            throw std::length_error("nuo_vector: n is negative");

        if (_capacity < n)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = _size = n;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                for (; i < _size; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
            }
            catch (const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (size_type i = 0; i < nuo_min(_size, n); i++)
                    _data[i] = u;

                if (_size > n)
                {
                    for (size_type i = _size; i < n; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < n)
                {
                    size_type i = _size;
                    try
                    {
                        for (; i < n; i++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
                    }
                    catch (const std::exception& e)
                    {
                        for (size_type j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        throw;
                    }
                }
            }
            else
            {
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                size_type i = 0;
                try
                {
                    for (; i < n; i++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, u);
                }
                catch (const std::exception& e)
                {
                    for (size_type j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
                    throw;
                }
            }

            _size = n;
        }
    }

    constexpr void assign(std::initializer_list<T> il)
    {
        size_type n = il.size();

        if (_capacity < n)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            if (_data != nullptr)
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _capacity = _size = n;
            _data = std::allocator_traits<Allocator>::allocate(_alloc, _capacity);

            size_type i = 0;
            try
            {
                for (auto it = il.begin(); i < _size && it != il.end(); i++, it++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
            }
            catch (const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            auto it = il.begin();

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (size_type i = 0; i < nuo_min(_size, n) && it != il.end(); i++, it++)
                    _data[i] = *it;

                if (_size > n)
                {
                    for (size_type i = _size; i < n; i++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                }
                else if (_size < n)
                {
                    size_type i = _size;
                    try
                    {
                        for (; i < n && it != il.end(); i++, it++)
                            std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                    }
                    catch (const std::exception& e)
                    {
                        for (size_type j = 0; j < i; j++)
                            std::allocator_traits<Allocator>::destroy(_alloc, _data);

                        std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                        _size = _capacity = 0;
                        _data = nullptr;
                        throw;
                    }
                }
            }
            else
            {
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                size_type i = 0;
                try
                {
                    for (; i < n && it != il.end(); i++, it++)
                        std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                }
                catch (const std::exception& e)
                {
                    for (size_type j = 0; j < i; j++)
                        std::allocator_traits<Allocator>::destroy(_alloc, _data);

                    std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                    _size = _capacity = 0;
                    _data = nullptr;
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
        if (this == nullptr)
            return true;

        return _size == 0;
    }

    constexpr size_type size() const noexcept
    {
        if (this == nullptr)
            return (size_type) 0;

        return _size;
    }

    constexpr size_type max_size() const noexcept
    {
        return std::allocator<Allocator>::max_size(_alloc);
    }

    constexpr size_type capacity() const noexcept
    {
        if (this == nullptr)
            return (size_type) 0;

        return _capacity;
    }

    constexpr void resize(size_type sz)
    {
        if (sz < _size)
        {
            for (size_type i = sz; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
        }
        else if (sz > _size)
        {
            reserve(sz);
        }
        _size = sz;
    }

    constexpr void resize(size_type sz, const T &c)
    {
        if (sz < _size)
        {
            for (size_type i = sz; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            _size = sz;
        }
        else if (sz > _size)
        {
            size_type new_capacity = sz;
            if (new_capacity > _capacity)
            {
                pointer new_data =
                    std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

                size_type i = 0;
                try
                {
                    for (; i < _size; i++)
                    {
                        std::allocator_traits<Allocator>::construct(_alloc, new_data + i, c);
                        std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
                    }
                    for (; i < sz; i++)
                        std::allocator_traits<Allocator>::construct(_alloc, new_data + i, c);
                }
                catch (const std::exception& e)
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

    constexpr void reserve(size_type n)
    {
        if (_capacity >= n)
            return;
        
        size_type new_capacity = n;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        size_type i = 0;
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
            for (int j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, new_data + j);
            
            std::allocator_traits<Allocator>::deallocate(new_data, new_capacity);
            new_data = nullptr;
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
            std::allocator_traits<Allocator>::deallocate(_alloc, _capacity);
            _data = nullptr;
            _size = _capacity = 0;
            return;
        }

        size_type new_capacity = _size;
        if (_capacity == new_capacity)
            return;

        pointer new_data = std::allocator_traits<Allocator>::allocate(_alloc, new_capacity);

        size_type i = 0;
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
                "nuo_vector::_M_range_check: __n (which is " + std::to_string(n) + ") must be larger than 0"
            );
        
        if (n >= _size)
            throw std::out_of_range(
                "nuo_vector::_M_range_check: __n (which is " +
                std::to_string(n) + ") >= this->size() (which is " + std::to_string(n) + ")"
            );
        
        return _data[n];
    }

    constexpr const_reference operator[](size_type n) const
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

    constexpr const_reference at(size_type n) const
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

    constexpr reference at(size_type n)
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
        return *prev(_data + _size - 1);
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
            size_type new_capacity = (_capacity > 0) ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(
                _alloc, _data + _size, std::forward<Args>(args)...);
        }
        catch (const std::exception& e)
        {
            for (size_type i = 0; i <= _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
        
        _size++;

        return _data[_size - 1];
    }

    constexpr void push_back(const T &x)
    {
        if (_size >= _capacity)
        {
            size_type new_capacity = (_capacity > 0) ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(_alloc, _data + _size, x);
        }
        catch (const std::exception& e)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }

        _size++;
    }

    constexpr void push_back(T &&x)
    {
        if (_size >= _capacity)
        {
            size_type new_capacity = (_capacity > 0) ? (_capacity << 1) : 1;
            reserve(new_capacity);
        }

        try
        {
            std::allocator_traits<Allocator>::construct(_alloc, _data + _size, std::move(x));
        }
        catch (const std::exception& e)
        {
            for (size_type i = 0; i < _size; i++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }

        _size++;
    }

    template <std::ranges::input_range R>
    constexpr void append_range(R &&rg)
    {
        size_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        if (_size >= _capacity)
        {
            size_type new_capacity =
                (_size + n > 0) ? ((_size + _capacity + n - 1) / _capacity * _capacity) : 1;
            reserve(new_capacity);
        }

        size_type i = _size;
        try
        {
            auto first = std::ranges::begin(rg), last = std::ranges::end(rg);
            for (auto it = first; i < _size + n && it != last; i++, it++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
                
        }
        catch(const std::exception& e)
        {
            for (int j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
            _size = _capacity = 0;
            _data = nullptr;
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
        size_type pos = position - _data;

        if (_size >= _capacity)
        {
            size_type new_capacity = _capacity ? _capacity << 1 : 1;
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
                for (size_type j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (size_type i = _size; i > pos; i--)
                    _data[i] = _data[i - 1];
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (size_type i = _size; i > pos; i--)
                    _data[i] = std::move(_data[i - 1]);
            }

            try
            {
                std::allocator_traits<Allocator>::destroy(_alloc, _data + pos);
                std::allocator_traits<Allocator>::construct(_alloc, _data + pos, std::forward<Args>(args)...);
            }
            catch(const std::exception& e)
            {
                for (size_type j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
        else
        {
            size_type i = _size;
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
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                for (size_type j = i + 1; j < _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
                
                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }
        }
    }

    constexpr iterator insert(const_iterator position, const T &x)
    {
        size_type pos = position - _data;

        if (_size >= _capacity)
        {
            size_type new_capacity = _capacity ? _capacity << 1 : 1;
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
                for (size_type i = 0; i < _size; i++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + i);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (size_type i = _size - 1; i > pos; i--)
                    _data[i] = _data[i - 1];
                _data[pos] = x;
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (size_type i = _size - 1; i > pos; i--)
                    _data[i] = std::move(_data[i - 1]);
                _data[pos] = x;
            }
            
        }
        else
        {
            size_type i = _size;
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
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (size_type j = i + 1; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }

            try
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + pos, x);
            }
            catch(const std::exception& e)
            {
                for (size_type j = 0; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }            
            
            _size++;
        }
    }

    constexpr iterator insert(const_iterator position, T &&x)
    {
        const T& nx = std::move(x);
        insert(position, nx);
    }

    constexpr iterator insert(const_iterator position, size_type n, const T &x)
    {
        size_type pos = position - _data;

        if (_size + n >= _capacity)
        {
            size_type new_capacity =
                _size + n > 0 ? (_size + _capacity + n - 1) / _capacity * _capacity : 1;
            reserve(new_capacity);
        }

        if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>)
        {
            size_type i = _size;
            try
            {
                for (; i < _size + n; i++)
                    std::allocator_traits<Allocator>::construct(_alloc, _data + i);
            }
            catch(const std::exception& e)
            {
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }

            if constexpr (std::is_copy_assignable_v<T>)
            {
                for (size_type i = _size - 1; i > pos; i--)
                    _data[i + n] = _data[i];
                _data[pos] = x;
            }
            else if constexpr (std::is_move_assignable_v<T>)
            {
                for (size_type i = _size - 1; i > pos; i--)
                    _data[i + n] = std::move(_data[i]);
                _data[pos] = x;
            }
            
            _size += n;
        }
        else
        {
            size_type i = _size - 1;
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
                for (size_type j = 0; j <= i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (size_type j = i + n + 1; j <= _size; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

                _size = _capacity = 0;
                _data = nullptr;
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
                for (size_type j = 0; j < i; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                for (size_type j = pos + n - 1; j < _size + n; j++)
                    std::allocator_traits<Allocator>::destroy(_alloc, _data + j);

                std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);
                _size = _capacity = 0;
                _data = nullptr;
                throw;
            }  
            
            _size += n;
        }
    }
    
    template <typename InputIter>
    constexpr iterator insert(const_iterator position, InputIter first, InputIter last)
    {
        if (first >= last)
            throw std::length_error("nuo_vector: first is smaller or equal to last");

        size_type n = last - first, pos = position - _data;

        if (_size >= _capacity)
        {
            size_type new_capacity =
                (_size + n > 0) ? ((_size + _capacity + n - 1) / _capacity * _capacity) : 1;
            reserve(new_capacity);
        }

        size_type i = _size - 1;
        try
        {
            for (;i >= pos; i--)
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + i + n, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch(const std::exception& e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (size_type j = _size + n - 1; j > _size + i; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }

        i = pos;
        try
        {
            for (InputIter it = first; it != last && i < pos + last - first; i++)
                std::allocator_traits<Allocator>::construct(_alloc, _data + i, *it);
        }
        catch(const std::exception& e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (size_type j = _size + n - 1; j >= _size + pos; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
        
        _size += last - first;
    }

    template <std::ranges::input_range R>
    constexpr iterator insert_range(const_iterator position, R &&rg)
    {
        size_type n;
        if constexpr (std::ranges::sized_range<R>)
            n = std::ranges::size(rg);
        else
            n = std::ranges::distance(rg);

        size_type pos = position - _data;

        if (_size >= _capacity)
        {
            size_type new_capacity =
                (_size + n > 0) ? ((_size + _capacity + n - 1) / _capacity * _capacity) : 1;
            reserve(new_capacity);
        }

        size_type i = _size - 1;
        try
        {
            for (;i >= pos; i--)
            {
                std::allocator_traits<Allocator>::construct(_alloc, _data + i + n, _data[i]);
                std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
            }
        }
        catch(const std::exception& e)
        {
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (size_type j = _size + n - 1; j > _size + i; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
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
            for (size_type j = 0; j < i; j++)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            for (size_type j = _size + n - 1; j >= _size + pos; j--)
                std::allocator_traits<Allocator>::destroy(_alloc, _data + j);
            
            std::allocator_traits<Allocator>::deallocate(_alloc, _data, _capacity);

            _size = _capacity = 0;
            _data = nullptr;
            throw;
        }
        
        _size += n;
    }

    /* TODO */
    constexpr iterator insert(const_iterator position, std::initializer_list<T> il);
    constexpr iterator erase(const_iterator position);
    constexpr iterator erase(const_iterator first, const_iterator last);
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
        for (size_type i = 0; i < _size; i++)
            std::allocator_traits<Allocator>::destroy(_alloc, _data + i);
        _size = 0;
    }
};

/* TODO */
// template<class InputIter, class Allocator = std::allocator</*iter-value-type*/<InputIter>>>
//   nuo_vector(InputIter, InputIter, Allocator = Allocator())
//     -> nuo_vector</*iter-value-type*/<InputIter>, Allocator>;

// template<std::ranges::input_range R, class Allocator = std::allocator<ranges::range_value_t<R>>>
//   nuo_vector(from_range_t, R&&, Allocator = Allocator())
//     -> nuo_vector<ranges::range_value_t<R>, Allocator>;

} /* namespace nuostl */

#endif