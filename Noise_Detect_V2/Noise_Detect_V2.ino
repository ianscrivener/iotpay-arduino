#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"

// ########################################################################################################################
// start GPRS declarations

  InetGSM inet;
  
  char httpData[50];
  int lengthData;
  char inSerial[50];
  int scriv=0;
  boolean gprsStarted=false;
  
    
  //SET the REST URL & PATH HERE
  char* url = "iot-pay.herokuapp.com";
//  char* url = "2cc2bd8a.ngrok.com";
  char* path = "/logs/IoT/EPA-Monitor";

// start GPRS declarations
// ########################################################################################################################

int noiseValue = 0;
int sensorPin = A0; // input pin for the sound analog
int ledPins[] = { 4, 5, 6 }; // sound level output pins
int sendPin = 7;
long noiseCounter[] = {0,0,0}; //  running sum of each type of noise level in sample period
int noiseLowPC = 0;
int noiseMediumPC = 0;
int noiseHighPC = 0;
int ledNum = 0; //highest led to light
int i = 0;
int j = 0;

int sampleLoopNumber = 2000; // number of noise sample loops in n/100 seconds ie 2000 = 20 seconds

// thresholds
int ledlevel0 = 5;
int ledlevel1 = 20;
int ledlevel2 = 40;

// the setup function runs at 100 samples second

// START SETUP ###################################################
void setup() {
      analogReference(INTERNAL);
      // initialize digital pins as LED output.
      for (i = 0; i <= 2; i++) {
        pinMode( ledPins[i], OUTPUT );
      }
      pinMode (sendPin, OUTPUT);
      Serial.begin (9600);
      noiseCounter[0] = 0;
      noiseCounter[1] = 0;
      noiseCounter[2] = 0;

      // ########################################################################################################################
      // Start GPRS setup code
      
     //Start configuration to GPRS shield
     if (gsm.begin(4800)) {
          Serial.println("GPRS Status: READY");
          gprsStarted=true;
     } 
     else {
       Serial.println("GPRS Status: ERROR");
     }

     if(gprsStarted) {
     
       if (inet.attachGPRS("mdata.net.au", "", "")){
               Serial.println("status=CONNECTED");
          }
          else {
            Serial.println("status=ERROR");
          }
          delay(200);
          
          //Read until serial buffer is empty.
          gsm.WhileSimpleRead();
          //Serial.println("serial buffer empty");
          
     }

      // end GPRS setup code
      // ########################################################################################################################
      

}
// END SETUP ###################################################

// START MAIN LOOP ###################################################
// the loop function runs over and over again forever
void loop() {
  
  
  Serial.println("START MAIN LOOP");
  
  //START TALKING WITH GPRS SHIELD &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  
       //Read for new byte on serial hardware, and write them on NewSoftSerial.
           scriv=0;
           if (Serial.available() > 0) {
                while (Serial.available() > 0) {
                     inSerial[scriv]=(Serial.read());
                     delay(10);
                     scriv++;
                }
      
                inSerial[scriv]='\0';
                if(!strcmp(inSerial,"/END")) {
                     Serial.println("_");
                     inSerial[0]=0x1a;
                     inSerial[1]='\0';
                     gsm.SimpleWriteln(inSerial);
                }
                //Send a saved AT command using serial port.
                if(!strcmp(inSerial,"TEST")) {
                     Serial.println("SIGNAL QUALITY");
                     gsm.SimpleWriteln("AT+CSQ");
                }
                //Read last message saved.
                if(!strcmp(inSerial,"httpData")) {
                     Serial.println(httpData);
                } else {
                     Serial.println(inSerial);
                     gsm.SimpleWriteln(inSerial);
                }
                inSerial[0]='\0';
           }
           
     //Read for new byte on NewSoftSerial.
     serialswread();
     
  //END TALKING WITH GPRS SHIELD &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&  
  
  
  
  // read samples for n seconds
  for (j = 0; j < sampleLoopNumber; j++){
    //---------------------------------------------------------------------------------------
    // read noise level
    noiseValue = analogRead (sensorPin); // noise value
    //   
    if (noiseValue < ledlevel0)
    {
      ledNum = 0; //ok
      noiseCounter[0]++;
    }
    else if ((noiseValue >= ledlevel0) && (noiseValue < ledlevel1) )
    {
      ledNum = 1; // warning
      noiseCounter[1]++;
    }
    else
    {
       ledNum = 2; // too loud!
       noiseCounter[2]++;
    }
    
    // light LEDs
    for (i = 0; i <= 2; i++) {
      if (i <= ledNum)
         digitalWrite(ledPins[i], HIGH); 
      else
         digitalWrite(ledPins[i], LOW);  
    }
     
    //-------------------------------------------------------------------------------------------
   
    delay(10);  // 100 samples second
  }
  
  digitalWrite(sendPin, HIGH); 
  // calculate % s
  noiseLowPC = ((float)noiseCounter[0] / sampleLoopNumber) * 100;
  noiseMediumPC = ((float)noiseCounter[1] / sampleLoopNumber) * 100;
  noiseHighPC = ((float)noiseCounter[2] / sampleLoopNumber) * 100;
  

//  
//  Serial.println(noiseCounter[0] );
//  Serial.println(noiseCounter[1] );
//  Serial.println(noiseCounter[2] );
//  Serial.println();
  
  // reset summers
  noiseCounter[0] = 0;
  noiseCounter[1] = 0;
  noiseCounter[2] = 0;

  Serial.println(noiseLowPC);
  Serial.println(noiseMediumPC);
  Serial.println(noiseHighPC);
  

  //SEND VALUES TO REST SERVICE
  bool resultBool = sendDataFn(noiseLowPC, noiseMediumPC, noiseHighPC);
 
  Serial.println(resultBool);
 
  digitalWrite(sendPin, LOW); 
}
// END MAIN LOOP ###################################################




void serialswread()
{
     gsm.SimpleRead();
}


// ########################################################################################################################
// start REST API send function

bool sendDataFn(int percUnder, int percWarn, int percExceeded)
{
   Serial.println('SENDING...');
    String jsonStr = "percExceeded="  + String(percExceeded);
    jsonStr += "&percWarn="  + String(percWarn);
    jsonStr += "&percUnder="  + String(percUnder);
    
    char jsonChar[50];
    jsonStr.toCharArray(jsonChar, 50);
    
    lengthData =  inet.httpPOST(url, 80, path, jsonChar, httpData, 50);
      
    return true;

  }

//end  REST API send function
// ########################################################################################################################
