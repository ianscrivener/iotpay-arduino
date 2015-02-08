#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"


// ########################################################################################################################
// start GPRS declarations

  InetGSM inet;
  
  char httpData[50];
  int lengthData;
  char inSerial[50];
  int i=0;
  boolean gprsStarted=false;
  
    
  //SET the REST URL & PATH HERE
  char* url = "iot-pay.herokuapp.com";
//  char* url = "2cc2bd8a.ngrok.com";
  char* path = "/logs/IoT/EPA-Monitor";

// start GPRS declarations
// ########################################################################################################################


void setup()
{
     // Start Arduino Serial connection for debugging
     Serial.begin(9600);


      // ########################################################################################################################
      // Start GPRS setup code
      
     //Start configuration to GPRS shield
     if (gsm.begin(4800)) {
          //Serial.println("GPRS Status: READY");
          gprsStarted=true;
     } 
     else {
       //Serial.println("GPRS Status: ERROR");
     }

     if(gprsStarted) {
     
       if (inet.attachGPRS("mdata.net.au", "", "")){
               //Serial.println("status=CONNECTED");
          }
          else {
            //Serial.println("status=ERROR");
          }
          delay(200);
          
          //Read until serial buffer is empty.
          gsm.WhileSimpleRead();

          int percUnder      = 80;
          int percWarn       = 15;
          int percExceeded   = 5;          
          
          
          sendDataFn(percUnder, percWarn, percExceeded);
          //Serial.println("end sendData");
     }

      // end GPRS setup code
      // ########################################################################################################################
      

};

void loop()
{
     //Read for new byte on serial hardware,
     //and write them on NewSoftSerial.
     serialhwread();
     
     //Read for new byte on NewSoftSerial.
     serialswread();
};



// ########################################################################################################################
// start GPRS helper functions

void serialhwread()
{
     i=0;
     if (Serial.available() > 0) {
          while (Serial.available() > 0) {
               inSerial[i]=(Serial.read());
               delay(10);
               i++;
          }

          inSerial[i]='\0';
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
}

void serialswread()
{
     gsm.SimpleRead();
}


// end GPRS helper functions
// ########################################################################################################################


// ########################################################################################################################
// start REST API send function

bool sendDataFn(int percUnder, int percWarn, int percExceeded)
{
  
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
