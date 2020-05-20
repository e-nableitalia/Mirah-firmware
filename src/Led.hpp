//
// Led: we have led, so let's use it
//
// Author: A.Navatta

#ifndef LED_H

#define LED_H

class Led {
public:
    Led();
    void init(bool publish = false);
    void on();
    void off();
    void toggle();
    inline bool isOn() { return led_on; }
private:
    int led1;
    int led2;
    bool led_on;
};

#endif