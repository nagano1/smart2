set(BINARY "test")
#set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB cpp_lib_files
    "./utils/utils.cpp"
    "./code_nodes/*.cpp"
    "./code_nodes/nodes/*.cpp"
    "./script_runtime/*.cpp"
)


add_executable(${BINARY}_run ${cpp_lib_files} "./test.cpp")

install(TARGETS ${BINARY}_run
        RUNTIME	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		BUNDLE	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
 		LIBRARY	DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


# add_library(${BINARY}_lib STATIC ${cpp_files})

find_package(Threads REQUIRED)
target_link_libraries(${BINARY}_run Threads::Threads)