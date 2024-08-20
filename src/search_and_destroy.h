#ifndef __SEARCH_AND_DESTROY_H__
#define __SEARCH_AND_DESTROY_H__

#include "game_mode.h"
#include <Adafruit_Keypad.h>

class SearchAndDestroy : public GameMode{
  public:
    SearchAndDestroy();
    ~SearchAndDestroy() = default;

    Teams start() override final;
    Teams stop() override final {};

  private:
    static constexpr unsigned int password_size = 5;
    static constexpr unsigned int max_feedback_delay_ms = 1000;
    static constexpr unsigned int min_feedback_delay_ms = 25;
    static constexpr unsigned int fuse_time_sec_ = 10;
    static constexpr unsigned int user_input_sec_ = 20;
    
    bool is_armed;
    bool defused;
  
    // 0 is invalid
    // 1 is valid
    // -1 is not enough input
    int validate_password(int8_t input[password_size]);
    void reset_user_input();
    int8_t* generate_password();
    int8_t password_[password_size];
    char password_str_[password_size + 1];
    int8_t user_input_[password_size];
    int8_t user_input_index;
};

#endif