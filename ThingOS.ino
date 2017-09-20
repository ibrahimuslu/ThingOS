/*
  ThingOS
  Runs device according to the message coming from serial Input SmaartU

 Created by ibrahim USLU
  17 Sep 2017

 This example code is in the public domain.

 */
int DEBUG_MODE =1;
int sensorValue = 0;  // variable to store the value coming from the sensor

int runningMode=0;  // default runMode 0
int configMode =1;

struct opMode
{
  char* ifCond='\0';
  int state=0;
  int fromPin=0;
  int fromAPin = A0;
  char* doOutput='\0';
  int toPin=0;
  char* sendWhat='\0';
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
  char* factoryMode =  "C*0#Q*0#I*E#S*1023#F*3#D*D#W*HIGH#T*7";
  //#M*normal

  // declare the ledPin as an OUTPUT:
  
  Serial.begin(9600); 
  if(DEBUG_MODE)
    Serial.println("ThingOS serialport opened and ready to use!");
  //deSerialize(factoryMode);
}

void loop() {
  // read the value from the sensor:
  
      char *runningModeToken,readenCommand[100],*token;
      String serialRead,commandName,command;   
      
    if (Serial.available()) {
      if(DEBUG_MODE)
        Serial.println("serial port is active!");
      
      serialRead = Serial.readString();
      
      serialRead.trim(); 
      if(DEBUG_MODE)      
        Serial.println(serialRead);
      serialRead.toCharArray(readenCommand,serialRead.length()+1);
      
      
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

    }
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
            String strOut(opModes[p].doOutput);
            if( strOut== "D"){
              pinMode(opModes[p].toPin,OUTPUT);
              if(DEBUG_MODE){
                Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
              }
              String str(opModes[p].sendWhat);
              if(str == "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str == "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }else if(strOut == "S"){
              Serial.println(opModes[p].sendWhat);
            }else if(strOut == "RS"){
              Serial.println(opModes[p].sendWhat);
            }
          }else{
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
              if(str != "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str != "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }
          }//fromPin
        /*
        // SEND COMMAND
        */
        }else if(ifCondition == "C"){
          
          if(DEBUG_MODE){
            Serial.print(" set : ");Serial.print(opModes[p].state);
          }
          String strOut(opModes[p].doOutput);
          if( strOut== "D"){
            pinMode(opModes[p].toPin,OUTPUT);  
            if(DEBUG_MODE){
              Serial.print(" doOutput : ");Serial.print(opModes[p].sendWhat);Serial.print(" to ");Serial.print(opModes[p].toPin);
            }
            String str(opModes[p].sendWhat);
            if(str == "LOW"){
              digitalWrite(opModes[p].toPin,LOW);
            }else if(str == "HIGH"){
              digitalWrite(opModes[p].toPin,HIGH);
            }
          }else if(strOut == "S"){
            Serial.println(opModes[p].sendWhat);
          }else if(strOut == "RS"){
            Serial.println(opModes[p].sendWhat);
          }
        /*
        // GREATER
        */
        }else if(ifCondition == "G"){
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
            String strOut(opModes[p].doOutput);
            if( strOut== "D"){
            pinMode(opModes[p].toPin,OUTPUT);
              if(DEBUG_MODE){
                Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
              }
              String str(opModes[p].sendWhat);
              if(str == "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str == "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }else if(strOut == "S"){
              Serial.println(opModes[p].sendWhat);
            }else if(strOut == "RS"){
              Serial.println(opModes[p].sendWhat);
            }
          }else{
            if(DEBUG_MODE){
              Serial.print(" greater than : ");Serial.print(opModes[p].state);
            }
            String strOut(opModes[p].doOutput);
            if( strOut== "D"){
              pinMode(opModes[p].toPin,OUTPUT);
              if(DEBUG_MODE){
                Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
              }
              String str(opModes[p].sendWhat);
              if(str != "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str != "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }
          }//fromPin
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
            String strOut(opModes[p].doOutput);
            if( strOut== "D"){
              pinMode(opModes[p].toPin,OUTPUT);
              if(DEBUG_MODE){
                Serial.print("doOutput : ");Serial.print(opModes[p].toPin);
              }
              String str(opModes[p].sendWhat);
              if(str == "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str == "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }else if(strOut == "S"){
              Serial.print(opModes[p].sendWhat);
            }else if(strOut == "RS"){
              Serial.print(opModes[p].sendWhat);
            }
          }else{
            if(DEBUG_MODE){
              Serial.print(" lower than : ");Serial.print(opModes[p].state);
            }
            String strOut(opModes[p].doOutput);
            if( strOut== "D"){
              pinMode(opModes[p].toPin,OUTPUT);
              if(DEBUG_MODE){
                Serial.print(" doOutput to : ");Serial.print(opModes[p].toPin);
              }
              String str(opModes[p].sendWhat);
              if(str != "LOW"){
                digitalWrite(opModes[p].toPin,LOW);
              }else if(str != "HIGH"){
                digitalWrite(opModes[p].toPin,HIGH);
              }
            }
          }//fromPin
        } //ifCond//ifCond//ifCond//ifCond//ifCond//ifCond
      }      
    }
 
}


void deSerialize(char *value){
  //char *modeName="";
  int count=0,queue=0,fromPin=0,toPin=0,state=0;
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
  int j=0;
  while(token != NULL ){
     
    temp_mode[j]=token; 
    if(DEBUG_MODE)
      Serial.println(temp_mode[j]);
    token = strtok(NULL, "#");
  j++;
  }// parse grain
  
  for(j=0;j<8;j++){
     token = NULL;
     token = strtok(temp_mode[j], "*");
     
     /* walk through other tokens */
     if(*token=='C'){
       token = strtok(NULL, "*");
       mode[j]=token;
       if(DEBUG_MODE)     
         Serial.println(mode[j]);
       count = atoi(mode[j]);
     }
     
     if(*token=='Q'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[j]);
       queue = atoi(mode[j]);
     }
     
     
     
     if(*token=='I'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)  
         Serial.println(mode[j]);
       strncpy(ifCond,mode[j],1);
     }
     if(*token=='S'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[j]);
       state = atoi(mode[j]);
     }
     if(*token=='F'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(token);
       fromPin = atoi(mode[j]);
     }
     if(*token=='D'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)  
         Serial.println(mode[j]);
       strncpy(doOutput,mode[j],1);
     }
     if(*token=='W'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[j]);
       strncpy(sendWhat,mode[j],4);
     }
     if(*token=='T'){
       token = strtok(NULL, "*");
       mode[j]=token;   
       if(DEBUG_MODE)
         Serial.println(mode[j]);
       toPin = atoi(mode[j]);
     }
     if(*token=='R'){
       token = strtok(NULL, "*");
       mode[j]=token;
       if(DEBUG_MODE)
         Serial.println(mode[j]);
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
   strncpy(opModes[queue].ifCond,ifCond,1);
   strncpy(opModes[queue].doOutput,doOutput,1);
   strncpy(opModes[queue].sendWhat,sendWhat,1);
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

