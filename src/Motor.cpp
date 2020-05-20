#include <Motor.hpp>
#include <Telemetry.hpp>

Motor PQ12Motor;

int _setSpeed(String args) {
    C_DEBUG("Speed(%s)", args.c_str());

    int value  = args.toInt();

    PQ12Motor.speed(value);
    
    return 0;
}

int _getState(String args) {

    int value  = digitalRead(MOTOR_DRV8833_FAULT);

    C_DEBUG("getState(%d)",value);

    return value;
}

int _getPosition(String args) {

    int value = PQ12Motor.getPosition();

    C_DEBUG("getState(%d)",value);

    return value;
}

Motor::Motor() {
}

void Motor::init(bool _use8835, bool enableSpeed) {
    C_DEBUG("Init motor");

    use8835 = _use8835;
    speedControl = enableSpeed;

    if (use8835) {
        // use DRV8835
        // set pin output
        pinMode(MOTOR_DRV8835_ENABLE, OUTPUT);
        pinMode(MOTOR_DRV8835_ENA, OUTPUT);
        pinMode(MOTOR_DRV8835_PHASE, OUTPUT);

        digitalWrite(MOTOR_DRV8835_ENABLE,TRUE);
    } else {
        // use DRV8833    
        // set pin output
        pinMode(MOTOR_DRV8833_ENABLE, OUTPUT);
        pinMode(MOTOR_DRV8833_IN1, OUTPUT);
        pinMode(MOTOR_DRV8833_IN2, OUTPUT);

        if (speedControl) {
            analogWriteResolution(MOTOR_DRV8833_IN1, MOTOR_SPEED_BIT);
            analogWriteResolution(MOTOR_DRV8833_IN2, MOTOR_SPEED_BIT);
        }
        pinMode(MOTOR_DRV8833_FAULT, INPUT_PULLUP);

        digitalWrite(MOTOR_DRV8833_ENABLE,TRUE);
    }

    Particle.function("Motor.speed", _setSpeed);
    Particle.function("Motor.state", _getState);
    Particle.function("Motor.position", _getPosition);

    stop(String("Init"));

    current = current_max = 0;

    poll();
}

void Motor::speed(int speed) {

    int pwm_speed = abs(speed) * 10;

    if (speed > 0) 
        direction = FORWARD;
    else if (speed < 0) 
        direction = REVERSE;
    else 
        stop("user stop");
    
    poll();

    if (direction == FORWARD)
        forward(pwm_speed);
    else if (direction ==REVERSE)
        reverse(pwm_speed);
}

void Motor::forward(int pwm_speed) {
    if (use8835) {
        digitalWrite(MOTOR_DRV8835_ENA,1);
        digitalWrite(MOTOR_DRV8835_PHASE,0);
        Particle.publish("Motor forward",PRIVATE);
    } else {
        if (speedControl) {
#ifdef MOTOR_DRV8833_FASTDECAY_MODE
            // DRV8833 forward fast decay IN1(pwm), IN2(0)
            analogWrite(MOTOR_DRV8833_IN1, pwm_speed);
            analogWrite(MOTOR_DRV8833_IN2, 0);
            Particle.publish("Motor forward, fast",String(pwm_speed), PRIVATE);
#else
            // DRV8833 forward slow decay IN1(1), IN2(pwm)
            analogWrite(MOTOR_DRV8833_IN1, MOTOR_SPEED_MAX);
            analogWrite(MOTOR_DRV8833_IN2, pwm_speed);
            Particle.publish("Motor forward, slow",String(pwm_speed), PRIVATE);        
#endif
        } else {
            digitalWrite(MOTOR_DRV8833_IN1,1);
            digitalWrite(MOTOR_DRV8833_IN2,0);
            Particle.publish("Motor forward",PRIVATE);
        }
    }    
}

void Motor::reverse(int pwm_speed) {
    if (use8835) {
        digitalWrite(MOTOR_DRV8835_ENA,1);
        digitalWrite(MOTOR_DRV8835_PHASE,1);
        Particle.publish("Motor reverse",PRIVATE);
    } else {
        if (speedControl) {
#ifdef MOTOR_DRV8833_FASTDECAY_MODE
            // DRV8833 reverse fast decay IN1(0), IN2(pwm)
            analogWrite(MOTOR_DRV8833_IN1, 0);
            analogWrite(MOTOR_DRV8833_IN2, pwm_speed);
            Particle.publish("Motor reverse, fast",String(pwm_speed), PRIVATE);        
#else
            // DRV8833 reverse slow decay IN1(1), IN2(pwm)
            analogWrite(MOTOR_DRV8833_IN1, pwm_speed);
            analogWrite(MOTOR_DRV8833_IN2, MOTOR_SPEED_MAX);
            Particle.publish("Motor reverse, slow",String(pwm_speed), PRIVATE);        
#endif
        } else { // no speed control 
            digitalWrite(MOTOR_DRV8833_IN1,0);
            digitalWrite(MOTOR_DRV8833_IN2,1);
            Particle.publish("Motor reverse",PRIVATE);
        }
    }

}

void Motor::stop(String message) {

    if (use8835) {
        digitalWrite(MOTOR_DRV8835_ENA,0);
        digitalWrite(MOTOR_DRV8835_PHASE,0);
    } else {
        if (speedControl) {     
            // DRV8833 brake slow decay IN1(1), IN2(pwm)
            analogWrite(MOTOR_DRV8833_IN1, MOTOR_SPEED_MAX);
            analogWrite(MOTOR_DRV8833_IN2, MOTOR_SPEED_MAX);
        } else {
            digitalWrite(MOTOR_DRV8833_IN1,1);
            digitalWrite(MOTOR_DRV8833_IN2,1);
        }
    }

    C_DEBUG("stop");

    direction = STOP;
    Particle.publish("Motor stop",message, PRIVATE);
}

void Motor::poll() {
#ifdef MOTOR_POSITION_PROTECTION
    position = analogRead(MOTOR_WIPER);
    if ((position > MOTOR_HIGH_THRESHOLD) && (direction == FORWARD)) {
        stop("Poll, reverse motor protection enabled");
        return;
    } else if ((position < MOTOR_LOW_THRESHOLD) && (direction == REVERSE)) {
        stop("Poll, forward motor protection enabled");
        return;
    }
#endif
#ifdef MOTOR_CURRENT_PROTECTION
    current = analogRead(MOTOR_ISENSE);
    String msg;
    String newmsg = msg.format("Motor current(%d), max(%d)",current, current_max);
    Serial.println(newmsg);
    if (current > current_max) 
        current_max = current;
    if (current > MOTOR_CURRENT_THRESHOLD) {
        stop("Poll, current limit reached");
        return;
    }
#endif
}