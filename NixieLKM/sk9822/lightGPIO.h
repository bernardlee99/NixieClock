#include <linux/gpio.h>
#include <linux/delay.h>

struct cRGB { uint8_t b; uint8_t g; uint8_t r; };

#define HIGH 1
#define LOW 0

#define DOWN 0
#define UP 1

#define PG6 198
#define PG7 199

#define LENGTH 30
struct cRGB led[LENGTH];

#define GRAD_LENGTH 15
const int grad[GRAD_LENGTH] = {0,10,25,50,100,150,200,255,200,150,100,50,25,10,0};

int topIndex = 0;
int bottomIndex = 0;
int delta = 0;


int loadingProgress = 1;
int loadingPulse = 0;
int loadingProgressDirection = UP;


int pulseBrightness = 0;
int pulseDirection = UP;


void SPI_write(int c);
void set_led_brightness(struct cRGB *ledarray, int leds, int brightness);
void set_led(struct cRGB *ledarray, int leds);
int init_gpio(void);
int close_gpio(void);
void loading(void);
void loading2(void);
void pulsing(void);
void setAll(int r, int g, int b);