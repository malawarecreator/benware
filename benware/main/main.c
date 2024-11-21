#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define DEBUG_PIN 2
TaskHandle_t debughandle = NULL;
int debugstatus = 0;
bool debug_killed = false;

void debug_detect(void* arg) {
    gpio_reset_pin(DEBUG_PIN);
    gpio_set_direction(DEBUG_PIN, GPIO_MODE_INPUT);

    while (1) {
        int status = gpio_get_level(DEBUG_PIN);
        if (status == 1) {
            ESP_LOGI(pcTaskGetName(NULL), "Debug detected\nSetting debug on\n");
            debugstatus = 1;
            ESP_LOGI(pcTaskGetName(NULL), "Debug started, passing control to debug_mode_handler");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void) {
    xTaskCreatePinnedToCore(debug_detect, "debug_detect", 4096, NULL, 10, &debughandle, 0);

    while (1) {
        if (debugstatus == 1 && !debug_killed) {
            ESP_LOGI(pcTaskGetName(NULL), "Debug detected, killing debug_detect\n");
            if (debughandle != NULL) {
                vTaskDelete(debughandle);
                debughandle = NULL;
                debug_killed = true;
                ESP_LOGI(pcTaskGetName(NULL), "Killed task successfully\n");
            } else {
                ESP_LOGI(pcTaskGetName(NULL), "Unable to kill task, it was never started\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}