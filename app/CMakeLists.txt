# find all source files
file(GLOB_RECURSE APP_SOURCES "*.cpp")
list(FILTER APP_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_BINARY_DIR}/.*")

add_executable(fade ${APP_SOURCES})

target_compile_options(fade PRIVATE -Wall -Wextra)

target_link_libraries(fade PRIVATE FADE)

set_target_properties(fade PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
