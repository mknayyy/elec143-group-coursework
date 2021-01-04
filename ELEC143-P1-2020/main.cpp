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
DigitalOut traf1RedLED(TRAF_RED1_PIN);
DigitalOut traf1YelLED(TRAF_YEL1_PIN);
DigitalOut traf1GrnLED(TRAF_GRN1_PIN);
DigitalInOut traf2RedLED(TRAF_RED2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);
DigitalInOut traf2YelLED(TRAF_YEL2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);
DigitalInOut traf2GrnLED(TRAF_GRN2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);

//Light Levels
AnalogIn ldr(AN_LDR_PIN);

//Environmental sensor
EnvironmentalSensor sensor;

//LCD Backlight - consider PwmOut for this :)
PwmOut backLight(LCD_BKL_PIN);

int main()
{
    //LCD Backlight ON
    backLight = 1;

    while (true) {

        unsigned int lightVal = ldr.read_u16();                                                             //Unsigned int to range the values from 0 - 2^16, lightVal is registered with the values from the LDR.

        int darkPercentage = 100 * (lightVal)/(1 << 16);                                                    //Integer specified to translate the raw value from the LDR into a percentage of darkness.

        printf("darkPercentage == %d\n", darkPercentage);                                                   //Prints the value the darkness percentage to the serial monitor.

        wait_us(50000);                                                                                    //Waits for 0.5s to refresh the code.

        while (darkPercentage < 25) {                                                                       //This while loop runs while the light value is high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("INTENSE\n");                                                                          //Writes the string in the quotation marks to the LCD screen.

            backLight = 0;

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the  1st while loop.

        while ((darkPercentage >= 25) && (darkPercentage < 50)) {                                           //This while loop runs while the light value is between high and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("DAY\n");                                                                              //Writes the string in the quotation marks to the LCD screen.

            backLight.period(4.0f);
            backLight.write(0.50f);

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 2nd while loop.

        while ((darkPercentage >= 50) && (darkPercentage < 75)) {                                           //This while loop runs while the light value is between mid-low and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.        
            lcd.puts("LOW\n");                                                                              //Writes the string in the quotation marks to the LCD screen.

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 3rd while loop.

        while (darkPercentage >= 75) {                                                                      //This while loop runs while the light value is low.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("DARK\n");                                                                             //Writes the string in the quotation marks to the LCD screen.

            backLight = 1;

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 4th while loop.
 
        while (1) {

            break;

        }


        float temperature, pressure;
        temperature = sensor.getTemperature();
        pressure = sensor.getPressure();

        printf("%.1fC %.1fmBar\n",temperature,pressure);     
    }
}