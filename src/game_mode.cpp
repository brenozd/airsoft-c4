#include "game_mode.h"
#include <Arduino.h>

#include "lcd.h"
#include "keypad.h"

void GameMode::prompt_for_game_duration() {
  LCD* lcd = LCD::instance();
  Keypad* keypad = Keypad::instance();

  int timeParts[3] = { 0, 0, 0 };    // HH, MM, SS
  int currentPart = 0;               // 0 - HH, 1 - MM, 2 - SS
  int digitsEntered = 0;             // Contagem de dígitos inseridos para o campo atual
  unsigned long pressStartTime = 0;  // Tempo em que a tecla # foi pressionada

  LiquidCrystal_I2C* lcd_hw = lcd->get();
  lcd_hw->clear();
  last_blink_ = millis();
  bool blink_state = false;
  while (true) {
    lcd_hw->setCursor(0, 0);
    lcd_hw->print("Set Game Time:");

    char timeString[9];  // Formato HH:MM:SS + '\0'
    snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeParts[0], timeParts[1], timeParts[2]);
    lcd->print_centered(timeString, 1);

    // Se já passou o tempo para alternar o estado de piscar
    if (millis() - last_blink_ >= 500) {
      blink_state = !blink_state;  // Alterna o estado de piscar
      last_blink_ = millis();      // Atualiza o tempo do último piscar
    }

    if (blink_state) {
      // Criar uma string com o algarismo piscante substituído por espaço
      char displayString[9];
      strcpy(displayString, timeString);
      int index = (currentPart * 3);
      displayString[index] = ' ';      // Substitui o algarismo atual por espaço
      displayString[index + 1] = ' ';  // Substitui também o segundo dígito de tempo (se necessário)

      // Imprime o tempo com o algarismo piscante substituído
      lcd->print_centered(displayString, 1);
    } else {
      // Imprime o tempo normal
      lcd->print_centered(timeString, 1);
    }

    // Ler as teclas pressionadas
    keypad->tick();

    Adafruit_Keypad* keypad_adafruit = keypad->get();
    while (keypad_adafruit->available()) {
      keypadEvent e = keypad_adafruit->read();

      if (e.bit.EVENT == KEY_JUST_PRESSED) {
        char key = e.bit.KEY;

        if (key >= '0' && key <= '9') {
          int value = key - '0';

          // Se já inserimos 2 dígitos, resetamos o campo para 0 antes de continuar
          if (digitsEntered == 2) {
            timeParts[currentPart] = 0;
            digitsEntered = 0;
          }

          timeParts[currentPart] = timeParts[currentPart] * 10 + value;
          digitsEntered++;

          // Limitar os valores de HH, MM, SS
          if (currentPart == 0 && timeParts[currentPart] > 23) {
            timeParts[currentPart] = 23;  // Limitar a 23 horas
            digitsEntered = 2;            // Bloqueia novos dígitos
          }
          if ((currentPart == 1 || currentPart == 2) && timeParts[currentPart] > 59) {
            timeParts[currentPart] = 59;  // Limitar a 59 minutos/segundos
            digitsEntered = 2;            // Bloqueia novos dígitos
          }
        } else if (key == '#') {
          // Iniciar contagem do tempo de pressão da tecla
          pressStartTime = millis();
        } else if (key == '*') {
          // Permite apagar o último dígito inserido
          timeParts[currentPart] = timeParts[currentPart] / 10;
          digitsEntered = max(0, digitsEntered - 1);  // Reduzir a contagem de dígitos
        }
      } else if (e.bit.EVENT == KEY_JUST_RELEASED && e.bit.KEY == '#') {
        // Checar se o tempo de pressão foi longo o suficiente para finalizar a configuração
        if (millis() - pressStartTime >= 500) {
          // Confirmar a configuração e retornar o tempo total em segundos
          unsigned long totalSeconds = timeParts[0] * 3600 + timeParts[1] * 60 + timeParts[2];
          duration_ = totalSeconds;
          return totalSeconds;
        } else {
          // Se não foi longo, apenas mudar para o próximo campo
          currentPart = (currentPart + 1) % 3;
          digitsEntered = 0;  // Resetar a contagem de dígitos ao mudar de campo
        }
      }
    }
  }
}