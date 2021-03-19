# Replace TMDS with 10 bit UART (same baud rate):
# add_definitions(-DDVI_SERIAL_DEBUG=1)
# add_definitions(-DRUN_FROM_CRYSTAL)

add_executable(nes_hdmi)

target_compile_options(nes_hdmi PRIVATE -Wall)

target_link_libraries(nes_hdmi
	pico_stdlib
	pico_multicore
	pico_util
	libdvi
)

target_sources(nes_hdmi PRIVATE main.c
                infones/InfoNES.c
                infones/InfoNES_Mapper.c
                infones/InfoNES_pAPU.c
                infones/InfoNES_System.c
                infones/K6502.c
                 )

target_include_directories(nes_hdmi PRIVATE infones/)

# create map/bin/hex file etc.
pico_add_extra_outputs(nes_hdmi)
