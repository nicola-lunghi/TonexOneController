/*
 * CH422G IO Expander ESP-IDF Component
 * Refactored from original implementation
 */
#include "CH422G.h"
// #include "esp_log.h"
#include "esp_check.h"
// #include <string.h>

#define DEVICE_I2C_MASTER_FREQUENCY 400000
#define I2C_TIMEOUT_MS 10
#define CH422G_REG_WR_SET (0x48 >> 1)
#define CH422G_REG_WR_OC  (0x46 >> 1)
#define CH422G_REG_WR_IO  (0x70 >> 1)
#define CH422G_REG_RD_IO  (0x4D >> 1)
#define CH422G_Mode       0x24
#define CH422G_Mode_IO_OE 0x01
#define REG_WR_SET_DEFAULT_VAL 0x01UL
#define REG_WR_OC_DEFAULT_VAL 0x0FUL
#define REG_WR_IO_DEFAULT_VAL 0xFFUL
#define REG_WR_SET_BIT_IO_OE (1 << 0)
#define REG_DIR_DEFAULT_VAL 0xFFFUL
#define DIR_OUT_VALUE 0xFFF
#define DIR_IN_VALUE  0xF00

struct CH422G_t {
    i2c_master_dev_handle_t dev_handle_wr_set;
    i2c_master_dev_handle_t dev_handle_wr_io;
    i2c_master_dev_handle_t dev_handle_rd_io;
    i2c_master_dev_handle_t dev_handle_mode;
    SemaphoreHandle_t mutex;
    struct {
        uint8_t wr_set;
        uint8_t wr_oc;
        uint8_t wr_io;
    } regs;
};

static const char *TAG = "CH422G";

esp_err_t CH422G_new(i2c_master_bus_handle_t bus_handle, SemaphoreHandle_t mutex_handle, CH422G_t **out_handle) {
    if (!out_handle) return ESP_ERR_INVALID_ARG;
    CH422G_t *handle = calloc(1, sizeof(CH422G_t));
    if (!handle) return ESP_ERR_NO_MEM;
    handle->mutex = mutex_handle;
    handle->regs.wr_set = REG_WR_SET_DEFAULT_VAL;
    handle->regs.wr_oc = REG_WR_OC_DEFAULT_VAL;
    handle->regs.wr_io = REG_WR_IO_DEFAULT_VAL;

    i2c_device_config_t dev_config_1 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CH422G_REG_WR_SET,
        .scl_speed_hz = DEVICE_I2C_MASTER_FREQUENCY,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config_1, &handle->dev_handle_wr_set));

    i2c_device_config_t dev_config_2 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CH422G_REG_WR_IO,
        .scl_speed_hz = DEVICE_I2C_MASTER_FREQUENCY,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config_2, &handle->dev_handle_wr_io));

    i2c_device_config_t dev_config_3 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CH422G_Mode,
        .scl_speed_hz = DEVICE_I2C_MASTER_FREQUENCY,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config_3, &handle->dev_handle_mode));

    i2c_device_config_t dev_config_4 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CH422G_REG_RD_IO,
        .scl_speed_hz = DEVICE_I2C_MASTER_FREQUENCY,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config_4, &handle->dev_handle_rd_io));

    *out_handle = handle;
    return ESP_OK;
}

void CH422G_del(CH422G_t *handle) {
    if (handle) {
        // TODO: remove devices from bus if needed
        free(handle);
    }
}

esp_err_t CH422G_set_direction(CH422G_t *handle, uint8_t pin_bit, uint8_t value) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    esp_err_t res = ESP_FAIL;
    uint8_t data = handle->regs.wr_set;
    if (value) {
        data |= 1 << pin_bit;
    } else {
        data &= ~(1 << pin_bit);
    }
    if (xSemaphoreTake(handle->mutex, (TickType_t)100) == pdTRUE) {
        res = i2c_master_transmit(handle->dev_handle_wr_set, &data, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        if (res == ESP_OK) {
            handle->regs.wr_set = data;
        } else {
            ESP_LOGE(TAG, "CH422G_set_direction() failed");
        }
        xSemaphoreGive(handle->mutex);
    }
    return res;
}

