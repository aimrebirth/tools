include_directories(common)

file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})

if (WIN32)
file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})
target_link_libraries(db_extractor common)
endif()

file(GLOB obj_extractor_src "obj_extractor/*")
add_executable(obj_extractor ${obj_extractor_src})
target_link_libraries(obj_extractor DatabaseManager common)

file(GLOB script2txt_src "script2txt/*")
add_executable(script2txt ${script2txt_src})

file(GLOB mmp_extractor_src "mmp_extractor/*")
add_executable(mmp_extractor ${mmp_extractor_src})

file(GLOB mod_converter_src "mod_converter/*")
add_executable(mod_converter ${mod_converter_src})
target_link_libraries(mod_converter common)

file(GLOB tm_converter_src "tm_converter/*")
add_executable(tm_converter ${tm_converter_src})
target_link_libraries(tm_converter common)

add_subdirectory(common)