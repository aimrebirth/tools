#pragma sw require header org.sw.demo.lexxmark.winflexbison.bison

void build(Solution &s)
{
    auto &tools = s.addProject("Polygon4.Tools", "master");
    tools += Git("https://github.com/aimrebirth/tools", "", "{v}");

    auto cppstd = cpp23;

    auto &common = tools.addStaticLibrary("common");
    common += cppstd;
    common.setRootDirectory("src/common");
    common.Public += "pub.egorpugin.primitives.filesystem"_dep;
    common.Public += "pub.egorpugin.primitives.templates2"_dep;

    auto add_exe_base = [&](const String &name) -> decltype(auto)
    {
        auto &t = tools.addExecutable(name);
        t.PackageDefinitions = true;
        t += cppstd;
        t.setRootDirectory("src/" + name);
        return t;
    };
    auto add_exe = [&](const String &name) -> decltype(auto)
    {
        auto &t = add_exe_base(name);
        t += "pub.egorpugin.primitives.sw.main"_dep;
        return t;
    };

    auto add_exe_with_common = [&](const String &name) -> decltype(auto)
    {
        auto &t = add_exe(name);
        t.Public += common;
        return t;
    };

    auto add_exe_with_data_manager = [&](const String &name) -> decltype(auto)
    {
        auto &t = add_exe_with_common(name);
        t.Public += "pub.lzwdgc.Polygon4.DataManager-master"_dep;
        return t;
    };

    add_exe_with_data_manager("db_add_language") += "pub.egorpugin.primitives.executor"_dep;
    add_exe_with_data_manager("db_extractor");
    add_exe_with_data_manager("db_extractor2");
    add_exe_with_data_manager("mmm_extractor");
    add_exe_with_data_manager("mmo_extractor");
    add_exe_with_common("mmp_extractor") += "org.sw.demo.intel.opencv.highgui"_dep;
    add_exe_with_common("mpj_loader");
    add_exe_with_common("paker");
    add_exe_with_common("script2txt");
    add_exe_with_common("txt2script");
    add_exe_with_common("tm_converter");
    add_exe("name_generator");
    add_exe_with_common("save_loader");
    add_exe_with_common("unpaker") +=
        "org.sw.demo.oberhumer.lzo.lzo"_dep,
        "org.sw.demo.xz_utils.lzma"_dep
        ;

    // not so simple targets
    auto &model = tools.addStaticLibrary("model");
    {
        model += cppstd;
        model.setRootDirectory("src/model");
        model.Public += common,
            "org.sw.demo.unicode.icu.i18n"_dep,
            "org.sw.demo.eigen"_dep,
            "pub.egorpugin.primitives.yaml"_dep,
            "pub.egorpugin.primitives.sw.settings"_dep
            ;
    }

    auto &aim1_mod_maker = tools.addStaticLibrary("aim1_mod_maker");
    {
        auto &t = aim1_mod_maker;
        auto name = "aim1_mod_maker";
        t.PackageDefinitions = true;
        t += cppstd;
        t.setRootDirectory("src/"s + name);
        //t.Public += "pub.egorpugin.primitives.sw.main"_dep;
        t.Public += "pub.egorpugin.primitives.command"_dep;
        t.Public += common;
    }

    add_exe("mod_reader") += model;

    path sdk = "d:/arh/apps/Autodesk/FBX/FBX SDK/2019.0";
    if (fs::exists(sdk)) {
        auto &mod_converter = add_exe("mod_converter");
        mod_converter += model;
        mod_converter += "org.sw.demo.xmlsoft.libxml2"_dep; // fbx 2020 sdk requires libxml2
        mod_converter += IncludeDirectory(sdk / "include");
        String cfg = "release";
        if (mod_converter.getBuildSettings().Native.ConfigurationType == ConfigurationType::Debug)
            cfg = "debug";
        String arch = "x64";
        if (mod_converter.getBuildSettings().TargetOS.Arch == ArchType::x86)
            arch = "x86";
        String md = "md";
        if (mod_converter.getBuildSettings().Native.MT)
            md = "mt";
        mod_converter += LinkLibrary(sdk / ("lib/vs2015/" + arch + "/" + cfg + "/libfbxsdk-" + md + ".lib"));
    }
}
