file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})

file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})

file(GLOB obj_extractor_src "obj_extractor/*")
add_executable(obj_extractor ${obj_extractor_src})
target_link_libraries(obj_extractor DatabaseManager)

file(GLOB script2txt_src "script2txt/*")
add_executable(script2txt ${script2txt_src})
