#pragma once

#include "aim.exe.injections.h"

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

inline const path aim_exe = "aim.exe";

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

auto &log_file(const path *p = {}) {
    static std::ofstream ofile = [&](){
        if (!p) {
            throw std::runtime_error{"pass log filename first!"};
        }
        return std::ofstream{*p};
    }();
    return ofile;
}
void log_internal(const std::string &s) {
    std::cout << s << "\n";
    log_file() << s << std::endl;
}
void log(auto &&format, auto &&arg, auto &&...args) {
    auto s = std::format("{}", std::vformat(format, std::make_format_args(arg, args...)));
    log_internal(s);
}
void log(auto &&str) {
    auto s = std::format("{}", str);
    log_internal(s);
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

struct bin_patcher {
    enum return_code {
        ok,
        error,
        already_patched,
        pattern_not_found,
    };

    template <typename T>
    static return_code patch(const path &fn, uint32_t offset, T val, T *in_old = nullptr) {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        if (in_old) {
            *in_old = old;
        }
        old = val;
        return ok;
    }
    template <typename T>
    static return_code patch(const path &fn, uint32_t offset, T expected, T val, T *in_old = nullptr) {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &old = *(T *)(f.p + offset);
        if (in_old) {
            *in_old = old;
        }
        if (old == expected) {
            old = val;
            return ok;
        } else if (old == val) {
            return already_patched;
        } else {
            return error;
        }
    }
    template <typename T>
    static return_code patch_after_pattern(const path &fn, const std::string &pattern, uint32_t offset, T expected, T val) {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto p = memmem(f.p, f.sz, pattern);
        if (!p) {
            //return pattern_not_found;
            throw std::runtime_error{"pattern not found"};
        }
        f.close();
        return patch<T>(fn, p - f.p + offset, expected, val);
    }
    static void insert(const path &fn, uint32_t offset, auto &&data) {
        fs::resize_file(fn, fs::file_size(fn) + data.size());
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        memmove(f.p + offset + data.size(), f.p + offset, f.sz - offset - data.size());
        memcpy(f.p + offset, data.data(), data.size());
        f.close();
    }
    static void erase(const path &fn, uint32_t offset, uint32_t amount) {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        memmove(f.p + offset, f.p + offset + amount, f.sz - (offset + amount));
        f.close();
        fs::resize_file(fn, fs::file_size(fn) - amount);
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
        memcpy(old.data(), ptr, old.size());
        memcpy(ptr, to.data(), to.size());
        return std::tuple{ptr, old};
    }
    template <typename T>
    static void xor_(const path &fn, uint32_t offset, T value, bool enable) {
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &v = *(T *)(f.p + offset);
        if (enable && ((v & value) == value)) {
            return;
        }
        v ^= value;
    }
    static void replace_bin_in_file_raw(const path &fn, const std::string &from, const std::string &to) {
        if (from.size() != to.size()) {
            throw std::runtime_error{"mismatched size"};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        while (auto p = memmem(f.p, f.sz, from)) {
            memcpy(p, to.data(), to.size());
        }
    }
};

struct politics {
    struct org {
        int aggressiveness{};
        int authority{};
        std::map<std::string, int> relation;

        auto &operator[](const std::string &s) {
            return relation[s];
        }
    };
    path fn;
    std::map<std::string, org> relation;

    auto &operator[](const std::string &s) {
        if (!relation.contains(s)) {
            relation[s];
            for (auto &[n,o] : relation) {
                o[s];
                relation[s][n];
            }
        }
        return relation[s];
    }

    void init(auto &&fn) {
        this->fn = fn;
        auto lines = read_lines(fn);
        constexpr auto service_lines = 3;
        auto n_records = lines.size() - service_lines;
        auto title_vals = split_string(lines.at(0), ";", true);
        if (title_vals.size() - 1 != n_records) {
            throw std::runtime_error{"malformed title line"};
        }
        for (int other_org{1}; auto &&val : split_string(lines.at(1), ";", true) | std::views::drop(1)) {
            auto s = boost::trim_copy(val);
            if (s.empty()) {
                throw std::runtime_error{"empty aggressiveness"};
            }
            relation[title_vals[other_org]].aggressiveness = std::stoi(s);
            ++other_org;
        }
        for (int other_org{1}; auto &&val : split_string(lines.at(2), ";", true) | std::views::drop(1)) {
            auto s = boost::trim_copy(val);
            if (s.empty()) {
                throw std::runtime_error{"empty authority"};
            }
            relation[title_vals[other_org]].authority = std::stoi(s);
            ++other_org;
        }
        for (int norg{1}; auto &&line : lines | std::views::drop(service_lines)) {
            auto vals = split_string(line, ";", true);
            if (vals.size() - 1 > n_records) {
                throw std::runtime_error{"malformed line"};
            }
            vals.resize(n_records + 1);
            if (vals[0] != title_vals[norg]) {
                throw std::runtime_error{"bad org header"};
            }
            for (int other_org{1}; auto &&val : vals | std::views::drop(1)) {
                auto s = boost::trim_copy(val);
                if (s.empty()) {
                    s = "0";
                }
                relation[vals[0]].relation[title_vals[other_org]] = std::stoi(s);
                ++other_org;
            }
            ++norg;
        }
    }
    void write() {
        if (relation.empty()) {
            return;
        }
        std::string s;
        s += "organization relations";
        for (auto &&[n,_] : relation) {
            s += std::format(";{}", n);
        }
        s += "\n";
        s += "aggressiveness";
        for (auto &&[_,o] : relation) {
            s += std::format(";{}", o.aggressiveness);
        }
        s += "\n";
        s += "authority";
        for (auto &&[_,o] : relation) {
            s += std::format(";{}", o.authority);
        }
        s += "\n";
        for (auto &&[n,o] : relation) {
            s += n;
            for (auto &&[n2,o2] : o.relation) {
                s += std::format(";{}", o2);
            }
            s += "\n";
        }
        write_file(fn, s);
    }
};

struct mod_maker {
    struct db_wrapper {
        mod_maker &mm;
        db2::files_type::db2_internal m;
        db2::files_type::db2_internal m2;
        path fn;
        int codepage{1251};
        bool written{};

        ~db_wrapper() {
            if (!written) {
                write();
            }
        }
        void write() {
            if (!fn.empty()) {
                m.save(fn, codepage);
            }
            written = true;
        }
        auto write(const path &fn) {
            auto files = m.save(fn, codepage);
            written = true;
            return files;
        }
        auto &operator[](this auto &&d, const std::string &s) {
            return d.m[s];
        }
        auto &copy_from_aim2(db2::files_type::db2_internal &other_db, auto &&table_name, auto &&value_name, auto &&field_name) {
            m[table_name][value_name][field_name] = other_db.at(table_name).at(value_name).at(field_name);
            return m[table_name][value_name][field_name];
        }
        auto &copy_from_aim2(db2::files_type::db2_internal &other_db, auto &&table_name, auto &&value_name) {
            m[table_name][value_name] = other_db.at(table_name).at(value_name);
            return m[table_name][value_name];
        }
        void copy_from_aim2(auto && ... args) {
            if (!mm.aim2_available()) {
                return;
            }
            copy_from_aim2(m2, args...);
        }
        bool empty() const { return m.empty(); }
    };
    struct quest_wrapper {
        mod_maker &mm;
        std::map<std::string, db_wrapper> m;
        bool written{};

        auto write(const path &datadir) {
            std::set<path> files;
            for (auto &&[fn,v] : m) {
                files.merge(v.write(datadir / ("quest_" + fn)));
            }
            written = true;
            return files;
        }
        auto &operator[](this auto &&d, const std::string &s) {
            if (!d.m.contains(s)) {
                d.m.emplace(s, db_wrapper{d.mm});
            }
            return d.m.find(s)->second;
        }
        void copy_from_aim2(auto && ... args) {
            if (!mm.aim2_available()) {
                return;
            }
            for (auto &&[_, v] : m) {
                try {
                    if (!v.m2.empty()) {
                        v.copy_from_aim2(args...);
                    } else {
                        // fallback
                        v.copy_from_aim2(this->operator[]("en_US").m2, args...);
                    }
                } catch (std::exception &e) {
                    // can be missing
                }
            }
        }
        bool empty() const { return m.empty(); }
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
    std::set<path> files_to_pak_mmp;
    std::set<path> files_to_distribute;
    //std::map<path, path> files_to_distribute2;
    std::set<path> code_files_to_distribute;
    std::set<path> restored_files;
    std::set<path> copied_files;
    std::source_location loc;
    db_wrapper dw{*this};
    quest_wrapper qw{*this};
    bool injections_prepared{};
    uint8_t number_of_sectors{8};
    politics pol;

    mod_maker(std::source_location loc = std::source_location::current()) : loc{loc} {
        init(fs::current_path());
    }
    mod_maker(const std::string &name, std::source_location loc = std::source_location::current()) : name{name}, loc{loc} {
        init(fs::current_path());
    }
    mod_maker(const std::string &name, const path &dir, std::source_location loc = std::source_location::current()) : name{name}, loc{loc} {
        init(dir);
    }

    auto &politics() {
        auto f = get_data_dir() / "startpolitics.csv";
        backup_or_restore_once(f);
        files_to_distribute.insert(f);
        pol = decltype(pol){};
        pol.init(f);
        return pol;
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
            replace_text_in_file_raw(txt, from, to);
            run_p4_tool("txt2script", txt);
            files_to_pak.insert(get_mod_dir() / txt.stem());
            break;
        }
        case file_type::model: {
            auto p = find_real_filename(fn);
            log("replacing bin in file {} from '{}' to '{}'", p.string(), from, to);
            bin_patcher::replace_bin_in_file_raw(p, from, to);
            files_to_pak.insert(get_mod_dir() / p.stem());
            break;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    void apply() {
        pol.write();
        dw.write();
        auto quest_dbs = qw.write(get_data_dir());
        files_to_distribute.merge(quest_dbs);

        auto do_pak = [&](auto &&in_files, auto &&in_fn) {
            std::vector<std::string> files;
            for (auto &&p : in_files) {
                if (p.filename() == aim_exe) {
                    continue;
                }
                files.push_back(p.string());
            }
            auto fn = get_mod_dir() / in_fn += ".pak"s;
            if (!files.empty()) {
                run_p4_tool("paker", fn, files);
                fs::copy_file(fn, get_data_dir() / fn.filename(), fs::copy_options::overwrite_existing);
                files_to_distribute.insert(path{"data"} / fn.filename());
            }
        };
        do_pak(files_to_pak, get_full_mod_name());
        do_pak(files_to_pak_mmp, get_full_mod_name() += "_mmp");
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
        log("Done! Your mod {} is ready!", get_full_mod_name());
    }

    template <typename T>
    void patch_after_pattern(path fn, const std::string &pattern, uint32_t offset, T oldval, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        log("patching {} offset 0x{:X} after pattern {} from {} to {}", fn.string(), offset, pattern, oldval, val);
        auto r = bin_patcher::patch_after_pattern(fn, pattern, offset, oldval, val);
    }
    template <typename T>
    void patch(path fn, uint32_t offset, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        log("patching {} offset 0x{:08X} to {}", fn.string(), offset, val);
        auto old = val;
        auto r = bin_patcher::patch(fn, offset, val, &old);
        log("patched {} offset 0x{:08X} to {} (old value: {})", fn.string(), offset, val, old);
    }
    // this one checks for old value as well, so incorrect positions (files) won't be patched
    template <typename T>
    void patch(path fn, uint32_t offset, T oldval, T val) {
        fn = find_real_filename(fn);
        files_to_pak.insert(fn);
        auto old = val;
        auto r = bin_patcher::patch(fn, offset, oldval, val, &old);
        if (r == bin_patcher::ok) {
        log("patching {} offset 0x{:08X} from {} to {}", fn.string(), offset, oldval, val);
            log("ok");
        } else if (r == bin_patcher::already_patched) {
            log("ok, already patched");
        } else {
            log("old value {} != expected {}", old, oldval);
        }
    }
    void insert(path fn, uint32_t offset, const byte_array &data) {
        files_to_pak.insert(find_real_filename(fn));

        log("inserting into {} offset 0x{:08X} {} bytes", fn.string(), offset, data.size());

        auto rfn = find_real_filename(fn);
        bin_patcher::insert(rfn, offset, data);
    }
    std::string add_map_good(path mmo_fn, const std::string &building_name, const std::string &after_good_name, const mmo_storage2::map_good &mg) {
        log("adding map good to {} after {}: ", building_name, after_good_name, std::string{mg.name});
        if (!std::string{mg.cond}.empty()) {
            log("cond: {}", std::string{mg.cond});
        }

        byte_array data((uint8_t*)&mg, (uint8_t*)&mg + sizeof(mg));
        add_map_good(mmo_fn, building_name, after_good_name, data);
        return mg.name;
    }
    void add_map_good(path mmo_fn, const std::string &building_name, const std::string &after_good_name, const byte_array &data) {
        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

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
    auto get_mmo_storage(path mmo_fn) {
        auto fn = find_real_filename(mmo_fn);
        mmo_storage2 m{fn};
        m.load();
        return m;
    }
    void hide_mech_group(path mmo_fn, const std::string &name) {
        log("hiding mech group {} in loc {}", name, mmo_fn.string());

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto n = *(uint32_t*)(f.p + it->second.n_mechs_offset);
        auto &hidden = *(uint8_t*)(f.p + it->second.mechs_offset + n * 0x20);
        hidden = 1;
    }
    void delete_mech_group(path mmo_fn, const std::string &name) {
        log("deleting mech group {} in loc {}", name, mmo_fn.string());

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &n = *(uint32_t*)(f.p + m.n_mech_groups_offset);
        --n;
        f.close();
        bin_patcher::erase(fn, it->second.offset, it->second.size);
    }
    void clone_mech_group(path mmo_fn, const std::string &name, const std::string &newname) {
        log("cloninig mech group {} in loc {}, new name {}", name, mmo_fn.string(), newname);

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        if (newname.size() > 0x20-1) {
            throw std::runtime_error{"too long name: " + newname};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &n = *(uint32_t*)(f.p + m.n_mech_groups_offset);
        ++n;
        std::string data{f.p + it->second.offset, f.p + it->second.offset + it->second.size};
        strcpy(data.data(), newname.data());
        f.close();
        bin_patcher::insert(fn, m.mech_groups_offset, data);
    }
    bool set_mech_group_organization(path mmo_fn, const std::string &name, const std::string &orgname) {
        log("setting mech group {} organization {} in loc {}", name, orgname, mmo_fn.string());

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        if (orgname.size() > 0x20-1) {
            throw std::runtime_error{"too long organization name: " + orgname};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        memcpy(f.p + it->second.name_offset + 0x20, orgname.data(), orgname.size() + 1);
        return true;
    }
    void set_mech_group_type(path mmo_fn, const std::string &name, int newtype, int new_road_id = 100) {
        log("setting mech group {} in loc {} to {}", name, mmo_fn.string(), newtype);

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &mg = *(mmo_storage2::mech_group*)(f.p + it->second.offset);
        mg.type = newtype;
        mg.value1 = new_road_id;
        if (newtype == 1) {
            auto &unk = *(float*)(f.p + it->second.post_base_offset);
            unk = 0;
        }
    }
    bool rename_mech_group(path mmo_fn, const std::string &name, const std::string &newname) {
        log("renaming mech group {} in loc {} to {}", name, mmo_fn.string(), newname);

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            return false;
        }
        if (newname.size() > 0x20-1) {
            throw std::runtime_error{"too long name: " + newname};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        memcpy(f.p + it->second.name_offset, newname.data(), newname.size() + 1);
        return true;
    }
    void update_mech_group_configurations(path mmo_fn, const std::string &name, auto &&cfg, auto &&...cfgs) {
        std::string format;
        auto addname = [&](const std::string &cfg) {
            format += cfg + ",";
        };
        addname(cfg);
        (addname(cfgs),...);
        log("updating mech group {} configurations in loc {} to {}", name, mmo_fn.string(), format);

        auto fn = find_real_filename(mmo_fn);
        files_to_pak.insert(fn);

        mmo_storage2 m{fn};
        m.load();

        auto it = m.mechs.find(name);
        if (it == m.mechs.end()) {
            throw std::runtime_error{"no such mechanoid or group: " + name};
        }
        auto new_n = 1 + sizeof...(cfgs);
        if (new_n > 10) {
            throw std::runtime_error{"aim1 allows only 10 mechanoids in a group max"};
        }
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto &n = *(uint32_t*)(f.p + it->second.n_mechs_offset);
        auto oldn = n;
        n = new_n;
        f.close();

        bin_patcher::erase(fn, it->second.mechs_offset, oldn * 0x20);
        std::string newcfgs;
        newcfgs.resize(0x20 * new_n);
        auto p = newcfgs.data();
        auto add = [&](const std::string &cfg) {
            if (cfg.size() > 0x20-1) {
                throw std::runtime_error{"too long config name: " + cfg};
            }
            if (!db()["Конфигурации"].contains(cfg)) {
                throw std::runtime_error{"there is no such configuration in the database: " + cfg};
            }
            strcpy(p, cfg.data());
            p += 0x20;
        };
        add(cfg);
        (add(cfgs),...);
        bin_patcher::insert(fn, it->second.mechs_offset, newcfgs);
    }

    // all you need is to provide injection address (virtual) with size
    // handle the call instruction in 'dispatcher' symbol (naked) of your dll
    constexpr static inline auto call_command_length = 5;
    void make_injection(uint32_t virtual_address) {
        make_injection(virtual_address, get_injection_size(virtual_address));
    }
    void make_injection(uint32_t virtual_address, uint32_t size) {
        if (!injections_prepared) {
            prepare_injections();
            injections_prepared = true;
        }
        uint32_t len = size;
        if (len < call_command_length) {
            throw std::runtime_error{"jumppad must be 5 bytes atleast"};
        }
        primitives::templates2::mmap_file<uint8_t> f{find_real_filename(aim_exe),
                                                     primitives::templates2::mmap_file<uint8_t>::rw{}};
        auto ptr = f.p + virtual_address - aim_exe_v1_06_constants::our_code_start_virtual + aim_exe_v1_06_constants::trampoline_target_real;
        memcpy_and_move_ptr(ptr, make_insn_with_address("e8"_bin, aim_exe_v1_06_constants::free_data_base_virtual -
                                                         (virtual_address + call_command_length)));
        memcpy_and_move_ptr(ptr, make_nops(len - call_command_length));
        log("making injection on the virtual address 0x{:0X} (real address 0x{:0X}), size {}", virtual_address, ptr - f.p,
                     size);
    }
    void make_double_weapon_injections() {
        make_injection(aim1_fix::trade_actions_weapon_checks);
        make_injection(aim1_fix::setup_proper_weapon_slots_for_a_glider);
        make_injection(aim1_fix::put_weapon_into_the_right_slot_after_purchase);
        make_injection(aim1_fix::sell_correct_weapon);
        make_injection(aim1_fix::empty_light_weapon_message);
        make_injection(aim1_fix::empty_heavy_weapon_message);
        make_injection(aim1_fix::can_leave_trade_window);
    }
    void make_script_engine_injections() {
        make_injection(aim1_fix::script_function__ISGLIDER);
    }

#define ENABLE_DISABLE_FUNC(name, enable, disable) \
    void enable_##name() { name(enable); } \
    void disable_##name() { name(disable); }
    ENABLE_DISABLE_FUNC(large_address_aware, 1, 0)
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
    void setup_aim2_path(const path &p = {}) {
        if (!p.empty()) {
            aim2_game_dir = p;
            return;
        }
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
    void copy_from_aim2(const path &object) {
        if (object.empty()) {
            return;
        }
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
            for (auto &&t : textures) {
                try {
                    // m2 TEX_GUN_X-PARTICLE_ACCELERATORÑ.TM has bad letter C in it
                    // should be TEX_GUN_X-PARTICLE_ACCELERATORC.TM
                    path fn = std::get<std::string>(db().m2.at("Текстуры").at(t).at("FILENAME"));
                    if (fn.empty()) {
                        throw std::runtime_error{"Can't find texture: "s + t};
                    }
                    copy_from_aim2(fn);
                } catch (std::exception &) {
                    log("Can't find texture: "s + t);
                }
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
                    throw std::runtime_error{std::format("aim2: texture is not found: {}", p.string())};
                }
            }
            auto copied_fn = get_mod_dir() / path{object}.filename().string();
            fs::copy_file(p, copied_fn, fs::copy_options::overwrite_existing);
            add_resource(copied_fn);
            db().copy_from_aim2("Текстуры", path{object}.stem().string());
            break;
        }
        case file_type::sound: {
            auto p = aim2_game_dir / object;
            if (!fs::exists(p)) {
                log("aim2: sound is not found: {}", p.string());
                return;
            }
            auto copied_fn = get_mod_dir() / object;
            fs::create_directories(copied_fn.parent_path());
            fs::copy_file(p, copied_fn, fs::copy_options::overwrite_existing);
            files_to_pak.insert(copied_fn.string() + "="s + object.string());
            break;
        }
        default:
            SW_UNIMPLEMENTED;
        }
    }
    void copy_glider_from_aim2(const std::string &object) {
        if (object.empty()) {
            return;
        }
        log("copying glider from aim2: {}", object);

        db().copy_from_aim2("Глайдеры", object);
        quest().copy_from_aim2("INFORMATION", object);
        copy_from_aim2(db()["Глайдеры"][object]["MODEL"]);
    }
    void copy_texture_from_aim2(const std::string &object) {
        if (object.empty() || object == "_DEFAULT_"s) {
            return;
        }
        log("copying texture from aim2: {}", object);
        copy_from_aim2(object + ".tm");
    }
    void copy_explosion_from_aim2(const std::string &object) {
        if (object.empty()) {
            return;
        }
        log("copying explosion from aim2: {}", object);

        db().copy_from_aim2("Взрывы", object);
        for (int i = 0; i < 8; ++i) {
            std::string s = db()["Взрывы"][object]["MODEL" + std::to_string(i)];
            if (s.empty() || s == "_DEFAULT_") {
                continue;
            }
            copy_from_aim2(s);
        }
        for (int i = 0; i < 8; ++i) {
            std::string s = db()["Взрывы"][object]["TEXTURE" + std::to_string(i)];
            copy_texture_from_aim2(s);
        }
    }
    void copy_sound_from_aim2(const std::string &object) {
        if (object.empty()) {
            return;
        }
        log("copying sound from aim2: {}", object);

        db().copy_from_aim2("Звуки", object);
        copy_from_aim2(db()["Звуки"][object]["FILENAME"]);
    }
    void copy_missile_from_aim2(const std::string &object) {
        if (object.empty()) {
            return;
        }
        log("copying sound from aim2: {}", object);

        db().copy_from_aim2("Снаряды", object);
        auto &mis = db()["Снаряды"][object];
        copy_explosion_from_aim2(mis["EXPLO"]);
        copy_from_aim2(mis["MODEL"]);
        copy_from_aim2(mis["TAIL_MODEL"]);
        copy_texture_from_aim2(mis["TEXTURE"]);
        copy_texture_from_aim2(mis["TEXTURE2"]);
        copy_missile_from_aim2(mis["SUBMISSILE"]);
    }
    void copy_weapon_from_aim2(const std::string &object) {
        if (object.empty()) {
            return;
        }
        log("copying weapon from aim2: {}", object);

        db().copy_from_aim2("Оружие", object);
        quest().copy_from_aim2("INFORMATION", object);
        auto &db_ = this->db();
        auto &gun = db_["Оружие"][object];
        copy_explosion_from_aim2(gun["EXPLO"]);
        copy_from_aim2(gun["FXMODEL"]);
        copy_from_aim2(gun["FXMODEL2"]);
        copy_sound_from_aim2(gun["IDSOUND"]);
        copy_sound_from_aim2(gun["IDSOUNDEND"]);
        copy_missile_from_aim2(gun["MISSILE"]);
        copy_from_aim2(gun["MODEL"]);
        copy_texture_from_aim2(gun["SHOOTTEX"]);
        copy_texture_from_aim2(gun["SHOOTTEX1"]);
    }
    void copy_sector_from_aim1(int id_from) {
        ++number_of_sectors;
        copy_sector_from_aim1(id_from, number_of_sectors);
        //patch<uint8_t>(aim_exe, 0x12CC2, number_of_sectors - 1, number_of_sectors);
    }
    void copy_sector_from_aim1(int id_from, int id_to) {
        auto from = std::format("location{}", id_from);
        auto to = std::format("location{}", id_to);
        auto mmp = find_real_filename(from + ".mmp");
        auto mmo = find_real_filename(from + ".mmo");
        auto mmm = find_real_filename(from + ".mmm");
        //auto mmp_dest = get_mod_dir() / (to + ".mmp");
        auto mmp_dest = game_dir / (to + ".mmp");
        if (mmp != mmp_dest)
        fs::copy_file(mmp, mmp_dest, fs::copy_options::update_existing);
        if (mmo != get_mod_dir() / (to + ".mmo"))
        fs::copy_file(mmo, get_mod_dir() / (to + ".mmo"), fs::copy_options::update_existing);
        if (mmm != get_mod_dir() / (to + ".mmm"))
        fs::copy_file(mmm, get_mod_dir() / (to + ".mmm"), fs::copy_options::update_existing);
        files_to_distribute.insert(mmp_dest);
        //files_to_pak_mmp.insert(get_mod_dir() / (to + ".mmp")); // we cannot pack .mmp properly now
        files_to_pak.insert(get_mod_dir() / (to + ".mmo"));
        files_to_pak.insert(get_mod_dir() / (to + ".mmm"));
        quest()["ru_RU"]["INFORMATION"][boost::to_upper_copy(to)] = quest()["ru_RU"]["INFORMATION"][boost::to_upper_copy(from)];
        std::string s = quest()["ru_RU"]["INFORMATION"][boost::to_upper_copy(to)]["NAME"];
        s += " Copy";
        quest()["ru_RU"]["INFORMATION"][boost::to_upper_copy(to)]["NAME"] = s;
        quest()["ru_RU"]["INFORMATION"][std::format("INFO_SECTOR{}", id_to)] = quest()["ru_RU"]["INFORMATION"][std::format("INFO_SECTOR{}", id_from)];

        auto var = "INFO_LET_JUMP_NEW_SECTORS"s;
        // Здесь ты можешь совершить Переход в новые Сектора.
        s.clear();
        for (int i = 9; i <= number_of_sectors; ++i) {
            s += std::format("<link: Enter Sector {0}=LINKJUMPTO location{0}.mmp><br>", i);
        }

        quest()["ru_RU"]["INFORMATION"][var]["NAME"] = "Совершить Переход в новые Сектора";
        quest()["ru_RU"]["INFORMATION"][var]["TEXT"] = s;

        quest()["en_US"]["INFORMATION"][var]["NAME"] = "Go through the Passage into new Sectors";
        quest()["en_US"]["INFORMATION"][var]["TEXT"] = s;

        static std::once_flag o;
        std::call_once(o, [&]() {
            for (int i = 1; i <= 7; ++i) {
                replace(std::format("Script/bin/B_L{}_TUNNEL.scr", i), "_INFO(INFO_LET_JUMP)", "_INFO(INFO_LET_JUMP)\n_INFO(" + var + ")");
            }
        });
    }

    auto &db() {
        if (dw.empty()) {
            auto cp = 1251; // always 1251 or 0 probably for db
            open_db("db", cp);
            if (aim2_available()) {
                auto f = db2{aim2_game_dir / "data" / "db"}.create();
                f.open();
                dw.m2 = f.to_map(cp);
            }
        }
        return dw;
    }
    auto &quest() {
        // check if it's possible to use utf8/16 in aim game
        // | set codepages here until we fix or implement unicode
        // probably not possible, so use default codepages
        if (qw.empty()) {
            // TODO: maybe add vanilla db into translations repository as well?
            prepare_languages();
        }
        return qw;
    }
    const auto &open_aim2_db() {
        if (!aim2_available()) {
            throw std::runtime_error{"aim2 is not available, setup it first"};
        }
        return db().m2;
    }
    void prepare_languages() {
        auto trdirname = "translations";
        auto trdir = get_mod_dir().parent_path() / trdirname;
        primitives::Command c;
        c.push_back("git");
        if (!fs::exists(trdir)) {
            c.working_directory = get_mod_dir().parent_path();
            c.push_back("clone");
            c.push_back("https://github.com/aimrebirth/translations");
            c.push_back(trdirname);
        } else {
            c.working_directory = trdir;
            c.push_back("pull");
            c.push_back("origin");
            c.push_back("master");
        }
        run_command(c);
        for (auto &&p : fs::directory_iterator{trdir / "aim1"}) {
            if (!fs::is_regular_file(p) || p.path().extension() != ".json") {
                continue;
            }
            auto s = split_string(p.path().stem().string(), "_");
            auto lang = std::format("{}_{}", s.at(1), s.at(2));
            qw[lang].m.load_from_json(p);
            qw[lang].codepage = code_pages.at(s.at(1));
            auto m2fn = trdir / "aim2" / p.path().filename();
            if (fs::exists(m2fn)) {
                qw[lang].m2.load_from_json(m2fn);
            }
        }
    }

private:
    void copy_file_once(const path &from, const path &to) {
        if (auto [_, i] = copied_files.emplace(to); i) {
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
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
    void open_db(auto &&name, int db_codepage) {
        auto d = db2{get_data_dir() / name};
        auto files = d.create().get_files();
        for (auto &&f : files) {
            backup_or_restore_once(f);
            files_to_distribute.insert(f);
        }
        auto &w = dw;
        auto f = d.create();
        f.open();
        w.m = f.to_map(db_codepage);
        w.fn = d.fn;
        w.codepage = db_codepage;
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
        // can write to mod dir
        auto logfn = get_mod_dir() / "log.txt";
        log_file(&logfn);
        auto src_fn = get_mod_dir() / get_full_mod_name() += ".cpp";
        fs::copy_file(loc.file_name(), src_fn, fs::copy_options::overwrite_existing);
        code_files_to_distribute.insert(src_fn);
        detect_tools();
        create_backup_exe_file();
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
    static void memcpy_and_move_ptr(auto &ptr, const byte_array &data) {
        memcpy(ptr, data.data(), data.size());
        ptr += data.size();
    }
    static auto make_insn_with_address(auto &&insn, uint32_t addr) {
        byte_array arr(insn.size() + sizeof(addr));
        memcpy(arr.data(), insn.data(), insn.size());
        *(uint32_t *)(&arr[insn.size()]) = addr;
        return arr;
    }
    static byte_array make_nops(uint32_t len) {
        byte_array arr(len, 0x90);
        return arr;
    }
    void make_injected_dll() {
        log("making injected dll");

        auto fn = get_mod_dir() / "inject.cpp";
        write_file_if_different(fn, R"(#include <aim.exe.fixes.h>)");
        //fs::copy_file(fn, get_mod_dir() / fn.filename(), fs::copy_options::overwrite_existing);
        std::string contents;
        contents += "void build(Solution &s) {\n";
        contents += "    auto &t = s.addSharedLibrary(\"" + name + "\"";
        if (!version.empty()) {
            contents += ", \"" + version + "\"";
        }
        contents += ");\n";
        contents += "    t += cpp23;\n";
        contents += "    t += \"" + boost::replace_all_copy(fn.string(), "\\", "/") + "\";\n";
        //contents += "    t += \"INJECTED_DLL\"_def;\n";
#if !defined(NDEBUG)
        contents += "    t += \"DONT_OPTIMIZE\"_def;\n";
#endif
        contents += "t += \"pub.lzwdgc.polygon4.tools.aim1.mod_maker.injections-master\"_dep;\n";
        contents += "}\n";
        write_file_if_different(get_mod_dir() / "sw.cpp", contents);

        // when you enable debug build, you cannot distribute this dll,
        // because user systems does not have debug dll dependencies!!!
        // so we use rwdi
        auto conf = "rwdi"s;
#if defined(NDEBUG)
        conf = "r";
#endif

        primitives::Command c;
        c.working_directory = get_mod_dir();
        c.push_back("sw");
        c.push_back("build");
        c.push_back("-platform");
        c.push_back("x86");
        c.push_back("-config");
        c.push_back(conf);
        c.push_back("-config-name");
        c.push_back(conf);
        run_command(c);

        auto dllname = get_mod_dir() / ".sw" / "out" / conf / get_sw_dll_name();
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
        memcpy(p + off, to.data(), to.size());
    }
    void prepare_injections() {
#if 1 || defined(NDEBUG)
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
#if 1 || defined(NDEBUG)
        strcpy(get_sw_dll_name());
#else
        strcpy("h:\\Games\\AIM\\1\\.sw\\out\\d\\aim_fixes-0.0.1.dll"s);
#endif
        const auto jumppad = "68 30 B8 51 00"_bin; // push    offset SEH_425100
        uint32_t jump_offset = ptr - f.p - aim_exe_v1_06_constants::trampoline_base_real - jumppad.size() * 2;
        patch(f.p, virtual_to_real(0x00425105), jumppad, make_insn_with_address("e9"_bin, jump_offset));
        memcpy_and_move_ptr(ptr, jumppad); // put our removed insn
        memcpy_and_move_ptr(ptr, R"(
60                      ; pusha
)"_bin);
        memcpy_and_move_ptr(ptr, push_dll_name);
        memcpy_and_move_ptr(ptr, R"(
8B 3D D8 10 52 00       ; mov     edi, ds:LoadLibraryA
FF D7                   ; call    edi
61                      ; popa
)"_bin);
        memcpy_and_move_ptr(ptr, make_insn_with_address("e9"_bin, -(ptr - f.p - aim_exe_v1_06_constants::trampoline_base_real - jumppad.size())));
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
        case file_type::mmp: {
            auto p = find_file_in_paks(fn, "maps2.pak", "maps.pak");
            if (!fs::exists(p)) {
                throw SW_RUNTIME_ERROR("Cannot find file in archives: "s + fn.string());
            }
            auto dst = get_mod_dir() / p.filename();
            copy_file_once(p, dst);
            return dst;
        }
        case file_type::mmm: {
            auto p = find_file_in_paks(fn, "minimaps.pak");
            if (!fs::exists(p)) {
                throw SW_RUNTIME_ERROR("Cannot find file in archives: "s + fn.string());
            }
            auto dst = get_mod_dir() / p.filename();
            copy_file_once(p, dst);
            return dst;
        }
        case file_type::model: {
            if (fn.filename() == fn) {
                fn = path{"Data"} / "Models" / fn;
            }
            auto p = find_file_in_paks(fn, "res3.pak", "res2.pak", "res0.pak");
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
    void large_address_aware(uint8_t val) {
        bin_patcher::xor_(aim_exe, 0x136, (uint8_t)0x20, val);
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
    path get_mod_dir() const {
        return get_data_dir() / "mods" / get_full_mod_name();
    }
    path get_data_dir() const {
        return game_dir / "data";
    }
    static void replace_text_in_file_raw(const path &fn, const std::string &from, const std::string &to) {
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
        // for languages/translations support
        check_in_path("git");
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
