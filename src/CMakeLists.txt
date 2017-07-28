include_directories(common)

if (WIN32)
file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})

file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})
target_link_libraries(db_extractor common)
endif()

file(GLOB mmm_extractor_src "mmm_extractor/*")
add_executable(mmm_extractor ${mmm_extractor_src})
target_link_libraries(mmm_extractor common pvt.lzwdgc.polygon4.data_manager.data_manager)

file(GLOB mmo_extractor_src "mmo_extractor/*")
add_executable(mmo_extractor ${mmo_extractor_src})
target_link_libraries(mmo_extractor common pvt.lzwdgc.polygon4.data_manager.data_manager)

file(GLOB mmp_extractor_src "mmp_extractor/*")
add_executable(mmp_extractor ${mmp_extractor_src})
target_link_libraries(mmp_extractor common)

if (DEFINED FBX_SDK_ROOT)
    file(GLOB mod_converter_src "mod_converter/*")
    add_executable(mod_converter ${mod_converter_src})
    target_link_libraries(mod_converter
        common
        #pvt.cppan.demo.eigen
        debug "${FBX_SDK_ROOT}/lib/vs2015/x86/debug/libfbxsdk-md.lib"
        optimized "${FBX_SDK_ROOT}/lib/vs2015/x86/release/libfbxsdk-md.lib"
    )
    target_include_directories(mod_converter
        PUBLIC "${FBX_SDK_ROOT}/include"
    )
else()
    message(STATUS "provide fbx sdk path to build mod converter")
endif()

file(GLOB mpj_loader_src "mpj_loader/*")
add_executable(mpj_loader ${mpj_loader_src})
target_link_libraries(mpj_loader common)

file(GLOB tm_converter_src "tm_converter/*")
add_executable(tm_converter ${tm_converter_src})
target_link_libraries(tm_converter common)

file(GLOB name_generator_src "name_generator/*")
add_executable(name_generator ${name_generator_src})

file(GLOB save_loader_src "save_loader/*")
add_executable(save_loader ${save_loader_src})
target_link_libraries(save_loader common)

add_subdirectory(common)
add_subdirectory(script2txt)
