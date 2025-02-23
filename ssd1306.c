
#include "ssd1306.h"

static const char * TAG = "SSD1306";


uint8_t flipByte(uint8_t byte)
{
    uint8_t flippedByte = 0x00;
    flippedByte =   ((byte & 0x1) << 7) | 
                    ((byte & 0x2) << 5) | 
                    ((byte & 0x4) << 3) | 
                    ((byte & 0x8) << 1) | 
                    ((byte & 0x10) >> 1) | 
                    ((byte & 0x20) >> 3) | 
                    ((byte & 0x40) >> 5) | 
                    ((byte & 0x80) >> 7);
    return flippedByte;
}


void initI2C(struct ssd1306 * device)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd;

    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = SSD1306SCL,
        .sda_io_num = SSD1306SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SSD1306CLKSPEED,
    };


    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // Device address and write flag.
    i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);

    // Control byte
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
    // Display off
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xae, true));
    // MUX Ratio
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xa8, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (uint8_t)(HEIGHT - 1), true));
    // Display Offset
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xd3, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
    // Display start line
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true));
    // Segment Remap
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xa0, true));
    // COM output scan direction.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xc8, true));
    // Display clock division.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xd5, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x80, true));
    // COM pin hardware configuration
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xda, true));
    if (HEIGHT == 32)
    {
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x02, true));
    }
    else
    {
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x12, true));
    }
    // Contrast
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x81, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xff, true));
    // Display RAM
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xa4, true));
    // VCOMH Deselect
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xdb, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true));
    // Memory address mode.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x20, true));
    // Page address mode.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x02, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x10, true));
    // Enable charge pump regulator
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x8d, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x14, true));
    // Deactivate scroll.
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x2e, true));
    // Set normal display
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xa6, true));
    //Set oscillator frequency
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xd5, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x80, true));
    // Display on
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xaf, true));

    // Stop bit.
    i2c_master_stop(cmd);

    // Send command.
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2CCOMMANDWAITMS / portTICK_PERIOD_MS);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Successfully initialized SSD1306.");
    }
    else
    {
        ESP_LOGI(TAG, "Failed to initialize SSD1306 with code %d.", err);
    }

    i2c_cmd_link_delete(cmd);
}


void writeDisplay(struct ssd1306 * device)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd;
    uint8_t * currPage;

    // Memory must be written page-by-page.
    for (int i = 0; i < NUM_PAGES; i++)
    {
        currPage = &device->framebuffer[i][0];

        // Control signal.
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);

        // Control byte sequence.
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x10, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xB0 | i, true));

        // Send command.
        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2CCOMMANDWAITMS / portTICK_PERIOD_MS);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write command sequence for page %d, code %d.", i, err);
        }
        i2c_cmd_link_delete(cmd);

        // Write page data command.
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);

        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true));
        ESP_ERROR_CHECK(i2c_master_write(cmd, currPage, BYTES_PER_PAGE, true));

        // Send data.
        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2CCOMMANDWAITMS / portTICK_PERIOD_MS);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to write page %d, code %d.", i, err);
        }
        i2c_cmd_link_delete(cmd);
    }

}


void clearDisplay(struct ssd1306 * device)
{
    memset(device->framebuffer, 0x00, TOTALDISPLAYMEMORY);

    writeDisplay(device);
}


int writeBitmap(struct ssd1306 * device, uint8_t * bitmap, size_t bitmapSize)
{
    if (bitmapSize != TOTALDISPLAYMEMORY)
    {
        ESP_LOGE(TAG, "Bitmap is wrong size for display.");
        return -1;
    }

    // The display expects bitmap data to come in a different format. This consists in something similar to a transpose operation.
    // For example, a bitmap image looking like:

    // Byte 0: 11111111
    // Byte 1: 00000000
    // Byte 2: 11111111
    // Byte 3: 00000000
    // Byte 4: 11111111
    // Byte 5: 00000000
    // Byte 6: 11111111
    // Byte 7: 00000000
    // ...

    // Is transformed to look like:

    // Byte 0: 10101010
    // Byte 1: 10101010
    // Byte 2: 10101010
    // Byte 3: 10101010
    // Byte 4: 10101010
    // Byte 5: 10101010
    // Byte 6: 10101010
    // Byte 7: 10101010
    // ...

    // The display has a memory layout like this:

    //                 SEG0  SEG1  ...  SEG127
    // Page 0:  Bit 0   x     x            x
    //          Bit 1   x     x            x
    //          Bit 2   x     x            x
    //          Bit 3   x     x            x
    //          Bit 4   x     x            x
    //          Bit 5   x     x            x
    //          Bit 6   x     x            x
    //          Bit 7   x     x            x

    // Which is akin to how the bits are actually shown on the display.

    clearDisplay(device);

    // Populate the bitmap bit-by-bit since we are mangling every byte into a new format.
    for (int page = 0; page < NUM_PAGES; page++)
    {
        // Loop through every bit of this page.
        for (int i = 0; i < (BYTES_PER_PAGE * BITS_PER_BYTE); i++)
        {
            int arr2width = BYTES_PER_PAGE;
            int arr2row = i / arr2width;
            int arr2col = arr2width - (i % arr2width);
            int arr1idx = (i / BITS_PER_BYTE) + (page * BYTES_PER_PAGE);
            int arr1bit = 7 - (i % BITS_PER_BYTE);

            // Perform bitmask of the current bit, populate the framebuffer with it in the correct offset.
            device->framebuffer[page][arr2col] |= (((bitmap[arr1idx] & (0x1 << arr1bit)) >> arr1bit) << (arr2row));
        }
    }

    writeDisplay(device);

    return 0;
}


int displayText(struct ssd1306 * device, char * text)
{
    clearDisplay(device);

    int fontSize = 8;

    int charsPerRow = WIDTH / fontSize;
    int numRows = HEIGHT / (fontSize + 1);
    int stringLength = strlen(text);
    int maxStringLength = charsPerRow * numRows;

    if (stringLength > maxStringLength)
    {
        stringLength = maxStringLength;
    }

    uint8_t bitmap[TOTALDISPLAYMEMORY];

    memset(bitmap, 0x00, TOTALDISPLAYMEMORY);

    // Populate bitmap with text row by row, character by character.
    for (int character = 0; character < stringLength; character++)
    {
        int textWrapRow = character / charsPerRow;

        // Each character has a number of rows equal to the fontSize.
        for (int row = 0; row < fontSize; row++)
        {
            // Calculate the index of the bitmap based on what character we are within the row, what row we're on, and how many times we've wrapped the text.
            // Add to font size here for spacing between rows.
            int bitmapIdx = (character % charsPerRow) + (charsPerRow * textWrapRow * (fontSize + 2)) + (row * charsPerRow);

            // Grab the current row of the current character.
            uint8_t currCharRowByte = font8x8_basic[(uint8_t)(text[character])][row];
            // Font is backwards, flip bits.
            uint8_t flippedFontByte = flipByte(currCharRowByte);
            bitmap[bitmapIdx] = flippedFontByte;
        }
    }

    writeBitmap(device, bitmap, TOTALDISPLAYMEMORY);

    return 0;

}