#ifndef __CONFIG_H__
#define __CONFIG_H__

#define M_PI 3.14159265358979323846  // Definição de PI

static constexpr double DELAY_ = static_cast<double>(1000) / 60;

static constexpr unsigned int buzzer_pin = 9;
static constexpr unsigned int led_live_pin = 10;
static constexpr unsigned int led_count_pin = 11;
static constexpr unsigned int led_is_powered = 12;

static constexpr unsigned long reset_time_ms_ = 2000;

const unsigned int buzzer_tone_khz = 2050;

#endif