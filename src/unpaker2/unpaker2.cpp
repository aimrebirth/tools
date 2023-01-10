/*
 * AIM mod_converter
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <buffer.h>

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>
#include <primitives/templates2/mmap.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

//#include <archive.h>
#include <lzma.h>
#include <lzo/lzo1x.h>

using namespace std;

constexpr auto supported_block_size = 32768;

#pragma pack(push, 1)
struct pak {
    uint32_t magic;
    uint16_t unk0;
    uint32_t n_files;
    uint32_t n_blocks;
    uint32_t block_size;
    uint32_t unk1;
};
struct file_description {
    const char name[0x50];
    uint32_t offset;
    uint32_t size;
};
struct segment {
    enum decode_algorithm : uint32_t {
        none = 0x0,
        lzo = 0x1,
        lzma = 0x2,
        rlew = 0x4,
    };

    uint32_t unk1; // some file offset? trash?
    decode_algorithm algorithm;
    uint32_t offset;
};
#pragma pack(pop)

struct stream {
    primitives::templates2::mmap_file<uint8_t> &m;
    uint8_t *p{m.p};

    template <typename T>
    operator T&() {
        auto &r = *(T*)p;
        p += sizeof(T);
        return r;
    }
    template <typename T>
    auto span(size_t len) {
        auto s = std::span<T>((T*)p, len);
        p += sizeof(T) * len;
        return s;
    }
};


/// Type of a function to do some kind of coding work (filters, Stream,
/// Block encoders/decoders etc.). Some special coders use don't use both
/// input and output buffers, but for simplicity they still use this same
/// function prototype.
typedef lzma_ret (*lzma_code_function)(void *coder, const lzma_allocator *allocator, const uint8_t *in, size_t *in_pos,
                                       size_t in_size, uint8_t *out, size_t *out_pos, size_t out_size,
                                       lzma_action action);

/// Type of a function to free the memory allocated for the coder
typedef void (*lzma_end_function)(void *coder, const lzma_allocator *allocator);

/// Hold data and function pointers of the next filter in the chain.
struct lzma_next_coder_s {
    /// Pointer to coder-specific data
    void *coder;

    /// Filter ID. This is LZMA_VLI_UNKNOWN when this structure doesn't
    /// point to a filter coder.
    lzma_vli id;

    /// "Pointer" to init function. This is never called here.
    /// We need only to detect if we are initializing a coder
    /// that was allocated earlier. See lzma_next_coder_init and
    /// lzma_next_strm_init macros in this file.
    uintptr_t init;

    /// Pointer to function to do the actual coding
    lzma_code_function code;

    /// Pointer to function to free lzma_next_coder.coder. This can
    /// be NULL; in that case, lzma_free is called to free
    /// lzma_next_coder.coder.
    lzma_end_function end;

    /// Pointer to a function to get progress information. If this is NULL,
    /// lzma_stream.total_in and .total_out are used instead.
    void (*get_progress)(void *coder, uint64_t *progress_in, uint64_t *progress_out);

    /// Pointer to function to return the type of the integrity check.
    /// Most coders won't support this.
    lzma_check (*get_check)(const void *coder);

    /// Pointer to function to get and/or change the memory usage limit.
    /// If new_memlimit == 0, the limit is not changed.
    lzma_ret (*memconfig)(void *coder, uint64_t *memusage, uint64_t *old_memlimit, uint64_t new_memlimit);

    /// Update the filter-specific options or the whole filter chain
    /// in the encoder.
    lzma_ret (*update)(void *coder, const lzma_allocator *allocator, const lzma_filter *filters,
                       const lzma_filter *reversed_filters);

    /// Set how many bytes of output this coder may produce at maximum.
    /// On success LZMA_OK must be returned.
    /// If the filter chain as a whole cannot support this feature,
    /// this must return LZMA_OPTIONS_ERROR.
    /// If no input has been given to the coder and the requested limit
    /// is too small, this must return LZMA_BUF_ERROR. If input has been
    /// seen, LZMA_OK is allowed too.
    lzma_ret (*set_out_limit)(void *coder, uint64_t *uncomp_size, uint64_t out_limit);
};
typedef struct lzma_next_coder_s lzma_next_coder;

/// Largest valid lzma_action value as unsigned integer.
#define LZMA_ACTION_MAX ((unsigned int)(LZMA_FULL_BARRIER))

/// Internal data for lzma_strm_init, lzma_code, and lzma_end. A pointer to
/// this is stored in lzma_stream.
struct lzma_internal_s {
    /// The actual coder that should do something useful
    lzma_next_coder next;

    /// Track the state of the coder. This is used to validate arguments
    /// so that the actual coders can rely on e.g. that LZMA_SYNC_FLUSH
    /// is used on every call to lzma_code until next.code has returned
    /// LZMA_STREAM_END.
    enum {
        ISEQ_RUN,
        ISEQ_SYNC_FLUSH,
        ISEQ_FULL_FLUSH,
        ISEQ_FINISH,
        ISEQ_FULL_BARRIER,
        ISEQ_END,
        ISEQ_ERROR,
    } sequence;

    /// A copy of lzma_stream avail_in. This is used to verify that the
    /// amount of input doesn't change once e.g. LZMA_FINISH has been
    /// used.
    size_t avail_in;

    /// Indicates which lzma_action values are allowed by next.code.
    bool supported_actions[LZMA_ACTION_MAX + 1];

    /// If true, lzma_code will return LZMA_BUF_ERROR if no progress was
    /// made (no input consumed and no output produced by next.code).
    bool allow_buf_error;
};
typedef struct lzma_internal_s lzma_internal;


typedef struct {
    enum {
        SEQ_STREAM_HEADER,
        SEQ_BLOCK_HEADER,
        SEQ_BLOCK_INIT,
        SEQ_BLOCK_RUN,
        SEQ_INDEX,
        SEQ_STREAM_FOOTER,
        SEQ_STREAM_PADDING,
    } sequence;

    /// Block decoder
    lzma_next_coder block_decoder;

    /// Block options decoded by the Block Header decoder and used by
    /// the Block decoder.
    lzma_block block_options;

    /// Stream Flags from Stream Header
    lzma_stream_flags stream_flags;

    /// Index is hashed so that it can be compared to the sizes of Blocks
    /// with O(1) memory usage.
    lzma_index_hash *index_hash;

    /// Memory usage limit
    uint64_t memlimit;

    /// Amount of memory actually needed (only an estimate)
    uint64_t memusage;

    /// If true, LZMA_NO_CHECK is returned if the Stream has
    /// no integrity check.
    bool tell_no_check;

    /// If true, LZMA_UNSUPPORTED_CHECK is returned if the Stream has
    /// an integrity check that isn't supported by this liblzma build.
    bool tell_unsupported_check;

    /// If true, LZMA_GET_CHECK is returned after decoding Stream Header.
    bool tell_any_check;

    /// If true, we will tell the Block decoder to skip calculating
    /// and verifying the integrity check.
    bool ignore_check;

    /// If true, we will decode concatenated Streams that possibly have
    /// Stream Padding between or after them. LZMA_STREAM_END is returned
    /// once the application isn't giving us any new input (LZMA_FINISH),
    /// and we aren't in the middle of a Stream, and possible
    /// Stream Padding is a multiple of four bytes.
    bool concatenated;

    /// When decoding concatenated Streams, this is true as long as we
    /// are decoding the first Stream. This is needed to avoid misleading
    /// LZMA_FORMAT_ERROR in case the later Streams don't have valid magic
    /// bytes.
    bool first_stream;

    /// Write position in buffer[] and position in Stream Padding
    size_t pos;

    /// Buffer to hold Stream Header, Block Header, and Stream Footer.
    /// Block Header has biggest maximum size.
    uint8_t buffer[LZMA_BLOCK_HEADER_SIZE_MAX];
} lzma_stream_coder;


void unpack_file(path fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    pak p = s;
    if (p.block_size != supported_block_size) {
        throw std::runtime_error{"block size mismatch"};
    }
    auto descs = s.span<file_description>(p.n_files);
    auto segments = s.span<segment>(p.n_blocks);
    std::vector<uint8_t> bbb;
    bbb.resize((segments.size() + 1) * supported_block_size);
    auto pp = bbb.data();
    for (auto &&seg : segments) {
        s.p = f.p + seg.offset;
        uint32_t len = s;
        switch (seg.algorithm) {
        case segment::decode_algorithm::none: {
            memcpy(pp, s.p, len);
            pp += len;
            break;
        }
        case segment::decode_algorithm::lzo: {
            if (seg.algorithm == segment::decode_algorithm::lzma) {
                int a = 5;
                a++;
            }
            size_t outsz = supported_block_size;
            auto r2 = lzo1x_decompress(s.p, len, pp, &outsz, 0);
            if (r2 != LZO_E_OK) {
                throw std::runtime_error{"lzo error"};
            }
            pp += outsz;
            break;
        }
        case segment::decode_algorithm::rlew: {
            break;
        }
        case segment::decode_algorithm::lzma: {
            uint64_t memlimit = 0;
            size_t in_pos = 0;
            size_t out_pos = 0;
            auto r2 = lzma_stream_buffer_decode(&memlimit, 0, 0, s.p, &in_pos, len, pp, &out_pos, bbb.size() - (pp - bbb.data()));
            lzma_stream strm{};
            strm.next_in = s.p;
            strm.avail_in = len;
            strm.total_in = len;
            strm.next_out = pp;
            //strm.avail_out =
            auto r3 = lzma_stream_decoder(&strm, 1'000'000, 0);
            ((lzma_stream_coder*)strm.internal->next.coder)->sequence = lzma_stream_coder::SEQ_BLOCK_RUN;
            auto r4 = lzma_code(&strm, LZMA_RUN);
            auto r = lzma_microlzma_decoder(&strm, len, 0, false, 1'000'000);
            if (r != LZMA_OK) {
                throw std::runtime_error{"lzma error"};
            }
            r = lzma_code(&strm, lzma_action::LZMA_RUN);

            int a = 5;
            a++;
        }
        default:
            throw std::runtime_error{"compression unsupported: "s + std::to_string(seg.algorithm)};
        }
    }
    pp = bbb.data();

    auto dir = fn += ".dir2";
    fs::create_directories(dir);
    for (auto &&d : descs) {
        auto fn = dir / d.name;
        fs::create_directories(fn.parent_path());
        std::cout << "unpacking " << fn << "\n";
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        f.alloc_raw(d.size);
        memcpy(f.p, pp + d.offset, d.size);
    }
}

int main(int argc, char *argv[]) {
    cl::opt<path> p(cl::Positional, cl::desc("<pack file or dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        unpack_file(p);
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files_like(p, ".*\\.pak", false);
        for (auto &f : files) {
            std::cout << "processing: " << f << "\n";
            try {
                unpack_file(f);
            } catch (std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        }
    } else {
        throw std::runtime_error("Bad fs object");
    }
    return 0;
}
