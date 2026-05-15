set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

file(GLOB cpp_lib_files
    "${SRC_ROOT}/utils/*.cpp"
    "${SRC_ROOT}/abstract_syntax_tree/*.cpp"
    "${SRC_ROOT}/abstract_syntax_tree/nodes/*.cpp"
    "${SRC_ROOT}/script_runtime/*.cpp"
)

file(GLOB test_files
    "${SRC_ROOT}/tests/*.cpp"
)

foreach (file ${test_files})
    get_filename_component(filename ${file} NAME_WE)
    add_executable(${filename}_run ${cpp_lib_files} ${file})
    install(TARGETS ${filename}_run RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endforeach()

# add_library(${BINARY}_lib STATIC ${cpp_files})

#find_package(Threads REQUIRED)
#target_link_libraries(${BINARY}_run Threads::Threads)