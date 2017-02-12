#include <iostream>
#include <vector>
#include <iomanip>

constexpr size_t MAX_N_ELEMENTS = 101;

std::vector<size_t> read_input() {
  size_t n_elements, height;
  std::cin >> n_elements >> height;
  return std::vector<size_t>({n_elements, height});
}

long double get_answer(size_t n_elements, size_t height) {
  if (height >= n_elements) {
    return 0.0;
  }
  std::vector<std::vector<long double>> prob(MAX_N_ELEMENTS,
                                             std::vector<long double>(MAX_N_ELEMENTS, 0.0));
  for (size_t i = 0 ; i < n_elements + 1; ++i) {
    if (i != 0) {
      prob[i][0] = 0.0;
    } else {
      prob[i][0] = 1.0;
    }
  }
  prob[1][0] = 1.0;
  prob[1][1] = 0.0;
  for (size_t num = 2; num <= n_elements; ++num) {
    for (size_t h = 1; h <= height; ++h) {
      long double temp_prob = 0.0;
      for (size_t k = 0; k <= num - 1; ++k) {
        for (size_t m = 0; m < h - 1; ++m) {
          temp_prob += 2.0 * prob[k][h - 1] * prob[num - k - 1][m];
        }
      }
      for (size_t k = 0; k <= num - 1; ++k) {
        temp_prob += prob[k][h - 1] * prob[num - k - 1][h - 1];
      }
      prob[num][h] = temp_prob / num;
    }
  }
  return prob[n_elements][height];
}

int main() {
  auto input = read_input();
  auto answer = get_answer(input[0], input[1]);
  printf("%.10Lf\n", answer);
}
