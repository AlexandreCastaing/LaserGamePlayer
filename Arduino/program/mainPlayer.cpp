#include "mainPlayer.h"
#include "TimedAction.h"
#include <SoftwareSerial.h>

int isInitialised_Internal = false;

bool playerIsLaserReady = false;
bool playerIsLaserSensorReady = false;
int idPlayer = 0;

String colorOfPlayer = "NONE";

bool laserOn = false;
int laserIdCounter;

int laserSensorIdCounter = 0;
int last_laserSensorIdCounter = 0;
int n_laserSensorIdCounter = 0;
int time_playerCurrentlyDeadCounter = time_playerDead_default;

bool isPlayerKilled = false;

bool commandWaitForID1dgt = false;
bool commandWaitForID2dgt = false;
bool commandWaitForTMDTH1dgt = false;
bool commandWaitForTMDTH2dgt = false;

bool isSendingMessage = false;
  
int temp_timeDeath;
int temp_idPlayer;

int time_playerDead;

String fireButtonEvent = "IS_NOT_INIT";

TimedAction threadPlayer = TimedAction(delay_playerAction, blinkPLAYER);
TimedAction threadLaser = TimedAction(delay_laserAction, blinkLASER);
TimedAction threadLaserSensor = TimedAction(delay_laserAction / delay_laserAction__laserSensorActionDivisionTime, blinkLASERSENSOR);
TimedAction threadESPCOM = TimedAction(delay_ESPCOMAction, blinkESPCOM);
TimedAction threadTimerDead = TimedAction(delay_playerAction, blinkPLAYERISDEAD);

SoftwareSerial serialESP(pin_RXESP, pin_TXESP); // RX, TX

void framePlayer() {
  if (!isInitialised_Internal) return;

  threadPlayer.check();
  threadLaser.check();
  threadLaserSensor.check();
  threadESPCOM.check();
  threadTimerDead.check();
}

void initPlayer()
{
  pinMode(pin_fireButton, INPUT);
  pinMode(pin_laserSensor, INPUT);
  pinMode(pin_ledsPlayerTeamRed, OUTPUT);
  pinMode(pin_ledsPlayerTeamBlue, OUTPUT);
  pinMode(pin_laser, OUTPUT);
  
  pinMode(pin_IDOx1, INPUT);
  pinMode(pin_IDOx2, INPUT);
  pinMode(pin_IDOx3, INPUT);
  pinMode(pin_IDOx4, INPUT);
  pinMode(pin_IDOx5, INPUT);
  
  digitalWrite(pin_ledsPlayerTeamRed, LOW);
  digitalWrite(pin_ledsPlayerTeamBlue, LOW);
  digitalWrite(pin_laser, LOW); 
  serialESP.begin(serial_baudrate_ESP);

  // time dead player
  time_playerDead = time_playerDead_default;

  // calcul ID
  setIdByPins();
    
  // code player here
  isNotReady();
  colorPlayer("NONE");
  setPlayerId(idPlayer);

  // ####

  isInitialised_Internal = true;
}

void setPlayerId(int _idPlayer) {
  idPlayer = _idPlayer;
  laserIdCounter = 0;
}
void isReady() {
  playerIsLaserReady = true;
  playerIsLaserSensorReady = true;
  laserIdCounter = 0;
}
void isNotReady() {
  playerIsLaserReady = false;
  playerIsLaserSensorReady = false;
}

void colorPlayer(String _idColor) {
  int A = 0;
  int B = 0;

  if (_idColor == "BLUE") {
    A = 0;
    B = HIGH;
  } else if (_idColor == "RED") {
    A = HIGH;
    B = 0;
  }
  
  digitalWrite(pin_ledsPlayerTeamRed, A);
  digitalWrite(pin_ledsPlayerTeamBlue, B);
}

void blinkPLAYER()
{
  int fireButton = digitalRead(pin_fireButton);
  fireButtonEvents(fireButton);
}

void fireButtonEvents(int _fireButton) {
  
  if ( _fireButton <= fireButtonSensitive ) {
    if (fireButtonEvent != "OFF")
      fireButtonEvent = "GOING_OFF";
  } else {
    if (fireButtonEvent != "ON")
      fireButtonEvent = "GOING_ON";
  }
  if ( fireButtonEvent == "GOING_OFF" ) {
    fireButton_GoingOFF();
    fireButtonEvent = "OFF";
  }
  if ( fireButtonEvent == "OFF" ) {
    fireButton_IsOFF();
  }
  if ( fireButtonEvent == "GOING_ON" ) {
    fireButton_GoingON();
    fireButtonEvent = "ON";
  }
  if ( fireButtonEvent == "ON" ) {
    fireButton_IsON();
  }
}

