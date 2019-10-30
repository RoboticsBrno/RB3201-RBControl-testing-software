#include <Arduino.h>

#include "RBControl_manager.hpp"

using namespace rb;

void setup() {
  // put your setup code here, to run once:
  Manager::get().install();
}

void loop() {
  // put your main code here, to run repeatedly:
  auto& man = Manager::get();
  delay(500);
  man.leds().blue();
  delay(500);
  man.leds().green();
  delay(500);
  man.leds().yellow();
  delay(500);
  man.leds().red();
  delay(500);
  man.leds().blue(0);
  delay(500);
  man.leds().green(0);
  delay(500);
  man.leds().yellow(0);
  delay(500);
  man.leds().red(0);
  delay(500);
}