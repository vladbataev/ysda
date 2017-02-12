#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

void read_input(std::vector<std::vector<int>>& triples, std::istream& in_stream = std::cin) {
  int size;
  in_stream >> size;
  for (int i = 0; i < size; ++i) {
    std::vector<int> triple(3);
    for (int j = 0; j < 3; ++j) {
      in_stream >> triple[j];
    }
    triples.push_back(triple);
  }
}

void print_triples(const std::vector<std::vector<int>>& triples) {
  for (auto &i: triples) {
    for (auto &j :i) {
      std::cout << j << " ";
    }
    std::cout << std::endl;
  }
}

bool compare_triples(const std::vector<int>& first, const std::vector<int>& second) {
  for (int i = 0; i < first.size(); ++i) {
    if (first[i] >= second[i])
      return false;
  }
  return true;
}

int get_max_chain(const std::vector<std::vector<int>>& triples) {
  auto size = triples.size();
  std::vector<int> dynamics(size);
  for (int i = size - 1; i >= 0; --i) {
    dynamics[i] = 1;
    for (int j = size - 1; j > i; --j) {
      if (compare_triples(triples[i], triples[j])) {
        dynamics[i] = std::max(dynamics[i], dynamics[j] + 1);
      }
    }
  }
  return *std::max_element(dynamics.begin(), dynamics.end());
}

int main() {
  std::vector<std::vector<int>> triples;
  read_input(triples);
  std::cout << get_max_chain(triples) << std::endl;
  return 0;
}
