cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME clock-v1) # <-- Name your project/executable here!
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
)

set(WIFI_SSID "Redpath" CACHE INTERNAL "WiFi SSID")
set(WIFI_PASSWORD "Darnbrook1" CACHE INTERNAL "WiFi password")

target_compile_definitions(${NAME} PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

target_include_directories(${NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_cyw43_arch_lwip_threadsafe_background
    pico_multicore
    hardware_rtc
    hardware_adc
    hardware_pio
    pico_stdlib
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)

pico_enable_stdio_usb(clock-v1 1)
pico_enable_stdio_uart(clock-v1 0)

pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)
