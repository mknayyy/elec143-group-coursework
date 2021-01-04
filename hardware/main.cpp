#include "mbed.h"
#include "sample_hardware.hpp"
#include "buffer.hpp"
#include "lcd.hpp"


#define RED_DONE 1
#define YELLOW_DONE 2

Timeout sd_time_to_flush;
Timeout user_button_debounce;

Mutex sdWriteLock;
Mutex samplingLock;

//Digital outputs
DigitalOut onBoardLED(LED1);
DigitalOut redLED(PE_15);
DigitalOut yellowLED(PB_10);
DigitalOut greenLED(PB_11);

//Inputs
InterruptIn onBoardSwitch(USER_BUTTON);
AnalogIn adcIn(PA_0);

//Environmental Sensor driver
#ifdef BME
BME280 sensor(PB_9, PB_8);
#else
BMP280 sensor(PB_9, PB_8);
#endif

//SD Card
SDBlockDevice sd(PB_5, D12, D13, D10); // mosi, miso, sclk, cs 

//Initial variable declarationsREAD 
Ticker takeSamples;
Ticker sd_write_flasher;
Timer timeTaken;
Timeout flushRetryTimeout;
Timeout mountRetryTimeout;
float currentSamplingRate;
bool sampleAllowed;
bool sd_mounted = 0;
bool serialLoggingAllowed = 0;
bool bufferLoggingAllowed = 1;
bool UB_allowed = 1;
int status_SD = 0;
int mountRetryCounter = 0;
int flushRetryCounter = 0;

//Buffer class initialisation for the data we want to store
//Size is +10 to allow for the additional reserve buffer while the data is being written to the SD card
Buffer<unsigned int> timeBuffer(DATA_BUFFER_SIZE+10);
Buffer<float> tempBuffer(DATA_BUFFER_SIZE+10);
Buffer<float> pressBuffer(DATA_BUFFER_SIZE+10);
Buffer<float> lightBuffer(DATA_BUFFER_SIZE+10);

void user_button_timeout(void)  //Called by timeout interrupt, only accept user button input when = 1
{
    UB_allowed = 1;
}

void user_button_input(void)    //Called on user button activity
{
    if(UB_allowed ==1)          //If the press is allowed(>100mS since last press)
    {
        if(sd_mounted==0)
        {
            queueSD(2);        //MOUNT SD card if not already mounted
        }
        else if(sd_mounted==1)
        {
            queueSD(3);     //EJECT SD card if already mounted
        }
        UB_allowed = 0;     //Set allowed = 0
        user_button_debounce.attach(&user_button_timeout, 0.1);     //Debouce timeout
    }
}

void greenFlasher(void)
{
    greenLED = !greenLED;       //Flashes the green LED when ticker asks for it - handled by SD flush function
}

void flushTimeout(void)
{
    redLED=1;
    lpQueue.call(printf, "Flush failed after 10 seconds!\r\n");     //Light RED LED and send warning to terminal if flush failed
}

