// Arquivos de cabecalho (includes)
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/gpio_periph.h"

// Definicoes de pino (defines)
#define LED_PIN 2
// variaveis globais

unsigned char statusLED;
// protótipos de funções

#define delay(value) vTaskDelay(value / portTICK_RATE_MS)
// Main
void app_main(void)
{
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);
    volatile int contador = 0;
    int enabled = 0;
    REG_WRITE(GPIO_ENABLE_REG, BIT2);
    for (;;) { // loop infinito

        if(gpio_get_level(GPIO_NUM_33)) {
            if(enabled == 0) {
                REG_WRITE(GPIO_OUT_W1TS_REG, BIT2);
                contador ++;
            }
            printf("%d\n", contador);
            enabled = 1;
        }
        else {
            REG_WRITE(GPIO_OUT_W1TC_REG, BIT2);
            enabled = 0;
        }
        delay(150);

    } // end loop
} // end main

