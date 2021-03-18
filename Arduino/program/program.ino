#include "mainPlayer.h"

void setup(){
  Serial.begin(9600);
  initPlayer();  
}

void loop(){
  framePlayer();
} 
