#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

constexpr int MAX_VALUE = 200000;

std::pair<int, std::vector<int>> read_input() {
  int parking_slots;
  std::cin >> parking_slots;
  int number_of_events;
  std::cin >> number_of_events;
  std::vector<int> events;
  for (int i = 0; i < number_of_events; ++i) {
    char type_event;
    int event;
    std::cin >> type_event >> event;
    if (type_event == '+') {
      events.emplace_back(event);
    } else {
      events.emplace_back(-event);
    }
  }
  return std::make_pair(parking_slots, events);
}

void build_segment_tree(std::vector<int>& segment_tree, const std::vector<int>& massive,
                        int vertex_index, int left, int right) {
  if (left == right) {
    segment_tree[vertex_index] = massive[left];
  } else {
    int middle = left + (right - left) / 2;
    build_segment_tree(segment_tree, massive, 2 * vertex_index, left, middle);
    build_segment_tree(segment_tree, massive, 2 * vertex_index + 1, middle + 1, right);
    segment_tree[vertex_index] = std::min(segment_tree[2 * vertex_index],
                                          segment_tree[2 * vertex_index + 1]);
  }
}

int get_min(const std::vector<int>& segment_tree, int vertex_index,
            int tree_left, int tree_right, int left, int right) {
  if (left > right) {
    return MAX_VALUE;
  }
  if (tree_left == left && tree_right == right) {
    return segment_tree[vertex_index];
  }
  int tree_middle = tree_left + (tree_right - tree_left) / 2;
  return std::min(get_min(segment_tree, 2 * vertex_index, tree_left, tree_middle, left,
                          std::min(right, tree_middle)),
      get_min(segment_tree, 2 * vertex_index + 1, tree_middle + 1, tree_right,
              std::max(left, tree_middle + 1), right));
}

void update(std::vector<int>& segment_tree, int vertex_index, int tree_left, int tree_right,
            int position, int new_value) {
  if (tree_left == tree_right) {
    segment_tree[vertex_index] = new_value;
  } else {
    int tree_middle = tree_left + (tree_right - tree_left) / 2;
    if (position <= tree_middle) {
      update(segment_tree, 2 * vertex_index, tree_left, tree_middle, position, new_value);
    } else {
      update(segment_tree, 2 * vertex_index + 1, tree_middle + 1, tree_right, position, new_value);
    }
    segment_tree[vertex_index] = std::min(segment_tree[2 * vertex_index],
                                          segment_tree[2 * vertex_index + 1]);
  }
}

std::vector<int> get_answers(std::pair<int, std::vector<int>> input) {
  auto number_of_parking_slots = input.first;
  auto events = input.second;
  std::vector<int> parking_slots;
  for (int i = 0; i < number_of_parking_slots; ++i) {
    parking_slots.emplace_back(i + 1);
  }
  std::vector<int> answers;
  std::vector<int> tree(4 * number_of_parking_slots);
  build_segment_tree(tree, parking_slots, 1, 0, number_of_parking_slots - 1);
  for (auto &event: events) {
    if (event > 0) {
      auto min_element = get_min(tree, 1, 0, number_of_parking_slots - 1,
                                 event - 1, number_of_parking_slots - 1);
      if (min_element == MAX_VALUE) {
        auto min_element = get_min(tree, 1, 0, number_of_parking_slots - 1, 0, event - 2);
        if (min_element == MAX_VALUE) {
          answers.emplace_back(-1);
        } else {
          answers.emplace_back(min_element);
          update(tree, 1, 0, number_of_parking_slots - 1, min_element - 1, MAX_VALUE);
        }
      } else {
        answers.emplace_back(min_element);
        update(tree, 1, 0, number_of_parking_slots - 1, min_element - 1, MAX_VALUE);
      }
    } else {
      if (get_min(tree, 1, 0, number_of_parking_slots - 1,
                  -event - 1, number_of_parking_slots - 1) != -event) {
        answers.emplace_back(0);
        update(tree, 1, 0, number_of_parking_slots - 1, -event - 1, -event);
      } else {
        answers.emplace_back(-2);
      }
    }
  }
  return answers;
}

void write_answers(const std::vector<int>& answers) {
  for (auto &answer: answers) {
    std::cout << answer << "\n";
  }
}

int main() {
  auto input = read_input();
  auto answers = get_answers(input);
  write_answers(answers);
  return 0;
}
