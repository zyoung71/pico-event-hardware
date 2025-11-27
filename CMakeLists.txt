if (NOT TARGET pico-event-hardware)

    add_library(pico-event-hardware STATIC
        src/event/Event.cpp
        src/event/Command.cpp
        src/event/USBOverrides.cpp
        src/hardware/GPIODevice.cpp
        src/hardware/Timer.cpp
        src/hardware/Button.cpp
        src/hardware/SerialUSB.cpp
    )
    
    add_subdirectory(lib/pico-util)
    add_subdirectory(lib/pico-comms)

    target_include_directories(pico-event-hardware PUBLIC
        include
    )
    
    target_link_libraries(pico-event-hardware
        pico-util
        pico-comms
    )
    
endif()