#ifndef MAIN_PLAYER_H
#define MAIN_PLAYER_H
#include <Arduino.h>

#define pin_ledsPlayerTeamBlue 9
#define pin_ledsPlayerTeamRed 10
#define pin_laser             8
#define pin_fireButton        11
#define pin_laserSensor       6
#define pin_RXESP            13
#define pin_TXESP            12
#define pin_IDOx1            5
#define pin_IDOx2            4
#define pin_IDOx3            3
#define pin_IDOx4            2
#define pin_IDOx5            1

#define delay_playerAction    1 // ms
#define delay_laserAction     1// ms 6
#define delay_laserAction__laserSensorActionDivisionTime 1 // ms 3
#define delay_ESPCOMAction    5 // ms
#define delay_flashKill       100 // ms
#define time_playerDead_default      4000 // x  playerAction
#define offset_playerDeadCmd  250
#define serial_baudrate_ESP   9600
#define totalNCheck_LaserId   4
#define fireButtonSensitive   0
#define startIdPlayerOffset   10

void initPlayer();
void framePlayer();
void colorPlayer(String);
void blinkPLAYER();
void blinkLASER();
void blinkLASERSENSOR();
void blinkESPCOM();
void blinkPLAYERISDEAD();
void setPlayerId(int);
void isReady();
void isNotReady();
void fireButtonEvents(int);
void fireButton_GoingON();
void fireButton_GoingOFF();
void fireButton_IsON();
void fireButton_IsOFF();
int arround(int, int);
void isKilled(int);
void commandESP(char);
void setIdByPins();
#endif
