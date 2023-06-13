add_executable(blink
        blink.c
        )

# pull in common dependencies
target_link_libraries(blink     pico_stdlib
                                pico_cyw43_arch_none)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)

pico_enable_stdio_uart(blink 0)
pico_enable_stdio_usb(blink 1)

# add url via pico_set_program_url
# example_auto_set_url(blink)
