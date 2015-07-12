file(GLOB common_src "*.h" "*.cpp")
add_library(common ${common_src})
add_dependencies(common version)