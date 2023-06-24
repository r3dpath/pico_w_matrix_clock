#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hub75.pio.h"
#include <stdio.h>
#include <time.h>
#include "fonts.h"
#include "pico/multicore.h"
#include <string.h>
#include "hardware/adc.h"
#include "pico/time.h"

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define ROWSEL_N_PINS 4
#define CLK_PIN 11
#define STROBE_PIN 12
#define OEN_PIN 13

#define WIDTH 64
#define HEIGHT 32
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define R_I 255
#define G_I 200
#define B_I 200

double multiplyer;

unsigned char screen[3][64][32];
uint8_t R,G,B;
static volatile bool sync_time = false;

#include "rtc_functions.h"
#include "draw_functions.h"


datetime_t RTC_init() {
    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2023,
            .month = 1,
            .day   = 1,
            .dotw  = 0, 
            .hour  = 3,
            .min   = 59,
            .sec   = 50
    };
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);
    return t;
}

void adjust_brightness() {
    double result = 4096 - adc_read();
    if (result > 2500) {
        result = 2500;
    }
    if (result < 20) {
        R = 1;
        G = 0;
        B = 0;
        //printf("Brightness: %f, Red: 1, Green: 0, Blue: 0\r\n", result);
        return;
    }
    multiplyer = (result/2500);
    R = R_I * multiplyer;
    G = G_I * multiplyer;
    B = B_I * multiplyer;
    //printf("Brightness: %f, Multiplyer: %f, Red: %u, Green: %u, Blue: %u\r\n", result, multiplyer, R, G, B);
}

void core1() {
    datetime_t t = RTC_init();
    adjust_brightness();
    while(1) {
        rtc_get_datetime(&t);
        if ((double)(t.sec % 20)==0) {
            adjust_brightness();
            draw_hour(t, 8, 9, R, G, B);
            draw_min(t, 8, 9, R, G, B);
            draw_semi(8, 9, R, G, B);
            draw_date(t, 5, 18, R, G, B);
        }
        draw_sec(t, 8, 9, R, G, B);
        tick_col_bars();
        //printf("Time is %.2u:%.2u:%.2u\n", t.hour, t.min, t.sec);
        sleep_ms(1000);
    }
    
}

int main() {
    stdio_init_all();
    PIO pio = pio0;
    uint sm_data = 0;
    uint sm_row = 1;
    repeating_timer_t timer;
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);
    clear(0);

    multicore_launch_core1(core1);
    sleep_ms(1000);
    datetime_t sync_alarm = {
            .year  = -1,
            .month = -1,
            .day   = -1,
            .dotw  = -1,
            .hour  = 4,
            .min   = 0,
            .sec   = 0
    };
    //update_rtc();
    rtc_set_alarm(&sync_alarm, &alarm_callback);

    uint data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    uint row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, DATA_BASE_PIN, CLK_PIN);
    hub75_row_program_init(pio, sm_row, row_prog_offs, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);

    static uint32_t gc_row[2][WIDTH];

    while (1) {
        if (sync_time) {
            update_rtc();
            sync_time = false;
        }
        for (int rowsel = 0; rowsel < (1 << ROWSEL_N_PINS); ++rowsel) {
            for (int x = 0; x < WIDTH; ++x) {
                gc_row[0][x] = get_pixel(x, rowsel);
                gc_row[1][x] = get_pixel(x, rowsel+16);
            }
            for (int bit = 0; bit < 8; ++bit) {
                hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
                for (int x = 0; x < WIDTH; ++x) {
                    pio_sm_put_blocking(pio, sm_data, gc_row[0][x]);
                    pio_sm_put_blocking(pio, sm_data, gc_row[1][x]);
                }
                // Dummy pixel per lane
                pio_sm_put_blocking(pio, sm_data, 0);
                pio_sm_put_blocking(pio, sm_data, 0);
                // SM is finished when it stalls on empty TX FIFO
                hub75_wait_tx_stall(pio, sm_data);
                // Also check that previous OEn pulse is finished, else things can get out of sequence
                hub75_wait_tx_stall(pio, sm_row);

                // Latch row data, pulse output enable for new row.
                pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 5));
            }
        }
    }
}