file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(riscview
        main.cpp
        ethernet.cpp
        display.cpp
        ${HEADERS})

target_link_libraries(riscview rv32 platform-common ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscview RUNTIME DESTINATION bin)
