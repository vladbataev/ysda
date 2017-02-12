// INTERFACE /////////////////////////////////////
#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

/*
 * Мы реализуем стандартный класс для хранения кучи с возможностью удаления
 * элемента по индексу. Для оповещения элементов об их текущих значениях
 * индексов мы используем функцию index_change_observer.
 */

template <class T, class Compare = std::less<T>>
class MaxHeap {
 public:
  using IndexChangeObserver =
  std::function<void(const T& element, size_t new_element_index)>;

  static constexpr size_t kNullIndex = static_cast<size_t>(-1);

  explicit MaxHeap(
      Compare compare = Compare(),
      IndexChangeObserver index_change_observer = IndexChangeObserver()):
      index_change_observer_(index_change_observer),
      compare_(compare)
  {}

  void Push(const T& value) {
    elements_.push_back(value);
    NotifyIndexChange(value, size() - 1);
    SiftUp(size() - 1);
  }

  void Erase(size_t index) {
    NotifyIndexChange(elements_[index], kNullIndex);
    if (size() > 1) {
      elements_[index] = elements_[size() - 1];
      NotifyIndexChange(elements_[index], index);
    }
    elements_.pop_back();
    SiftDown(index);
    SiftUp(index);
  }

  const T& top() const {
    return elements_[0];
  }

  void Pop() {
    Erase(0);
  }

  size_t size() const {
    return elements_.size();
  }

  bool empty() const {
    return elements_.empty();
  }

 private:
  IndexChangeObserver index_change_observer_;
  Compare compare_;
  std::vector<T> elements_;

  size_t parent(size_t index) const {
    if (index != 0) {
      return (index - 1) / 2;
    }
    return kNullIndex;
  }

  size_t left_son(size_t index) const {
    if (2 * index + 1 < size()) {
      return 2 * index + 1;
    }
    return kNullIndex;
  }

  size_t right_son(size_t index) const {
    if (2 * index + 2 < size()) {
      return 2 * index + 2;
    }
    return kNullIndex;
  }

  bool CompareElements(size_t first_index, size_t second_index) const {
    return compare_(elements_[second_index], elements_[first_index]);
  }

  void NotifyIndexChange(const T& element, size_t new_element_index) {
    if (!index_change_observer_) {
      return;
    }
    index_change_observer_(element, new_element_index);
  }

  void SwapElements(size_t first_index, size_t second_index) {
    std::swap(elements_[first_index], elements_[second_index]);
    NotifyIndexChange(elements_[first_index], first_index);
    NotifyIndexChange(elements_[second_index], second_index);
  }

  size_t SiftUp(size_t index) {
    auto parent_index = parent(index);
    while (parent_index != kNullIndex) {
      if (CompareElements(index, parent_index)) {
        SwapElements(index, parent_index);
      }
      index = parent_index;
      parent_index = parent(index);
    }
    return index;
  }

  void SiftDown(size_t index) {
    if (left_son(index) == kNullIndex && right_son(index) == kNullIndex) {
      return;
    }
    auto left_son_index = left_son(index);
    auto right_son_index = right_son(index);
    auto dominating_index = index;
    if (left_son_index != kNullIndex && CompareElements(left_son_index, dominating_index)) {
      dominating_index = left_son_index;
    }
    if (right_son_index != kNullIndex && CompareElements(right_son_index, dominating_index)) {
      dominating_index = right_son_index;
    }
    if (dominating_index == index) {
      return;
    }
    SwapElements(index, dominating_index);
    SiftDown(dominating_index);
  }
};

struct MemorySegment {
  size_t left;
  size_t right;
  size_t heap_index;

  MemorySegment(size_t left_, size_t right_);

  size_t size() const {
    return right - left;
  }

  MemorySegment Unite(const MemorySegment& other) const {
    return MemorySegment(std::min(left, other.left), std::max(right, other.right));
  }
};

using MemorySegmentIterator = std::list<MemorySegment>::iterator;
using MemorySegmentConstIterator = std::list<MemorySegment>::const_iterator;

struct MemorySegmentSizeCompare {
  bool operator()(MemorySegmentIterator first,
                  MemorySegmentIterator second) const {
    if (first->size() == second->size()) {
      return first->left >= second->left;
    }
    return first->size() < second->size();
  }
};

