#ifndef __GAME_MODE_H__
#define __GAME_MODE_H__

enum Teams {
  Terrorists,
  CounterTerrorists,
};

class GameMode {
public:
  GameMode() = default;
  virtual ~GameMode() = default;

  virtual Teams start(){};
  virtual Teams stop(){};

  const char* get_name() const {
    return name_;
  }
  void prompt_for_game_duration();
  inline Teams get_winner() const {
    return winner_;
  }
  
protected:
  Teams winner_;
  bool game_ended_;
  unsigned long duration_;
  char name_[16];

private:
  unsigned long last_blink_;
};

#endif