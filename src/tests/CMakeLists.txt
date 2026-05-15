set(BINARY_A "test")
set(BINARY_B "ParseUtilsTest")

set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

file(GLOB cpp_lib_files
    "${SRC_ROOT}/utils/*.cpp"
    "${SRC_ROOT}/abstract_syntax_tree/*.cpp"
    "${SRC_ROOT}/abstract_syntax_tree/nodes/*.cpp"
    "${SRC_ROOT}/script_runtime/*.cpp"
)


add_executable(${BINARY_A}_run ${cpp_lib_files} "${SRC_ROOT}/tests/test.cpp")
add_executable(${BINARY_B}_run ${cpp_lib_files} "${SRC_ROOT}/tests/ParseUtils_test.cpp")

install(TARGETS ${BINARY_A}_run RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS ${BINARY_B}_run RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


# add_library(${BINARY}_lib STATIC ${cpp_files})

#find_package(Threads REQUIRED)
#target_link_libraries(${BINARY}_run Threads::Threads)