void mountTimeout(void)
{
    mountRetryCounter++;
    if(mountRetryCounter<5)
    {
        lpQueue.call(printf, "Mount failed after 10 seconds!\r\n");
        queueSD(2);         //Retry mounting if unsuccessful
    }
    else if(mountRetryCounter >= 5)     //After 5 unsuccessful mounts, light the red LED and send alert to terminal
    {
        redLED=1;
        lpQueue.call(printf, "Mount failed after 5 attempts!\r\n");
        mountRetryCounter=0;
    }
}
void post() 
{
    //POWER ON TEST (POT)
    puts("**********STARTING POWER ON SELF TEST (POST)**********");
    
    //Test LEDs
    puts("ALL LEDs should be blinking");
    for (unsigned int n=0; n<10; n++) {
        redLED    = 1;
        yellowLED = 1;
        greenLED  = 1;
        ThisThread::sleep_for(100);
        redLED    = 0;
        yellowLED = 0;
        greenLED  = 0;     
        ThisThread::sleep_for(100);         
    } 
    
    //Output the switch states (hold them down to test)
	//	uint8_t sw1 = SW1.read();
	//	uint8_t sw2 = SW2.read();
    //printf("SW1: %d\tSW2: %d\n\r", sw1, sw2);    
    //printf("USER: %d\n\r", onBoardSwitch.read()); 
    
    //Output the ADC
		float ldr = adcIn.read();
    printf("ADC: %f\n\r", ldr);
    
    //Read Sensors (I2C)
    float temp = sensor.getTemperature();
    float pressure = sensor.getPressure();
    //#ifdef BME
    //float humidity = sensor.getHumidity();
    //#endif
   
    //Display in PuTTY
    printf("Temperature: %5.1f\n", temp);
    printf("Pressure: %5.1f\n", pressure);
    #ifdef BME
    printf("Pressure: %5.1f\n", humidity);
    #endif
    
    //Display on LCD
    redLED = 1;
    lcd.cls();
    lcd.locate(0,0);
    lcd.printf("LCD TEST...");
    wait_us(500000);
    redLED = 0;
    
    //Network test (if BOTH switches are held down)
    //networktest();
    
    puts("**********POST END**********");
 
}

//Converts CURRENT UNIX TIME to a custom string and sends to terminal
//Timestamp format is in DD/MM/YY HH/MM/SS
void printTimeStamp(unsigned int seconds)
{
    //if the time isn't anywhere near plausible, reject it
    if(seconds <= 1000000000)
    {
        lpQueue.call(printf, "\033[01;31mTIME NOT SET\033[0m");
    }
    //if it is, print the correct time
    else{
        static char tCharBuffer[64];
        strftime(tCharBuffer, 64, "%d/%m/%y %I:%M:%S%p", localtime(&seconds));
        lpQueue.call(printf,"%s", tCharBuffer);
    }
}

//Handles serial input LOGGING ON/OFF
//Prints message with timestamp to show change and sets the flag
//serialLoggingAllowed = 1
void dataLoggingStatus(bool status)
{
    if(status==1)
    {
        lpQueue.call(printf, "\r\033[01;32mLogging enabled at ");
        printTimeStamp(time(NULL));
        lpQueue.call(printf, " \033[0m \r\n");
        serialLoggingAllowed = 1;
    }
    else if(status==0)
    {
        lpQueue.call(printf, "\r\033[01;33mLogging disabled at ");
        printTimeStamp(time(NULL));
        lpQueue.call(printf, " \033[0m \r\n");
        serialLoggingAllowed = 0;
    }
}


//Handles the serial input READ NOW
//One time measurement and print out of all sensor current data
//ldr is sampled 10 times over 20ms and averaged to give a percentage from 10 data points 
void oneTimeSerialData(void)
{
        float ldr;
        float temp = sensor.getTemperature();
        float pressure = sensor.getPressure();

        for(int i=0; i<10;i++)
        {
            ldr = ldr+adcIn.read();
            ThisThread::sleep_for(2);
        }

        lpQueue.call(printf, "\r[");
        printTimeStamp(time(NULL));
        lpQueue.call(printf, "]Temp: %5.1f ", temp);
        lpQueue.call(printf, "Pressure: %5.1f ", pressure);
        lpQueue.call(printf, "Light: %3.2f %% \r\n", ldr*10);
}

