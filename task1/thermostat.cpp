#include "mbed.h"
#include "board_freedom.h"
#include "adc.h"
#include "oled_ssd1322.h"
#include <cstdint>
#include <cstdio>
#include <vector>
 
#define MESSAGE_MAX_SIZE 50
#define OVERHEAT_TEMP 36.0
#define MAX_TEMP 35.5 // small buffer for cooling 
#define MIN_TEMP 30.0

// To set up the control board put the following wires into the control board:
// PWM Outputs:
// - Red wire from heating panel goes into PWM3
// - Black wire from heating panel goes into PWM GND
// Digital Outputs:
// - Red led cathode goes into DO6
// - Red led anode goes into DO GND
// Analog Inputs
// - Orange wire from thermostat goes into +3V3
// - Green wire goes into AI 0
// - Black wire goes into ground

// main() runs in its own thread in the OS
int main()
{
    float voltage;
    float temp;

    int overheatCount = 0;

    DigitalOut greenLed(PTB3);
    DigitalOut redBoardLed(PTB2);
    DigitalOut heatingLed(PTC12);

    board_init();

    u8g2_ClearBuffer(&oled);
    u8g2_SetFont(&oled, u8g2_font_6x12_mr);
    u8g2_SendBuffer(&oled);

    // Ready a single reusable buffer for writing text to.
    char message[MESSAGE_MAX_SIZE + 1];
    message[MESSAGE_MAX_SIZE] = '\0';

    PwmOut heater_power(PTA7); // this corresponds to PWM
    heater_power = 1;
    redBoardLed = 1;
    heatingLed = 1;

    while (true) {
        uint16_t analog_in_value = adc_read(0);

        // Assume that the values will be low when there is no voltage
        // Keep in mind that this could fail if the temp is in the negatives: edge case
        if (analog_in_value < 3500) {
            snprintf(message, MESSAGE_MAX_SIZE, "WARNING: sensor failure. Ain: %5d", analog_in_value);
            // Clear screen and write a message.
            u8g2_ClearBuffer(&oled);
            u8g2_DrawUTF8(&oled, 10, 10, message);
            u8g2_SendBuffer(&oled);

            heater_power = 0;
            redBoardLed = 0;
            heatingLed = 0;

            break;
        }

        // detect if the system overheats more than 5 consecutive seconds in a row
        // and shut down if it happens
        if (temp > OVERHEAT_TEMP) {
            overheatCount++;

            if (overheatCount > 4) {
                snprintf(message, MESSAGE_MAX_SIZE, "WARNING: Overheating detected. Exiting...");
                // Clear screen and write a message.
                u8g2_ClearBuffer(&oled);
                u8g2_DrawUTF8(&oled, 10, 10, message);
                u8g2_SendBuffer(&oled);

                heater_power = 0;
                redBoardLed = 0;
                greenLed = 0;
                heatingLed = 0;

                break;
            }
        }
        else {
            overheatCount = 0;
        }

        // Convert 16 bit value to voltage and temperature.
        voltage = analog_in_value * 3 / 65535.0;
        temp = ((voltage * 1000 - 400) / 19.5);

        snprintf(message, MESSAGE_MAX_SIZE, "Temp is: %5.2f, analog value: %5d", temp, analog_in_value);

        // Clear screen and write a message.
        u8g2_ClearBuffer(&oled);
        u8g2_DrawUTF8(&oled, 10, 10, message);
        u8g2_SendBuffer(&oled);

        // Control LEDs
        if (temp >= MIN_TEMP && temp <= MAX_TEMP) {
            redBoardLed = 0;
            greenLed = 1;
        }
        else if (temp > OVERHEAT_TEMP) {
            // Display yellow when the plate is overheating
            redBoardLed = 1;
            greenLed = 1;
        }
        else {
            // Board is warming up
            redBoardLed = 1;
            greenLed = 0;
        }

        // Control temp
        if (temp >= MAX_TEMP) {
            heater_power = 0;
            heatingLed = 0;
        }
        else if (temp <= MIN_TEMP + 0.5) { // buffer for heating
            heater_power = 1;
            heatingLed = 1;
        }

        ThisThread::sleep_for(1000ms);
    }
}