add_executable(redv-vp
        redv_main.cpp
        can.cpp
        oled.cpp
        gpio.cpp
	i2cserver.cpp
        gpio/gpiocommon.cpp
        gpio/gpio-server.cpp
        gpio/gpio-client.cpp
        ../../util/elegantEnums.cpp #sorry :(
        oled/common.cpp
        ${HEADERS})

target_link_libraries(redv-vp rv32 platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS redv-vp RUNTIME DESTINATION bin)
