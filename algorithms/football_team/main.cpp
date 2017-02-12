#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>

struct Player {
  int number;
  int efficiency;
  Player(int number_, int efficiency_) :
      number(number_),
      efficiency(efficiency_) {}
};

struct SegmentTeam {
  std::vector<Player>::iterator left;
  std::vector<Player>::iterator right;
  int64_t efficiency;
};

struct ComparePlayersEfficiency {
  bool operator()(const Player &first_player, const Player &second_player) {
    return first_player.efficiency < second_player.efficiency;
  }
};

struct ComparePlayersNumber {
  bool operator()(const Player &first_player, const Player &second_player) {
    return first_player.number < second_player.number;
  }
};

std::vector<Player> ReadPlayers(std::istream &input_stream = std::cin) {
  int size;
  input_stream >> size;
  std::vector<Player> team;
  team.reserve(size);
  for (int number = 0; number < size; ++number) {
    int cur_efficiency;
    input_stream >> cur_efficiency;
    team.emplace_back(number + 1, cur_efficiency);
  }
  return team;
}

template<class Iterator,
    class Comparator = std::less<typename std::iterator_traits<Iterator>::value_type>>
void QuickSort(Iterator begin, Iterator end, Comparator comparator) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  QuickSort(begin, end, comparator, generator);
}

template<class Iterator, class Comparator, class Generator>
void QuickSort(Iterator begin, Iterator end, Comparator comparator, Generator &generator) {
  if (begin >= end - 1) {
    return;
  }
  std::uniform_int_distribution<int> gen_uniform_unit(0, end - begin - 1);
  auto pivot = begin + gen_uniform_unit(generator);
  pivot = Partition(begin, end, pivot, comparator);
  QuickSort(begin, pivot, comparator, generator);
  QuickSort(pivot, end, comparator, generator);
}

template<class Iterator, class Comparator>
Iterator Partition(Iterator begin, Iterator end, Iterator pivot, Comparator comparator) {
  auto pivot_value = *pivot;
  std::iter_swap(pivot, begin);
  pivot = begin;
  auto left = begin;
  auto right = end;
  while (left <= right) {
    --right;
    while (comparator(pivot_value, *right)) {
      --right;
    }
    while (comparator(*left, pivot_value)) {
      ++left;
    }
    if (!(left < right)) {
      return left;
    }
    std::iter_swap(left, right);
    ++left;
  }
}

std::vector<Player> BuildMostEffectiveSolidaryTeam(std::vector<Player> players) {
  QuickSort(players.begin(), players.end(), ComparePlayersEfficiency());
  SegmentTeam team{players.begin(), players.begin() + 1, players.begin()->efficiency};
  auto best_team = team;
  while (team.right < players.end()) {
    int64_t efficiency_threshold = team.left->efficiency;
    if (team.left + 1 < team.right) {
      efficiency_threshold += (team.left + 1)->efficiency;
    }
    if (efficiency_threshold >= team.right->efficiency ||
        team.right - team.left < 2) {
      team.efficiency += team.right->efficiency;
      ++team.right;
      if (team.efficiency > best_team.efficiency) {
        best_team = team;
      }
    } else {
      team.efficiency -= team.left->efficiency;
      ++team.left;
    }
  }
  return {best_team.left, best_team.right};
}

int64_t CountSummaryEfficiency(const std::vector<Player> &team) {
  int64_t summary_efficiency = 0;
  for (const auto &player: team) {
    summary_efficiency += player.efficiency;
  }
  return summary_efficiency;
}

void WriteTeam(const std::vector<Player> &team, std::ostream &output_stream = std::cout) {
  auto summary_efficiency = CountSummaryEfficiency(team);
  output_stream << summary_efficiency << "\n";
  for (const auto &player: team) {
    output_stream << player.number << " ";
  }
  output_stream << "\n";
}

int main() {
  std::ios_base::sync_with_stdio(false);
  const auto team = ReadPlayers();
  auto most_effective_solidary_team = BuildMostEffectiveSolidaryTeam(team);
  QuickSort(most_effective_solidary_team.begin(), most_effective_solidary_team.end(),
            ComparePlayersNumber());
  WriteTeam(most_effective_solidary_team);
}
