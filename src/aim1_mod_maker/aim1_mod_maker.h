#pragma once

#include <db2.h>
#include <mmo2.h>

#include <boost/container_hash/hash.hpp>
#include <primitives/command.h>
#include <primitives/filesystem.h>
#include <primitives/sw/main.h>

#include <format>
#include <fstream>
#include <set>
#include <source_location>
#include <print>

constexpr auto aim_exe = "aim.exe"sv;

using byte_array = std::vector<uint8_t>;

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

void log(auto &&format, auto &&arg, auto &&...args) {
    std::println("{}", std::vformat(format, std::make_format_args(arg, args...)));
}
void log(auto &&str) {
    std::println("{}", str);
}

struct aim_exe_v1_06_constants {
    enum : uint32_t {
        trampoline_base_real = 0x00025100,
        trampoline_target_real = 0x001207f0,
        code_base = 0x00401000,
        //data_base = 0x00540000,
        //free_data_base_virtual = 0x006929C0,
        free_data_base_virtual = 0x00692FF0,
        our_code_start_virtual = 0x005207F0, // place to put out dll load
    };
};

struct mod_maker {
    struct db_wrapper {
        db2::files::db2_internal m;
        db2::files::db2_internal *m2_{};
        path fn;
        int codepage{1251};
        bool written{};

        ~db_wrapper() {
            if (!written) {
                write();
            }
        }
        void write() {
            m.save(fn, codepage);
            written = true;
        }
        auto &operator[](this auto &&d, const std::string &s) {
            return d.m[s];
        }
        auto &m2() {
            return *m2_;
        }
        void copy_from_aim2(auto &&table_name, auto &&value_name, auto &&field_name) {
            auto check_val = [](auto &&m, const std::string &key, auto &&err) {
                if (auto it = m.find(key); it == m.end()) {
                    throw std::runtime_error{err};
                }
            };
            check_val(m2().m, (const char *)table_name, "aim2: no such table");
            check_val(m2()[table_name], value_name, "aim2: no such value");
            check_val(m2()[table_name][value_name], field_name, "aim2: no such field");
            m[table_name][value_name][field_name] = m2()[table_name][value_name][field_name];
        }
        void copy_from_aim2(auto &&table_name, auto &&value_name) {
            auto check_val = [](auto &&m, const std::string &key, auto &&err) {
                if (auto it = m.find(key); it == m.end()) {
                    throw std::runtime_error{err};
                }
            };
            check_val(m2().m, (const char *)table_name, "aim2: no such table");
            check_val(m2()[table_name], value_name, "aim2: no such value");
            m[table_name][value_name] = m2()[table_name][value_name];
        }
    };
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

    std::string name;
    std::string version;
    path game_dir;
    path aim2_game_dir;
    std::set<path> files_to_pak;
    std::set<path> files_to_distribute;
    std::set<path> code_files_to_distribute;
    std::set<path> restored_files;
    std::set<path> copied_files;
    std::source_location loc;

