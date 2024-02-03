#pragma once

#include <mmap.h>

#include <primitives/command.h>
#include <primitives/filesystem.h>

#include <set>
#include <print>

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
    }
    mod_maker(const std::string &name, const path &dir) : name{name} {
        detect_game_dir(dir);
        detect_tools();
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
            if (p.filename() == "aim.exe") {
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
    path find_real_filename(path fn) const {
        auto s = fn.wstring();
        boost::to_lower(s);
        fn = s;
        if (fs::exists(fn)) {
            return fn;
        }
        if (fn == "aim.exe") {
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
        patch("aim.exe", 0x1F805, val);
    }
    void win_key(uint8_t val) {
        patch("aim.exe", 0x4A40D, val);
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
        if (fs::exists(make_unpak_dir(p.filename()))) {
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
        const auto aim1_exe = "aim.exe"sv;
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
