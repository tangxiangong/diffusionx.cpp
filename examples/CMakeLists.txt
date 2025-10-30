set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/examples)

file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE diffusionx)
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/examples
        CXX_SCAN_FOR_MODULES ON
    )
endforeach()
