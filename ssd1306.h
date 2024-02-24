#define WIDTH 128
#define HEIGHT 64
#define NUM_PAGES 8
#define BYTES_PER_PAGE WIDTH
#define SSD1306ADDRESS 0x3c
#define BITS_PER_BYTE 8
#define SSD1306SDA 11
#define SSD1306SCL 12
#define SSD1306CLKSPEED 400000
#define I2CCOMMANDWAITMS 10
#define TOTALDISPLAYMEMORY NUM_PAGES * BYTES_PER_PAGE


struct ssd1306
{
    uint8_t address;
    uint8_t framebuffer[NUM_PAGES][BYTES_PER_PAGE];
};


void initI2C(struct ssd1306 * device);

void writeDisplay(struct ssd1306 * device);

void clearDisplay(struct ssd1306 * device);

int writeBitmap(struct ssd1306 * device, uint8_t * bitmap, size_t bitmapSize);

int displayText(struct ssd1306 * device, char * text, uint8_t fontSize);