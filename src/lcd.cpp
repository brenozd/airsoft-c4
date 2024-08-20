#include "lcd.h"
#include <Arduino.h>

LCD* LCD::singleton_ = nullptr;

LCD::LCD() { 
  lcd_ = new LiquidCrystal_I2C(0x27, COLS, ROWS);
  Serial.println("Initializing LCD display");
  lcd_->init();
  lcd_->backlight();
  lcd_->clear();
}

LCD::~LCD() {
  delete lcd_;
}

LCD* LCD::instance() {
  if (singleton_ == nullptr)
    singleton_ = new LCD();
  return singleton_;
}