esp_err_t CH422G_set_all_input(CH422G_t *handle) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    uint8_t data = (uint8_t)(handle->regs.wr_set & ~REG_WR_SET_BIT_IO_OE);
    esp_err_t res = ESP_FAIL;
    if (xSemaphoreTake(handle->mutex, (TickType_t)100) == pdTRUE) {
        res = i2c_master_transmit(handle->dev_handle_wr_set, &data, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        if (res == ESP_OK) {
            handle->regs.wr_set = data;
        } else {
            ESP_LOGE(TAG, "CH422G_set_all_input() failed");
        }
        xSemaphoreGive(handle->mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(2));
    return res;
}

esp_err_t CH422G_read_input(CH422G_t *handle, uint8_t pin_bit, uint8_t *value) {
    if (!handle || !value) return ESP_ERR_INVALID_ARG;
    esp_err_t res = ESP_FAIL;
    uint16_t values = 0;
    *value = 1;
    res = CH422G_read_all_input(handle, &values);
    if (res == ESP_OK) {
        *value = (values >> pin_bit) & 0x01;
    } else {
        ESP_LOGE(TAG, "CH422G_read_input() failed");
    }
    return res;
}

esp_err_t CH422G_read_all_input(CH422G_t *handle, uint16_t *values) {
    if (!handle || !values) return ESP_ERR_INVALID_ARG;
    uint8_t temp = 0;
    uint8_t write_buf = 0;
    esp_err_t res = ESP_FAIL;
    *values = 0;
    if (xSemaphoreTake(handle->mutex, (TickType_t)100) == pdTRUE) {
        res = i2c_master_transmit(handle->dev_handle_mode, &write_buf, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        esp_rom_delay_us(1000);
        if (res == ESP_OK) {
            res = i2c_master_receive(handle->dev_handle_mode, &temp, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
            if (res == ESP_OK) {
                res = i2c_master_receive(handle->dev_handle_rd_io, &temp, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
                write_buf = CH422G_Mode_IO_OE;
                res = i2c_master_transmit(handle->dev_handle_mode, &write_buf, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
            }
        }
        xSemaphoreGive(handle->mutex);
    }
    if (res == ESP_OK) {
        *values = (uint16_t)temp;
    } else {
        ESP_LOGE(TAG, "CH422G_read_all_input() failed");
    }
    return res;
}

esp_err_t CH422G_write_output(CH422G_t *handle, uint8_t pin_bit, uint8_t value) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    esp_err_t res = ESP_FAIL;
    if (value) {
        handle->regs.wr_io |= (1 << pin_bit);
    } else {
        handle->regs.wr_io &= ~(1 << pin_bit);
    }
    if (xSemaphoreTake(handle->mutex, (TickType_t)100) == pdTRUE) {
        res = i2c_master_transmit(handle->dev_handle_wr_io, &handle->regs.wr_io, sizeof(handle->regs.wr_io), pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        if (res != ESP_OK) {
        ESP_LOGE(TAG, "CH422G_write_output() failed");
        }
        xSemaphoreGive(handle->mutex);
    }
    return res;
}

esp_err_t CH422G_set_io_mode(CH422G_t *handle, uint8_t output_mode) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    esp_err_t res = ESP_FAIL;
    uint8_t write_buf = output_mode ? CH422G_Mode_IO_OE : 0;
    if (xSemaphoreTake(handle->mutex, (TickType_t)100) == pdTRUE) {
        res = i2c_master_transmit(handle->dev_handle_mode, &write_buf, 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        xSemaphoreGive(handle->mutex);
    }
    return res;
}

esp_err_t CH422G_reset(CH422G_t *handle) {
    // Optionally reset registers here
    return ESP_OK;
}
