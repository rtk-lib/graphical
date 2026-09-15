if(NOT DEFINED INPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE not defined")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE not defined")
endif()
if(NOT DEFINED VARIABLE_NAME)
    message(FATAL_ERROR "VARIABLE_NAME not defined")
endif()

file(READ "${INPUT_FILE}" HEX_CONTENT HEX)
string(LENGTH "${HEX_CONTENT}" HEX_LENGTH)

math(EXPR REMAINDER "${HEX_LENGTH} % 8")
if(NOT REMAINDER EQUAL 0)
    message(FATAL_ERROR "File size is not a multiple of 4 bytes: ${INPUT_FILE}")
endif()

string(REGEX MATCHALL "........" WORDS "${HEX_CONTENT}")
list(LENGTH WORDS WORD_COUNT)

set(ARRAY_CONTENT "")
foreach(WORD IN LISTS WORDS)
    string(SUBSTRING "${WORD}" 0 2 B0)
    string(SUBSTRING "${WORD}" 2 2 B1)
    string(SUBSTRING "${WORD}" 4 2 B2)
    string(SUBSTRING "${WORD}" 6 2 B3)

    string(APPEND ARRAY_CONTENT "    0x${B3}${B2}${B1}${B0},\n")
endforeach()

set(HEADER_CONTENT "#pragma once\n\n#include <array>\n#include <cstdint>\n\nnamespace rtk::shaders {\n\ninline constexpr std::array<std::uint32_t, ${WORD_COUNT}>\n${VARIABLE_NAME}{\n${ARRAY_CONTENT}};\n\n}\n")

file(WRITE "${OUTPUT_FILE}" "${HEADER_CONTENT}")
