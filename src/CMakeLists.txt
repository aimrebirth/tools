file(GLOB common_src "common/*")
add_library(common ${common_src})
target_include_directories(common PUBLIC common)
target_link_libraries(common
    pvt.egorpugin.primitives.filesystem
    pvt.egorpugin.primitives.executor
)
add_dependencies(common version)

if (WIN32)
file(GLOB db_add_language_src "db_add_language/*")
add_executable(db_add_language ${db_add_language_src} db_extractor/db.cpp)
target_link_libraries(db_add_language
    common
    pvt.lzwdgc.polygon4.data_manager.data_manager
)

file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})
target_link_libraries(db_extractor common)

file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})
endif()

file(GLOB mmm_extractor_src "mmm_extractor/*")
add_executable(mmm_extractor ${mmm_extractor_src})
target_link_libraries(mmm_extractor
    common
    pvt.lzwdgc.polygon4.data_manager.data_manager
)

file(GLOB mmo_extractor_src "mmo_extractor/*")
add_executable(mmo_extractor ${mmo_extractor_src})
target_link_libraries(mmo_extractor
    common
    pvt.lzwdgc.polygon4.data_manager.data_manager
)

file(GLOB mmp_extractor_src "mmp_extractor/*")
add_executable(mmp_extractor ${mmp_extractor_src})
target_link_libraries(mmp_extractor common)

file(GLOB model_src "model/*")
add_library(model ${model_src})
target_link_libraries(model
    common
    pvt.cppan.demo.unicode.icu.i18n
)
target_include_directories(model PUBLIC model)

if (DEFINED FBX_SDK_ROOT)
    file(GLOB mod_converter_src "mod_converter/*")
    add_executable(mod_converter ${mod_converter_src})
    target_link_libraries(mod_converter
        common
        model
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

file(GLOB mod_reader_src "mod_reader/*")
add_executable(mod_reader ${mod_reader_src})
target_link_libraries(mod_reader model)

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

add_subdirectory(script2txt)