using MemorySegmentHeap =
MaxHeap<MemorySegmentIterator, MemorySegmentSizeCompare>;

struct MemorySegmentsHeapObserver {
  void operator()(const MemorySegmentIterator& segment, size_t new_index) const {
    segment->heap_index = new_index;
  }
};

MemorySegment::MemorySegment(size_t left_, size_t right_):
    left(left_),
    right(right_),
    heap_index(MemorySegmentHeap::kNullIndex)
{}

/*
 * Мы храним сегменты в виде двусвязного списка (std::list).
 * Быстрый доступ к самому левому из наидлиннейших свободных отрезков
 * осуществляется с помощью кучи, в которой (во избежание дублирования
 * отрезков в памяти) хранятся итераторы на список — std::list::iterator.
 * Чтобы быстро определять местоположение сегмента в куче для его изменения,
 * мы внутри сегмента в списке храним heap_index, актуальность которого
 * поддерживаем с помощью index_change_observer. Мы не храним отдельной метки
 * для маркировки занятых сегментов: вместо этого мы кладём в heap_index
 * специальный kNullIndex. Более того, мы скрываем истинный тип
 * MemorySegmentIterator за названием SegmentHandle. Таким образом,
 * пользовательский
 * код абсолютно не зависит того, что мы храним сегменты в списке и в куче,
 * что позволяет нам легко поменять реализацию класса.
*/

class MemoryManager {
 public:
  using SegmentHandle = MemorySegmentIterator;

  explicit MemoryManager(size_t memory_size):
      free_memory_segments_(MemorySegmentHeap(MemorySegmentSizeCompare(),
                                              MemorySegmentsHeapObserver())) {
    memory_segments_.push_back(MemorySegment(1, memory_size + 1));
    free_memory_segments_.Push(memory_segments_.begin());
  }

  SegmentHandle Allocate(size_t size) {
    if (free_memory_segments_.size() == 0) {
      return undefined_handle();
    }
    auto max_size_segment_iterator = free_memory_segments_.top();
    if (max_size_segment_iterator->size() < size) {
      return undefined_handle();
    }
    auto max_left = max_size_segment_iterator->left;
    auto allocated_segment = MemorySegment(max_left, max_left + size);
    auto allocated_iterator = memory_segments_.insert(max_size_segment_iterator, allocated_segment);
    free_memory_segments_.Pop();
    if (max_size_segment_iterator->size() != size) {
      max_size_segment_iterator->left += size;
      free_memory_segments_.Push(max_size_segment_iterator);
    } else {
      memory_segments_.erase(max_size_segment_iterator);
    }
    return allocated_iterator;
  }

  void Free(SegmentHandle segment_handle) {
    if (segment_handle != std::prev(memory_segments_.end())) {
      AppendIfFree(segment_handle, std::next(segment_handle));
    }
    if (segment_handle != memory_segments_.begin()) {
      AppendIfFree(segment_handle, std::prev(segment_handle));
    }
    free_memory_segments_.Push(segment_handle);
  }

  SegmentHandle undefined_handle() {
    return memory_segments_.end();
  }

 private:
  MemorySegmentHeap free_memory_segments_;
  std::list<MemorySegment> memory_segments_;

  void AppendIfFree(SegmentHandle remaining, SegmentHandle appending) {
    if (appending->heap_index != MemorySegmentHeap::kNullIndex) {
      free_memory_segments_.Erase(appending->heap_index);
      (*remaining) = remaining->Unite(*appending);
      memory_segments_.erase(appending);
    }
  }
};

size_t ReadMemorySize(std::istream& stream = std::cin) {
  size_t memory_size;
  stream >> memory_size;
  return memory_size;
}

struct AllocationQuery {
  size_t allocation_size;
};

struct FreeQuery {
  size_t allocation_query_index;
};

/*
 * Для хранения запросов используется специальный класс-обёртка
 * MemoryManagerQuery. Фишка данной реализации в том, что мы можем удобно
 * положить в него любой запрос, при этом у нас есть методы, которые позволят
 * гарантированно правильно проинтерпретировать его содержимое. При реализации
 * нужно воспользоваться тем фактом, что dynamic_cast возвращает nullptr
 * при неудачном приведении указателей.
*/

