#include "keypad.h"
#include "config.h"
Keypad* Keypad::singleton_ = nullptr;

Keypad::Keypad() {
  keypad_ = new Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
  Serial.println("Initializing Keypad");
  keypad_->begin();
}

Keypad::~Keypad() {
  delete keypad_;
}

Keypad* Keypad::instance() {
  if (singleton_ == nullptr)
    singleton_ = new Keypad();
  return singleton_;
}

void Keypad::block_until_key_pressed(char* keys, unsigned long time_ms, void (*callback_pressed)(unsigned long press_duration), void (*callback_released)(unsigned long press_duration)) {
  unsigned long press_start_time = millis();
  unsigned long press_duration = 0;
  bool pressed = false;
  bool released = false;

  while (true) {
    keypad_->tick();
    while (keypad_->available()) {
      keypadEvent e = keypad_->read();
      char pressed_key = e.bit.KEY;
      bool key_match = false;

      if (keys == nullptr) {
        key_match = true;  // Considera qualquer tecla
      } else {
        // Verifica se a tecla pressionada está no array de teclas
        for (int i = 0; keys[i] != '\0'; ++i) {
          if (pressed_key == keys[i]) {
            key_match = true;
            break;
          }
        }
      }

      if (e.bit.EVENT == KEY_JUST_PRESSED && key_match) {
        press_start_time = millis();
        pressed = true;
        released = false;
      } else if (e.bit.EVENT == KEY_JUST_RELEASED && key_match) {
        pressed = false;
        released = true;
      }
    }

    press_duration = millis() - press_start_time;
    if (pressed) {
      if (callback_pressed != nullptr) {
        callback_pressed(press_duration);
      }
    } else if (released) {
      if (callback_released != nullptr) {
        callback_released(press_duration);
      }
      press_start_time = millis();
      released = false;
    } else {
      press_start_time = millis();
    }

    if (press_duration >= time_ms)
      return;
  }
}