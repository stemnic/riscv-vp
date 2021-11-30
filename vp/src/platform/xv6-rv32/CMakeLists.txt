file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(xv6-vp
		xv6_main.cpp uartsocket.cpp
        ${HEADERS})

target_link_libraries(xv6-vp rv32 platform-common gdb-mc
	${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS xv6-vp RUNTIME DESTINATION bin)
