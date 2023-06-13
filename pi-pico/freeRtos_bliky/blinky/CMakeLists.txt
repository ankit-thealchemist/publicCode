add_executable(blinky
        blinky.c
        )


target_include_directories(blinky PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )


pico_enable_stdio_uart(blinky 0)
pico_enable_stdio_usb(blinky 1)


target_link_libraries(blinky
        pico_cyw43_arch_none
        pico_stdlib
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap3
        )


pico_add_extra_outputs(blinky)

# add_executable(picow_wifi_scan_poll
#         picow_wifi_scan.c
#         )
# target_include_directories(picow_wifi_scan_poll PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}
#         ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
#         )
# target_link_libraries(picow_wifi_scan_poll
#         pico_cyw43_arch_lwip_poll
#         pico_stdlib
#         )
# pico_add_extra_outputs(picow_wifi_scan_poll)

