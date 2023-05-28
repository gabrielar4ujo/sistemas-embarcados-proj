/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "ds18b20.h" //Include library

#define TRIGGER_PIN GPIO_NUM_12  // pino trigger do sensor ultrassonico
#define ECHO_PIN GPIO_NUM_14     // pino echo do sensor ultrassonico
#define WATER_TANK_HEIGHT_CM 300 // altura maxima do reservatorio

#define PIN_DS18B20 GPIO_NUM_32 // pino data do sensor de temp

#define DISTANCE_CONTROL GPIO_NUM_18
#define TEMPERATURE_CONTROL GPIO_NUM_19

#define MIN_TEMPERATURE 35

volatile double waterDistance = 0;

double calculateWaterPercent()
{
    return ((WATER_TANK_HEIGHT_CM - waterDistance) / WATER_TANK_HEIGHT_CM) * 100; // determina a porcentagem ocupada por agua
}

void hcsr04_task(void *pvParameters)
{
    gpio_pad_select_gpio(TRIGGER_PIN);
    gpio_pad_select_gpio(ECHO_PIN);
    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

    while (1) // rotina de leitura de distancia do nivel de agua até o sensor
    {
        // Pulso no pino de trigger
        gpio_set_level(TRIGGER_PIN, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(TRIGGER_PIN, 0);

        // Aguardar resposta no pino echo
        int64_t pulse_start = 0;
        int64_t pulse_end = 0;

        while (gpio_get_level(ECHO_PIN) == 0)
        {
            pulse_start = esp_timer_get_time();
        }
        while (gpio_get_level(ECHO_PIN) == 1)
        {
            pulse_end = esp_timer_get_time();
        }

        // Calcular a duração do pulso em microssegundos
        int64_t pulse_duration = pulse_end - pulse_start;

        // Calcular a distância em centímetros
        waterDistance = pulse_duration * 0.0343 / 2; // Fórmula para calcular a distância
        float waterPercentage = calculateWaterPercent();

        if (waterPercentage > (90) || (waterDistance < 10))
        {
            gpio_set_level(DISTANCE_CONTROL, 1);
        }
        else
        {
            gpio_set_level(DISTANCE_CONTROL, 0);
        }
        printf("%d level \n", gpio_get_level(DISTANCE_CONTROL));
        printf("%d checking", waterPercentage > (90) || (waterDistance < 10));

        printf("Distancia: %.2f cm\n", waterDistance);
        printf("Porcentagem de agua: %.2f %%\n", waterPercentage);

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void temperature_task(void *pvParameters)
{

    ds18b20_init(PIN_DS18B20);

    while (1)
    {
        float current_temp = ds18b20_get_temp();
        if (current_temp < MIN_TEMPERATURE)
        {
            gpio_set_level(TEMPERATURE_CONTROL, 1);
        }
        else
        {
            gpio_set_level(TEMPERATURE_CONTROL, 0);
        }
        printf("Temperature: %0.1f\n", current_temp);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{

    gpio_pad_select_gpio(DISTANCE_CONTROL);
    gpio_set_direction(DISTANCE_CONTROL, GPIO_MODE_OUTPUT);
    // gpio_set_level(DISTANCE_CONTROL, 0);

    gpio_pad_select_gpio(TEMPERATURE_CONTROL);
    gpio_set_direction(TEMPERATURE_CONTROL, GPIO_MODE_OUTPUT);
    // gpio_set_level(TEMPERATURE_CONTROL, 0);

    /*
      No lugar do 2048 estava "configMINIMAL_STACK_SIZE" (768 bytes), com isso
      a task só rodava uma vez, ai coloquei 2048 bytes
    */
    xTaskCreate(hcsr04_task, "hcsr04_task", 2048, NULL, 5, NULL);

    xTaskCreatePinnedToCore(&temperature_task, "mainTask", 2048, NULL, 5, NULL, 0);
}
