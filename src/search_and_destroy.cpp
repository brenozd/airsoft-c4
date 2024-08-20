#include "Arduino.h"
#include "search_and_destroy.h"

#include <string.h>
#include "lcd.h"
#include "keypad.h"
#include "config.h"

SearchAndDestroy::SearchAndDestroy()
  : GameMode() {
  char name[] = "Search&Destroy";
  strcpy(name_, name);
  is_armed = false;
}

int SearchAndDestroy::validate_password(int8_t input[password_size]) {
  if (user_input_index < password_size)
    return -1;

  for (int i = 0; i < password_size; i++) {
    if (input[i] == -1) return -1;
    if (input[i] != password_[i]) return 0;
  }

  return 1;
}

void SearchAndDestroy::reset_user_input() {
  for (int i = 0; i < password_size; i++) user_input_[i] = -1;
  user_input_index = 0;
}


double easeInSine(double x) {
  return 1 - cos((x * M_PI) / 2);
}

int8_t* SearchAndDestroy::generate_password() {
  for (int i = 0; i < password_size; i++) {
    password_[i] = random(0, 10);
    password_str_[i] = '0' + password_[i];
  }
  password_str_[password_size] = '\0';
  return password_;
}

Teams SearchAndDestroy::start() {
  user_input_index = 0;
  bool armed_feedback_state = false;

  unsigned int remaining_duration = duration_;
  Keypad* keypad_singleton = Keypad::instance();
  Adafruit_Keypad* keypad = keypad_singleton->get();
  LCD* lcd_singleton = LCD::instance();
  LiquidCrystal_I2C* lcd = lcd_singleton->get();

  this->generate_password();
  lcd_singleton->clear_line(0);
  lcd->setCursor(0, 0);
  lcd_singleton->print_centered(password_str_, 0);
  static char buffer[9];  // HH:MM:SS precisa de 8 caracteres + '\0'
  unsigned int hours = remaining_duration / 3600;
  unsigned int minutes = (remaining_duration % 3600) / 60;
  unsigned int seconds = remaining_duration % 60;
  snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes, seconds);
  lcd_singleton->clear_line(1);
  lcd_singleton->print_centered(buffer, 1);

  unsigned long last_loop_ = millis();
  unsigned long last_user_input_ = last_loop_;
  unsigned long last_feedback_ = last_loop_;

  digitalWrite(led_is_powered, LOW);
  digitalWrite(led_count_pin, HIGH);
  digitalWrite(led_live_pin, HIGH);

  is_armed = false;
  defused = false;
  while (remaining_duration > 0) {
    long delta = millis() - last_loop_;
    if (delta >= 1000) {
      remaining_duration--;
      last_loop_ = millis();
      // Formatação do tempo
      static char buffer[9];  // HH:MM:SS precisa de 8 caracteres + '\0'
      unsigned int hours = remaining_duration / 3600;
      unsigned int minutes = (remaining_duration % 3600) / 60;
      unsigned int seconds = remaining_duration % 60;
      snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes, seconds);
      lcd_singleton->clear_line(1);
      lcd_singleton->print_centered(buffer, 1);
    }

    keypad->tick();
    while (keypad->available()) {
      keypadEvent e = keypad->read();
      char key = e.bit.KEY;
      if (e.bit.EVENT == KEY_JUST_PRESSED && key >= '0' && key <= '9') {
        user_input_[user_input_index] = key - '0';
        user_input_index++;
        Serial.println(key);
      }
      last_user_input_ = millis();
    }

    int is_passwd_valid = validate_password(user_input_);
    switch (is_passwd_valid) {
      case 0:
        {
          Serial.println("Password is invalid, resetting user input");
          reset_user_input();
          break;
        }
      case 1:
        {
          Serial.println("Password is valid, toggling C4 state");
          if (is_armed)
            defused = true;
          else {
            reset_user_input();
            remaining_duration = fuse_time_sec_;
            digitalWrite(led_is_powered, HIGH);
            digitalWrite(led_live_pin, LOW);
          }
          is_armed = true;

          break;
        }
    }

    if (!is_armed && (millis() - last_user_input_) >= user_input_sec_ * 1000) {
      Serial.println("Too much time between user inputs, resetting");
      user_input_index = 0;
      for (int i = 0; i < password_size; i++) user_input_[i] = -1;
      this->generate_password();
      lcd_singleton->clear_line(0);
      lcd->setCursor(0, 0);
      lcd_singleton->print_centered(password_str_, 0);
      last_user_input_ = millis();
    }


    if (is_armed) {
      double fuse_ratio = static_cast<double>(remaining_duration) / fuse_time_sec_;
      double eased_ratio = easeInSine(fuse_ratio);
      int delay_ms = eased_ratio * max_feedback_delay_ms;
      if (delay_ms < min_feedback_delay_ms) delay_ms = min_feedback_delay_ms;

      digitalWrite(led_live_pin, LOW);
      long delta = millis() - last_feedback_;
      if (delta >= delay_ms && armed_feedback_state) {
        digitalWrite(led_count_pin, LOW);
        tone(buzzer_pin, 2050);
        last_feedback_ = millis();
        armed_feedback_state = !armed_feedback_state;
      } else if (delta >= delay_ms) {
        digitalWrite(led_count_pin, HIGH);
        noTone(buzzer_pin);
        last_feedback_ = millis();
        armed_feedback_state = !armed_feedback_state;
      }
    }

    if (defused) {
      Serial.println("Defused C4");
      reset_user_input();
      is_armed = false;
      remaining_duration = 0;
      digitalWrite(led_is_powered, LOW);
      digitalWrite(led_live_pin, LOW);
      digitalWrite(led_count_pin, LOW);
    }
    delay(DELAY_);
  }

  noTone(buzzer_pin);
  lcd->clear();
  lcd->setCursor(0, 0);
  if (!defused) {
    winner_ = Teams::Terrorists;
  } else {
    winner_ = Teams::CounterTerrorists;
  }
  return winner_;
}