#include "../lib/uopmsb/uop_msb_2_0_0.h"                                                                    //Includes the library to use the module support board, that the LCD is on.
#include "../lib/BMP280_SPI/BMP280_SPI.h"
#include <mbed.h>
using namespace uop_msb_200;                                                                                //Similar function as above.

DigitalOut red(TRAF_RED1_PIN);                                                                              //Red traffic light for the output.
DigitalOut yellow(TRAF_YEL1_PIN);                                                                           //Yellow traffic light for the output.
DigitalOut green(TRAF_GRN1_PIN);                                                                            //Green traffic light for the output.


LCD_16X2_DISPLAY lcd;                                                                                       //Initialises the display so that we can print text out.
Buzzer buzz;                                                                                                //Buzzer is initialised as an output.

#define MBED_BMP280_SPI_H

#ifdef _DEBUG
extern Serial pc;
#define DEBUG_PRINT(...) pc.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


AnalogIn ldr(PC_0);                                                                                         //Initialising the LDR as an anaolgue input to read from.
AnalogIn BMP280(D13);

BMP280_SPI sensor(D11, D12, D13, D9); // mosi, miso, sclk, cs

int main() {

    while (1) {                                                                                             //While loop to constantly check for the light reading.

        unsigned int lightVal = ldr.read_u16();                                                             //Unsigned int to range the values from 0 - 2^16, lightVal is registered with the values from the LDR.

        int darkPercentage = 100 * (lightVal)/(1 << 16);                                                    //Integer specified to translate the raw value from the LDR into a percentage of darkness.

        //printf("--------------------\n");                                                                   //Printing a line to make the serial monitor look neater :).
        //printf("darkPercentage == %d\n", darkPercentage);                                                   //Prints the value the darkness percentage to the serial monitor.

        wait_us(500000);                                                                                    //Waits for 0.5s to refresh the code.

        while (darkPercentage < 25) {                                                                       //This while loop runs while the light value is high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("INTENSE");                                                                            //Writes the string in the quotation marks to the LCD screen.

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the  1st while loop.

        while ((darkPercentage >= 25) && (darkPercentage < 50)) {                                           //This while loop runs while the light value is between high and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("DAY");                                                                                //Writes the string in the quotation marks to the LCD screen.

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 2nd while loop.

        while ((darkPercentage >= 50) && (darkPercentage < 75)) {                                           //This while loop runs while the light value is between mid-low and mid-high.

            lcd.cls();                                                                                      //Clears the LCD screen.        
            lcd.puts("LOW");                                                                                //Writes the string in the quotation marks to the LCD screen.

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 3rd while loop.

        while (darkPercentage >= 75) {                                                                      //This while loop runs while the light value is low.

            lcd.cls();                                                                                      //Clears the LCD screen.
            lcd.puts("DARK");                                                                               //Writes the string in the quotation marks to the LCD screen.

            break;                                                                                          //Breaks out of the while loop.

        }                                                                                                   //End of the 4th while loop.

    }                                                                                                       //End of the constant-check while loop.

    while(1) {

        printf("%2.2f degC, %04.2f hPa\n", sensor.getTemperature(), sensor.getPressure());

        wait_us(1000000);

    }
    
}                                                                                                           //End of the int main function.