#include "lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lcd";

// I2C配置
#define BSP_I2C_SDA           (GPIO_NUM_1)   // SDA引脚
#define BSP_I2C_SCL           (GPIO_NUM_2)   // SCL引脚
#define BSP_I2C_NUM           (0)            // I2C外设
#define BSP_I2C_FREQ_HZ       100000         // 100kHz

// PCA9557 IO扩展芯片配置
#define PCA9557_INPUT_PORT              0x00
#define PCA9557_OUTPUT_PORT             0x01
#define PCA9557_POLARITY_INVERSION_PORT 0x02
#define PCA9557_CONFIGURATION_PORT      0x03
#define PCA9557_SENSOR_ADDR             0x19        // PCA9557 I2C地址

#define LCD_CS_GPIO                 BIT(0)    // PCA9557_GPIO_NUM_1
#define PA_EN_GPIO                  BIT(1)    // PCA9557_GPIO_NUM_2
#define DVP_PWDN_GPIO               BIT(2)    // PCA9557_GPIO_NUM_3

#define SET_BITS(_m, _s, _v)  ((_v) ? (_m)|((_s)) : (_m)&~((_s)))

// LCD配置
#define BSP_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI3_HOST)
#define LCD_CMD_BITS               (8)
#define LCD_PARAM_BITS             (8)
#define BSP_LCD_BITS_PER_PIXEL     (16)
#define BSP_LCD_H_RES              (320)
#define BSP_LCD_V_RES              (240)
#define LCD_LEDC_CH          LEDC_CHANNEL_0

#define BSP_LCD_SPI_MOSI      (GPIO_NUM_40)
#define BSP_LCD_SPI_CLK       (GPIO_NUM_41)
#define BSP_LCD_SPI_CS        (GPIO_NUM_NC)
#define BSP_LCD_DC            (GPIO_NUM_39)
#define BSP_LCD_RST           (GPIO_NUM_NC)
#define BSP_LCD_BACKLIGHT     (GPIO_NUM_42)

// LCD面板句柄
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;

// 初始化I2C接口
bool lcd_i2c_init(void)
{
    ESP_LOGI(TAG, "Initializing I2C...");
    
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BSP_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = BSP_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BSP_I2C_FREQ_HZ
    };
    i2c_param_config(BSP_I2C_NUM, &i2c_conf);

    esp_err_t ret = i2c_driver_install(BSP_I2C_NUM, i2c_conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "I2C initialized successfully");
    return true;
}

// 读取PCA9557寄存器
static esp_err_t pca9557_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(BSP_I2C_NUM, PCA9557_SENSOR_ADDR, &reg_addr, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

// 写入PCA9557寄存器
static esp_err_t pca9557_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_write_to_device(BSP_I2C_NUM, PCA9557_SENSOR_ADDR, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);
}

// 设置PCA9557输出状态
static esp_err_t pca9557_set_output_state(uint8_t gpio_bit, uint8_t level)
{
    uint8_t data;
    esp_err_t res = pca9557_register_read(PCA9557_OUTPUT_PORT, &data, 1);
    if (res != ESP_OK) {
        return res;
    }
    return pca9557_register_write_byte(PCA9557_OUTPUT_PORT, SET_BITS(data, gpio_bit, level));
}

// 控制LCD CS引脚
static void lcd_cs(uint8_t level)
{
    pca9557_set_output_state(LCD_CS_GPIO, level);
}

// 控制摄像头电源引脚
void lcd_dvp_pwdn(uint8_t level)
{
    pca9557_set_output_state(DVP_PWDN_GPIO, level);
}

// 初始化PCA9557
bool lcd_pca9557_init(void)
{
    ESP_LOGI(TAG, "Initializing PCA9557...");
    
    // 写入控制引脚默认值 DVP_PWDN=1  PA_EN = 0  LCD_CS = 1
    esp_err_t ret = pca9557_register_write_byte(PCA9557_OUTPUT_PORT, 0x05);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9557 output port config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 把PCA9557芯片的IO1 IO1 IO2设置为输出 其它引脚保持默认的输入
    ret = pca9557_register_write_byte(PCA9557_CONFIGURATION_PORT, 0xf8);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCA9557 configuration port config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "PCA9557 initialized successfully");
    return true;
}

