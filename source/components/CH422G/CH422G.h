/*
 * CH422G IO Expander ESP-IDF Component
 */
#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>

#define CH422G_IO_COUNT 8

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct CH422G_t CH422G_t;

    /**
     * @brief Create a new CH422G IO expander instance
     * @param bus_handle I2C bus handle
     * @param mutex_handle Semaphore handle for I2C
     * @param[out] out_handle Pointer to store new instance
     * @return ESP_OK on success
     */
    esp_err_t CH422G_new(i2c_master_bus_handle_t bus_handle, SemaphoreHandle_t mutex_handle, CH422G_t** out_handle);

    /**
     * @brief Free CH422G instance
     */
    void CH422G_del(CH422G_t* handle);

    /**
     * @brief Set pin direction (output=1, input=0)
     */
    esp_err_t CH422G_set_direction(CH422G_t* handle, uint8_t pin_bit, uint8_t value);

    /**
     * @brief Set all pins to input
     */
    esp_err_t CH422G_set_all_input(CH422G_t* handle);

    /**
     * @brief Read input value of a pin
     */
    esp_err_t CH422G_read_input(CH422G_t* handle, uint8_t pin_bit, uint8_t* value);

    /**
     * @brief Read all input pins
     */
    esp_err_t CH422G_read_all_input(CH422G_t* handle, uint16_t* values);

    /**
     * @brief Write output value to a pin
     */
    esp_err_t CH422G_write_output(CH422G_t* handle, uint8_t pin_bit, uint8_t value);

    /**
     * @brief Set IO mode (output=1, input=0)
     */
    esp_err_t CH422G_set_io_mode(CH422G_t* handle, uint8_t output_mode);

    /**
     * @brief Reset device
     */
    esp_err_t CH422G_reset(CH422G_t* handle);

#ifdef __cplusplus
}
#endif
