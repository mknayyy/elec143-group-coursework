#include "mbed.h"                                                                                   //Includes mbed.h to use certain functions in the code below.
#include "../lib/uopmsb/uop_msb_2_0_0.h"                                                            //Includes the library to use the uop module  support board.
#include "BMP280_SPI.h"                                                                             //Library in order to use the BMP280 temperature and pressure sensor.
#include <cstdarg>                                                                                  //Library to reference the corresponding functions seen below in the code.
#include <string>                                                                                   //Gives the ability to use strings within the code.

using namespace uop_msb_200;                                                                        //Similar function to the above, allows the code to communicate with the uop support board.

//On board LEDs
PwmOut led1(LED1);                                                                                  //Uses the led on the nucleo board, this is used to control the PWM that would be normally used as the LCD backlight.
DigitalOut led2(LED2, 0);                                                                           //Uses the led on the nucleo board, this is from the template.
DigitalOut led3(LED3, 0);                                                                           //Uses the led on the nucleo board, this is from the template.

//On board switch
DigitalIn BlueButton(USER_BUTTON);                                                                  //Uses the blue button on the nucleo board, this is from the template.

//LCD Display
LCD_16X2_DISPLAY lcd;                                                                               //Initialises the LCD, in order to print characters onto it.

//Buzzer
Buzzer buzz;                                                                                        //Initialises the buzzer, in order to be used later on in the code.

//Traffic Lights
DigitalOut trLED(TRAF_RED1_PIN, 0);                                                                 //Red traffic light initialised, it's set to 0 as this will cause it to be off by default.
DigitalOut tyLED(TRAF_YEL1_PIN, 0);                                                                 //Yellow traffic light initialised, it's set to 0 as this will cause it to be off by default.
DigitalOut tgLED(TRAF_GRN1_PIN, 0);                                                                 //Green traffic light initialised, it's set to 0 as this will cause it to be off by default.

//Light Levels
AnalogIn ldr(AN_LDR_PIN);                                                                           //Used to initialise the analogue in of the LDR, to be used later in the code.

//Environmental sensor
EnvironmentalSensor sensor;                                                                         //Referencing the BMP280 library in order to use functions in order to call either the temperature or pressure, whichever is needed.

void frostWarning();                                                                                //Initialises the void loop, found at the bottom of the code to signal the frost warning to play.

float temperature = sensor.getTemperature();                                                        //Used in order to be set as a global variable, of the temperature that we can easily call if need be.
float pressure = sensor.getPressure();                                                              //Used in order to be set as a global variable, of the pressure that we can easily call if need be.
float pressure2 = sensor.getPressure();                                                             //Used in order to be set as a global variable, of the second pressure value that we can easily call if need be.
int riskofrain = 0;                                                                                 //Used in order to be set as a global variable, of the rain testing system that we can easily call if need be.

int main()                                                                                          //Int main initialises, so a code may be written.

