// ESP32 FM Synth.
// version 1.0
// Requirements: One generic I2S DAC (PCM5102).
// References: https://github.com/uncle-yong/sk1632-i2s-dma
// Note: This is an overall improvement for the original PIC32 FM synth from the
// reference link. Since the ESP32 has more horsepower and features, floating point
// operations are being used there instead of a fixed-point.

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "math.h"

#include "playtune.h"
#include "wavetable.h"
#include "songdata.h"
#include "tuningwords.h"
#include "envelope.h"

extern FMchannel ch[NUM_OF_CHANNELS];

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
}

extern "C" void app_main(void)
{
    nvs_flash_init();

   static const i2s_config_t i2s_config = {
         .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
         .sample_rate = 44100,
         .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
         .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
         .intr_alloc_flags = 0, // default interrupt priority
         .dma_buf_count = 8,
         .dma_buf_len = 64,
         .use_apll = false
    };

    static const i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 22,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver
    i2s_set_pin(I2S_NUM_0, &pin_config);

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);

    for(unsigned int i = 0; i < NUM_OF_CHANNELS; i++) {
      // Sets the ratio of the C:M :
      ch[i].setFreqMult_c(1.000f);
      ch[i].setFreqMult_m(2.000f);
      ch[i].setModMultiplier(2048);
      // The setADSR's parameters are in seconds:
      ch[i].setADSR(0.05f, 0.25f, 0.00f,0.010f);
    }

    //xTaskCreate(&generateSineTask, "generateSineTask", 2048, NULL, 5, NULL);
    xTaskCreate(&generateFModTask, "generateFModTask", 2048, NULL, 5, NULL);
    xTaskCreate(&updateNoteTask, "updateNoteTask", 2048, NULL, 5, NULL);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
