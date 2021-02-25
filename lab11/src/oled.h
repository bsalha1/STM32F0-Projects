void setup_spi2_oled();
void init_oled();

// Overwrites display with message
void oled_display1(const char * message); // line 1
void oled_display2(const char * message); // line 2

// Clears display before writing to display
void oled_cdisplay1(const char * message); // line 1
void oled_cdisplay2(const char * message); // line 2

// Clears displays
void oled_clear_display1();
void oled_clear_display2();
void oled_clear();
