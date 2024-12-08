add_executable(main
        main.c
        LCDI2C.c
        )

# pull in common dependencies
target_link_libraries(main pico_stdlib hardware_i2c hardware_spi hardware_pwm hardware_uart)



# create map/bin/hex file etc.
pico_add_extra_outputs(main)

# add url via pico_set_program_url

