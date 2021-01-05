#include "mbed.h"
#include "../lib/uopmsb/uop_msb_2_0_0.h"
#include "BMP280_SPI.h"

using namespace uop_msb_200;

//On board LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

//On board switch
DigitalIn BlueButton(USER_BUTTON);

//LCD Display
LCD_16X2_DISPLAY lcd;

//Buzzer
Buzzer buzz;

//Traffic Lights
DigitalOut trLED(TRAF_RED1_PIN);
DigitalOut tyLED(TRAF_YEL1_PIN);
PwmOut tgLED(TRAF_GRN1_PIN);
DigitalOut rLED(TRAF_RED2_PIN);
DigitalOut yLED(TRAF_YEL2_PIN);
DigitalOut gLED(TRAF_GRN2_PIN);

//Light Levels
AnalogIn ldr(AN_LDR_PIN);

//Environmental sensor
EnvironmentalSensor sensor;

//LCD Backlight - consider PwmOut for this :)
PwmOut backLight(LCD_BKL_PIN);

void rLEDFlash();
void timer();

int main() {

    //LCD Backlight ON
    backLight = 1;

    while (true) {

        unsigned int lightVal = ldr.read_u16();                                                             //Unsigned int to range the values from 0 - 2^16, lightVal is registered with the values from the LDR.
        int darkPercentage = 100 * (lightVal)/(1 << 16);                                                    //Integer specified to translate the raw value from the LDR into a percentage of darkness.
        printf("darkPercentage == %d\n", darkPercentage);                                                   //Prints the value the darkness percentage to the serial monitor.
        wait_us(1000000);                                                                                   //Waits for 0.5s to refresh the code.


        float temperature, pressure;
        temperature = sensor.getTemperature();
        pressure = sensor.getPressure();
        lcd.locate(1,0);
        lcd.printf("%.1fC, %.1fmB", temperature, pressure);

        while (darkPercentage < 25) {                                                                       //This while loop runs while the light value is high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("INTENSE\n");                                                                          //Writes the string in the quotation marks to the LCD screen.

            tgLED = 0;

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the  1st while loop.

        while ((darkPercentage >= 25) && (darkPercentage < 50)) {                                           //This while loop runs while the light value is between high and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.printf("DAY\n");                                                                            //Writes the string in the quotation marks to the LCD screen.

            tgLED.period(2.0f);
            tgLED.write(0.25f);

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 2nd while loop.

        while ((darkPercentage >= 50) && (darkPercentage < 75)) {                                           //This while loop runs while the light value is between mid-low and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.        
            lcd.puts("LOW\n");                                                                              //Writes the string in the quotation marks to the LCD screen.

            tgLED.period(2.0f);
            tgLED.write(0.25f);

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 3rd while loop.

        while (darkPercentage >= 75) {                                                                      //This while loop runs while the light value is low.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("DARK\n");                                                                             //Writes the string in the quotation marks to the LCD screen.

            tgLED = 1;

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 4th while loop.

        if ((temperature > 0) && (temperature <= 10)) {

            rLEDFlash();

            lcd.locate(1,0);
            lcd.printf("%.1fC, %.1fmB", temperature, pressure);
            
        } else if ((temperature > 10) && (temperature <= 20)) {

            rLED = 1;
            gLED = 0;
            yLED = 0;

            lcd.locate(1,0);
            lcd.printf("%.1fC, %.1fmB", temperature, pressure);

        } else if ((temperature > 20) && (temperature <= 30)) {

            rLED = 0;
            gLED = 0;
            yLED = 1;

            lcd.locate(1,0);
            lcd.printf("%.1fC, %.1fmB", temperature, pressure);

        } else if ((temperature > 30) && (temperature <= 40)) {

            rLED = 0;
            gLED = 1;
            yLED = 0;

            lcd.locate(1,0);
            lcd.printf("%.1fC, %.1fmB", temperature, pressure);

        } else {

            buzz.playTone("A", Buzzer::HIGHER_OCTAVE);

        }

        rLED = 0;
        gLED = 0;
        yLED = 0;
        buzz.rest();
        
    }

}

void rLEDFlash() {

        rLED = !rLED;
        buzz.playTone("A", Buzzer::HIGHER_OCTAVE);
        wait_us(500000);


        rLED = !rLED;
        buzz.rest();
        wait_us(500000);

}

void timer() {

    for (int count = 0; count < 3600; count++) {

        wait_us(1000000);

    }

}