//This function is called by the sampling ticker, and as the name suggests
//Samples everything, averages ldr over 10 samples 
void sampleEverything(int status)
{   
    float ldr;
    float temp = sensor.getTemperature();
    float pressure = sensor.getPressure();
    unsigned int timeSeconds = time(NULL);


    for(int i=0; i<10;i++)          //Sample the LDR and add values
    {
        ldr = ldr+adcIn.read();
    }

    float ldrAvg = ldr*10;      //Multiply by 10, we already have 10 samples, so 10*10=100 for our percentage
    queueLCDData(ldrAvg, pressure, temp, timeSeconds);
    
    timeBuffer.add(timeSeconds);        //Add all the respective data to their buffers
    tempBuffer.add(temp);
    pressBuffer.add(pressure);
    lightBuffer.add(ldrAvg);

    switch(timeBuffer.status())             //Read the status from the time buffer(All buffers should be synchronous, so the status will be the same)
    {
        case 1:
            break;
        case 2:
            if(serialLoggingAllowed==1)  //If LOGGING ON, tell the terminal what is happening. In this case, buffer is 10 samples from end
            {
                lpQueue.call(printf, "Buffer 10 samples from end\r\n");
            }
            queueSD(1);     //Queue the SD to store the data. The buffers have a 10 sample reserve to give minimum 1 second to keep sampling while writing. SD writing takes time
            break;
        case 3:
            if(serialLoggingAllowed==1)
            {
                lpQueue.call(printf, "Error:Buffer full \r\n");     //Throw error when buffer is full
            }
            redLED=1;           //Illuminate the RED LED to show a fault, and store as much data as we can to SD
            queueSD(1);
            break;
        case 4:
            if(serialLoggingAllowed==1)     
            {
                lpQueue.call(printf, "Buffer empty\r\n");
            }
            break;
    }
                
    

    if(serialLoggingAllowed==1)             //If LOGGING ON, send the data to the terminal as well
    {
        lpQueue.call(printf, "\r[");
        printTimeStamp(timeSeconds);
        lpQueue.call(printf, "]Temp: %5.1f Pressure: %5.1f Light: %3.2f %%\r\n", temp, pressure, ldrAvg);
    }

}

//Ticker ISR to set a flag=1 to allow the thread to take a sample on it's next iteration
void setSampleFlag(void)
{
    sampleAllowed = 1;
    yellowLED = 1;
}

//The sample thread runs this function exclusively, checks for the sample allowed flag
//If it =1, run sampleEverything.
void runSamples(void)
{
    if (sampleAllowed ==1)
    {        
       sampleEverything(1);
       sampleAllowed= 0;
       yellowLED = 0;
       ThisThread::sleep_for(20);
    }
}


//Read buffer to terminal. uses PEEK on the buffer instead of GET so it doesn't erase or modify location information at all
void serialReadBuffer(void)  
{
    int buffer_start_point = timeBuffer.startPoint();   //Finds the start location in the buffer
    for(int i=0; i<timeBuffer.samplesInBuffer(); i++)   //Loops around for however many samples are present in buffer
    {
        if(buffer_start_point+i <= DATA_BUFFER_SIZE)        //Display data up to the very end of the buffer
        {
            lpQueue.call(printf,"[");
            printTimeStamp(timeBuffer.peek(buffer_start_point+i));
            lpQueue.call(printf, "]Temp: %5.1f Pressure: %5.1f Light: %3.2f %%\r\n", tempBuffer.peek(buffer_start_point+i), pressBuffer.peek(buffer_start_point+i), lightBuffer.peek(buffer_start_point+i));
            ThisThread::sleep_for(1);   //Small delay, seems to flow better with this.
        }
        else                    //If samples loop around in a circular fashion, continue from the beginning of the buffer
        {
            lpQueue.call(printf,"[");
            printTimeStamp(timeBuffer.peek((buffer_start_point+i)-DATA_BUFFER_SIZE));
            lpQueue.call(printf, "]Temp: %5.1f Pressure: %5.1f Light: %3.2f %%\r\n", tempBuffer.peek((buffer_start_point+i)-DATA_BUFFER_SIZE), pressBuffer.peek((buffer_start_point+i)-DATA_BUFFER_SIZE), lightBuffer.peek((buffer_start_point+i)-DATA_BUFFER_SIZE));
            ThisThread::sleep_for(1);            
        }
    }
}

