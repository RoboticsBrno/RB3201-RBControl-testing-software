    
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
  man.leds().blue(man.expander().digitalRead(SW1));
  man.leds().green(man.expander().digitalRead(SW2));
  man.leds().yellow(man.expander().digitalRead(SW3));
}