#ifndef __LCD_H__
#define __LCD_H__

#include <LiquidCrystal_I2C.h>

class LCD {
public:
  LCD(LCD &other) = delete;
  LCD operator=(const LCD &) = delete;
  static LCD *instance();

  const inline void print(const char *msg) {
    lcd_->print(msg);
  }

  const inline void set_cursor(unsigned int column, unsigned int row) {
    lcd_->setCursor(column, row);
  }

  const inline void clear() {
    lcd_->clear();
  }

  const inline LiquidCrystal_I2C *get() {
    return lcd_;
  }

  const inline void clear_line(unsigned int line_no) {
    lcd_->setCursor(0, line_no);
    for (int n = 0; n < COLS; n++) {
      lcd_->print(" ");
    }
  }

  const inline void print_centered(const char *text, int line) {
    int len = strlen(text);
    int pos = (16 - len) / 2;

    lcd_->setCursor(pos, line);
    lcd_->print(text);
  }

protected:
  static const int ROWS = 2;
  static const int COLS = 16;
  LCD();
  ~LCD();
  LiquidCrystal_I2C *lcd_;
  static LCD *singleton_;
};

#endif