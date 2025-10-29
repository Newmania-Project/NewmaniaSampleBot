#include <main.h>

PS2X remote;
void setup() {
    Serial.begin(921600);
    byte error = remote.config_gamepad(PS2_CLK, PS2_CMD, PS2_CS, PS2_DAT);
    if (error) {
        ledcSetup(7, 10, 12); // Channel 7, 10 kHz, 12-bit resolution
        ledcAttachPin(LED, 7);
        ledcWrite(7, 2048); // Turn on LED to indicate error
        while (error) error = remote.config_gamepad(PS2_CLK, PS2_CMD, PS2_CS, PS2_DAT);
        ledcWrite(7, 0); // Turn off LED after successful connection
        ledcDetachPin(LED);
    }
    Motor1.Reverse();
    Motor2.Reverse();
}

#define HOLD_THRESHOLD 155
#define RELEASE_THRESHOLD 50
#define PICKUP_THRESHOLD 60
#define DROP_THRESHOLD 25
bool holdState = false;
bool pickupState = true;
#define ang0 0
#define ang1 130
#define ang2 150
uint8_t servo3angle = 0;
bool servoDebug = false;
void loop() {
    getRemoteState(remote);
    if (!servoDebug) {
        if (holdState) {
            servo1.write(HOLD_THRESHOLD);
        } else {
            servo1.write(RELEASE_THRESHOLD);
        }
    } else {
        servo1.write(0); // Debug position
    }
    if (pressedButton(Shoulder.R1, lastShoulder.R1)) {
        holdState = !holdState;
    }
    if (pickupState) {
        servo4.write(PICKUP_THRESHOLD);
    } else {
        servo4.write(DROP_THRESHOLD);
    }
    if (pressedButton(Shoulder.R2, lastShoulder.R2)) {
        pickupState = !pickupState;
    }
    if (pressedButton(DPad.right, lastDPad.right)) {
    servo3angle = ang0;
    }
    if (pressedButton(DPad.up, lastDPad.up)) {
        servo3angle = ang1;
    }
    if (pressedButton(DPad.left, lastDPad.left)) {
        servo3angle = ang2;
    }
    if (pressedButton(GeoPad.cross, lastGeoPad.cross)) {
        servoDebug = !servoDebug;   
    }
    servo3.write(servo3angle);
    calculateMotorSpeeds();
    Motor1.Run(m1speed);
    Motor2.Run(m2speed);
    Motor3.Run(m3speed);
    Motor4.Run(m4speed);
    leftJoy.lastPressed = leftJoy.pressed;
    rightJoy.lastPressed = rightJoy.pressed;
    lastDPad = DPad;
    lastGeoPad = GeoPad;
    lastShoulder = Shoulder;
    delay(10);
}
