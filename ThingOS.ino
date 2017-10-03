/*
  ThingOS
  Runs device according to the message coming from serial Input SmaartU

 Created by ibrahim USLU
  17 Sep 2017

 This example code is in the public domain.

 */
#include <XBee.h>
#include <SoftwareSerial.h>

/*
  This example is for Series 2 XBee
  Receives a ZB RX packet and prints the packet to softserial
*/

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 0;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 1;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

int DEBUG_MODE =1;
int sensorValue = 0;  // variable to store the value coming from the sensor

int runningMode=0;  // default runMode 0
int configMode =0;

struct opMode
{
  char ifCond;
  int state=-1;
  int fromPin=-1;
  int fromAPin = A0;
  int bindedTo=-1;
  char doOutput;
  int toPin=-1;
  char sendWhat;
  boolean curCond=false;
  //char* modeName="\0";
};
struct runMode
{
  int count =0;
  int queue[];
  //char* modeName="\0";
};

struct runMode runModes[10];
struct opMode opModes[10];

void setup() {
  //C*(value)#Q*(value)#IF*(value)#STATE*(condition)#FROM*(input)#DO*(value)#WHAT*#TO*(output)#IN*(mode)
  char* factoryMode =  "C*0#Q*0#I*G#S*400#F*0#D*D#W*HIGH#T*7";
  //#M*normal

  // declare the ledPin as an OUTPUT:
  
  Serial.begin(9600); 
  if(DEBUG_MODE)
    Serial.println("ThingOS serialport opened and ready to use!");

  pinMode(ssRX, INPUT);
  pinMode(ssTX, OUTPUT);

  xbee.setSerial(Serial);
  nss.begin(9600);
  deSerialize(factoryMode);
}

