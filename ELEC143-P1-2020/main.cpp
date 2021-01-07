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

void rLEDFlash();
void frostWarning();

int main() {

    float temperature, pressure;
    temperature = sensor.getTemperature();
    pressure = sensor.getPressure();

    enum states {determine = 0, frost, cold, warm, hot};
    enum ldrstates {ldrdetermine = 0, dark, low, day, intense};
    int state = determine;
    int ldrstates = ldrdetermine;

    while (1) {

        unsigned int lightVal = ldr.read_u16();
        int darkPercentage = 100 * (lightVal)/(65536);

        wait_us(100000);

        /* switch (ldrstates) {

            case ldrdetermine:


                if (ldrPercentage <= 25) {

                    ldrstates = intense;

                } else if ((ldrPercentage <= 50) && (ldrPercentage > 25)) {

                    ldrstates = day;

                } else if ((ldrPercentage <= 75) && (ldrPercentage > 50)) {

                    ldrstates = low;

                } else if (ldrPercentage > 75) {

                    ldrstates = dark;

                } else {

                    ldrstates = ldrdetermine;

                }

                break;

            case intense:

                lcd.printf("INTENSE");

                ldrstates = ldrdetermine;

                break;

            case day:

                lcd.printf("DAY");

                ldrstates = ldrdetermine;

                break;

            case low:

                lcd.printf("LOW");

                ldrstates = ldrdetermine;

                break;

            case dark:

                lcd.printf("DARK");

                ldrstates = ldrdetermine;

                break;

        } */

        switch (state) {

            case determine:

                temperature = sensor.getTemperature();
                pressure = sensor.getPressure();

                lcd.locate(1,0);
                lcd.printf("%.1fC, %.1fmB", temperature, pressure);

                trLED = 0;
                tyLED = 0;
                tgLED = 0;

                if ((temperature > 0) && (temperature <= 10)) {

                    state = frost;

                } else if ((temperature > 10) && (temperature <= 20)) {

                    state = cold;

                } else if ((temperature > 20) && (temperature <= 24)) {

                    state = warm;

                } else if ((temperature > 24) && (temperature <= 30)) {

                    state = hot;

                } else {

                    state = determine;

                }

                //float ldrValue = ldr.read_u16();
                //float ldrPercentage = (ldrValue/65536) * 100;
                //lcd.locate(0,0);
                //printf("ldrPercentage = %.1f\n", ldrPercentage);

                //if (())

                break;

            case frost:

                frostWarning();

                state = determine;

                break;

            case cold:

                trLED = 1;

                state = determine;

                break;

            case warm:

                tyLED = 1;

                state = determine;

                break;

            case hot:

                tgLED = 1;

                state = determine;

                break;

        }


        //************************WRITE CODE FROM HERE ONWARDS, ANY CHANGES MAKE A // COMMENT TO SAY WHAT IT IS YOU DID
        
    }

}

void rLEDFlash() {

        buzz.playTone("D", Buzzer::HIGHER_OCTAVE);
        wait_us(500000);

        buzz.rest();
        wait_us(500000);

}

void frostWarning() {

    buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
    trLED = 1;

    wait_us(1000000);

    buzz.rest();
    trLED = 0;

    wait_us(1000000);

}