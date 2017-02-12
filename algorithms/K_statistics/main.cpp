#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>
#include <chrono>


class steady_timer {
 public:
  steady_timer() {
    reset();
  }

  void reset() {
    start_ = std::chrono::steady_clock::now();
  }

  double seconds_elapsed() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast< std::chrono::duration<double> >(now - start_);
    return elapsed.count();
  }

 private:
  std::chrono::steady_clock::time_point start_;
};


template <class T, class Compare = std::less<T>>
class Heap {
 public:
  using IndexChangeObserver = std::function<void(const T& element, int new_element_index)>;

  static constexpr int invalid_index = -1;

  Heap(Compare compare = Compare(),
       IndexChangeObserver index_change_observer = IndexChangeObserver()) {
    compare_ = compare;
    index_change_observer_ = index_change_observer;
  }

  int size() const {
    return elements_.size();
  }

  bool empty() const {
    return elements_.empty();
  }

  int push(const T& value) {
    elements_.push_back(value);
    NotifyIndexChange(value, this->size() - 1);
    return SiftUp(this->size() - 1);
  }

  void erase(int index) {
    elements_[index] = elements_[0];
    NotifyIndexChange(elements_[index], index);
    SiftUp(index);
    pop();
  }

  const T& top() const {
    return elements_[0];
  }

  void pop() {
    NotifyIndexChange(elements_[0], invalid_index);
    if (this->size() > 1) {
      elements_[0] = elements_[this->size() - 1];
      NotifyIndexChange(elements_[0], 0);
    }
    elements_.pop_back();
    SiftDown(0);
  }

 private:
  IndexChangeObserver index_change_observer_;
  Compare compare_;
  std::vector<T> elements_;

  int Parent(int index) const {
    if (index != 0) {
      return (index - 1) / 2;
    }
    return invalid_index;
  }

  int LeftSon(int index) const {
    if (2 * index + 1 < this->size()) {
      return 2 * index + 1;
    }
    return invalid_index;
  }

  int RightSon(int index) const {
    if (2 * index + 2 < size()) {
      return 2 * index + 2;
    }
    return invalid_index;
  }

  bool CompareElements(int first_index, int second_index) const {
    return compare_(elements_[first_index], elements_[second_index]);
  }

  void NotifyIndexChange(const T& element, int new_element_index) {
    index_change_observer_(element, new_element_index);
  }

  void SwapElements(int first_index, int second_index) {
    NotifyIndexChange(elements_[first_index], second_index);
    NotifyIndexChange(elements_[second_index], first_index);
    std::swap(elements_[first_index], elements_[second_index]);
  }

  int SiftUp(int index) {
    bool flag = true;
    while (flag) {
      int parent = Parent(index);
      if (parent == invalid_index) {
        break;
      }
      if (CompareElements(index, parent)) {
        SwapElements(index, parent);
      }
      index = parent;
    }
    return index;
  }

  void SiftDown(int index) {
    bool flag = true;
    while (flag) {
      int left_index = LeftSon(index);
      int right_index = RightSon(index);
      int swap_index;
      if (left_index == invalid_index && right_index == invalid_index) {
        break;
      }
      if (left_index == invalid_index && right_index != invalid_index) {
        swap_index = right_index;
      }
      if (left_index != invalid_index && right_index == invalid_index) {
        swap_index = left_index;
      }
      if (left_index != invalid_index && right_index != invalid_index) {
        if (CompareElements(left_index, right_index)) {
          swap_index = left_index;
        } else {
          swap_index = right_index;
        }
      }
      if (!CompareElements(index, swap_index)) {
        SwapElements(index, swap_index);
      }
      index = swap_index;
    }
  }
};

struct HeapElement {
  int value;
  bool min_heap;
  bool max_heap;
  int heap_index;
};

using Iterator = std::vector<HeapElement>::iterator;

struct MinHeapCompare {
  bool operator() (Iterator first, Iterator second) const {
    if ((*first).value == (*second).value) {
      return first <= second;
    }
    return (*first).value < (*second).value;
  }
};

struct MaxHeapCompare {
  bool operator() (Iterator first, Iterator second) const {
    if ((*first).value == (*second).value)
      return first <= second;
    return (*first).value > (*second).value;
  }
};

struct ValueIndexObserver {
  void operator() (Iterator element_iterator, int new_index) const {
    (*element_iterator).heap_index = new_index;
  }
};

std::vector<int> GetKthStatistics(const std::vector<int> &input_vector,
                                  const std::string &commands, int k_order) {
  std::vector<int> k_order_statistics;
  std::vector<HeapElement> elements;
  auto min_heap =  Heap<Iterator, MinHeapCompare>(MinHeapCompare(), ValueIndexObserver());
  auto max_heap =  Heap<Iterator, MaxHeapCompare>(MaxHeapCompare(), ValueIndexObserver());
  for (int i = 0; i < input_vector.size(); ++i)
    elements.emplace_back(HeapElement({input_vector[i], false, false,
                                       Heap<Iterator>::invalid_index}));
  Iterator left = elements.begin();
  Iterator right = elements.begin();

  (*right).max_heap = true;
  max_heap.push(right);
  for (int command_number = 0; command_number < commands.length(); ++command_number) {
    if (commands[command_number] == 'R') {
      right++;
      if (max_heap.size() < k_order) {
        (*right).max_heap = true;
        max_heap.push(right);
      } else if ((*right).value < (*max_heap.top()).value) {
        auto value = max_heap.top();
        max_heap.pop();
        (*value).max_heap = false;
        (*value).min_heap = true;
        min_heap.push(value);
        (*right).max_heap = true;
        max_heap.push(right);
      } else {
        (*right).max_heap = false;
        (*right).min_heap = true;
        min_heap.push(right);
      }
    } else {
      if ((*left).min_heap) {
        min_heap.erase((*left).heap_index);
      } else {
        max_heap.erase((*left).heap_index);
        if (min_heap.size() > 0) {
          auto value = min_heap.top();
          min_heap.pop();
          (*value).max_heap = true;
          (*value).min_heap = false;
          max_heap.push(value);
        }
      }
      left++;
    }

    if (max_heap.size() >= k_order) {
      k_order_statistics.push_back((*max_heap.top()).value);
    } else {
      k_order_statistics.push_back(-1);
    }
  }
  return k_order_statistics;
}

std::vector<int> read_elements(int size) {
  std::vector<int> elements;
  for (int i = 0; i < size; ++i) {
    int cur_element;
    std::cin >> cur_element;
    elements.push_back(cur_element);
  }
  return elements;
}

std::string read_commands() {
  std::string commands;
  std::cin >> commands;
  return commands;
}

void print_answer(const std::vector<int> &answer) {
  for (auto &element: answer) {
    std::cout << element << "\n";
  }
}

int main() {
  int number_of_elements, number_of_commands, k_order;
  std::cin >> number_of_elements >> number_of_commands >> k_order;
  auto elements = read_elements(number_of_elements);
  auto commands = read_commands();
  auto answer = GetKthStatistics(elements, commands, k_order);
  print_answer(answer);
  return 0;
}
