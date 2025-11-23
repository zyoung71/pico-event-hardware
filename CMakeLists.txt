if (NOT TARGET pico-event-hardware)

    add_library(pico-event-hardware STATIC
        src/event/Event.cpp
        src/hardware/GPIODevice.cpp
        src/hardware/Timer.cpp
        src/hardware/Button.cpp
    )
    
    target_include_directories(pico-event-hardware PUBLIC
        include
    )
    
    target_link_libraries(pico-event-hardware
        pico_stdlib
        pico_multicore
    )
    
endif()