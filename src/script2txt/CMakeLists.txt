#
# Polygon-4 Data generator
#

set(BSRC ${CMAKE_CURRENT_BINARY_DIR})

set(_CPP ".*\\.cpp")
set(CPP "${_CPP}$")

set(_H "(.*\\.h|.*\\.hpp)")
set(H "${_H}$")

set(H_CPP "(${H}|${CPP})")

source_group("Generated" "${BSRC}/${H_CPP}")

BISON_TARGET(grammar
    "${CMAKE_CURRENT_SOURCE_DIR}/grammar.yy"
    "${CMAKE_CURRENT_BINARY_DIR}/grammar.cpp"
)
FLEX_TARGET(lexer
    "${CMAKE_CURRENT_SOURCE_DIR}/lexer.ll"
    "${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp"
    COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.h
)
ADD_FLEX_BISON_DEPENDENCY(lexer grammar)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(parser_src
    ${BISON_grammar_OUTPUTS}
    ${FLEX_lexer_OUTPUTS}
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.h
)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/lexer.h PROPERTIES GENERATED TRUE)

file(GLOB script2txt_src "*.cpp" "*.h")

set(script2txt_src ${script2txt_src}
    grammar.yy
    lexer.ll
    ${parser_src}
    ${SCHEMA_FILE}
)

add_executable              (script2txt ${script2txt_src} ${script2txt_inc})
target_link_libraries       (script2txt common pvt.lzwdgc.polygon4.data_manager.schema)
