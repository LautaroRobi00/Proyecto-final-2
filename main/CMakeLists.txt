add_executable(main
        main.c
        LCDI2C.c
        )

# pull in common dependencies
target_link_libraries(main pico_stdlib hardware_i2c hardware_spi hardware_pwm hardware_uart)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)

# add url via pico_set_program_url

