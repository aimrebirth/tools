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
};