// 背光PWM初始化
bool lcd_backlight_init(void)
{
    ESP_LOGI(TAG, "Initializing LCD backlight...");
    
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = BSP_LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 0,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = true
    };
    
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t ret = ledc_timer_config(&LCD_backlight_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC timer config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = ledc_channel_config(&LCD_backlight_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "LCD backlight initialized successfully");
    return true;
}

// 设置背光亮度
bool lcd_backlight_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    } else if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * brightness_percent) / 100;
    
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH, duty_cycle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC set duty failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_LEDC_CH);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC update duty failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    return true;
}

// 打开背光
bool lcd_backlight_on(void)
{
    return lcd_backlight_set(100);
}

// 关闭背光
bool lcd_backlight_off(void)
{
    return lcd_backlight_set(0);
}

// LCD显示初始化
static bool lcd_display_new(void)
{
    esp_err_t ret = ESP_OK;
    
    // 初始化SPI总线
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_SPI_CLK,
        .mosi_io_num = BSP_LCD_SPI_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t),
    };
    
    ret = spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI init failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 液晶屏控制IO初始化
    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_SPI_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 2,
        .trans_queue_depth = 10,
    };
    
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "New panel IO failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 初始化液晶屏驱动芯片ST7789
    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
    };
    
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "New panel failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    esp_lcd_panel_reset(panel_handle);  // 液晶屏复位
    lcd_cs(0);  // 拉低CS引脚
    esp_lcd_panel_init(panel_handle);  // 初始化配置寄存器
    esp_lcd_panel_invert_color(panel_handle, true); // 颜色反转
    esp_lcd_panel_swap_xy(panel_handle, true);  // 显示翻转 
    esp_lcd_panel_mirror(panel_handle, true, false); // 镜像

    return true;
}

// LCD初始化
bool lcd_init(void)
{
    ESP_LOGI(TAG, "Initializing LCD...");
    
    if (!lcd_display_new()) {
        ESP_LOGE(TAG, "LCD display new failed");
        return false;
    }
    
    lcd_set_color(0x0000); // 设置整屏背景黑色
    
    esp_err_t ret = esp_lcd_panel_disp_on_off(panel_handle, true); // 打开液晶屏显示
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD display on failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    if (!lcd_backlight_on()) { // 打开背光显示
        ESP_LOGE(TAG, "LCD backlight on failed");
        return false;
    }
    
    ESP_LOGI(TAG, "LCD initialized successfully");
    return true;
}

// 设置液晶屏颜色
void lcd_set_color(uint16_t color)
{
    if (!panel_handle) {
        ESP_LOGE(TAG, "LCD panel not initialized");
        return;
    }
    
    // 分配内存 这里分配了液晶屏一行数据需要的大小
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(BSP_LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    
    if (NULL == buffer) {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return;
    } else {
        for (size_t i = 0; i < BSP_LCD_H_RES; i++) { // 给缓存中放入颜色数据
            buffer[i] = color;
        }
        for (int y = 0; y < BSP_LCD_V_RES; y++) { // 显示整屏颜色
            esp_lcd_panel_draw_bitmap(panel_handle, 0, y, BSP_LCD_H_RES, y+1, buffer);
        }
        free(buffer); // 释放内存
    }
}

// 显示图片
void lcd_draw_picture(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage)
{
    if (!panel_handle || !gImage) {
        ESP_LOGE(TAG, "LCD panel or image data not available");
        return;
    }
    
    // 分配内存 分配了需要的字节大小 且指定在外部SPIRAM中分配
    size_t pixels_byte_size = (x_end - x_start)*(y_end - y_start) * 2;
    uint16_t *pixels = (uint16_t *)heap_caps_malloc(pixels_byte_size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (NULL == pixels) {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return;
    }
    
    memcpy(pixels, gImage, pixels_byte_size);  // 把图片数据拷贝到内存
    esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end, (uint16_t *)pixels); // 显示整张图片数据
    heap_caps_free(pixels);  // 释放内存
}

// 显示摄像头帧
void lcd_draw_camera_frame(int x_start, int y_start, int width, int height, const uint8_t *frame_buf)
{
    if (!panel_handle || !frame_buf) {
        ESP_LOGE(TAG, "LCD panel or frame buffer not available");
        return;
    }
    
    esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, width, height, (uint16_t *)frame_buf);
}
