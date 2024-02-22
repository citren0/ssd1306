#include "driver/i2c.h"
#include "string.h"
#include "esp_log.h"
#include "ssd1306.h"


static const char * TAG = "SSD1306";



void initI2C(struct ssd1306 * device)
{

    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = 12,
        .sda_io_num = 11,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));

    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));


    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    // Device address and write flag.
    i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);
    // Control byte
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
    // Display off
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xae, true));
    // MUX Ratio
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xa8, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x3f, true));
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
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x12, true));
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
    // Set oscillator frequency
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xd5, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x80, true));
    // Display on
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xaf, true));

    // Stop bit.
    i2c_master_stop(cmd);

    // Send command.
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);

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

    // Memory page.
    for (int i = 0; i < NUM_PAGES; i++)
    {
        uint8_t * currpage = &device->framebuffer[i][0];

        // Control signal.
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);

        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x10, true));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xB0 | i, true));

        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        // Write page.
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SSD1306ADDRESS << 1) | I2C_MASTER_WRITE, true);

        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true));
        ESP_ERROR_CHECK(i2c_master_write(cmd, currpage, BYTES_PER_PAGE, true));

        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }

}


void clearDisplay(struct ssd1306 * device)
{
    memset(device->framebuffer, 0x00, NUM_PAGES * BYTES_PER_PAGE);

    writeDisplay(device);
}


int writeBitmap(struct ssd1306 * device, uint8_t * bitmap, size_t bitmapSize)
{
    if (bitmapSize != NUM_PAGES * BYTES_PER_PAGE)
    {
        ESP_LOGE(TAG, "Bitmap is wrong size for display.");
        return -1;
    }

    clearDisplay(device);

    for (int page = 0; page < NUM_PAGES; page++)
    {
        for (int i = 0; i < bitmapSize * BITS_PER_BYTE / NUM_PAGES; i++)
        {
            int arr2width = BYTES_PER_PAGE;
            int arr2row = i / arr2width;
            int arr2col = i % arr2width;
            int arr1idx = (i / BITS_PER_BYTE) + (page * BYTES_PER_PAGE);
            int arr1bit = 7 - (i % BITS_PER_BYTE);

            device->framebuffer[page][arr2col] |= (((bitmap[arr1idx] & (0x1 << arr1bit)) >> arr1bit) << (arr2row));
        }
    }

    writeDisplay(device);

    return 0;
}