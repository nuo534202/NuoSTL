#include "core/data_types/nuo_string.hpp"

#include <math.h>
#include <string.h>

namespace nuostl {

/* expand space */
void nuo_string::expand_space() {
    expand_space(capacity << 1);
}

void nuo_string::expand_space(size_t new_size) {
    /* restore current string */
    char _s[capacity];
    strncpy(_s, s, capacity);

    /* assign a new space with double size */
    delete s;
    s = new char[new_size];

    /* store the string */
    strncpy(s, _s, capacity);
    capacity = new_size;
}

/* Todo: reduce space */



/* Constructor */
nuo_string::nuo_string() {
    s = new char[1];
    siz = 0, capacity = 1;
}

nuo_string::nuo_string(char c) {
    s = new char[1];
    s[0] = c;
    siz = capacity = 1;
}

nuo_string::nuo_string(char * _s) {
    siz = strlen(_s);
    capacity = 1ull << (size_t)ceil(log2(siz));
    s = new char[capacity];
    strncpy(s, _s, siz);
}

nuo_string::nuo_string(char * _s, size_t n) {
    siz = strlen(_s) * n;
    capacity = 1ull << (size_t)ceil(log2(siz));
    s = new char[capacity];
    for (int i = 0; i < n; i++)
        strncpy(s + strlen(_s) * i, _s, strlen(_s));
}

/* Destructor */
nuo_string::~nuo_string() {
    siz = capacity = 0;
    delete s;
    s = nullptr;
}

/* ToDo: operator */



/* push back */
void nuo_string::push_back(char c) {
    if (siz >= capacity)
        expand_space();
    
    s[siz] = c;
    siz++;
}

void nuo_string::push_back(char * _s) {
    if (siz + strlen(_s) > capacity)
        expand_space(1ull << (size_t)(ceil(log2(siz + strlen(_s)))));

    strcat(s + siz, _s);
    siz += strlen(_s);
}

void nuo_string::pop_back() {
    pop_back(1);
}

void nuo_string::pop_back(size_t len) {
    if (siz - len <= 0) siz = 0;
    else siz -= len;
}

/* get the size of string */
size_t nuo_string::size() {
    return siz;
}

/* if string is empty or not */
bool nuo_string::empty() {
    return siz == 0;
}
/* get the front element */
char nuo_string::front() {
    return s[0];
}

/* get the back element */
char nuo_string::back() {
    return s[siz - 1];
}

/* get the index of substr (-1 if not found) */
int32_t nuo_string::strstr(nuo_string& _s) {
    size_t len = _s.size() + siz + 1, s_size = _s.size();
    char * tmp_s = new char[len];
    int nxt[len];

    for (int i = 0; i < s_size; i++)
        tmp_s[i] = _s[i];
    tmp_s[s_size] = '\0';
    strncpy(tmp_s + s_size + 1, s, siz);

    for (int i = 0; i < len; i++)
        nxt[i] = 0;
    
    /* use kmp to find substr index */
    for (int i = 1; i < len; i++) {
        int j = nxt[i - 1];
        while (j > 0 && s[i] != s[j]) j = nxt[j - 1];
        if (s[i] == s[j]) j++;
        nxt[i] = j;
        if (i > s_size && nxt[i] == s_size) return i - s_size - 1;
    }
    return -1;
}

/* get the substring */
nuo_string nuo_string::substr(size_t l, size_t r) {
    char * _s = new char[r - l + 1];
    nuostl::nuo_string ans(_s);
    delete _s;
    return ans;
}

} /* namespace */