#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <Adafruit_Keypad.h>

class Keypad {
public:
  Keypad(Keypad &other) = delete;
  Keypad operator=(const Keypad &) = delete;
  static Keypad *instance();

  const inline Adafruit_Keypad *get() {
    return keypad_;
  }

  const inline void tick() {
    keypad_->tick();
  }

  void block_until_key_pressed(char* key, unsigned long time_ms, void (*callback_pressed)(unsigned long press_duration) = nullptr, void (*callback_release)(unsigned long press_duration) = nullptr);

protected:
  static const byte ROWS = 4;
  static const byte COLS = 3;
  char keys[ROWS][COLS] = {
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
  };
  byte rowPins[ROWS] = { 8, 7, 6, 5 };
  byte colPins[COLS] = { 4, 3, 2 };

  Keypad();
  ~Keypad();
  Adafruit_Keypad *keypad_;
  static Keypad *singleton_;
};

#endif