class MemoryManagerQuery {
 public:
  explicit MemoryManagerQuery(AllocationQuery allocation_query) :
      query_(new ConcreteQuery<AllocationQuery>(allocation_query)) {
  }

  explicit MemoryManagerQuery(FreeQuery free_query) :
      query_(new ConcreteQuery<FreeQuery>(free_query)) {
  }

  const AllocationQuery* AsAllocationQuery() const {
    auto ptr = dynamic_cast<ConcreteQuery<AllocationQuery>*>(query_.get());
    if (ptr) {
      return &(ptr)->body;
    } else {
      return nullptr;
    }
  }

  const FreeQuery* AsFreeQuery() const {
    auto ptr = dynamic_cast<ConcreteQuery<FreeQuery>*>(query_.get());
    return (ptr) ? &(ptr)->body : nullptr;
  }

 private:
  class AbstractQuery {
   public:
    virtual ~AbstractQuery() {}

   protected:
    AbstractQuery() {}
  };

  template <typename T>
  struct ConcreteQuery : public AbstractQuery {
    T body;

    explicit ConcreteQuery(T _body) : body(std::move(_body)) {}
  };

  std::unique_ptr<AbstractQuery> query_;
};


std::vector<MemoryManagerQuery> ReadMemoryManagerQueries(std::istream& stream = std::cin) {
  int queries_count;
  stream >> queries_count;
  int number;
  std::vector<MemoryManagerQuery> queries_vector;
  for (int i = 0; i < queries_count; ++i) {
    std::cin >> number;
    if (number > 0) {
      queries_vector.emplace_back(AllocationQuery({static_cast<size_t>(number)}));
    } else {
      queries_vector.emplace_back(FreeQuery({static_cast<size_t>(-number - 1)}));
    }
  }
  return queries_vector;
}

struct MemoryManagerAllocationResponse {
  bool success;
  size_t position;
};

MemoryManagerAllocationResponse MakeSuccessfulAllocation(size_t position) {
  return {true, position};
}

MemoryManagerAllocationResponse MakeFailedAllocation() {
  return {false, 0};
}

std::vector<MemoryManagerAllocationResponse> RunMemoryManager(
    size_t memory_size, const std::vector<MemoryManagerQuery>& queries) {
  MemoryManager memory_manager(memory_size);
  std::vector<MemoryManagerAllocationResponse> responses;
  std::vector<MemoryManager::SegmentHandle> segments_iterators;
  for (const auto &query: queries) {
    if (const auto* allocation_query = query.AsAllocationQuery()) {
      auto segment_iterator = memory_manager.Allocate(allocation_query->allocation_size);
      segments_iterators.push_back(segment_iterator);
      if (segment_iterator == memory_manager.undefined_handle()) {
        responses.emplace_back(MakeFailedAllocation());
      } else {
        responses.push_back(MakeSuccessfulAllocation((*segment_iterator).left));
      }
    } else if (const auto* free_query = query.AsFreeQuery()) {
      auto segment_iterator = segments_iterators[free_query->allocation_query_index];
      if (segment_iterator != memory_manager.undefined_handle()) {
        memory_manager.Free(segment_iterator);
      }
      segments_iterators.push_back(memory_manager.undefined_handle());
    } else {
      throw std::invalid_argument("Unknown type of query");
    }
  }
  return responses;
}

void OutputMemoryManagerResponses(
    const std::vector<MemoryManagerAllocationResponse>& responses,
    std::ostream& ostream = std::cout) {
  for (const auto &response: responses) {
    if (response.success) {
      ostream << response.position << "\n";
    } else {
      ostream << "-1" << "\n";
    }
  }
}

int main() {
  std::istream &input_stream = std::cin;
  std::ostream &output_stream = std::cout;

  const size_t memory_size = ReadMemorySize(input_stream);
  const std::vector<MemoryManagerQuery> queries =
      ReadMemoryManagerQueries(input_stream);
  const std::vector<MemoryManagerAllocationResponse> responses =
      RunMemoryManager(memory_size, queries);

  OutputMemoryManagerResponses(responses, output_stream);
  return 0;
}
