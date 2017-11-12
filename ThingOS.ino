#include <Arduino.h>
/*
  ThingOS
  Runs device according to the message coming from serial Input SmaartU

 Created by ibrahim USLU
  17 Sep 2017

 This example code is in the public domain.

 */

/*
  This example is for Series 2 XBee
  Receives a ZB RX packet and prints the packet to softserial
*/

#include <XBee.h>
#include <SoftwareSerial.h>
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x408d0d06);
// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 0;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 1;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

boolean DEBUG_MODE =true;
boolean CONFIG_MODE =false;
boolean counting=false;
unsigned long startMillis=0;
int sensorValue = 0;  // variable to store the value coming from the sensor
int runningMode=0;  // default runnigMode 0
struct opMode
{
  char ifCond;
  int state=-1;
  int fromPin=-1;
  int fromAPin = A0;
  int bindedTo=-1;
  char doOutput;
  int toPin=-1;
  String sendWhat;
  unsigned long startMillis=0;
  boolean curCond=true;
  //char* modeName="\0";
};
struct runMode
{
  int count =0;
  int queue[10];
  int runningO=0;
  //char* modeName="\0";
};

struct runMode runModes[10];
struct opMode opModes[10];
uint8_t ok[] = { 'O', 'K' };
uint8_t nok[] = { 'N', 'O', 'K' };
int runningO = 0;
void deSerialize(char* value);
boolean calculatedCond(int p);
void operate(int p);
void runDetail();
void debugDetail(int p);
void setup() {
  //C*(value)#Q*(value)#IF*(value)#STATE*(condition)#FROM*(input)#DO*(value)#WHAT*#TO*(output)#IN*(mode)
  char* factoryMode0 =      "C*0#Q*0#I*G#S*400#F*0#D*D#W*HIGH#T*2";
  char* factoryMode1 =      "C*0#Q*1#I*T#S*4000";
  char* factoryMode2 =  "B*1#C*0#Q*2#I*C#D*X#W*INPUTS";
  char* factoryMode3 =  "B*1#C*0#Q*3#I*C#D*M#W*1";
  char* factoryMode4 =      "C*1#Q*4#I*T#S*7000";
  char* factoryMode5 =  "B*4#C*1#Q*5#I*C#D*M#W*0";
  //#M*normal

  // declare the ledPin as an OUTPUT:
   init ();  // initialize timers
  Serial.begin(9600); 
  if(DEBUG_MODE)
    Serial.println("ThingOS serialport opened and ready to use!");

  pinMode(ssRX, INPUT);
  pinMode(ssTX, OUTPUT);

  xbee.setSerial(Serial);
  nss.begin(9600);
  deSerialize(factoryMode0);
  deSerialize(factoryMode1);
  deSerialize(factoryMode2);
  deSerialize(factoryMode3);
  deSerialize(factoryMode4);
  deSerialize(factoryMode5);
}

void loop() {
  char *readenCommand='\0',*token='\0';
  String runningModeToken="\0",serialRead="\0",commandName="\0",command="\0";   
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      xbee.getResponse().getZBRxResponse(rx);
      addr64=rx.getRemoteAddress64();
    
      char copy[rx.getDataLength()];
      String readenCommand((char*)rx.getData());
      readenCommand.toCharArray(copy, rx.getDataLength());
      token = strtok(copy, "+");
      String commandName(token);

      if(DEBUG_MODE)      
        Serial.println(commandName);
      if(commandName=="CONFIG"){
          token = strtok(NULL, "+");
          String command(token);
          if(DEBUG_MODE)      
            Serial.println(command);
          if(command=="ON"){
            CONFIG_MODE = true; 
            ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
            xbee.send(zbTx);
          }else if(command=="OFF"){
            CONFIG_MODE = false;
            ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
            xbee.send(zbTx);
          }else{
            deSerialize(token);
          }
     }else if(commandName =="RUN"){
        token = strtok(NULL, "+");
        String runningModeToken(token);
        Serial.println(runningModeToken);
        if(*token == '?'){
          Serial.print("RUNNING+");
          Serial.println(runningMode);
        }else if(*token=='D'){
          runDetail();
        }else{
          runningMode = runningModeToken.toInt();
          ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
          xbee.send(zbTx);
        }

     }else if(commandName =="DEBUG"){
          token = strtok(NULL, "+");
          String command(token);
          if(DEBUG_MODE)      
            Serial.println(command);
          if(command=="ON"){
            DEBUG_MODE = 1; 
            ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
            xbee.send(zbTx);
          }else if(command=="OFF"){
            DEBUG_MODE = 0;
            ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
            xbee.send(zbTx);
          }
     }
   }
  }
  ////////////////////////////////////////
  // OPERATION 
  ////////////////////////////////////////
  if(!CONFIG_MODE){
    if(DEBUG_MODE){
      Serial.print("\n runningMode: ");
      Serial.print(runningMode);
      Serial.print(" queue Size: ");
      Serial.print(runModes[runningMode].count);Serial.print("\n ");
    }
    for(runModes[runningMode].runningO=0;runModes[runningMode].runningO<runModes[runningMode].count;runModes[runningMode].runningO++){
      int p=runModes[runningMode].queue[runModes[runningMode].runningO];
      if(DEBUG_MODE){
        Serial.print(" queue: ");
        Serial.print(p);
      }
      if(opModes[p].bindedTo>-1){
        if(opModes[opModes[p].bindedTo].curCond){ // stop if the binded condition is true else run this condition also
          debugDetail(opModes[p].bindedTo);
          opModes[opModes[p].bindedTo].curCond=true;
          continue;
        }
      }
      operate(p);
    }      
  }else{
    delay(100);
  }

  Serial.flush (); // let serial printing finish
}

