include_directories(common)

if (WIN32)
file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})

file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})
target_link_libraries(db_extractor common)
endif()

file(GLOB script2txt_src "script2txt/*")
add_executable(script2txt ${script2txt_src})

file(GLOB mmm_extractor_src "mmm_extractor/*")
add_executable(mmm_extractor ${mmm_extractor_src})
target_link_libraries(mmm_extractor DatabaseManager common)

file(GLOB mmo_extractor_src "mmo_extractor/*")
add_executable(mmo_extractor ${mmo_extractor_src})
target_link_libraries(mmo_extractor DatabaseManager common)

file(GLOB mmp_extractor_src "mmp_extractor/*")
add_executable(mmp_extractor ${mmp_extractor_src})
target_link_libraries(mmp_extractor common)

file(GLOB mod_converter_src "mod_converter/*")
add_executable(mod_converter ${mod_converter_src})
target_link_libraries(mod_converter common)

file(GLOB tm_converter_src "tm_converter/*")
add_executable(tm_converter ${tm_converter_src})
target_link_libraries(tm_converter common)

add_subdirectory(common)