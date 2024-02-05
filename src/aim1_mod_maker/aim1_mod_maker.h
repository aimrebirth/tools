#pragma once

#include <mmap.h>

#include <primitives/command.h>
#include <primitives/filesystem.h>

#include <set>
#include <print>

constexpr auto aim_exe = "aim.exe"sv;

using byte_array = std::vector<uint8_t>;

struct patcher {
};

auto operator""_bin(const char *ptr, uint64_t len) {
    byte_array ret;
    auto lines = split_lines(ptr);
    for (auto &&line : lines) {
        auto d = line.substr(0, line.find(';'));
        auto bytes = split_string(d, " \r\n");
        for (auto &&v : bytes) {
            if (v.size() != 2) {
                throw std::runtime_error{"bad input string"};
            }
            auto hex2int1 = [](auto c) {
                if (isdigit(c)) {
                    return c - '0';
                } else if (isupper(c)) {
                    return c - 'A' + 10;
                } else {
                    return c - 'a' + 10;
                }
            };
            auto hex2int = [&](auto c) {
                auto v = hex2int1(c);
                if (v < 0 || v > 15) {
                    throw std::runtime_error{"bad input char"};
                }
                return v;
            };
            auto d1 = hex2int(v[0]);
            auto d2 = hex2int(v[1]);
            ret.push_back((d1 << 4) | d2);
        }
    }
    return ret;
}

struct mod_maker {
    enum class file_type {
        unknown,

        mmp,
        mmo,
        mmm,
        model,
        tm,
        script,
        sound,
    };
    enum pak_files {
    };

    std::string name;
    path game_dir;
    std::set<path> files_to_mod;

    mod_maker(const std::string &name) : name{name} {
        detect_game_dir(fs::current_path());
        detect_tools();
        prepare_injections();
    }
    mod_maker(const std::string &name, const path &dir) : name{name} {
        detect_game_dir(dir);
        detect_tools();
        prepare_injections();
    }