void fireButton_GoingON() {
  laserOn = true;
};
void fireButton_GoingOFF() {

};
void fireButton_IsON() {

};
void fireButton_IsOFF() {
  if (laserIdCounter == 0) {
    laserOn = false;
  }
};

void blinkLASER()
{
  if (playerIsLaserReady && laserOn && !isPlayerKilled ) {

    if (laserIdCounter < idPlayer) {
      digitalWrite(pin_laser, HIGH);
      laserIdCounter++;
    } else {
      digitalWrite(pin_laser, LOW);
      laserIdCounter = 0;
    }

  } else {
    digitalWrite(pin_laser, LOW);
  }

}

void blinkLASERSENSOR()
{

  if (playerIsLaserSensorReady && !isPlayerKilled ) {

    bool hasLaserOnSensor = digitalRead(pin_laserSensor);
    
    if (hasLaserOnSensor) {
      laserSensorIdCounter++;
    }
    else {
      if (laserSensorIdCounter > 0){
        int arroundedPlayerId = arround(laserSensorIdCounter, delay_laserAction__laserSensorActionDivisionTime);
        arroundedPlayerId /= delay_laserAction__laserSensorActionDivisionTime;

        if(last_laserSensorIdCounter == arroundedPlayerId){
          n_laserSensorIdCounter++;
  
          // Killed
          if(n_laserSensorIdCounter >= totalNCheck_LaserId){

             isKilled(arroundedPlayerId);
             
          }
        }
        else{
          n_laserSensorIdCounter = 0;
        }
  
        last_laserSensorIdCounter = arroundedPlayerId;
      }
        
      laserSensorIdCounter = 0;
    }
    
  }

}

void isKilled(int _byPlayerId){

  if(isPlayerKilled == true) return;

  // if(_byPlayerId == idPlayer) return;
  
  isPlayerKilled = true;

  // send killer to server
  
  
  char  msgStr[2];

  msgStr[0] = 'K';
  msgStr[3] = 'z';
  
  if(_byPlayerId < 10){
    msgStr[1] = '0';
    String idStr = String(_byPlayerId);
    msgStr[2] = idStr[0];
  }else{
    char  idStr_2[1];
    itoa(_byPlayerId, idStr_2, 10);
    msgStr[1] = idStr_2[0];
    msgStr[2] = idStr_2[1];
  }

  isSendingMessage = true;

  delay(delay_ESPCOMAction);
  serialESP.write(msgStr[0]);
  delay(delay_ESPCOMAction);
  serialESP.write(msgStr[1]);
  delay(delay_ESPCOMAction);
  serialESP.write(msgStr[2]);
  delay(delay_ESPCOMAction);
  serialESP.write(msgStr[3]);

  isSendingMessage = false;

  // ####
  
  colorPlayer("NONE");
  delay(delay_flashKill);
  colorPlayer(colorOfPlayer);
  delay(delay_flashKill);
  
  colorPlayer("NONE");
  delay(delay_flashKill);
  colorPlayer(colorOfPlayer);
  delay(delay_flashKill);
  
  colorPlayer("NONE");
  delay(delay_flashKill);
  colorPlayer(colorOfPlayer);
  delay(delay_flashKill);
  
  colorPlayer("NONE");
  
}

int arround(int _value, int _offset){
  int offsetD2 = _offset / 2;
  double test = ((double)_value / (double)_offset);
  test += ((double)offsetD2 / (double)_offset);
  return (int)test * _offset;
}

void blinkESPCOM()
{
  
  if(serialESP.available() > 0) {
    if(isSendingMessage == false){
      char resp = serialESP.read();
      commandESP(resp);
    }
  }
}

