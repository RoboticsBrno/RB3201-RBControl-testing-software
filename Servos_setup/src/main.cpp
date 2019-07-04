#include "RBControl.hpp"

using namespace rb;

// Config:
constexpr auto SERVO_PIN = GPIO_NUM_32; // CHANGE ME IF USING DIFFERENT PIN
constexpr auto SERVO_COUNT  = 3;
constexpr auto SERVO_POS = 120.0; // degrees

/********** README - HOW TO USE **********
 *
 * This program sets ids for servos and rotates them to 120deg position.
 *
 * LEDs:
 *  * Red blinking: error, servo is not responding.
 *  * Yellow: setting servo id 0
 *  * Green: setting servo id 1
 *  * Blue: setting servo id 2
 *  * All leds at once: setup is complete.
 *
 * Instructions:
 *  0) Start this program on ESP
 *  1) Connect ONLY the servo for ID 0
 *  2) Wait if the red LED stops blinking. If not, check serial output and fix
 *  3) Disconnect the servo
 *  4) Press button SW1, next LED in the series will light up
 *  5) Connect the next servo
 *  6) Go to 2) until all servos are set-up
 *  7) Once complete, this program lights up all the servo LEDs (Y,G,B) at once.
 *  8) Connect all the servos to the ESP at once
 *  9) See if the red error LED stops blinking. If not, check the serial output and fix it.
 *     The set-up is only successful if the red LED stops blinking. At that point, all
 *     servos have correct IDs and they are at 120deg position.
 *
 */



#define MS(x) (x / portTICK_PERIOD_MS)

void loop() { }

static bool gErroring = false;
static bool gErrorLed = false;
static int gServoId = 0;
static TickType_t gLastIdSwitch = 0;
static uint16_t gBtnDebounce = 0;
static std::mutex gMutex;

static void setErrorStateLocked(bool error) {
    if(gErroring == error)
        return;

    gErroring = error;
    if(!error)
        return;

    Manager::get().schedule(200, []() -> bool {
        gMutex.lock();
        gErrorLed = !gErrorLed;
        Manager::get().leds().red(gErroring && gErrorLed);
        bool res = gErroring;
        gMutex.unlock();
        return res;
    });
}

static bool setupServo() {
    std::lock_guard<std::mutex> l(gMutex);
    if(gServoId >= SERVO_COUNT)
        return false;

    auto& servos = Manager::get().servoBus();

    servos.setId(gServoId);

    auto pos = servos.pos(gServoId);
    printf("\nSet servo ID #%d, returned pos: %f", gServoId, pos.deg());
    setErrorStateLocked(pos.isNaN());
    if(pos.isNaN()) {
        printf(" <------ ERROR!!!\n\n");
    } else {
        printf(" OK\n\n");
    }

    servos.set(gServoId, Angle::deg(SERVO_POS + 0.1), 150);
    servos.set(gServoId, Angle::deg(SERVO_POS), 150);
    return true;
}

static void setLedsForId(int id) {
    id = id%3;
    Manager::get().leds().yellow(id == 0);
    Manager::get().leds().green(id == 1);
    Manager::get().leds().blue(id == 2);
}

void setup() {
    auto& man = Manager::get();
    man.install(MAN_DISABLE_MOTOR_FAILSAFE | MAN_DISABLE_BATTERY_MANAGEMENT);

    gLastIdSwitch = xTaskGetTickCount();

    man.initSmartServoBus(SERVO_COUNT, SERVO_PIN);

    setLedsForId(0);
    man.schedule(10, [&]() -> bool {
        if(man.expander().digitalRead(SW1) != 0) {
            gBtnDebounce = 0;
            return true;
        }

        if(++gBtnDebounce < 5)
            return true;

        const auto now = xTaskGetTickCount();
        if((now - gLastIdSwitch) < MS(1000))
            return true;

        gLastIdSwitch = now;

        gMutex.lock();
        const int id = ++gServoId;
        gMutex.unlock();

        if(id < SERVO_COUNT) {
            setLedsForId(gServoId);
            return true;
        }
        return false;
    });

    while(1) {
        if(!setupServo())
            break;
        vTaskDelay(MS(300));
    }

    auto& servos = man.servoBus();

    man.leds().green();
    man.leds().yellow();
    man.leds().blue();

    printf("\nSetup complete!\n");
    while(1) {
        bool error = false;
        for(int i = 0; i < SERVO_COUNT; ++i) {
            auto p = servos.pos(i);
            if(p.isNaN() || fabs(p.deg() - 120.0) > 1.5) {
                error = true;
                printf("%d: %.2f <------- ERROR!\n", i, p.deg());
                if(!p.isNaN()) {
                    servos.set(i, Angle::deg(SERVO_POS + 0.1), 150);
                    servos.set(i, Angle::deg(SERVO_POS), 150);
                }
            } else {
                printf("%d: %.2f OK!\n", i, p.deg());
            }
        }
        printf("\n");

        gMutex.lock();
        setErrorStateLocked(error);
        gMutex.unlock();

        vTaskDelay(MS(1000));
    }
}