void dataSamplingStatus(bool status)    //Update the STATE
{
    if (status ==1)     //if STATE ON, enables sampling at the current set rate and prints to terminal
    {
        takeSamples.detach();
        takeSamples.attach(&setSampleFlag, currentSamplingRate);
        lpQueue.call(printf, "\r\033[01;32mSampling enabled every %0.2f seconds at ", currentSamplingRate);
        printTimeStamp(time(NULL));
        lpQueue.call(printf, " \033[0m \r\n");
    }
    else if (status == 0)   //If STATE OFF, disables sampling and prints to terminal
    {
        takeSamples.detach();
        lpQueue.call(printf, "\r\033[01;33mSampling disabled at ");
        printTimeStamp(time(NULL));
        lpQueue.call(printf, " \033[0m \r\n");
    }
}

void updateSampleRate(float rate)   //update sample rate
{
    currentSamplingRate = rate;    //Sets current sampling rate to the one sent to this function
    takeSamples.detach();          //Detaches ticker
    takeSamples.attach(&setSampleFlag, currentSamplingRate);    //Reattaches at new rate
    //lpQueue.call(printf, "\r\033[01;32mSample rate updated: %0.2f seconds\033[0m\r\n\n", currentSamplingRate);
    lpQueue.call(printf, "\r\033[01;32mT UPDATED TO: %0.2f SECONDS\033[0m\r\n\n", currentSamplingRate); //Tells the terminal
}
void errorCode(ELEC350_ERROR_CODE err)
{
    switch (err) {
      case OK:
        greenLED = 1;
        wait_us(1000000);
        greenLED = 0;
        return;                
      case FATAL:
        while(1) {
            redLED = 1;
            wait_us(100000);
            redLED = 0;
            wait_us(100000);             
        }
    };
}


//#############################################################
//SD CARD STUFF
int sd_busy = 0;
int sd_pending_request=0;
//Function queues up SD card operations, it only stores 2 but there shouldn't be more than one anyway
//Just in case we manually call an operation when it is also doing one itself
//It should queue and handle the function
void queueSD(int status)
{
    if(sd_busy == 1)            //If the SD device is busy, queue the request until it isn't
    {
        sd_pending_request=status;
    }
    else if (sd_busy==0)        //If SD isn't busy, run the pending request and queue the new (if applicable) or run the current request
    {   
        if(sd_pending_request != 0)
        {
            status_SD = sd_pending_request; 
            sd_pending_request=0;
        }
        else
        {
            status_SD = status;
        }
    }
}

int first_30min;            //As timeout can only manage up to 30 minutes, this checks if it's been ran twice
void sd_write_override(void)    //Forces a FLUSH if it's been an hour since the last one
{
    if(first_30min==0)
    {
        sd_time_to_flush.detach();
        sd_time_to_flush.attach(&sd_write_override, 1800);  //Run timer again if it's the first 30 minutes
        first_30min=1;                                      //Set the flag
    }
    else if(first_30min==1)                                 //If the first 30 minutes has been completed
    {
        queueSD(1);                                         //Force the FLUSH and reset timer
        first_30min=0;
    }
}

