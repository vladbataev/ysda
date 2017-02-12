#include <iostream>
#include <vector>
#include <algorithm>

constexpr int TABLESIZE = 5000000;
constexpr long long PRIME = 1000001449;

struct Triangle {
  int a_segment;
  int b_segment;
  int c_segment;
  Triangle(int a_segment_, int b_segment_, int c_segment_):
      a_segment(a_segment_), b_segment(b_segment_), c_segment(c_segment_)
  {}
  Triangle() {}
  friend bool operator==(const Triangle &first, const Triangle &second) {
    return first.a_segment == second.a_segment && first.b_segment == second.b_segment
        && first.c_segment == second.c_segment;
  }
};

int gcd(int first_number, int second_number) {
  while (second_number) {
    first_number %= second_number;
    std::swap(first_number, second_number);
  }
  return first_number;
}

int hash(const Triangle& triangle) {
  auto temp_calc = (triangle.a_segment * PRIME) % TABLESIZE;
  temp_calc *= PRIME;
  temp_calc %= TABLESIZE;
  temp_calc += (triangle.b_segment * PRIME);
  temp_calc %= TABLESIZE;
  temp_calc += triangle.c_segment;
  return temp_calc % TABLESIZE;
}

Triangle NormalizeTriangle(const Triangle& triangle) {
  std::vector<int> segments({triangle.a_segment, triangle.b_segment, triangle.c_segment});
  std::sort(segments.begin(), segments.end());
  int factor = gcd(triangle.a_segment, gcd(triangle.b_segment, triangle.c_segment));
  Triangle normalized = Triangle(segments[0] / factor, segments[1] / factor, segments[2] / factor);
  return normalized;
}

std::vector<Triangle> ReadInput(int size) {
  std::vector<Triangle> triangles;
  for (int i = 0; i < size; ++i) {
    int a_segment, b_segment, c_segment;
    std::cin >> a_segment >> b_segment >> c_segment;
    triangles.emplace_back(Triangle({a_segment, b_segment, c_segment}));
  }
  return triangles;
}

int GetSimilarityClasses(const std::vector<Triangle>& triangles) {
  std::vector<std::vector<Triangle>> table;
  table.reserve(TABLESIZE);
  for (auto &triangle: triangles) {
    auto normalized = NormalizeTriangle(triangle);
    auto hash_value = hash(normalized);
    if (table[hash_value].empty()) {
      table[hash_value].push_back(normalized);
    } else {
      bool found = false;
      for (auto &other: table[hash_value]) {
        if (other == normalized) {
          found = true;
          break;
        }
      }
      if (!found) {
        table[hash_value].push_back(normalized);
      }
    }
  }
  int number_of_classes = 0;
  for (int i = 0; i < TABLESIZE; ++i) {
    number_of_classes += table[i].size();
  }
  return number_of_classes;
}

int main() {
  int size;
  std::cin >> size;
  auto triangles = ReadInput(size);
  std::cout << GetSimilarityClasses(triangles) << '\n';
  return 0;
}
