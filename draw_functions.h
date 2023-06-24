static inline uint32_t get_pixel(uint8_t x, uint8_t y) {
    uint32_t r_gamma = screen[0][x][y];
    uint32_t g_gamma = screen[1][x][y];
    uint32_t b_gamma = screen[2][x][y];
    return (b_gamma << 16) | (g_gamma << 8) | (r_gamma);
}

void clear(uint8_t colour) {
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            for (int c = 0; c < 3; c++) {
                screen[c][x][y] = colour;
            }
        }
    }
}

void draw_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    screen[0][x][y] = r;
    screen[1][x][y] = g;
    screen[2][x][y] = b;
}

void draw_char(char c, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t i,j;

    // Convert the character to an index
    c = c & 0x7F;
    if (c < ' ') {
        c = 0;
    } else {
        c -= ' ';
    }

    // printf("%u\n", c);

    // 'font' is a multidimensional array of [96][char_width]
    // which is really just a 1D array of size 96*char_width.
    const uint8_t* chr = font[c];

    // Draw pixels
    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (chr[j] & (1<<i)) {
                draw_pixel(x+j, y+i, r, g, b);
            }
            else {
                draw_pixel(x+j, y+i, 0, 0, 0);
            }
        }
    }
}

void draw_hour(datetime_t t, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    char hour[3];
    uint8_t nhour;
    if (t.hour < 13) {
        nhour = t.hour;
    } else {
        nhour = t.hour - 12;
    }
    sprintf(hour, "%.2d", nhour);
    for (int i = 0; i < 3; i++) {
        draw_char(hour[i],x+(i*CHAR_WIDTH), y, r, g, b);
    }
}

void draw_min(datetime_t t, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    char min[3];
    sprintf(min, "%.2d", t.min);
    for (int i = 0; i < 3; i++) {
        draw_char(min[i],x+(3*CHAR_WIDTH+i*CHAR_WIDTH), y, r, g, b);
    }
}

void draw_sec(datetime_t t, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    char sec[3];
    sprintf(sec, "%.2d", t.sec);
    for (int i = 0; i < 3; i++) {
        draw_char(sec[i],x+(6*CHAR_WIDTH+i*CHAR_WIDTH), y, r, g, b);
    }
}

void draw_semi(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    draw_char(':', x+(2*CHAR_WIDTH), y, r, g, b);
    draw_char(':', x+(5*CHAR_WIDTH), y, r, g, b);
}

void draw_date(datetime_t t, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    char sec[9];
    const char* day[7] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    sprintf(sec, "%s %.2d/%.2d", day[t.dotw], t.day, t.month);
    for (int i = 0; i < 9; i++) {
        draw_char(sec[i],x+(i*CHAR_WIDTH), y, r, g, b);
    }
}

void tick_col_bars() {
    if (R < 15) {
        if (get_pixel(0,0) != 0) {
            for (int i = 0; i < WIDTH; i++) {
                draw_pixel(i, 0, 0, 0, 0);
                draw_pixel(i, 31, 0, 0, 0);
            }
            pos_c = 0;
            col_c = 0;
        }
    }
    else {
        draw_pixel(pos_c, 0, colours[col_c][0]*multiplyer, colours[col_c][1]*multiplyer, colours[col_c][2]*multiplyer);
        draw_pixel(pos_c, 31, colours[col_c][0]*multiplyer, colours[col_c][1]*multiplyer, colours[col_c][2]*multiplyer);
        pos_c += 1;
        col_c += 1;
        if (pos_c == 64) {
            pos_c = 0;
        }
        if (col_c == 153) {
            col_c = 0;
        }
    }
}