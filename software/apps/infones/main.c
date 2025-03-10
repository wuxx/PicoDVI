#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "common_dvi_pin_configs.h"

//#include "testcard_320x240_rgb565.h"

// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

struct dvi_inst dvi0;

void core1_main()
{
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	while (queue_is_empty(&dvi0.q_colour_valid))
		__wfe();
	dvi_start(&dvi0);
	dvi_scanbuf_main_16bpp(&dvi0);
}

/* 256 x 240 rgb565 */
extern unsigned short *WorkFrame;


#define NES_DISP_WIDTH  256
#define NES_DISP_HEIGHT 240

void  __not_in_flash("fb_flush") tft_flush() {
    //static uint16_t scanline[320] = {0};
    uint16_t *fb = (unsigned short *)&WorkFrame;

    if (fb == NULL) {
        return;
    }

    for (uint y = 0; y < FRAME_HEIGHT; ++y) {

        //memcpy(scanline, &fb[y * NES_DISP_WIDTH], NES_DISP_WIDTH * sizeof(uint16_t));

        const uint16_t *scanline = &fb[y * NES_DISP_WIDTH];

        //const uint16_t *scanline = &((const uint16_t*)testcard_320x240)[y * FRAME_WIDTH];

        queue_add_blocking_u32(&dvi0.q_colour_valid, &scanline);
        while (queue_try_remove_u32(&dvi0.q_colour_free, &scanline));
    }

}

extern int InfoNES_Load( const char *pszFileName );
extern void InfoNES_Main();

int main() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

	setup_default_uart();

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DEFAULT_DVI_SERIAL_CONFIG;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Core 1 will wait until it sees the first colour buffer, then start up the
	// DVI signalling.
	multicore_launch_core1(core1_main);

    while (1) {
        if(InfoNES_Load(NULL) == 0) {
            InfoNES_Main();
        }
    }

    while(1);

#if 0
	// Pass out pointers into our preprepared image, discard the pointers when
	// returned to us. Use frame_ctr to scroll the image
	uint frame_ctr = 0;
	while (true) {
		for (uint y = 0; y < FRAME_HEIGHT; ++y) {
			uint y_scroll = (y + frame_ctr) % FRAME_HEIGHT;
			const uint16_t *scanline = &((const uint16_t*)testcard_320x240)[y_scroll * FRAME_WIDTH];
			queue_add_blocking_u32(&dvi0.q_colour_valid, &scanline);
			while (queue_try_remove_u32(&dvi0.q_colour_free, &scanline))
				;
		}
		++frame_ctr;
	}
#endif

}
