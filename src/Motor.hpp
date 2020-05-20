//
// Motor: DRV8833 / DRV8835 motor driver
//
// Author: A.Navatta

#ifndef MOTOR_H

#define MOTOR_H

#include <application.h>

#define MOTOR_DRV8833_IN1       D2
#define MOTOR_DRV8833_IN2       D3
#define MOTOR_DRV8833_ENABLE    D4
#define MOTOR_DRV8833_FAULT     D5

#define MOTOR_DRV8835_ENA       D2
#define MOTOR_DRV8835_PHASE     D3
#define MOTOR_DRV8835_ENABLE    D4

#define MOTOR_WIPER     A2
#define MOTOR_ISENSE    A4

#define MOTOR_SPEED_BIT 10
#define MOTOR_SPEED_MAX (1 << MOTOR_SPEED_BIT) - 1

// 96 is safety threshold
#define MOTOR_LOW_THRESHOLD 1700
#define MOTOR_HIGH_THRESHOLD 3950
#define MOTOR_CURRENT_THRESHOLD 2100

#define MOTOR_POSITION_PROTECTION   1
#define MOTOR_CURRENT_PROTECTION     1

#define MOTOR_DRV8833_FASTDECAY_MODE    0

enum MOTOR_DIRECTION { FORWARD, REVERSE, STOP };

class Motor {
public:
    Motor();
    void init(bool use8835 = false, bool enableSpeed = false);
    void speed(int speed);
    void forward(int speed);
    void reverse(int speed);
    void stop(String message);
    void poll();

    inline int getPosition() { return position; };

private:
    int position;
    int current;
    int current_max;
    bool use8835;
    bool speedControl;
    MOTOR_DIRECTION direction;
};

extern Motor PQ12Motor;

#endif