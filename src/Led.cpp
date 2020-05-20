#include <application.h>
#include <Led.hpp>

Led *_led = 0;

int ledToggle(String command) {
    if (!_led)
        return -1;
    if (command=="on") {
        _led->on();
        return 1;
    }
    else if (command=="off") {
        _led->off();
        return 0;
    } else if (command=="toggle") {
        _led->toggle();
        return (_led->isOn() ? 1 : 0);
    } else {
        return -1;
    }
}

Led::Led() : led_on(false) { }

void Led::init(bool publish) {
    led1 = D0;
    led2 = D7;
    // Here's the pin configuration, same as last time
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);

    // For good measure, let's also make sure both LEDs are off when we start:
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);

    if (publish) {
        _led = this;
        Particle.function("led",ledToggle);
    }
}

void Led::on() {
    digitalWrite(led1,HIGH);
    digitalWrite(led2,HIGH);        
}

void Led::off() {
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);        
}

void Led::toggle() {
    if (led_on) {
        // turn off
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
    } else {
        // turn on
        digitalWrite(led1,HIGH);
        digitalWrite(led2,HIGH);
    }
    led_on = !led_on;
}
