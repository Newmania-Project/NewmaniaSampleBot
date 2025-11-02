#include <Arduino.h>
#include <esp32_motor.hpp>
#include <esp32_servo.hpp>
#include <ps2x.hpp>

#define PS2_DAT  1
#define PS2_CMD  2
#define PS2_CS   4
#define PS2_CLK  5
#define LED 47

typedef struct {
    uint8_t pressed;
    uint8_t lastPressed;
    int8_t X;
    int8_t Y;
} JoyStickButton;
JoyStickButton leftJoy;
JoyStickButton rightJoy;

typedef struct {
    uint8_t up;
    uint8_t down;
    uint8_t left;
    uint8_t right;
} DPadButton;
DPadButton DPad;
DPadButton lastDPad;

typedef struct {
    uint8_t triangle;
    uint8_t circle;
    uint8_t cross;
    uint8_t square;
} geoPadButton;
geoPadButton GeoPad;
geoPadButton lastGeoPad;

typedef struct {
    uint8_t L1;
    uint8_t L2;
    uint8_t R1;
    uint8_t R2;
} shoulderButton;
shoulderButton Shoulder;
shoulderButton lastShoulder;

#define joyThres 0
void getRemoteState(PS2X &remote) {
    remote.read_gamepad();
    leftJoy.pressed = remote.Button(PSB_L3);
    leftJoy.X = remote.Analog(PSS_LX) - 128;
    leftJoy.Y = 127 - remote.Analog(PSS_LY);
    rightJoy.pressed = remote.Button(PSB_R3);
    rightJoy.X = remote.Analog(PSS_RX) - 128;
    rightJoy.Y = 127 - remote.Analog(PSS_RY);
    if (leftJoy.X > 0) leftJoy.X = map(leftJoy.X, 0, 127, 0, 100);
    else leftJoy.X = map(leftJoy.X, -128, 0, -100, 0);
    if (leftJoy.Y > 0) leftJoy.Y = map(leftJoy.Y, 0, 127, 0, 100);
    else leftJoy.Y = map(leftJoy.Y, -128, 0, -100, 0);
    if (rightJoy.X > 0) rightJoy.X = map(rightJoy.X, 0, 127, 0, 100);
    else rightJoy.X = map(rightJoy.X, -128, 0, -100, 0);
    if (rightJoy.Y > 0) rightJoy.Y = map(rightJoy.Y, 0, 127, 0, 100);
    else rightJoy.Y = map(rightJoy.Y, -128, 0, -100, 0);
    if (abs(rightJoy.X) < joyThres) rightJoy.X = 0;
    if (abs(rightJoy.Y) < joyThres) rightJoy.Y = 0;
    if (abs(leftJoy.X) < joyThres) leftJoy.X = 0;
    if (abs(leftJoy.Y) < joyThres) leftJoy.Y = 0;
    DPad.up = remote.Button(PSB_PAD_UP);
    DPad.down = remote.Button(PSB_PAD_DOWN);
    DPad.left = remote.Button(PSB_PAD_LEFT);
    DPad.right = remote.Button(PSB_PAD_RIGHT);
    GeoPad.triangle = remote.Button(PSB_TRIANGLE);
    GeoPad.circle = remote.Button(PSB_CIRCLE);
    GeoPad.cross = remote.Button(PSB_CROSS);
    GeoPad.square = remote.Button(PSB_SQUARE);
    Shoulder.L1 = remote.Button(PSB_L1);
    Shoulder.L2 = remote.Button(PSB_L2);
    Shoulder.R1 = remote.Button(PSB_R1);
    Shoulder.R2 = remote.Button(PSB_R2);
}
#define pressedButton(current, last) ((current == 1 && last == 0) ? 1 : 0)
#define releasedButton(current, last) ((current == 0 && last == 1) ? 1 : 0)
int m1speed = 0;
int m2speed = 0;
int m3speed = 0;
int m4speed = 0;

#define verticalVelocity rightJoy.Y
#define horizontalVelocity rightJoy.X
#define angularVelocity leftJoy.X
#define vRate 2.0
#define hRate 2.0
#define angularRate 2.0
#define vLimit 70
#define hLimit 70
#define aLimit 60
void calculateMotorSpeeds() {
    int V = verticalVelocity*vRate;   // Forward/Backward
    int H = horizontalVelocity*hRate; // Left/Right
    int A = angularVelocity*angularRate;    // Rotation
    // Apply rate limits
    if (V > vLimit) V = vLimit;
    if (H > hLimit) H = hLimit;
    if (A > aLimit) A = aLimit;
    if (V < -vLimit) V = -vLimit;
    if (H < -hLimit) H = -hLimit;
    if (A < -aLimit) A = -aLimit;

    m1speed = V - H + A; // Back Left
    m2speed = V + H + A; // Front Left
    m3speed = V - H - A; // Front Right
    m4speed = V + H - A; // Back Right

    // Normalize speeds to be within -100 to 100
    int maxSpeed = max(max(abs(m1speed), abs(m2speed)), max(abs(m3speed), abs(m4speed)));
    if (maxSpeed > 100) {
        m1speed = (m1speed * 100) / maxSpeed;
        m2speed = (m2speed * 100) / maxSpeed;
        m3speed = (m3speed * 100) / maxSpeed;
        m4speed = (m4speed * 100) / maxSpeed;
    }
}