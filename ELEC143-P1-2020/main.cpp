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
void timer();

int main() {

    float temperature, pressure;
    temperature = sensor.getTemperature();
    pressure = sensor.getPressure();

    enum states {determine = 0, frost, cold, warm, hot};
    int state = determine;

    while (1) {

        unsigned int lightVal = ldr.read_u16();
        int darkPercentage = 100 * (lightVal)/(65536);

        wait_us(100000);

        lcd.locate(1,0);
        lcd.printf("%.1fC, %.1fmB", temperature, pressure);

        switch (state) {

            case determine:

                if ((temperature > 0) && (temperature <= 10)) {

                    state = frost;

                } else if ((temperature > 10) && (temperature <= 20)) {

                    state = cold;

                } else if ((temperature > 20) && (temperature <= 24)) {

                    state = warm;

                } else if ((temperature > 24) && (temperature <= 30)) {

                    state = hot;

                }

                break;

            case frost:

                buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
                wait_us(10000000);
                buzz.rest();
                wait_us(10000000);

                temperature = sensor.getTemperature();
                pressure = sensor.getPressure();

                state = determine;

                break;

            case cold:

                buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
                wait_us(5000000);
                buzz.rest();
                wait_us(5000000);

                temperature = sensor.getTemperature();
                pressure = sensor.getPressure();

                state = determine;

                break;

            case warm:

                buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
                wait_us(1000000);
                buzz.rest();
                wait_us(1000000);

                temperature = sensor.getTemperature();
                pressure = sensor.getPressure();

                state = determine;

                break;

            case hot:

                buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
                wait_us(500000);
                buzz.rest();
                wait_us(500000);

                temperature = sensor.getTemperature();
                pressure = sensor.getPressure();

                state = determine;

                break;



        }
        
    }

}

void rLEDFlash() {

        buzz.playTone("D", Buzzer::HIGHER_OCTAVE);
        wait_us(500000);

        buzz.rest();
        wait_us(500000);

}

void timer() {

    for (int count = 0; count < 3600; count++) {

        wait_us(1000000);

    }

}