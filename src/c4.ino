#include "game_mode.h"
#include "search_and_destroy.h"
#include "lcd.h"
#include "keypad.h"
#include "config.h"

bool resetted = false;
GameMode* game_modes[] = { new SearchAndDestroy() };
const int num_modes = 1;
GameMode* selectedMode;

GameMode* select_game_mode() {
  unsigned long pressStartTime = 0;
  int selectedIndex = 0;
  LCD* lcd = LCD::instance();
  Keypad* keypad = Keypad::instance();
  lcd->clear();
  lcd->set_cursor(0, 0);
  lcd->print("Select Game Mode:");

  lcd->clear_line(1);
  lcd->set_cursor(0, 1);
  lcd->print_centered(game_modes[selectedIndex]->get_name(), 1);

  while (true) {
    keypad->tick();
    Adafruit_Keypad* keypad_adafruit = keypad->get();
    while (keypad_adafruit->available()) {
      keypadEvent e = keypad_adafruit->read();
      if (e.bit.EVENT == KEY_JUST_PRESSED) {
        char key = e.bit.KEY;

        if (key == '*') {
          // Mover para cima na lista
          selectedIndex = (selectedIndex - 1 + num_modes) % num_modes;
          lcd->clear_line(1);
          lcd->set_cursor(0, 1);
          lcd->print(game_modes[selectedIndex]->get_name());
        } else if (key == '0') {
          // Mover para baixo na lista
          selectedIndex = (selectedIndex + 1) % num_modes;
          lcd->clear_line(1);
          lcd->set_cursor(0, 1);
          lcd->print(game_modes[selectedIndex]->get_name());
        } else if (key == '#') {
          pressStartTime = millis();
        }
      } else if (e.bit.EVENT == KEY_JUST_RELEASED && e.bit.KEY == '#') {
        if (millis() - pressStartTime >= 500) {
          for (int i = 0; i < num_modes; i++) {
            if (i == selectedIndex) continue;
            delete game_modes[i];
          }
          return game_modes[selectedIndex];
        }

      } else
        selectedIndex = (selectedIndex + 1) % num_modes;
    }
  }
}

long getRandomSeed(int numBits) {
  // magic numbers tested 2016-03-28
  // try to speed it up
  // Works Well. Keep!
  if (numBits > 31 or numBits < 1) numBits = 31;  // limit input range

  const int baseIntervalMs = 1UL;    // minumum wait time
  const byte sampleSignificant = 7;  // modulus of the input sample
  const byte sampleMultiplier = 10;  // ms per sample digit difference

  const byte hashIterations = 3;
  int intervalMs = 0;

  unsigned long reading;
  long result = 0;
  int tempBit = 0;

  Serial.print("randomizing...");
  pinMode(A0, INPUT_PULLUP);
  pinMode(A0, INPUT);
  delay(200);
  // Now there will be a slow decay of the voltage,
  // about 8 seconds
  // so pick a point on the curve
  // offset by the processed previous sample:
  //

  for (int bits = 0; bits < numBits; bits++) {
    Serial.print('*');

    for (int i = 0; i < hashIterations; i++) {
      delay(baseIntervalMs + intervalMs);

      // take a sample
      reading = analogRead(A0);
      tempBit ^= reading & 1;

      // take the low "digits" of the reading
      // and multiply it to scale it to
      // map a new point on the decay curve:
      intervalMs = (reading % sampleSignificant) * sampleMultiplier;
    }
    result |= (long)(tempBit & 1) << bits;
  }
  Serial.println("");
  return result;
}

void feedback_reset_pressed(unsigned long press_duration) {
  int progress = map(press_duration, 0, reset_time_ms_, 0, 16);

  LCD* lcd_singleton = LCD::instance();
  LiquidCrystal_I2C* lcd = lcd_singleton->get();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print("Resetting... ");

  char progress_bar[17];
  progress_bar[0] = '[';
  progress_bar[15] = ']';
  progress_bar[16] = '\0';

  for (int i = 1; i < 15; i++) {
    if (i < progress)
      progress_bar[i] = '#';
    else
      progress_bar[i] = ' ';
  }

  lcd->setCursor(0, 1);
  lcd->print(progress_bar);
  delay(100);
  if (press_duration >= reset_time_ms_) {
    resetted = true;
  }
}

void feedback_reset_release(unsigned long press_duration) {
  char* winner_str;
  if (selectedMode->get_winner() == Teams::CounterTerrorists) winner_str = "CT Wins";
  else winner_str = "TR Wins";

  LCD* lcd_singleton = LCD::instance();
  LiquidCrystal_I2C* lcd = lcd_singleton->get();
  lcd->clear();
  lcd_singleton->print_centered(winner_str, 0);
}

void setup() {
  Serial.begin(9600);

  Serial.println("Initializing random");
  randomSeed(getRandomSeed(31) & millis());

  Serial.println("Initializing pin modes");
  pinMode(led_is_powered, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(led_live_pin, OUTPUT);
  pinMode(led_count_pin, OUTPUT);

  Serial.println("Setting pins");
  digitalWrite(led_is_powered, LOW);
  digitalWrite(led_count_pin, HIGH);
  digitalWrite(led_live_pin, HIGH);
  selectedMode = select_game_mode();
  selectedMode->prompt_for_game_duration();
}

void loop() {
  Teams winner = selectedMode->start();

  Keypad* keypad_singleton = Keypad::instance();
  keypad_singleton->block_until_key_pressed(nullptr, 2000, feedback_reset_pressed, feedback_reset_release);
  if (resetted) {
    resetted = false;
    return;
  }
}