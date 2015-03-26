file(GLOB unpaker_src "unpaker/*")
add_executable(unpaker ${unpaker_src})

file(GLOB db_extractor_src "db_extractor/*")
add_executable(db_extractor ${db_extractor_src})