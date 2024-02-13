#pragma once

#include <primitives/filesystem.h>
#include <primitives/templates2/mmap2.h>

struct stream {
    primitives::templates2::mmap_file<uint8_t> &m;
    uint8_t *p{m.p};

    template <typename T>
    operator T &() {
        auto &r = *(T *)p;
        p += sizeof(T);
        return r;
    }
    template <typename T>
    auto span(size_t len) {
        auto s = std::span<T>((T *)p, len);
        p += sizeof(T) * len;
        return s;
    }
    template <typename T>
    void operator=(const T &v) {
        memcpy(p, (uint8_t*)&v, sizeof(v));
        p += sizeof(v);
    }
    template <typename T>
    void read(std::vector<T> &v) {
        memcpy(v.data(), p, sizeof(T) * v.size());
        p += sizeof(T) * v.size();
    }
    void skip(size_t len) {
        p += len;
    }
};
