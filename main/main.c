#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "LCDI2C.h"

// Configuración del pin del DHT11
#define DHT_PIN 21
#define MAX_RETRIES 5  // Número máximo de reintentos

// Pines SPI y GPIO
#define MISO 4
#define CS   5
#define SCLK 2
#define MOSI 3
#define SPI_PORT spi0

// Configuración UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17

// Dirección I2C del LCD
#define ADDR_LCD 0x27

// Variables globales para la compensación de presión
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t t_fine;

// Prototipos de funciones
void Config_Uart(void);
void Config_I2C(void);
void Config_SPI(void);
bool read_dht11(uint8_t *humidity, uint8_t *temperature);
uint32_t compPress(int32_t adc_P);
void read_press_comp(void);
void read_sensors();
void LCD_Display1(float temperature, float humidity, float pressure);

int main() {
    stdio_init_all();
    Config_Uart();
    Config_I2C();
    Config_SPI();
    LCD_PICO_INIT_I2C();
    LCD_PICO_Clear();

    gpio_init(DHT_PIN);
    gpio_pull_up(DHT_PIN);
    read_press_comp();

    uint8_t data[2] = {0xF4 & 0x7F, 0x27};
    gpio_put(CS, 0);
    spi_write_blocking(SPI_PORT, data, 2);
    gpio_put(CS, 1);

    while (true) {
        read_sensors();
        sleep_ms(3000);
    }
}

void Config_Uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);
}

void Config_I2C() {
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);
}

void Config_SPI() {
    spi_init(SPI_PORT, 500000);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCLK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);
}

bool read_dht11(uint8_t *humidity, uint8_t *temperature) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    gpio_set_dir(DHT_PIN, GPIO_OUT);
    gpio_put(DHT_PIN, 0);
    sleep_us(20000);
    gpio_put(DHT_PIN, 1);
    sleep_us(50);
    gpio_set_dir(DHT_PIN, GPIO_IN);

    uint32_t timeout = 0;
    while (gpio_get(DHT_PIN) == 1) if (++timeout > 1000) return false;
    timeout = 0;
    while (gpio_get(DHT_PIN) == 0) if (++timeout > 1000) return false;
    timeout = 0;
    while (gpio_get(DHT_PIN) == 1) if (++timeout > 1000) return false;

    for (int i = 0; i < 40; i++) {
        timeout = 0;
        while (gpio_get(DHT_PIN) == 0) if (++timeout > 1000) return false;

        uint32_t start_time = time_us_32();
        timeout = 0;
        while (gpio_get(DHT_PIN) == 1) if (++timeout > 1000) return false;

        uint32_t pulse_duration = time_us_32() - start_time;
        data[i / 8] <<= 1;
        if (pulse_duration > 50) data[i / 8] |= 1;
    }

    if ((data[0] + data[1] + data[2] + data[3]) == data[4]) {
        *humidity = data[0];
        *temperature = data[2];
        return true;
    }
    return false;
}

uint32_t compPress(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)(p / 256);
}

void read_press_comp() {
    uint8_t buffer[24], reg = 0x88 | 0x80;
    gpio_put(CS, 0);
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_read_blocking(SPI_PORT, 0, buffer, 24);
    gpio_put(CS, 1);

    dig_P1 = buffer[6] | (buffer[7] << 8);
    dig_P2 = buffer[8] | (buffer[9] << 8);
    dig_P3 = buffer[10] | (buffer[11] << 8);
    dig_P4 = buffer[12] | (buffer[13] << 8);
    dig_P5 = buffer[14] | (buffer[15] << 8);
    dig_P6 = buffer[16] | (buffer[17] << 8);
    dig_P7 = buffer[18] | (buffer[19] << 8);
    dig_P8 = buffer[20] | (buffer[21] << 8);
    dig_P9 = buffer[22] | (buffer[23] << 8);
}

void read_sensors() {
    uint8_t humidity = 0, temperature = 0;
    uint8_t reg, buffer[6];
    int32_t rawpress, pressure = 0;

    if (read_dht11(&humidity, &temperature)) {
        reg = 0xF7 | 0x80;
        gpio_put(CS, 0);
        spi_write_blocking(SPI_PORT, &reg, 1);
        spi_read_blocking(SPI_PORT, 0, buffer, 6);
        gpio_put(CS, 1);

        rawpress = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((uint32_t)buffer[2] >> 4);
        pressure = compPress(rawpress);

        // Mostrar datos en UART
        char buffer_uart[64];
        snprintf(buffer_uart, sizeof(buffer_uart), "T:%dC H:%d%% P:%.2fhPa", temperature, humidity, pressure / 100.0);
        uart_puts(UART_ID, buffer_uart);
        uart_puts(UART_ID, "\n");

        // Mostrar datos en LCD
        LCD_Display1( temperature,  humidity,  pressure);
    } else {
        uart_puts(UART_ID, "Error leyendo sensores\n");
        LCD_Display1(0, 0, 0); // Mostrar ceros o mensaje de error
    }
}

void LCD_Display1(float temperature, float humidity, float pressure) {
    LCD_PICO_Clear();

    // Primera fila: Temperatura y Humedad
    LCD_PICO_SET_CURSOR(1, 0);
    char line1[16];
    snprintf(line1, sizeof(line1), "T:%dC H:%d%%", (int)temperature, (int)humidity);
    LCD_PICO_PRINT_STRINGi2c(line1);

    // Segunda fila: Presión
    LCD_PICO_SET_CURSOR(0, 1);
    char line2[16];
    snprintf(line2, sizeof(line2), "P:%.2fhPa", pressure / 100.0);
    LCD_PICO_PRINT_STRINGi2c(line2);
}
