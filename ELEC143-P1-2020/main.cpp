#include "mbed.h"
#include "../lib/uopmsb/uop_msb_2_0_0.h"
#include "BMP280_SPI.h"

using namespace uop_msb_200;

//On board LEDs
DigitalOut led1(LED1, 0);
DigitalOut led2(LED2, 0);
DigitalOut led3(LED3, 0);

//On board switch
DigitalIn BlueButton(USER_BUTTON);

//LCD Display
LCD_16X2_DISPLAY lcd;

//Buzzer
Buzzer buzz;

//Traffic Lights
DigitalOut trLED(TRAF_RED1_PIN, 0);
DigitalOut tyLED(TRAF_YEL1_PIN, 0);
DigitalOut tgLED(TRAF_GRN1_PIN, 0);

//Light Levels
AnalogIn ldr(AN_LDR_PIN);

//Environmental sensor
EnvironmentalSensor sensor;

void frostWarning();

float temperature = sensor.getTemperature();
float pressure = sensor.getPressure();


int main() {

    while (1) {

        while (temperature <= 0) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();

            lcd.locate(0,0);
            lcd.printf("%.1fC %.1fmB  \n", temperature, pressure);

            frostWarning();

            break;

        }

        while ((temperature > 0) && (temperature <= 10)) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();

            lcd.locate(0,0);
            lcd.printf("%.1fC, %.1fmB  \n", temperature, pressure);

            frostWarning();

            break;

        } 
        
        while ((temperature > 10) && (temperature <= 20)) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();

            lcd.locate(0,0);
            lcd.printf("%.1fC, %.1fmB  \n", temperature, pressure);

            break;

        } 
        
        while ((temperature > 20) && (temperature <= 24)) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();

            lcd.locate(0,0);
            lcd.printf("%.1fC, %.1fmB  \n", temperature, pressure);

            break;

        } 
        
        while ((temperature > 24) && (temperature <= 30)) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();


            lcd.locate(0,0);
            lcd.printf("%.1fC, %.1fmB\n", temperature, pressure);

            break;

        }

        while (temperature > 30) {

            temperature = sensor.getTemperature();
            pressure = sensor.getPressure();

            lcd.locate(0,0);
            lcd.printf("%.1fC, %.1fmB\n", temperature, pressure);

            break;

        }

        unsigned int lightVal = ldr.read_u16();
        int ldrPercentage = 100 * (lightVal)/(65536);

        while (ldrPercentage > 75) {
            
            lcd.locate(1,0);
            lcd.printf("DARK");

            wait_us(10000);

            break;

        }

        while ((ldrPercentage > 50) && (ldrPercentage <= 75)) {
            
            lcd.locate(1,0);
            lcd.printf("LOW");

            wait_us(10000);

            break;

        }

        while ((ldrPercentage > 25) && (ldrPercentage <= 50)) {
            
            lcd.locate(1,0);
            lcd.printf("DAY");

            wait_us(10000);

            break;

        }

        while ((ldrPercentage > 0) && (ldrPercentage <= 25)) {
            
            lcd.locate(1,0);
            lcd.printf("INTENSE");

            wait_us(10000);

            break;

        }

        lcd.cls();

        printf("%d\n", ldrPercentage);

    }

}



void frostWarning() {

    buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
    trLED = 1;

    wait_us(1000000);

    buzz.rest();
    trLED = 0;

    wait_us(1000000);

}

/*printf("%d\n", state);
			wait_us(100000);
			pressure = sensor.getPressure();
            int p2 = pressure;
			if (p1 == p2) {

                state = stable;

            } else if (p1 < p2) {

                state = falling;

            } else if (p1 < p2) {

                state = rising;

            } else {

                state = determine;
            }

            break;

            state = determine;
            printf("%d\n", state); */