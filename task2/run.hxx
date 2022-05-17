#include "mbed.h" 
#include "physcom.h" 
using namespace physcom;
Serial pc(USBTX, USBRX); // open a serial communication to terminal emulator
M3pi robot; //create an object of type M3pi

void set_speed(float left, float right) {
    robot.activate_motor (0,left); // drive left motor 1/10 max speed forward
    robot.activate_motor (1,right); // drive right motor 1/10 max speed forward
};

int main() {
    pc.printf("Start calibration!\r\n");
    wait(2);
    robot.sensor_auto_calibrate(); //robot executes calibration
    pc.printf("Finished calibration!\r\n");
    float min_black_value = 700; // assume something
    
    while (1) { // while loop to move the sensor around until an edge case
        int sensors[5]; // array init sensors
        robot.calibrated_sensors(sensors); //5 values are read in a vector
        pc.printf("calibrating light sensor\r\n");
        pc.printf("outside sensor values: %d; %d; %d; %d; %d;\r\n", sensors[0], sensors[1],sensors[2], sensors[3], sensors[4]);
        int current_sensor = 0;
        for (int i = 0; i < 5; i++) {
            if (sensors[i] > sensors[current_sensor]) {
                current_sensor = i;
            }
        }
        
        if (sensors[0] > 700 && sensors[1] > 700 && sensors[2] > 700 && sensors[3] > 700) {
            set_speed(0.0, 0.0);
            break;
        }
        else if (sensors[4] > 700 && sensors[1] > 700 && sensors[2] > 700 && sensors[3] > 700) {
            set_speed(0.0, 0.0);
            break;
        }
        else if (sensors[0] > 700 && sensors[1] > 700) { // right turn
            current_sensor = -1;
            set_speed(-0.3, 0.3);
        }
        else if (sensors[3] > 700 && sensors[4] > 700) { // right turn
            current_sensor = -1;
            set_speed(0.3, -0.3);
        }
        
        // left most, turn left
        if (current_sensor == 0) {
            set_speed(0, 0.3);
        }
        else if (current_sensor == 1) {
            set_speed(0.15, 0.2);
        }
        else if (current_sensor == 2) {
            set_speed(0.2, 0.2);
        }
        else if (current_sensor == 3) {
            set_speed(0.2, 0.15);
        }
        else if (current_sensor == 4) {
            set_speed(0.3, 0);
        }
        pc.printf("\n");
    } 
    pc.printf("motor stopped\r\n");
}