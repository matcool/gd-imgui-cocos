# modified https://gitlab.com/jhamberg/cmake-examples/-/blob/master/cmake/FileEmbed.cmake

add_library(embedded_files INTERFACE)
target_include_directories(embedded_files INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/file_embed/)

function(FileEmbedAdd file)
    FileEmbedGenerate(${file} var)
    target_sources(embedded_files INTERFACE ${var})
endfunction()

function(FileEmbedGenerate file generated_c)
    get_filename_component(base_filename ${file} NAME)
    set(output_filename "${base_filename}.c")
    string(MAKE_C_IDENTIFIER ${base_filename} c_name)

    file(READ ${file} content HEX)
    string(LENGTH "${content}" file_size)
    math(EXPR file_size "${file_size} / 2")

    string(REPEAT "[0-9a-f]" 32 PATTERN)
    set(content "\"${content}")
    string(REGEX REPLACE "${PATTERN}" "\\0\"\n    \"" content ${content})
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "\\\\x\\1" content ${content})
    set(content "${content}\"")

    set(output_c "#include \"${c_name}.h\"
uint8_t ${c_name}_data[] = {
    ${content}
}\;
size_t ${c_name}_size = ${file_size}\;
")

    set(output_h "#pragma once
#include \"stdint.h\"
extern uint8_t ${c_name}_data[]\;
extern size_t ${c_name}_size\;
")

    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/file_embed)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}file_embed)
    endif()

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/file_embed/${c_name}.cpp ${output_c})

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/file_embed/${c_name}.h ${output_h})

    set(${generated_c} ${CMAKE_CURRENT_BINARY_DIR}/file_embed/${c_name}.cpp PARENT_SCOPE)
endfunction()