    mod_maker(std::source_location loc = std::source_location::current()) : loc{loc} {
        init(fs::current_path());
    }
    mod_maker(const std::string &name, std::source_location loc = std::source_location::current()) : name{name}, loc{loc} {
        init(fs::current_path());
    }
    mod_maker(const std::string &name, const path &dir, std::source_location loc = std::source_location::current()) : name{name}, loc{loc} {
        init(dir);
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
            auto dst_txt = get_mod_dir() / txt.filename();
            copy_file_once(txt, dst_txt);
            txt = dst_txt;
            replace_in_file_raw(txt, from, to);
            run_p4_tool("txt2script", txt);
            files_to_pak.insert(get_mod_dir() / txt.stem());
            break;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    void apply() {
        std::vector<std::string> files;
        for (auto &&p : files_to_pak) {
            if (p.filename() == aim_exe) {
                continue;
            }
            files.push_back(p.string());
        }
        auto fn = get_mod_dir() / get_full_mod_name() += ".pak"s;
        run_p4_tool("paker", fn, files);
        fs::copy_file(fn, get_data_dir() / fn.filename(), fs::copy_options::overwrite_existing);
        files_to_distribute.insert(path{"data"} / fn.filename());
        // make patch notes
        auto patchnotes_fn = path{game_dir / get_full_mod_name()} += ".README.txt";
        files_to_distribute.insert(patchnotes_fn.filename());
        std::ofstream ofile{patchnotes_fn};
#ifndef NDEBUG
        ofile << "Developer Mode!!!\nOnly for testing purposes!\nDO NOT USE FOR ACTUAL PLAYING !!!\n\n";
#endif
        ofile << name;
        if (!version.empty()) {
            ofile << " (version: " << version << ")";
        }
        ofile << "\n\n";
        ofile << std::format("Release Date\n{:%d.%m.%Y %X}\n\n", std::chrono::system_clock::now());
        for (auto &&line : read_lines(loc.file_name())) {
            auto f = [&](auto &&a) {
                auto pos = line.find(a);
                if (pos != -1) {
                    auto s = line.substr(pos + a.size());
                    if (!s.empty() && s[0] == ' ') {
                        s = s.substr(1);
                    }
                    boost::trim_right(s);
                    if (!s.empty() && (s[0] >= 'a' && s[0] <= 'z' || s[0] >= '0' && s[0] <= '9')) {
                        s = "* " + s;
                    }
                    ofile << s << "\n";
                }
            };
            auto anchor = "patch note:"sv;
            auto anchor_dev = "patch note dev:"sv;
            f(anchor);
#ifndef NDEBUG
            f(anchor_dev);
#endif
        }
        ofile.close();

        // we do not check for presence of 7z command here
        if (has_in_path("7z")) {
            auto ar = get_full_mod_name() + ".zip";
            if (fs::exists(ar)) {
                fs::remove(ar);
            }

            primitives::Command c;
            c.working_directory = game_dir;
            c.push_back("7z");
            c.push_back("a");
            c.push_back(ar); // we use zip as more common
            for (auto &&f : files_to_distribute) {
                c.push_back(f.is_absolute() ? f.lexically_relative(game_dir) : f);
            }
            for (auto &&f : code_files_to_distribute) {
                c.push_back(f.is_absolute() ? f.lexically_relative(game_dir) : f);
            }
            run_command(c);
        } else {
            log("7z not found, skipping archive creation");
        }
    }

    template <typename T>
    void patch_after_pattern(path fn, const std::string &pattern, uint32_t offset, T oldval, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        patch_raw(fn, pattern, offset, oldval, val);
    }
    template <typename T>
    void patch(path fn, uint32_t offset, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        patch_raw(fn, offset, val);
    }
    // this one checks for old value as well, so incorrect positions (files) won't be patched
    template <typename T>
    bool patch(path fn, uint32_t offset, T oldval, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        return patch_raw(fn, offset, oldval, val);
    }
    void insert(path fn, uint32_t offset, const byte_array &data) {
        files_to_pak.insert(find_real_filename(fn));

        log("inserting into {} offset 0x{:08X} {} bytes", fn.string(), offset, data.size());

        auto rfn = find_real_filename(fn);
        fs::resize_file(rfn, fs::file_size(rfn) + data.size());
        primitives::templates2::mmap_file<uint8_t> f{rfn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        memmove(f.p + offset + data.size(), f.p + offset, f.sz - offset - data.size());
        ::memcpy(f.p + offset, data.data(), data.size());
        f.close();
    }
    std::string add_map_good(path mmo_fn, const std::string &building_name, const std::string &after_good_name, const mmo_storage2::map_good &mg) {
        byte_array data((uint8_t*)&mg, (uint8_t*)&mg + sizeof(mg));
        add_map_good(mmo_fn, building_name, after_good_name, data);
        return mg.name;
    }
    void add_map_good(path mmo_fn, const std::string &building_name, const std::string &after_good_name, const byte_array &data) {
        files_to_pak.insert(find_real_filename(mmo_fn));

        auto fn = find_real_filename(mmo_fn);
        mmo_storage2 m;
        m.load(fn);

        auto it = m.map_building_goods.find(building_name);
        if (it == m.map_building_goods.end()) {
            throw std::runtime_error{"no such building: "s + building_name};
        }

        uint32_t insertion_offset = it->second.offset + sizeof(uint32_t);
        if (!after_good_name.empty()) {
            auto it2 = it->second.building_goods.find(after_good_name);
            if (it2 == it->second.building_goods.end()) {
                throw std::runtime_error{"no such building good: "s + after_good_name};
            }
            insertion_offset = it2->second;
        }

        {
            primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
            if (::memcmp(f.p + insertion_offset, data.data(), data.size()) == 0) {
                return;
            }
        }

        insert(mmo_fn, insertion_offset, data);

        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        // increase section size
        *(uint32_t *)(f.p + m.sections.map_goods.offset) += data.size();
        // increase number of goods
        ++*(uint32_t *)(f.p + it->second.offset);
    }

    // all you need is to provide injection address (virtual) with size
    // handle the call instruction in 'dispatcher' symbol (naked) of your dll
    constexpr static inline auto call_command_length = 5;
    void make_injection(uint32_t virtual_address) {
        make_injection(virtual_address, get_injection_size(virtual_address));
    }
    void make_injection(uint32_t virtual_address, uint32_t size) {
        uint32_t len = size;
        if (len < call_command_length) {
            throw std::runtime_error{"jumppad must be 5 bytes atleast"};
        }
        primitives::templates2::mmap_file<uint8_t> f{find_real_filename(aim_exe),
                                                     primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto ptr = f.p + virtual_address - aim_exe_v1_06_constants::our_code_start_virtual + aim_exe_v1_06_constants::trampoline_target_real;
        memcpy(ptr, make_insn_with_address("e8"_bin, aim_exe_v1_06_constants::free_data_base_virtual -
                                                         (virtual_address + call_command_length)));
        memcpy(ptr, make_nops(len - call_command_length));
        log("making injection on the virtual address 0x{:0X} (real address 0x{:0X}), size {}", virtual_address, ptr - f.p,
                     size);
    }

#define ENABLE_DISABLE_FUNC(name, enable, disable) \
    void enable_##name() { name(enable); } \
    void disable_##name() { name(disable); }
    ENABLE_DISABLE_FUNC(free_camera, 1, 0)
    ENABLE_DISABLE_FUNC(win_key, 0x00, 0x10)
#undef ENABLE_DISABLE_FUNC

    void add_code_file_for_archive(path fn) {
        if (!fn.is_absolute()) {
            fn = path{loc.file_name()}.parent_path() / fn.filename();
        }
        auto dst_fn = get_mod_dir() / fn.filename();
        fs::copy_file(fn, dst_fn, fs::copy_options::overwrite_existing);
        code_files_to_distribute.insert(dst_fn);
    }
    void add_resource(path fn) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
    }
    void setup_aim2_path() {
        try {
            aim2_game_dir = read_file(game_dir / "aim2_path.txt");
            if (!fs::exists(aim2_game_dir)) {
                throw std::runtime_error{"aim2 dir does not exist"};
            }
            if (!fs::is_directory(aim2_game_dir)) {
                throw std::runtime_error{"aim2 path is not a directory"};
            }
        } catch (std::exception &e) {
            throw std::runtime_error{
                std::format("Can't read aim2_path.\n"
                            "Create aim2_path.txt near your aim.exe and write down aim2 path there.\n"
                            "Error: {}",
                            e.what())};
        }
    }
    void copy_from_aim2(auto &&object) {
        log("copying from aim2: {}", path{object}.filename().string());

        auto ft = detect_file_type(object);
        switch (ft) {
        case file_type::model: {
            auto p = aim2_game_dir / "data" / "aimmod.pak";
            unpak(p);
            p = make_unpak_dir(p);
            if (fs::exists(p / object)) {
                p /= object;
            } else {
                p /= "data";
                p /= "models";
                p /= object;
                if (!fs::exists(p)) {
                    throw std::runtime_error{std::format("aim2: model is not found: {}", p.string())};
                }
            }
            auto copied_fn = get_mod_dir() / path{object}.filename().string();
            fs::copy_file(p, copied_fn, fs::copy_options::overwrite_existing);
            run_p4_tool("mod_converter2", copied_fn);
            add_resource(copied_fn);
            db().copy_from_aim2("Модели", path{object}.stem().string());
            auto textures = read_lines(path{copied_fn} += ".textures.txt");
            auto &m2 = open_aim2_db();
            for (auto &&t : textures) {
                path fn = std::get<std::string>(m2["Текстуры"][t]["FILENAME"]);
                if (fn.empty()) {
                    throw std::runtime_error{"Can't find texture: "s + t};
                }
                copy_from_aim2(fn);
            }
            break;
        }
        case file_type::tm: {
            auto p = aim2_game_dir / "data" / "aimtex.pak";
            unpak(p);
            p = make_unpak_dir(p);
            if (fs::exists(p / object)) {
                p /= object;
            } else {
                p /= "data";
                p /= "tm";
                p /= object;
                if (!fs::exists(p)) {
                    throw std::runtime_error{std::format("aim2: model is not found: {}", p.string())};
                }
            }
            auto copied_fn = get_mod_dir() / path{object}.filename().string();
            fs::copy_file(p, copied_fn, fs::copy_options::overwrite_existing);
            add_resource(copied_fn);
            db().copy_from_aim2("Текстуры", path{object}.stem().string());
            break;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    void copy_glider_from_aim2(auto &&object) {
        log("copying glider from aim2: {}", object);

        copy_from_aim2("MOD_"s + object);
        db().copy_from_aim2("Глайдеры", path{object}.stem().string());
        // may be absent - try..catch?
        quest("ru_RU").copy_from_aim2("INFORMATION", path{object}.stem().string());
    }

    auto db() {
        auto w = open_db("db", 1251); // always 1251 probably
        if (aim2_available()) {
            w.m2_ = &open_aim2_db();
        }
        return w;
    }
    auto quest(const std::string &language = {}) {
        // TODO: check if it's possible to use utf8/16 in aim game
        // set codepages here until we fix or implement unicode
        int db_codepage = 1251;
        if (language == "fr_fr") {
            // change cp. Also for other langs
        }
        if (language.empty()) {
            auto w = open_db("quest", db_codepage);
            if (aim2_available()) {
                w.m2_ = &open_aim2_quest();
            }
            return w;
        } else {
            auto w = open_db("quest_" + language, db_codepage);
            if (aim2_available()) {
                w.m2_ = &open_aim2_quest();
            }
            return w;
        }
    }
    db2::files::db2_internal &open_aim2_db() {
        if (!aim2_available()) {
            throw std::runtime_error{"aim2 is not available, setup it first"};
        }
        static auto m2 = db2{aim2_game_dir / "data" / "db"}.open().to_map(1251);
        return m2;
    }

private:
    void copy_file_once(const path &from, const path &to) {
        if (auto [_, i] = copied_files.emplace(to); i) {
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
    }
    db2::files::db2_internal &open_aim2_quest() {
        if (!aim2_available()) {
            throw std::runtime_error{"aim2 is not available, setup it first"};
        }
        static auto m2 = db2{aim2_game_dir / "data" / "quest"}.open().to_map(1251);
        return m2;
    }
    bool aim2_available() const {
        return !aim2_game_dir.empty();
    }
    static path make_bak_file(const path &fn) {
        auto backup = path{fn} += ".bak";
        if (!fs::exists(backup)) {
            fs::copy_file(fn, backup);
        }
        return backup;
    }
    db_wrapper open_db(auto &&name, int db_codepage) {
        auto d = db2{get_data_dir() / name};
        auto files = d.open().get_files();
        for (auto &&f : files) {
            backup_or_restore_once(f);
            files_to_distribute.insert(f);
        }
        db_wrapper w;
        w.m = d.open().to_map(db_codepage);
        w.fn = d.fn;
        w.codepage = db_codepage;
        return w;
    }
    void backup_or_restore_once(const path &fn) {
        auto bak = make_bak_file(fn);
        if (fs::exists(bak) && !restored_files.contains(fn)) {
            fs::copy_file(bak, fn, fs::copy_options::overwrite_existing);
            restored_files.insert(fn);
        }
    }
    void init(const path &dir) {
        read_name();
        detect_game_dir(dir);
#ifdef NDEBUG
        if (fs::exists(get_mod_dir())) {
            fs::remove_all(get_mod_dir());
        }
#endif
        fs::create_directories(get_mod_dir());
        auto src_fn = get_mod_dir() / get_full_mod_name() += ".cpp";
        fs::copy_file(loc.file_name(), src_fn, fs::copy_options::overwrite_existing);
        code_files_to_distribute.insert(src_fn);
        detect_tools();
        create_backup_exe_file();
        prepare_injections();
#ifndef NDEBUG
        enable_win_key();
#endif
    }
    void read_name() {
        if (name.empty()) {
            name = path{loc.file_name()}.stem().string();
        }
        // use regex?
        auto p = name.find('-');
        if (p != -1) {
            version = name.substr(p + 1);
            name = name.substr(0, p);
        }
    }
    decltype(name) get_full_mod_name() const {
        auto s = name;
        if (!version.empty()) {
            s += "-" + version;
        }
        return s;
    }
    static void memcpy(auto &ptr, const byte_array &data) {
        ::memcpy(ptr, data.data(), data.size());
        ptr += data.size();
    }
    static auto memmem(auto ptr, auto sz, auto &&bytes) -> decltype(ptr) {
        sz -= bytes.size();
        for (int i = 0; i < sz; ++i) {
            if (memcmp(ptr + i, bytes.data(), bytes.size()) == 0) {
                return ptr + i;
            }
        }
        return nullptr;
    }
    static auto memreplace(auto base, auto sz, const byte_array &from, const byte_array &to) {
        if (from.size() != to.size()) {
            throw std::runtime_error{"size mismatch"};
        }
        auto ptr = memmem(base, sz, from);
        if (!ptr) {
            throw std::runtime_error{"oldmem not found"};
        }
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
    static byte_array make_nops(uint32_t len) {
        byte_array arr(len, 0x90);
        return arr;
    }
    void make_injected_dll() {
        log("making injected dll");

        path fn = loc.file_name();
        //fs::copy_file(fn, get_mod_dir() / fn.filename(), fs::copy_options::overwrite_existing);
        std::string contents;
        contents += "void build(Solution &s) {\n";
        contents += "auto &t = s.addSharedLibrary(\"" + name + "\"";
        if (!version.empty()) {
            contents += ", \"" + version + "\"";
        }
        contents += ");\n";
        contents += "t += cpp23;\n";
        contents += "t += \"" + boost::replace_all_copy(fn.string(), "\\", "/") + "\";\n";
        contents += "t += \"INJECTED_DLL\"_def;\n";
        contents += "}\n";
        write_file(get_mod_dir() / "sw.cpp", contents);

        primitives::Command c;
        c.working_directory = get_mod_dir();
        c.push_back("sw");
        c.push_back("build");
        c.push_back("-platform");
        c.push_back("x86");
        c.push_back("-config");
        c.push_back("r");
        c.push_back("-config-name");
        c.push_back("r");
        run_command(c);

        auto dllname = get_mod_dir() / ".sw" / "out" / "r" / get_sw_dll_name();
        fs::copy_file(dllname, game_dir / get_dll_name(), fs::copy_options::overwrite_existing);
        files_to_distribute.insert(get_dll_name());
    }
    decltype(name) get_sw_dll_name() const {
        if (!version.empty()) {
            return get_dll_name();
        }
        return name + "-0.0.1.dll";
    }
    decltype(name) get_dll_name() const {
        return get_full_mod_name() + ".dll";
    }
    uint32_t virtual_to_real(uint32_t v) {
        return v - aim_exe_v1_06_constants::code_base + 0x1000;
    }
    void patch(uint8_t *p, uint32_t off, const byte_array &from, const byte_array &to) {
        if (from.size() != to.size()) {
            throw std::runtime_error{"size mismatch"};
        }
        if (memcmp(p + off, to.data(), to.size()) == 0) {
            return; // ok, already patched
        }
        ::memcpy(p + off, to.data(), to.size());
    }
    void prepare_injections() {
#ifdef NDEBUG
        make_injected_dll();
#endif
        files_to_distribute.insert(aim_exe);
        primitives::templates2::mmap_file<uint8_t> f{find_real_filename(aim_exe), primitives::templates2::mmap_file<uint8_t>::rw{}};
        uint32_t our_data = aim_exe_v1_06_constants::our_code_start_virtual;

        auto ptr = f.p + aim_exe_v1_06_constants::trampoline_target_real;

        auto strcpy = [&](const std::string &s) {
            ::strcpy((char *)ptr, s.c_str());
            ptr += s.size() + 1;
            our_data += s.size() + 1;
        };

        auto push_dll_name = make_insn_with_address("68"_bin, our_data); // push
#ifdef NDEBUG
        strcpy(get_sw_dll_name());
#else
        strcpy("h:\\Games\\AIM\\1\\.sw\\out\\d\\aim_fixes-0.0.1.dll"s);
#endif
        const auto jumppad = "68 30 B8 51 00"_bin; // push    offset SEH_425100
        uint32_t jump_offset = ptr - f.p - aim_exe_v1_06_constants::trampoline_base_real - jumppad.size() * 2;
        patch(f.p, virtual_to_real(0x00425105), jumppad, make_insn_with_address("e9"_bin, jump_offset));
        memcpy(ptr, jumppad); // put our removed insn
        memcpy(ptr, R"(
60                      ; pusha
)"_bin);
        memcpy(ptr, push_dll_name);
        memcpy(ptr, R"(
8B 3D D8 10 52 00       ; mov     edi, ds:LoadLibraryA
FF D7                   ; call    edi
61                      ; popa
)"_bin);
        memcpy(ptr, make_insn_with_address("e9"_bin, -(ptr - f.p - aim_exe_v1_06_constants::trampoline_base_real - jumppad.size())));
    }
    path find_real_filename(path fn) {
        auto s = fn.wstring();
        boost::to_lower(s);
        fn = s;
        if (fs::exists(fn)) {
            return fn;
        }
        // free files
        if (fs::exists(get_data_dir() / fn)) {
            return get_data_dir() / fn;
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
            auto p = find_file_in_paks(fn, "res3.pak", "maps2.pak", "maps.pak");
            if (!fs::exists(p)) {
                throw SW_RUNTIME_ERROR("Cannot find file in archives: "s + fn.string());
            }
            auto dst = get_mod_dir() / p.filename();
            copy_file_once(p, dst);
            return dst;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    path find_file_in_paks(path fn, auto &&... paks) const {
        auto find_file = [&](const path &pak) {
            auto p = get_data_dir() / pak;
            if (!fs::exists(p)) {
                return false;
            }
            auto up = make_unpak_dir(p);
            if (!fs::exists(up)) {
                unpak(p);
            }
            p = up;
            if (!fs::exists(p / fn)) {
                p = p / fn.filename();
                if (!fs::exists(p)) {
                    return false;
                }
            } else {
                p /= fn;
            }
            fn = p;
            return true;
        };
        (find_file(paks) || ...);
        return fn;
    }
    // from https://github.com/Solant/aim-patches
    void free_camera(uint8_t val) {
        patch(aim_exe, 0x1F805, val);
    }
    void win_key(uint8_t val) {
        patch(aim_exe, 0x4A40D, val);
    }
    void create_backup_exe_file() {
        auto fn = find_real_filename(aim_exe);
        auto bak = make_bak_file(fn);
        if (fs::exists(bak)) {
            fs::copy_file(bak, fn, fs::copy_options::overwrite_existing);
        }
    }
    template <typename T>
    void patch_raw(path fn, uint32_t offset, T val) const {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        log("patching {} offset 0x{:08X} to {} (old value: {})", fn.string(), offset, val, old);
        old = val;
    }
    template <typename T>
    bool patch_raw(path fn, uint32_t offset, T expected, T val) const {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        log("patching {} offset 0x{:08X} from {} to {}", fn.string(), offset, expected, val);
        if (old == expected) {
            log("success");
            old = val;
            return true;
        } else if (old == val) {
            log("success, already patched");
            return true;
        } else {
            log("old value {} != expected {}", old, expected);
            return false;
        }
    }
    template <typename T>
    bool patch_raw(path fn, const std::string &pattern, uint32_t offset, T expected, T val) const {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto p = memmem(f.p, f.sz, pattern);
        if (!p) {
            throw std::runtime_error{"pattern not found"};
        }
        f.close();
        log("patching {} offset 0x{:X} after pattern {} from {} to {}", fn.string(), offset, pattern, expected, val);
        return patch_raw<T>(fn, p - f.p + offset, expected, val);
    }
    path get_mod_dir() const {
        return get_data_dir() / "mods" / get_full_mod_name();
    }
    path get_data_dir() const {
        return game_dir / "data";
    }
    static void replace_in_file_raw(const path &fn, const std::string &from, const std::string &to) {
        log("replacing in file {} from '{}' to '{}'", fn.string(), from, to);
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
    void unpak(const path &p, const path &fn = {}) const {
        unpak1(p,{});
    }
    void unpak1(const path &p, const path &fn = {}) const {
        auto udir = make_unpak_dir(p);
        if (fs::exists(udir) && (fn.empty() || fs::exists(udir / fn))) {
            return;
        }
        if (fn.empty()) {
            log("unpacking {}", p.string());
            run_p4_tool("unpaker", p);
        } else {
            log("unpacking {} from {}", fn.string(), p.string());
            run_p4_tool("unpaker", p, fn);
        }
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
        log(c.print());
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
        //check_in_path("git");
        // also --self-upgrade?
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
