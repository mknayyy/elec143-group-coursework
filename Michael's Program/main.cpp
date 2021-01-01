#include "mbed.h"
#include "../lib/uopmsb/uop_msb_2_0_0.h"
using namespace uop_msb_200;

#define WAIT_TIME_MS 500

DigitalOut led1(LED1);

LCD_16X2_DISPLAY lcd;

int main()
{

    lcd.write(LCD_16X2_DISPLAY::DATA, 'X');
    lcd.write(LCD_16X2_DISPLAY::DATA, 'Y');
    lcd.character(1,1,'*');

    lcd.locate(1,5);
    lcd.printf("Hello");
    

    while (true) {

        led1 = !led1;
        wait_us(500000);
        lcd.write(LCD_16X2_DISPLAY::DATA, '1');
        
    }
}

