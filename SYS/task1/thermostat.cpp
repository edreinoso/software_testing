#include "mbed.h"
#include "board_freedom.h"
#include "adc.h"
#include "oled_ssd1322.h"
#include <cstdint>
#include <cstdio>
 
#define MESSAGE_MAX_SIZE 50

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
        // TODO: Handle no sensor
        uint16_t analog_in_value = adc_read(0);

        // Convert 16 bit value to voltage and temperature.
        voltage = analog_in_value * 3 / 65535.0;
        temp = ((voltage * 1000 - 400) / 19.5);

        snprintf(message, MESSAGE_MAX_SIZE, "Thermostat tempature is %5.2f", temp);

        // Clear screen and write a message.
        u8g2_ClearBuffer(&oled);
        u8g2_DrawUTF8(&oled, 10, 10, message);
        u8g2_SendBuffer(&oled);

        // Control LEDs
        if (temp >= 30.0 && temp <= 35.9) {
            redBoardLed = 0;
            greenLed = 1;
        }
        else if (temp > 36.0) {
            // Display yellow when the plate is overheating
            redBoardLed = 1;
            greenLed = 1;
        }
        else {
            // Board is warming up
            redBoardLed = 1;
            greenLed = 0;
        }

        // TODO: Prevent overheating
        // Control temp
        if (temp >= 35.0) {
            heater_power = 0;
            heatingLed = 0;
        }
        else if (temp <= 30.5) {
            heater_power = 1;
            heatingLed = 1;
        }

        ThisThread::sleep_for(1000ms);
    }
}