{                                                                                                   //Beginning of the int main.

    while (1)                                                                                       //While loop, in order to check the code constantly for updates.

    {                                                                                               //Beginning of the main while loop.

        tgLED = 0;                                                                                  //Sets the corresponding traffic light to turn off, otherwise the light would stay on permenantely.
        trLED = 0;                                                                                  //Sets the corresponding traffic light to turn off, otherwise the light would stay on permenantely.
        tyLED = 0;                                                                                  //Sets the corresponding traffic light to turn off, otherwise the light would stay on permenantely.

        float tempArray[100];                                                                       //Initialises the temperature array with size of 100, this is to store the values of the temperature, to be used later.
        float totalArray = 0;                                                                       //Initialised float in order to store the total value of the temparray.

        for (int count = 0; count < 100; count++) {                                                 //For loop, to iterate through each of the values of the temp array, and call a new temperature to be stored in the array.

            tempArray[count] = sensor.getTemperature();                                             //For each iteration of count (1 - 100), the array is then stored with the called temperature. 

            totalArray = totalArray + tempArray[count];                                             //Simply adds all the values in the array to the totalArray float.

        }                                                                                           //End of the for loop.

        float finishedArray = (totalArray / 100);                                                   //Finished array here is calculated by dividing the totalArray by 100, in order to get a mean average of the values called.

        lcd.cls();                                                                                  //Clears the LCD before the bulk of the code begins in order to refresh the values.

        pressure = sensor.getPressure();                                                            //Gets the value of 'pressure' from the pressure sensor.

        if ((pressure + 0.1) < pressure2)                                                           //Compares the two pressure values in 'pressure' and 'pressure2', if the value of 'pressure2' is higher than 'pressure' (with 0.1 added) then continue with if statment.
            {                                                                                       
                lcd.locate(0,8);                                                                    //Locates the 0th row of the 8th collumn in order to print on.
                lcd.printf("Rising\n");                                                             //Prints "Rising" to the lcd screen.
                riskofrain = 0;                                                                     //Makes the value of 'riskofrain' 0.
            }                                                                                       
        else if ((pressure - 0.1) > pressure2)                                                      //Compares the two pressure values in 'pressure' and 'pressure2', if the value of 'pressure2' is lower than 'pressure' (with 0.1 taken away) then continue with the else if statment.
            {                                                                                      
                lcd.locate(0,8);                                                                    //Locates the 0th row of the 8th collumn in order to print on.
                lcd.printf("Falling\n");                                                            //Prints "Falling" to the lcd screen.
                riskofrain ++;                                                                      //Adds one to the value of 'riskofrain'.
            }                                                                                       
        else                                                                                        //In the event that the if and else if statements are flase, continue with whats inside the else statement.
            {                                                                                       
                lcd.locate(0,8);                                                                    //Locates the 0th row of the 8th collumn in order to print on.
                lcd.printf("Stable\n");                                                             //Prints "Stable" to the lcd screen.
                riskofrain = 0;                                                                     //makes the value of 'riskofrain' 0.
            }                                                                                       


        while (finishedArray <= 0) {                                                                //Simply a catch all while loop to continute to spit out a frost warning for values below 0.                                                    

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            frostWarning();                                                                         //Runs the void loop at the bottom of the code, to buzz a warning sound and flash a warning light.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        while ((finishedArray > 0) && (finishedArray <= 10.2)) {                                    //While loop to run between 0c and 10.2c.

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            frostWarning();                                                                         //Runs the void loop at the bottom of the code, to buzz a warning sound and flash a warning light.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.
        
        while ((finishedArray > 8.8) && (finishedArray <= 20.2)) {                                  //While loop to run between 8.8c and 20.2c.

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            trLED = 1;                                                                              //Toggles the red LED to ON, while this loop statement is true.     

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.
        
        while ((finishedArray > 19.8) && (finishedArray <= 26.2)) {                                 //While loop to run between 19.2c and 26.2c.

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            tyLED = 1;                                                                              //Toggles the yellow LED to ON, while this loop statement is true.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.
        
        while ((finishedArray > 25.8) && (finishedArray <= 30)) {                                   //While loop to run between 25.8c and 30c.

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            tgLED = 1;                                                                              //Toggles the green LED to ON, while this loop statement is true.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        while (finishedArray > 30) {                                                                //Simply a catch all while loop to continue to give a green LED for temps above 30.

            lcd.locate(0,0);                                                                        //Locates the 0th row of the 0th collumn in order to print on.
            lcd.printf("%.1fC\n", finishedArray);                                                   //Prints the value of the temperature to the LCD screen.

            tgLED = 1;                                                                              //Toggles the green LED to ON, while this loop statement is true.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        float lightArray[100];                                                                      //Initialises the lightArray with 100 values to be filled in with values called from the for loop.
        float totalLDRArray = 0;                                                                    //Initialises the store for the total value of the lightarray.

        for (int count = 0; count < 100; count++) {                                                 //For loop to iterate through each array value.

            lightArray[count] = ldr.read_u16();                                                     //Stores the lightArray with a value, from ldr.read at that exact point, storing multiple values.

            totalLDRArray = totalLDRArray + lightArray[count];                                      //Simply adds each array value to a total number.

        }                                                                                           //End of the for loop.

        float ldrtoperc = 100 * ((totalLDRArray)/(65536));                                          //Initialised value to convert the values we recieved, into a percentage. Though this percentage is multiplied by a factor of 100, which we deal with underneath this line.

        float finishedLDRArray = (ldrtoperc / 100);                                                 //This simply divides the total value from the previous line by 100, putting it into a percentage of 'darkness',  this is now a useable factor.

        while (finishedLDRArray > 73) {                                                             //While loop to act  as a catch all, for values above and exceeding 73%.
            
            lcd.locate(1,0);                                                                        //Locates the location of the LCD, to the 1st row of the 0th column.
            lcd.printf("DARK");                                                                     //Prints the words in the "" to the LCD screen on the 1st row of the 0th column.

            led1 = 1;                                                                               //The led (backlight) is set to 1 in order to clearly display the value that is printed on the LCD screen.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        while ((finishedLDRArray > 48) && (finishedLDRArray <= 77)) {                               //While loop to run while the values range from 48% to 77%.
            
            lcd.locate(1,0);                                                                        //Locates the location of the LCD, to the 1st row of the 0th column.
            lcd.printf("LOW");                                                                      //Prints the words in the "" to the LCD screen on the 1st row of the 0th column.

            led1.period(0.03f);                                                                     //Relating to PWM, this gives a period of 30ms.
            led1.pulsewidth(0.005);                                                                 //Relating to PWM, this determines the frequency of the pulse of on and off (5ms).

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        while ((finishedLDRArray > 23) && (finishedLDRArray <= 52)) {                               //While loop to run while the values range from 23% to 52%.
            
            lcd.locate(1,0);                                                                        //Locates the location of the LCD, to the 1st row of the 0th column.
            lcd.printf("DAY");                                                                      //Prints the words in the "" to the LCD screen on the 1st row of the 0th column.

            led1.period(0.03f);                                                                     //Relating to PWM, this gives a period of 30ms.
            led1.pulsewidth(0.005);                                                                 //Relating to PWM, this determines the frequency of the pulse of on and off (5ms).

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        while ((finishedLDRArray > 0) && (finishedLDRArray <= 27)) {                                //While loop to run while the values range from 0% to 27%.
            
            lcd.locate(1,0);                                                                        //Locates the location of the LCD, to the 1st row of the 0th column.
            lcd.printf("INTENSE");                                                                  //Prints the words in the "" to the LCD screen on the 1st row of the 0th column.

            led1 = 0;                                                                               //The led (backlight) is set to 0 in order to save energy in daylight.

            break;                                                                                  //Breaks out of the while loop, otherwise the code would run forever.

        }                                                                                           //End of the above while loop.

        pressure2 = sensor.getPressure();                                                           //Gets the value of 'pressure' from the pressure sensor.
        wait_us(500000);                                                                            //Waits for half a second.
        if (riskofrain >= 2)                                                                        //If the value of 'riskofrain' is more than or equal to the value of 2 then continue with the if statement.
            {                                                                                      
                lcd.locate(0,6);                                                                    //Locates the 0th row of the 6th collumn in order to print on.
                lcd.printf("  RAIN!  ");                                                            //Prints "RAIN!" to the lcd screen. (dramatic but needed).
                wait_us(2000000);                                                                   //Waits for 2 seconds.
            }                                                                                       
        else                                                                                        //This else statement has no purpose other than to close the if statement.
        {                                                                                           
                                                                                                
        }                                                                                          

    }                                                                                               //End of the main while loop inside int main.

}                                                                                                   //End of int main.


void frostWarning() {                                                                               //The void loop produced in order to make the code look just a bit neater.

    buzz.playTone("A", Buzzer::HIGHER_OCTAVE);                                                      //Buzzes the buzzer to play a tone of A at the highest octave.
    trLED = 1;                                                                                      //Turns the red traffic light on.

    wait_us(1000000);                                                                               //Allows the buzzer to play for a set amount of time (1s).

    buzz.rest();                                                                                    //Tells the buzzer to rest, ie, turn off.
    trLED = 0;                                                                                      //Turns the red traffic light off.

    wait_us(1000000);                                                                               //A wait signal is given to allow there to be space between each buzz, 1s on, 1s off.

}                                                                                                   //End of the void function.