void flushToSD(void)        //Flushes buffer to the SD card
{
    if(sd_mounted==1)         //Only try this if the SD is mounted
    {
    sd_busy=1;          //Set a flag to do nothing with the SD until it's not busy. Prevents accidental dismount during flush

    if(serialLoggingAllowed==1)     //Tell the terminal if allowed
    {
        lpQueue.call(printf, "Flushing buffer to SD card...\r\n");
    }

    flushRetryTimeout.attach(flushTimeout, 10);
    sd_write_flasher.attach(greenFlasher, 0.05);    //Start the ticker to flash green LED
    sd_time_to_flush.detach();                      //Detach 1HR max flush timer
    first_30min = 0;                                //Resets first half flush timer

    FATFileSystem fs("sd", &sd);                    //Set file system
    FILE* fp = fopen("/sd/output.csv","a");         //Open file
    if (fp == NULL) 
    {
        redLED = 1;
    }
    char timeString [64];           //Buffer to hold the data string for time information
    if(serialLoggingAllowed==1){    //tell the terminal if it's allowed
        lpQueue.call(printf, "Writing %d samples to SD\r\n", timeBuffer.samplesInBuffer()-1);
    }

    fprintf(fp, "\r\nDate & Time, Temperature(Celsius), Pressure(mBar), Light(%%)\r\n");  //Write column information once at every write. Keeps data purely numeric for easier manipulation and less storage
    int sampleNumber = timeBuffer.samplesInBuffer();                            //Get number of samples in buffer, don't want to write data that isn't real
    for(int i=0; i<sampleNumber-1; i++)                                         //Loop through these samples
    {
        unsigned int tempTime = timeBuffer.get();
        if(tempTime<= 1000000000)
        {
            sprintf(timeString, "TIME NOT SET");                    //Print invalid timestamp if time not set
        }
        else
        {
            strftime(timeString, 64, "%d/%m/%y %I:%M:%S%p", localtime(&tempTime));  //Format time string same as in terminal if it is
        }
        
        fprintf(fp, "%s,%5.1f,%5.1f,%3.2f\r\n", timeString, tempBuffer.get(), pressBuffer.get(), lightBuffer.get());    //Print the data to the file
    }
    if(serialLoggingAllowed==1){
        lpQueue.call(printf, "Buffer emptied to SD card \r\n");     //Tell the terminal that it has finished writing (If allowed)
    }
    fclose(fp);
    sd_write_flasher.detach();      //Close the file and detach the green LED flasher
    greenLED=1;                     //Green LED on to show we are still mounted and ready to go
    sd_busy=0;                      //SD no longer busy
    sd_time_to_flush.attach(&sd_write_override, 1800);  //Attach 1HR timer to function
    flushRetryTimeout.detach();
    }
    else
    {
        if(serialLoggingAllowed==1){
            printf("SD not mounted!\r\n");  //Tell the terminal if allowed and SD not mounted
        }   
    }
}

void ejectSD(void)                  //Unmounts the SD(Called after flushing)
{
    FATFileSystem fs("sd", &sd);   
    sd.deinit();
    fs.unmount();
    sd_mounted = 0;
    greenLED=0;
}

void mountSD(void)      //Mount SD function
{       
    if(sd_mounted==1)   //Don't mount, tell the terminal if we're already mounted
    {
        lpQueue.call(printf, "SD already mounted! \r\n");
    }
    else            //If not mounted, mount it
    {
        lpQueue.call(printf, "Mounting SD card... \r\n");
        sd_busy=1;
        mountRetryTimeout.attach(mountTimeout, 10);
        //Initialise the SD card
        sd.init();              
        if ( sd.init() != 0) 
        {
            lpQueue.call(printf, "Init failed \n");       
            errorCode(FATAL);
        } 
            
        //Create a filing system for SD Card
        FATFileSystem fs("sd", &sd);     
        fs.mount(&sd);

        //Open to WRITE
        FILE* fp = fopen("/sd/output.csv","a");
        if (fp == NULL) {
                redLED = 1;
            }
        else {
                dataSamplingStatus(1);  //Green light; enable sampling
                greenLED = 1;
                sd_mounted = 1;
            }
        fclose(fp);
        sd_time_to_flush.attach(&sd_write_override, 1800);  //start the 1Hr timer(Runs a 30 minute timer twice) for max FLUSH interval
        sd_busy=0;
        mountRetryTimeout.detach();
        flushToSD();            //Flush data to SD whenever mounted - prints a header at the very least
    }
}

    void request_sd_activity(void)  //Called in the thread, whichever SD operation is queued - run it.
    {
        if(status_SD == 1)  //Status 1 calls a FLUSH
        {
            flushToSD();
            status_SD=0;
        }
        
        else if(status_SD == 2) //Status 2 calls a MOUNT
        {
            mountSD();
            status_SD=0;
        }
        else if(status_SD == 3)     //Status 3 calls a FLUSH & EJECT
        {
            flushToSD();
            ejectSD();
            status_SD=0;
        }
        else if(sd_pending_request !=0) //or for anything else other than 0, just set it to 0 (idle)
        {
            queueSD(0);
    }
}
void startup_init(void) //Initialisation of the blue user button.
{
    onBoardSwitch.rise(&user_button_input);
}