#include <iostream>
#include <vector>
#include <algorithm>

constexpr int MAX_VALUE = 1000000;

struct Coin {
  int position, time;
  Coin(int position_, int time_): position(position_), time(time_)
  {}
};

struct CoinComparartor {
bool operator()(const Coin& first_coin, const Coin& second_coin) {
  if (first_coin.position == second_coin.position) {
    return first_coin.time < second_coin.time;
  }
  return first_coin.position < second_coin.position;
  }
};

std::vector<Coin> read_coins() {
  int size;
  std::vector<Coin> coins;
  std::cin >> size;
  for (int i = 0; i < size; ++i) {
      int cur_position, cur_time;
      std::cin >> cur_position >> cur_time;
      coins.emplace_back(Coin(cur_position, cur_time));
  }
  return coins;
}

int get_answer_for_time_and_direction(int cur_time, int direction, const std::vector<Coin>& coins) {
  int begin, end;
  if (direction == 1) {
      end = 0;
      begin = coins.size() - 1;
  } else {
      end = coins.size() - 1;
      begin = 0;
  }
  int cur_position = end;
  bool flag = true;
  while (flag) {
    if (cur_time > coins[cur_position].time) {
      if (cur_position == end) {
        return 2;
      }
      cur_time -= abs(coins[cur_position].position - coins[begin].position);
      end = begin;
      begin = cur_position;
      cur_position = end;
      direction *= -1;
    } else {
      if (cur_position != begin) {
        cur_time -= abs(coins[cur_position].position - coins[cur_position + direction].position);
        cur_position += direction;
      } else {
        if (cur_time >= 0) {
          return 0;
        } else {
          return cur_time;
        }
      }
    }
  }
}

int get_best_time(const std::vector<Coin>& coins) {
  for (int direction = -1; direction <= 1; direction += 2) {
    int cur_time = 0;
    int answer;
    bool flag = true;
    while (flag) {
      if (cur_time > MAX_VALUE) {
        break;
      }
      answer = get_answer_for_time_and_direction(cur_time, direction, coins);
      if (answer == 0) {
        return cur_time;
      }
      if (answer == 2) {
        break;
      }
      cur_time -= answer;
    }
    if (direction == 1) {
      return -1;
    }
  }
}

void erase_duplicates(std::vector<Coin>& coins) {
  for (int i = coins.size() - 1; i > 0; --i) {
    if (coins[i].position == coins[i - 1].position) {
      coins.erase(coins.begin() + i);
    }
  }
}

int main() {
  auto coins = read_coins();
  std::sort(coins.begin(), coins.end(), CoinComparartor());
  erase_duplicates(coins);
  int best_time = get_best_time(coins);
  if (best_time == -1) {
    std::cout << "No solution" << std::endl;
  } else {
    std::cout << best_time << std::endl;
  }
  return 0;
}
