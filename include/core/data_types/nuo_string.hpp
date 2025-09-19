#ifndef NUOSTL_CORE_DATA_TYPES_NUO_STRING_HPP_
#define NUOSTL_CORE_DATA_TYPES_NUO_STRING_HPP_

#include <stddef.h>
#include <stdint.h>

namespace nuostl {
    class nuo_string {
    private:
        size_t siz;
        size_t capacity;

        /* expand space */
        void expand_space();
        void expand_space(size_t new_size);

        /* reduce space */
        void reduce_space();
        void reduce_space(size_t new_size);

    public:
        char * s;

        /* Constructor */
        nuo_string();
        nuo_string(char c);
        nuo_string(char * _s);
        nuo_string(char * _s, size_t n);

        /* Destructor */
        ~nuo_string();

        /* operator */
        template<typename T>
        nuo_string& operator= (const nuo_string& _s) const;
        nuo_string operator+ (const nuo_string& _s);
        nuo_string operator* (size_t n);
        nuo_string& operator+= (const nuo_string& _s);
        nuo_string& operator*= (size_t n);

        char& operator[] (size_t index);
        const char& operator[] (size_t index) const;

        bool operator< (const nuo_string& _s) const;
        bool operator<= (const nuo_string& _s) const;
        bool operator== (const nuo_string& _s) const;
        bool operator> (const nuo_string& _s) const;
        bool operator>= (const nuo_string& _s) const;

        /* push back */
        void push_back(char c);
        void push_back(char * _s);

        /* pop_back */
        void pop_back();
        void pop_back(size_t len);

        /* get the size of string */
        size_t size();

        /* if string is empty or not*/
        bool empty();

        /* get the front element */
        char front();

        /* get the back element */
        char back();

        /* get the index of substring (-1 if not found) */
        int32_t strstr(nuo_string& _s);

        /* get the substring */
        nuo_string substr(size_t l, size_t r);
    };
}

#endif