    void replace(const path &fn, const std::string &from, const std::string &to) {
        auto ft = check_file_type(fn);
        switch (ft) {
        case file_type::script: {
            auto p = find_real_filename(fn);
            auto txt = make_script_txt_fn(p);
            if (!fs::exists(txt)) {
                run_p4_tool("script2txt", p);
            }
            replace_in_file_raw(txt, from, to);
            run_p4_tool("txt2script", txt);
            files_to_mod.insert(get_mod_dir() / txt.stem());
            break;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    void apply() {
        std::vector<std::string> files;
        for (auto &&p : files_to_mod) {
            if (p.filename() == aim_exe) {
                continue;
            }
            files.push_back(p.string());
        }
        auto fn = get_mod_dir() / name += ".pak"s;
        run_p4_tool("paker", fn, files);
        fs::copy_file(fn, get_data_dir() / fn.filename(), fs::copy_options::overwrite_existing);
    }
    template <typename T>
    void patch(path fn, uint32_t offset, T val) {
        fn = find_real_filename(fn);
        files_to_mod.insert(fn);
        patch_raw(fn, offset, val);
    }
    // this one checks for old value as well, so incorrect positions (files) won't be patched
    template <typename T>
    bool patch(path fn, uint32_t offset, T oldval, T val) {
        fn = find_real_filename(fn);
        files_to_mod.insert(fn);
        return patch_raw(fn, offset, oldval, val);
    }

#define ENABLE_DISABLE_FUNC(name, enable, disable) \
    void enable_##name() { name(enable); } \
    void disable_##name() { name(disable); }
    ENABLE_DISABLE_FUNC(free_camera, 1, 0)
    ENABLE_DISABLE_FUNC(win_key, 0x00, 0x10)
#undef ENABLE_DISABLE_FUNC

private:
    static void memcpy(auto &ptr, const byte_array &data) {
        ::memcpy(ptr, data.data(), data.size());
        ptr += data.size();
    }
    static auto memmem(auto ptr, auto sz, const byte_array &bytes) {
        sz -= bytes.size();
        for (int i = 0; i < sz; ++i) {
            if (memcmp(ptr + i, bytes.data(), bytes.size()) == 0) {
                return ptr + i;
            }
        }
        throw std::runtime_error{"not found"};
    }
    static auto memreplace(auto base, auto sz, const byte_array &from, const byte_array &to) {
        if (from.size() != to.size()) {
            throw std::runtime_error{"size mismatch"};
        }
        auto ptr = memmem(base, sz, from);
        byte_array old;
        old.resize(from.size());
        ::memcpy(old.data(), ptr, old.size());
        ::memcpy(ptr, to.data(), to.size());
        return std::tuple{ptr, old};
    }
    static auto make_insn_with_address(auto &&insn, uint32_t addr) {
        byte_array arr(insn.size() + sizeof(addr));
        ::memcpy(arr.data(), insn.data(), insn.size());
        *(uint32_t *)(&arr[insn.size()]) = addr;
        return arr;
    }
    void prepare_injections() {
        create_backup_exe_file();
        primitives::templates2::mmap_file<uint8_t> f{find_real_filename(aim_exe), primitives::templates2::mmap_file<uint8_t>::rw{}};
        constexpr uint32_t trampoline_base      = 0x00025100;
        constexpr uint32_t trampoline_target    = 0x001207f0;
        constexpr uint32_t code_base            = 0x00401000;
        constexpr uint32_t data_base            = 0x00540000;
        constexpr uint32_t free_data_base       = 0x006929C0;
        //constexpr uint32_t our_data  = 0x00550FD0;
        constexpr uint32_t our_data  = 0x005207F0;
        //constexpr uint32_t free_data_base_real  = 0x140000 + our_data - 0x00540000;

        auto ptr = f.p + trampoline_target;
        //strcpy((char *)f.p + free_data_base_real, "aim_fixes-0.0.1.dll");
        strcpy((char *)ptr, "aim_fixes-0.0.1.dll");
        ptr += 0x20;
        const auto jumppad = "68 30 B8 51 00"_bin; // push    offset SEH_425100
        uint32_t jump_offset = ptr - f.p - trampoline_base - jumppad.size() * 2;
        auto [oldaddr, oldcode] = memreplace(f.p, f.sz, jumppad, make_insn_with_address("e9"_bin, jump_offset));
        memcpy(ptr, jumppad); // put our removed insn
        memcpy(ptr, R"(
60                  ; pusha
)"_bin);
        auto push_dll_name = make_insn_with_address("68"_bin, our_data);
        memcpy(ptr, push_dll_name); //
        memcpy(ptr, R"(
8B 3D D8 10 52 00       ;  mov     edi, ds:LoadLibraryA - not working ; but do not remove, it does not work without it
bf 30 0f 91 75          ;  mov    edi, 0x75910f30 - load direct adress
; edi has wrong address after prev. insn, so we fix it manually
81 EF 00 BD 00 00       ;  sub     edi, 0BD00h
)"_bin);
        memcpy(ptr, R"(
FF D7               ; call    edi
61                  ; popa
)"_bin);
        memcpy(ptr, make_insn_with_address("e9"_bin, -(ptr - f.p - trampoline_base - jumppad.size())));
    }
    path find_real_filename(path fn) const {
        auto s = fn.wstring();
        boost::to_lower(s);
        fn = s;
        if (fs::exists(fn)) {
            return fn;
        }
        if (fn == aim_exe) {
            return game_dir / fn;
        }

        auto ft = check_file_type(fn);
        switch (ft) {
        case file_type::script: {
            auto p = get_data_dir() / "scripts.pak";
            unpak(p);
            p = make_unpak_dir(p);
            if (!fs::exists(p / fn)) {
                p = p / "Script" / "bin" / fn.filename();
            } else {
                p /= fn;
            }
            if (!fs::exists(p)) {
                throw SW_RUNTIME_ERROR("Cannot find file in archives: "s + fn.string());
            }
            return p;
        }
        case file_type::mmo: {
            auto p = get_data_dir() / "maps2.pak";
            if (fs::exists(p)) {
                unpak(p);
                p = make_unpak_dir(p);
                if (!fs::exists(p / fn)) {
                    p = p / fn.filename();
                } else {
                    p /= fn;
                }
                if (fs::exists(p)) {
                    return p;
                }
            }
            p = get_data_dir() / "maps.pak";
            unpak(p);
            p = make_unpak_dir(p);
            if (!fs::exists(p / fn)) {
                p = p / fn.filename();
            } else {
                p /= fn;
            }
            if (!fs::exists(p)) {
                throw SW_RUNTIME_ERROR("Cannot find file in archives: "s + fn.string());
            }
            return p;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    // from https://github.com/Solant/aim-patches
    void free_camera(uint8_t val) {
        create_backup_exe_file();
        patch(aim_exe, 0x1F805, val);
    }
    void win_key(uint8_t val) {
        create_backup_exe_file();
        patch(aim_exe, 0x4A40D, val);
    }
    void create_backup_exe_file() {
        auto fn = find_real_filename(aim_exe);
        auto backup = path{fn} += ".orig";
        if (!fs::exists(backup)) {
            fs::copy_file(fn, backup);
        }
    }
    template <typename T>
    void patch_raw(path fn, uint32_t offset, T val) const {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        std::println("patching {} offset 0x{:08X} to {} (old value: {})", fn.string(), offset, val, old);
        old = val;
    }
    template <typename T>
    bool patch_raw(path fn, uint32_t offset, T expected, T val) const {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        std::println("patching {} offset 0x{:08X} from {} to {}", fn.string(), offset, expected, val);
        if (old == expected) {
            std::println("success");
            old = val;
            return true;
        } else if (old == val) {
            std::println("success, already patched");
            return true;
        } else {
            std::println("old value {} != expected {}", old, expected);
            return false;
        }
    }
    path get_mod_dir() const {
        return get_data_dir() / "mods" / name;
    }
    path get_data_dir() const {
        return game_dir / "data";
    }
    static void replace_in_file_raw(const path &fn, const std::string &from, const std::string &to) {
        std::println("replacing in file {} from '{}' to '{}'", fn.string(), from, to);
        auto f = read_file(fn);
        boost::replace_all(f, from, to);
        boost::replace_all(f, "\r", "");
        write_file_if_different(fn, f);
    }
    static path make_unpak_dir(path p) {
        p += ".dir";
        return p;
    }
    static path make_script_txt_fn(path p) {
        p += ".txt";
        return p;
    }
    void unpak(const path &p) const {
        if (fs::exists(make_unpak_dir(p))) {
            return;
        }
        run_p4_tool("unpaker", p);
    }
    void run_p4_tool(const std::string &tool, auto && ... args) const {
        run_sw("pub.lzwdgc.Polygon4.Tools."s + tool + "-master", args...);
    }
    void run_sw(auto &&...args) const {
        primitives::Command c;
        c.working_directory = get_mod_dir();
        fs::create_directories(c.working_directory);
        c.push_back("sw");
        c.push_back("run");
        (c.push_back(args),...);
        run_command(c);
    }
    static void run_command(auto &c) {
        c.out.inherit = true;
        c.err.inherit = true;
        std::cout << c.print() << "\n";
        c.execute();
    }
    void detect_game_dir(const path &dir) {
        const auto aim1_exe = aim_exe;
        if (fs::exists(dir / aim1_exe)) {
            game_dir = dir;
        } else if (fs::exists(dir.parent_path() / aim1_exe)) {
            game_dir = dir.parent_path();
        } else {
            throw SW_RUNTIME_ERROR("Cannot detect aim1 game dir.");
        }
        game_dir = fs::absolute(game_dir).lexically_normal();
    }
    void detect_tools() {
        check_in_path("git");
        check_in_path("sw");
    }
    void check_in_path(const path &program) const {
        if (!has_in_path(program)) {
            throw SW_RUNTIME_ERROR("Cannot find "s + program.string() + " in PATH.");
        }
    }
    static bool has_in_path(const path &program) {
        return !primitives::resolve_executable(program).empty();
    }
    static file_type detect_file_type(const path &fn) {
        auto ext = fn.extension().string();
        boost::to_lower(ext);
        if (ext.empty()) {
            return file_type::model;
        } else if (ext == ".mmp") {
            return file_type::mmp;
        } else if (ext == ".mmo") {
            return file_type::mmo;
        } else if (ext == ".mmm") {
            return file_type::mmm;
        } else if (ext == ".scr" || ext == ".qst") {
            return file_type::script;
        } else if (ext == ".tm") {
            return file_type::tm;
        } else if (ext == ".ogg") {
            return file_type::sound;
        }
        return file_type::unknown;
    }
    file_type check_file_type(const path &fn) const {
        auto t = detect_file_type(fn);
        if (t == file_type::unknown) {
            throw SW_RUNTIME_ERROR("Unknown file type: "s + fn.string());
        }
        return t;
    }
};

int main1(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    try {
        return main1(argc, argv);
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    } catch (...) {
        std::cerr << "unknown exception" << "\n";
    }
}
#define main main1