void loop() {
  char *runningModeToken,*readenCommand,*token;
  String serialRead,commandName,command;   
  xbee.readPacket();
  uint8_t payload[] = { 'O', 'K' };
  
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      xbee.getResponse().getZBRxResponse(rx);
      
      if(DEBUG_MODE){
        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
          // the sender got an ACK
          Serial.println("packet acknowledged");
        } else {
          Serial.println("packet not acknowledged");
        }
        Serial.print("checksum is ");
        Serial.println(rx.getChecksum(), HEX);
  
        Serial.print("packet length is ");
        Serial.println(rx.getPacketLength(), DEC);
        Serial.println((char*)rx.getData());
      }
    if(DEBUG_MODE)
      Serial.println("serial port is active!");
    /*serialRead={"\0"};
    serialRead = rx.getData();
    
    serialRead.trim(); 
    if(DEBUG_MODE)      
      Serial.println(serialRead);
    serialRead.toCharArray(readenCommand,serialRead.length()+1);*/
    readenCommand=(char*)rx.getData();
    
    if(DEBUG_MODE)      
      Serial.println(readenCommand);
    token = strtok(readenCommand, "+");
    commandName = token;

    if(DEBUG_MODE)      
      Serial.println(commandName);
    if(commandName=="CONFIG"){
        token = strtok(NULL, "+");
        command = token;
        if(command=="ON"){
          configMode = 1; 
          Serial.println("OK");
        }else if(command=="OFF"){
          configMode = 0;
          Serial.println("OK");
        }else{
          deSerialize(token);
        }
        
   }
   else if(commandName =="RUN"){
        token = strtok(NULL, "+");
        runningModeToken = token;
        if(*token == '?'){
          Serial.print("RUNNING+");
          Serial.println(runningMode);
        }else if(*token=='D'){
          Serial.print("In Mode : ");Serial.println(runningMode);
          Serial.print("size : ");Serial.println(runModes[runningMode].count);
          for(int r=0;r<runModes[runningMode].count;r++){
            int p=runModes[runningMode].queue[r];  
            Serial.print("  ");Serial.print(runModes[runningMode].queue[r]);Serial.print(": ");
            Serial.print(" if A");Serial.print(opModes[r].fromPin);Serial.print(" ");Serial.print(opModes[r].ifCond);Serial.print(" ");Serial.print(opModes[r].state);Serial.print(" ");
            Serial.print(" then send ");Serial.print(opModes[r].sendWhat);Serial.print(" to ");Serial.print(opModes[r].doOutput);Serial.print(opModes[r].toPin);Serial.println(" ");
          }
          
          
        }else{
          runningMode = atoi(runningModeToken);
          Serial.println("OK");
        }

   }else if(commandName =="DEBUG"){
        token = strtok(NULL, "+");
        command = token;
        if(command=="ON"){
          DEBUG_MODE = 1; 
          Serial.println("OK");
        }else if(command=="OFF"){
          DEBUG_MODE = 0;
          Serial.println("OK");
        }
   }
   token = strtok(NULL, "+");

  }}
  ////////////////////////////////////////
  // OPERATION 
  ////////////////////////////////////////
  if(!configMode){
    if(DEBUG_MODE){
      Serial.print("\n runningMode: ");
      Serial.print(runningMode);
      Serial.print(" queue Size: ");
      Serial.print(runModes[runningMode].count);
    }
    for(int o=0;o<runModes[runningMode].count;o++){
      int p=runModes[runningMode].queue[o];
      if(DEBUG_MODE){
        Serial.print(" queue: ");
        Serial.print(p);
      }
      if(opModes[p].bindedTo>-1){
        if(opModes[opModes[p].bindedTo].curCond){
          continue;
        }
      }
      operate(p);
    }      
  }

}
boolean calculatedCond(int p)
{
  
  String ifCondition(opModes[p].ifCond);
     
        
      /*
      // EQUALS
      */
  if(ifCondition == "E"){
        pinMode(opModes[p].fromAPin,INPUT);
        int a = analogRead(opModes[p].fromAPin);
        if(DEBUG_MODE){
          Serial.print(" if fromPin ");Serial.print(opModes[p].fromPin);Serial.print(" : ");
          Serial.print(a);
        }
        
        if(a == opModes[p].state){
          if(DEBUG_MODE){
            Serial.print(" equals : ");Serial.print(opModes[p].state);
          }
          return true;
         }else{
          
          if(DEBUG_MODE){
            Serial.print(" not equals : ");Serial.print(opModes[p].state);
          }
          return false;
         }
      /*
      // GREATER
      */
  } else if(ifCondition == "G"){
        pinMode(opModes[p].fromAPin,INPUT);
        int a = analogRead(opModes[p].fromAPin);
        if(DEBUG_MODE){
          Serial.print(" if fromPin ");Serial.print(opModes[p].fromAPin);Serial.print(" : ");
          Serial.print(a);
        }
        
        if(a > opModes[p].state){
          
          if(DEBUG_MODE){
            Serial.print(" greater than : ");Serial.print(opModes[p].state);
          }
          return true;
        }else{
          if(DEBUG_MODE){
            Serial.print(" not greater than : ");Serial.print(opModes[p].state);
          }
          return false;
        }
        
      /*
      // LOWER
      */
  }else  if(ifCondition == "L"){
        pinMode(opModes[p].fromAPin,INPUT);
        int a = analogRead(opModes[p].fromAPin);
        if(DEBUG_MODE){
          Serial.print(" if fromPin ");Serial.print(opModes[p].fromAPin);Serial.print(" : ");
          Serial.print(a);
        }
        
        if(a < opModes[p].state){
          if(DEBUG_MODE){
            Serial.print(" lower than : ");Serial.print(opModes[p].state);
          }
          return true;
        }else{
           if(DEBUG_MODE){
            Serial.print(" not lower than : ");Serial.print(opModes[p].state);
          }
          return true;
        }
  }else  if(ifCondition == "C"){
        
          if(DEBUG_MODE){
            Serial.print(" set : ");Serial.print(opModes[p].state);
          }
          return true;
  }
}
void operate(int p){
  String ifCondition(opModes[p].ifCond);
     
        
        if(calculatedCond(p)){
          opModes[p].curCond=true;
          if(DEBUG_MODE){
            Serial.print(" equals : ");Serial.print(opModes[p].state);
          }
          String strOut(opModes[p].doOutput);
          if( strOut== "D"){
            pinMode(opModes[p].toPin,OUTPUT);
            if(DEBUG_MODE){
              Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
            }
            String str(opModes[p].sendWhat);
            if(str == "L"){
              digitalWrite(opModes[p].toPin,LOW);
            }else if(str == "H"){
              digitalWrite(opModes[p].toPin,HIGH);
            }
          }else if(strOut == "S"){
            Serial.println(opModes[p].sendWhat);
          }else if(strOut == "RS"){
            Serial.println(opModes[p].sendWhat);
          }
        }else if(ifCondition != "C"){
          
          opModes[p].curCond=false;
          if(DEBUG_MODE){
            Serial.print(" equals : ");Serial.print(opModes[p].state);
          }
          String strOut(opModes[p].doOutput);
          if( strOut== "D"){
            pinMode(opModes[p].toPin,OUTPUT);
            if(DEBUG_MODE){
              Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
            }
            String str(opModes[p].sendWhat);
            if(str != "L"){
              digitalWrite(opModes[p].toPin,LOW);
            }else if(str != "H"){
              digitalWrite(opModes[p].toPin,HIGH);
            }
          }
        }//fromPin
      /*
      // SEND COMMAND
      */
}
void deSerialize(char *value){
  //char *modeName="";
  int count=0,queue=-1,fromPin=-1,toPin=-1,state=-1,bindedTo=-1;
  char *ifCond,*doOutput,*sendWhat,*temp_mode[9],*mode[9];
  
  if(DEBUG_MODE){
    Serial.print("deSerialize String: ");
  
    Serial.println(value);
    Serial.print(count);
    Serial.print(queue);
    Serial.print(fromPin);
    Serial.print(toPin);
    Serial.print(state);
    Serial.print(ifCond);
    Serial.print(doOutput);
    Serial.print(sendWhat);
    Serial.print(*temp_mode[8]);
    Serial.println(*mode[8]);
  }
  char* token=NULL;
  token = strtok(value, "#");
   
  /* walk through other tokens */
  int j=0,k=0;
  while(token != NULL ){
     
    temp_mode[j]=token; 
    if(DEBUG_MODE)
      Serial.println(temp_mode[j]);
    token = strtok(NULL, "#");
  j++;
  }// parse grain
  
  for(k=0;k<j;k++){
     token = NULL;
     token = strtok(temp_mode[k], "*");
     
     /* walk through other tokens */
     if(*token=='C'){
       token = strtok(NULL, "*");
       mode[k]=token;
       if(DEBUG_MODE)     
         Serial.println(mode[k]);
       count = atoi(mode[k]);
     }
     
     if(*token=='Q'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       queue = atoi(mode[k]);
     }
     
     
     
     if(*token=='I'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)  
         Serial.println(mode[k]);
       ifCond=mode[k];
       if(DEBUG_MODE)  
         Serial.println(ifCond);
     }
     if(*token=='S'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       state = atoi(mode[k]);
     }
     if(*token=='F'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(token);
       fromPin = atoi(mode[k]);
     }
     if(*token=='D'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)  
         Serial.println(mode[k]);
       doOutput=mode[k];
     }
     if(*token=='W'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       sendWhat=mode[k];
     }
     if(*token=='T'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       toPin = atoi(mode[k]);
     }
      if(*token=='B'){
       token = strtok(NULL, "*");
       mode[k]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       bindedTo = atoi(mode[k]);
     }
     if(*token=='R'){
       token = strtok(NULL, "*");
       mode[k]=token;
       if(DEBUG_MODE)
         Serial.println(mode[k]);
       boolean found = false;
       for(int o =0;o<runModes[count].count;o++){
         if(found){
           runModes[count].queue[o-1]=runModes[count].queue[o];
         }
         if(runModes[count].queue[o]==queue){
           found=true;
         }
       }
       
       runModes[count].count--;
       return;

     }
     /*if(*token=='M'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(token);
       modeName = mode[j];
     }*/
     token = strtok(NULL, "*");

     
   } // parse fine
   opModes[queue].fromAPin = fromPin;
   opModes[queue].toPin = toPin;
   opModes[queue].state = state;
   opModes[queue].bindedTo = bindedTo;
   opModes[queue].ifCond=*ifCond;
   opModes[queue].doOutput=*doOutput;
   opModes[queue].sendWhat=*sendWhat;
   //opModes[queue].modeName = modeName; 
   //runModes[count].modeName = modeName;
   boolean isExist=false;
   int order=0;
   for(int o =0;o<runModes[count].count;o++){
     if(runModes[count].queue[o]==queue){
       isExist=true;
       order = o;
     }
   }
   if(isExist){
     if(DEBUG_MODE){
       Serial.println("Exist");
     }
     runModes[count].queue[order]=queue;
   }else{
     if(DEBUG_MODE){
       Serial.println("not Exist");
     }
     runModes[count].queue[runModes[count].count]=queue;
     runModes[count].count++;
   }
   Serial.println("OK");
   
  
}