void commandESP(char _cmd){
   if(_cmd == 'I'){ // init
      time_playerCurrentlyDeadCounter = time_playerDead;
      isNotReady();
      setPlayerId(-1);
      colorPlayer("NONE");
      colorOfPlayer = "NONE";
      idPlayer = -1;
      Serial.println("Debug: INIT OK");
   }
   if(_cmd == 'E'){ // end
      time_playerCurrentlyDeadCounter = time_playerDead;
      isNotReady();
   }
   if(_cmd == 'S'){ // start
      time_playerCurrentlyDeadCounter = time_playerDead;
      colorPlayer(colorOfPlayer);
      setPlayerId(idPlayer);
      isReady();
   }
   if(_cmd == 'R'){
      colorOfPlayer = "RED";
      colorPlayer(colorOfPlayer);
   }
   if(_cmd == 'B'){
      colorOfPlayer = "BLUE";
      colorPlayer(colorOfPlayer);
   }
   if(_cmd == 'N'){
      colorOfPlayer = "NONE";
      colorPlayer(colorOfPlayer);
   } 
   if(_cmd == 'P'){
      isSendingMessage = true;
      setIdByPins();  
      isSendingMessage = false; 
   }
   if(_cmd == 'C'){ // get id
    
      char  msgStr[2];

      msgStr[0] = 'D';
      msgStr[3] = 'z';

      int _playerId = idPlayer;
      
      if(_playerId < 10){
        msgStr[1] = '0';
        String idStr = String(_playerId);
        msgStr[2] = idStr[0];
      }else{
        char  idStr_2[1];
        itoa(_playerId, idStr_2, 10);
        msgStr[1] = idStr_2[0];
        msgStr[2] = idStr_2[1];
      }

      isSendingMessage = true;
      
      delay(delay_ESPCOMAction);
      serialESP.write(msgStr[0]); // send
      delay(delay_ESPCOMAction);
      serialESP.write(msgStr[1]);
      delay(delay_ESPCOMAction);
      serialESP.write(msgStr[2]);
      delay(delay_ESPCOMAction);
      serialESP.write(msgStr[3]);

      isSendingMessage = false;
      
   }
   if(_cmd == 'D'){
      commandWaitForID1dgt = true; 
      commandWaitForID2dgt = false; 
      idPlayer = -1;
   }
   if(_cmd == 'T'){
      commandWaitForTMDTH1dgt = true; 
      commandWaitForTMDTH2dgt = false; 
      time_playerDead = time_playerDead_default;
   }
 
   if (isDigit(_cmd)){  
      if(commandWaitForID1dgt == true){ // affect ID to player
        int vInt = _cmd - '0';
        if(commandWaitForID2dgt == true){// second Ox coming
          idPlayer += vInt;
          _cmd = 'z';
        }
        if(commandWaitForID2dgt == false){ // first Ox coming
          commandWaitForID2dgt = true;
          idPlayer = (vInt * 10);
        }        
      }
      else if(commandWaitForTMDTH1dgt == true){
        int vInt = _cmd - '0';
        if(commandWaitForTMDTH2dgt == true){// second Ox coming
          time_playerDead += vInt;
          time_playerDead *= offset_playerDeadCmd;
          _cmd = 'z';
        }
        if(commandWaitForTMDTH2dgt == false){ // first Ox coming
          commandWaitForTMDTH2dgt = true;
          time_playerDead = (vInt * 10);
        }    
      }
   }
   if(_cmd == 'z'){
    
      commandWaitForID1dgt = false;    
      commandWaitForID2dgt = false; 
      commandWaitForTMDTH1dgt = false; 
      commandWaitForTMDTH2dgt = false; 
  }
  
}

void blinkPLAYERISDEAD(){
  if(isPlayerKilled){
    if(time_playerCurrentlyDeadCounter > 0){
      time_playerCurrentlyDeadCounter -= 1;
    }
    else{

      colorPlayer(colorOfPlayer);

      isPlayerKilled = false;
      time_playerCurrentlyDeadCounter = time_playerDead;

    }
  }
}

void setIdByPins(){
  int resultId = 0;

  int int1 = digitalRead(pin_IDOx1);
  int int2 = digitalRead(pin_IDOx2);
  int int4 = digitalRead(pin_IDOx3);
  int int8 = digitalRead(pin_IDOx4);
  int int16 = digitalRead(pin_IDOx5);
  
  if(int1 > 0) resultId += 1;
  if(int2 > 0) resultId += 2;
  if(int4 > 0) resultId += 4;
  if(int8 > 0) resultId += 8;
  if(int16 > 0) resultId += 16;

  resultId += startIdPlayerOffset;
  idPlayer = 0;
  idPlayer = resultId;
  
  Serial.print("Debug: ID PLAYER: ");
  Serial.println(idPlayer);
  Serial.print(" -- ");
  Serial.print(int1);
  Serial.print(" ");
  Serial.print(int2);
  Serial.print(" ");
  Serial.print(int4);
  Serial.print(" ");
  Serial.print(int8);
  Serial.print(" ");
  Serial.println(int16);
}
