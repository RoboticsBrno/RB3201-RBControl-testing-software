// GY-652 - magnetometr lepsi,   dalsi je GY 531

#include <Arduino.h>
#include "RBControl_manager.hpp"
#include <Servo.h>
#include <Wire.h>
#include "time.hpp"
#include <stdint.h>
#include "stopwatch.hpp"
#include "nvs_flash.h"
#include "BluetoothSerial.h"

static inline rb::Manager& rbc() 
{
    return rb::Manager::get();
}

inline bool sw1() { return !rbc().expander().digitalRead(rb::SW1); }
inline bool sw2() { return !rbc().expander().digitalRead(rb::SW2); }
inline bool sw3() { return !rbc().expander().digitalRead(rb::SW3); }

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

int speed = 0;
long time_1 = 0;
long time_2 = 0;

void setup() 
{
    Serial.begin (115200);
    rbc().install(rb::ManagerInstallFlags::MAN_DISABLE_MOTOR_FAILSAFE | rb::ManagerInstallFlags::MAN_DISABLE_BATTERY_MANAGEMENT);
}
    
timeout send_data { msec(200) }; // timeout zajistuje posilani dat do PC kazdych 500 ms

void loop()
{
    rbc().leds().yellow(sw1()); 
    if (send_data) 
    {
        static bool L_G_light;
        send_data.ack();
        if (L_G_light) L_G_light = false; else  L_G_light = true;
        rbc().leds().green(L_G_light);
    }

    if(sw1())
    {
        time_1 = millis();
        while (time_1 + 1000 > millis())
        {
            if(time_2 + 10 < millis())
            {
              time_2 = millis();
              speed++;
            }
            rbc().setMotors().power(rb::MotorId::M1, speed)
                             .power(rb::MotorId::M2, speed)
                             .power(rb::MotorId::M3, speed)
                             .power(rb::MotorId::M4, speed)
                             .set();
        }
        time_1 = millis();
        while (time_1 + 2000 > millis())
        {
            if(time_2 + 10 < millis())
            {
              time_2 = millis();
              speed--;
            }
            rbc().setMotors().power(rb::MotorId::M1, speed)
                             .power(rb::MotorId::M2, speed)
                             .power(rb::MotorId::M3, speed)
                             .power(rb::MotorId::M4, speed)
                             .set();
        }
        time_1 = millis();
        while (time_1 + 1000 > millis())
        {
            if(time_2 + 10 < millis())
            {
              time_2 = millis();
              speed++;
            }
            rbc().setMotors().power(rb::MotorId::M1, speed)
                             .power(rb::MotorId::M2, speed)
                             .power(rb::MotorId::M3, speed)
                             .power(rb::MotorId::M4, speed)
                             .set();
        }
        rbc().setMotors().stop(rb::MotorId::M1)
                         .stop(rb::MotorId::M2)
                         .stop(rb::MotorId::M3)
                         .stop(rb::MotorId::M4)
                         .set();
    }
}