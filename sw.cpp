#pragma sw require header org.sw.demo.lexxmark.winflexbison.bison

void build(Solution &s)
{
    auto &tools = s.addProject("Polygon4.Tools", "master");
    tools += Git("https://github.com/aimrebirth/tools", "", "{v}");

    auto &common = tools.addStaticLibrary("common");
    common += cpp20;
    common.setRootDirectory("src/common");
    common.Public += "pub.egorpugin.primitives.filesystem-master"_dep;

    auto add_exe = [&tools](const String &name) -> decltype(auto)
    {
        auto &t = tools.addExecutable(name);
        t += cpp20;
        t.setRootDirectory("src/" + name);
        t += "pub.egorpugin.primitives.sw.main-master"_dep;
        return t;
    };

    auto add_exe_with_common = [&add_exe, &common](const String &name) -> decltype(auto)
    {
        auto &t = add_exe(name);
        t.Public += common;
        return t;
    };

    auto add_exe_with_data_manager = [&add_exe_with_common](const String &name) -> decltype(auto)
    {
        auto &t = add_exe_with_common(name);
        t.Public += "pub.lzwdgc.Polygon4.DataManager-master"_dep;
        return t;
    };

    add_exe_with_data_manager("db_add_language") += "pub.egorpugin.primitives.executor-master"_dep;
    add_exe_with_data_manager("db_extractor");
    add_exe_with_data_manager("mmm_extractor");
    add_exe_with_data_manager("mmo_extractor");
    add_exe_with_common("mmp_extractor") += "org.sw.demo.intel.opencv.highgui-*"_dep;
    add_exe_with_common("mpj_loader");
    add_exe_with_common("tm_converter");
    add_exe("name_generator");
    add_exe_with_common("save_loader");
    if (common.getBuildSettings().TargetOS.Arch == ArchType::x86)
        add_exe("unpaker"); // 32-bit only

    // not so simple targets
    auto &script2txt = tools.addStaticLibrary("script2txt");
    script2txt += cpp20;
    script2txt.setRootDirectory("src/script2txt");
    script2txt += "pub.lzwdgc.Polygon4.DataManager.schema-master"_dep;
    gen_flex_bison_pair("org.sw.demo.lexxmark.winflexbison"_dep, script2txt, "LALR1_CPP_VARIANT_PARSER", "script2txt");
    if (script2txt.getCompilerType() == CompilerType::MSVC)
        script2txt.CompileOptions.push_back("/Zc:__cplusplus");

    auto &model = tools.addStaticLibrary("model");
    model += cpp20;
    model.setRootDirectory("src/model");
    model.Public += common,
        "org.sw.demo.unicode.icu.i18n"_dep,
        "org.sw.demo.eigen"_dep,
        "pub.egorpugin.primitives.yaml-master"_dep,
        "pub.egorpugin.primitives.sw.settings-master"_dep
        ;

    add_exe("mod_reader") += model;

    auto &mod_converter = add_exe("mod_converter");
    mod_converter += model;
    path sdk = "d:/arh/apps/Autodesk/FBX/FBX SDK/2020.2";
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