void operate(int p){
  String ifCondition(opModes[p].ifCond);
     
  if(calculatedCond(p)){
    opModes[p].curCond=true;
    
    String strOut(opModes[p].doOutput);
    if(DEBUG_MODE){
       Serial.print(" doOutput : ");
    } 
    if(opModes[p].sendWhat.startsWith("INPUTS")){
        opModes[p].sendWhat="INPUTS";
        opModes[p].sendWhat+='+';
        for(int g=0;g<6;g++){
          int a = analogRead(g);
          opModes[p].sendWhat+='#';
          opModes[p].sendWhat+=g;
          opModes[p].sendWhat+='*';
          opModes[p].sendWhat+=a;
        }
        opModes[p].sendWhat+='\0';
    }else{
      if(DEBUG_MODE){
        Serial.print(opModes[p].sendWhat);
      }
    }
    
    if( strOut== "D"){
      pinMode(opModes[p].toPin,OUTPUT);
      
      if(DEBUG_MODE){
        Serial.print(" to : ");Serial.print(opModes[p].toPin);
      }
      if(opModes[p].sendWhat == "LOW"){
        digitalWrite(opModes[p].toPin,LOW);
      }else if(opModes[p].sendWhat == "HIGH"){
        digitalWrite(opModes[p].toPin,HIGH);
      }
    }else if(strOut == "S"){
      Serial.println(opModes[p].sendWhat);
    }else if(strOut == "RS"){
      Serial.println(opModes[p].sendWhat);
    }else if(strOut == "M"){
      Serial.print(" changeModeTo: ");
      Serial.println(opModes[p].sendWhat);
      runningMode=opModes[p].sendWhat.toInt();runningO=0;
    }else if(strOut == "X"){
      char mopy[opModes[p].sendWhat.length()];
      opModes[p].sendWhat.toCharArray(mopy,opModes[p].sendWhat.length());
      ZBTxRequest zbTx = ZBTxRequest(addr64, mopy, opModes[p].sendWhat.length());
      xbee.send(zbTx);
    }
  }else if(ifCondition == "T"){
  }else if(ifCondition != "C"){
    
    opModes[p].curCond=false;
    if(DEBUG_MODE){
       Serial.print(" doNot Output : ");Serial.print(opModes[p].sendWhat);
    } 
    String strOut(opModes[p].doOutput);
    if( strOut== "D"){
      pinMode(opModes[p].toPin,OUTPUT);
      if(DEBUG_MODE){
        Serial.print(" to : ");Serial.print(opModes[p].toPin);
      }
      if(opModes[p].sendWhat != "LOW"){
        digitalWrite(opModes[p].toPin,LOW);
      }else if(opModes[p].sendWhat != "HIGH"){
        digitalWrite(opModes[p].toPin,HIGH);
      }
    }
  }//fromPin
  if(DEBUG_MODE)
    Serial.print("\n ");
}
boolean calculatedCond(int p)
{
  String ifCondition(opModes[p].ifCond);
      
  pinMode(opModes[p].fromAPin,INPUT);digitalWrite(opModes[p].fromAPin, HIGH);
  digitalWrite(opModes[p].fromAPin, INPUT_PULLUP);
  int a = analogRead(opModes[p].fromAPin);
  if(DEBUG_MODE){
    Serial.print(" if fromPin ");Serial.print(opModes[p].fromAPin);Serial.print(" : ");
    Serial.print(a);
  }
  if(DEBUG_MODE){
    Serial.print(" ");Serial.print(ifCondition);Serial.print(" to/than : ");Serial.print(opModes[p].state);
  }
  if(ifCondition == "E"){
    if(a == opModes[p].state){
      return true;
     }else{
      return false;
     }
  } else if(ifCondition == "G"){
    int a = analogRead(opModes[p].fromAPin);
    if(a > opModes[p].state){
      return true;
    }else{
      return false;
    }
  }else  if(ifCondition == "L"){
    int a = analogRead(opModes[p].fromAPin);
    if(a < opModes[p].state){
      return true;
    }else{
      return true;
    }
  }else  if(ifCondition == "C"){
    return true;
  }else if(ifCondition=="T"){
    if(!counting){ // the beginning of time
      opModes[p].startMillis = millis();
      counting = true;   
       opModes[p].curCond=true;
      return false;
    }else{
      if(millis()-opModes[p].startMillis>opModes[p].state){ // end of the time delay
        counting=false;
        opModes[p].startMillis=0;
        
        opModes[p].curCond=false;
        return false;
      }else{ // normal condition so if any binded condition will not run
        opModes[p].curCond=true;
        return false;
      }
    }
  }
}
void deSerialize(char *value){
  //char *modeName="";
  int count=-1,removed=-1,queue=-1,fromPin=-1,toPin=-1,state=-1,bindedTo=-1;
  char *ifCond,*doOutput,*sendWhat,*temp_mode[9],*mode[9];
  
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
     count = atoi(mode[k]);
   }
   
   if(*token=='Q'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     queue = atoi(mode[k]);
   }
   
   
   
   if(*token=='I'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     ifCond=mode[k];
   }
   if(*token=='S'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     state = atoi(mode[k]);
   }
   if(*token=='F'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     fromPin = atoi(mode[k]);
   }
   if(*token=='D'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     doOutput=mode[k];
   }
   if(*token=='W'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     sendWhat=mode[k];
   }
   if(*token=='T'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     toPin = atoi(mode[k]);
   }
    if(*token=='B'){
     token = strtok(NULL, "*");
     mode[k]=token;   
     bindedTo = atoi(mode[k]);
   }
   if(*token=='R'){
     if(DEBUG_MODE)
      Serial.println("remove");
      token = strtok(NULL, "*");
      mode[k]=token;
      removed = atoi(mode[k]);
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
  if(removed>-1){
    
    Serial.print("removing ");Serial.print(removed);
    boolean found = false;
    for(int o =0;o<=runModes[removed].count;o++){
     if(found){
       runModes[removed].queue[o-1]=runModes[removed].queue[o];
       if(DEBUG_MODE){
         Serial.println("removed");
       }  
     }
     if(runModes[removed].queue[o]==queue){
      if(DEBUG_MODE){
         //Serial.println("Found");
         Serial.print(" queue ");Serial.print(queue);
         Serial.print(" in order ");Serial.println(o);
       }
       found=true;
       runModes[removed].count--;
       ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
       xbee.send(zbTx);
     }
    }
   return;
  }
  if(queue>-1 && j>2){
    Serial.print(" new queue ");Serial.println(queue);
    opModes[queue].fromAPin = fromPin;
    opModes[queue].toPin = toPin;
    opModes[queue].state = state;
    opModes[queue].bindedTo = bindedTo;
    opModes[queue].ifCond=*ifCond;
    opModes[queue].doOutput=*doOutput;
    opModes[queue].sendWhat  =sendWhat;
  }
  if(count>-1){
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
     runDetail();
     runModes[count].queue[runModes[count].count]=queue;
     runModes[count].count++;
    }
    ZBTxRequest zbTx = ZBTxRequest(addr64, ok, 2);
    xbee.send(zbTx);
  }
}
void debugDetail(int p){
  if(DEBUG_MODE){
    Serial.println();
    Serial.print("ifCond ");Serial.println(opModes[p].ifCond);
    Serial.print("state ");Serial.println(opModes[p].state);
    Serial.print("fromAPin ");Serial.println(opModes[p].fromAPin);
    Serial.print("bindedTo ");Serial.println(opModes[p].bindedTo);
    Serial.print("doOutput ");Serial.println(opModes[p].doOutput);
    Serial.print("toPin ");Serial.println(opModes[p].toPin);
    Serial.print("sendWhat ");Serial.println(opModes[p].sendWhat);
    Serial.print("startMillis ");Serial.println(opModes[p].startMillis);
    Serial.print("curCond ");Serial.println(opModes[p].curCond);
    Serial.println();
  }
 }
void runDetail(){
  Serial.print("In Mode : ");Serial.println(runningMode);
  Serial.print("size : ");Serial.println(runModes[runningMode].count);
  for(int r=0;r<runModes[runningMode].count;r++){
    int p=runModes[runningMode].queue[r];  
    Serial.print("  ");Serial.print(runModes[runningMode].queue[p]);Serial.print(": ");
    Serial.print(" if A");Serial.print(opModes[p].fromPin);Serial.print(" ");Serial.print(opModes[p].ifCond);Serial.print(" ");Serial.print(opModes[p].state);Serial.print(" ");
    Serial.print(" then send ");Serial.print(opModes[p].sendWhat);Serial.print(" to ");Serial.print(opModes[p].doOutput);Serial.print(opModes[p].toPin);Serial.println(" ");
  }
}
