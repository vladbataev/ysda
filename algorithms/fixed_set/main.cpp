#include <iostream>
#include <vector>
#include <random>
#include <functional>
#include <chrono>

constexpr int64_t PRIME_NUMBER = 2147483323;
constexpr int64_t NAN_VALUE = 1000000001;

struct Bucket {
  int first_hash_parameter;
  int second_hash_parameter;
  std::vector<int> hashed_keys;
  Bucket()
  {}
  Bucket(int first_hash_parameter_, int second_hash_parameter_,
         const std::vector<int>& hashed_keys_):
      first_hash_parameter(first_hash_parameter_), second_hash_parameter(second_hash_parameter_),
      hashed_keys(hashed_keys_)
  {}
};



template <class T, class HashFunction>
class FixedSet {
 private:
  std::vector<Bucket> hash_table;
  std::vector<bool> front_buckets;
  int front_first_hash_function_parameter;
  int front_second_hash_function_parameter;
  int GetFrontFirstHashParameter() const {
    return front_first_hash_function_parameter;
  }
  int GetFrontSecondHashParameter() const {
    return front_second_hash_function_parameter;
  }
  const std::vector<bool>& GetFrontBuckets() const {
    return front_buckets;
  }
  const std::vector<Bucket>& GetHashTable() const {
    return hash_table;
  }

 public:
  FixedSet() {}
  void Initialize(const std::vector<int>& numbers);
  bool Contains(int number) const;
};


int hash_function(int number, int number_of_buckets, int first_parameter, int second_parameter,
                  int prime_number = PRIME_NUMBER) {
  int64_t temp_value = static_cast<int64_t>(number) % (prime_number) *
      static_cast<int64_t>(first_parameter);
  temp_value += second_parameter;
  temp_value = (temp_value % prime_number) % number_of_buckets;
  if (temp_value < 0) {
    return temp_value + number_of_buckets;
  } else {
    return temp_value;
  }
}

void FixedSet::Initialize(const std::vector<int> &numbers) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<int> gen_uniform_unit_for_first_parameter(1, PRIME_NUMBER - 1);
  std::uniform_int_distribution<int> gen_uniform_unit_for_second_parameter(0, PRIME_NUMBER - 1);
  bool memory_size = false;
  int first_parameter;
  int second_parameter;
  std::vector<std::vector<int>> front_buckets(numbers.size());

  while (memory_size == false) {
    first_parameter = gen_uniform_unit_for_first_parameter(generator);
    second_parameter = gen_uniform_unit_for_second_parameter(generator);
    for (auto &number: numbers) {
      front_buckets[hash_function(number, numbers.size(), first_parameter,
                                  second_parameter)].push_back(number);
    }
    int64_t sum_of_squares = 0;
    for (auto &bucket: front_buckets) {
      sum_of_squares += bucket.size() * bucket.size();
    }
    if (sum_of_squares <= 4 * numbers.size()) {
      memory_size = true;
    } else {
      for (int i = 0 ; i < front_buckets.size(); ++i) {
        front_buckets[i].clear();
      }
    }
  }
  front_first_hash_function_parameter = first_parameter;
  front_second_hash_function_parameter = second_parameter;
  this->front_buckets.assign(numbers.size(), false);
  hash_table.assign(numbers.size(), Bucket());

  for (int i = 0 ; i < numbers.size(); ++i) {
    if (front_buckets[i].size() > 0) {
      this->front_buckets[i] = true;
    }
  }

  for (int number_of_bucket = 0; number_of_bucket < front_buckets.size(); ++number_of_bucket) {
    if (front_buckets[number_of_bucket].size() > 0) {
      bool unique_hashes = false;
      auto bucket = front_buckets[number_of_bucket];
      while (unique_hashes == false) {
        first_parameter = gen_uniform_unit_for_first_parameter(generator);
        second_parameter = gen_uniform_unit_for_second_parameter(generator);
        std::vector<int> hashed_bucket_keys(bucket.size() * bucket.size(), NAN_VALUE);
        bool success = true;
        for (int i = 0; i < bucket.size(); ++i) {
          auto hash_value = hash_function(bucket[i], bucket.size() * bucket.size(),
                                          first_parameter, second_parameter);
          if (hashed_bucket_keys[hash_value] != NAN_VALUE) {
            success = false;
            break;
          } else {
            hashed_bucket_keys[hash_value] = bucket[i];
          }
        }
        if (success) {
          unique_hashes = true;
          this->hash_table[number_of_bucket] = Bucket(first_parameter, second_parameter,
                                                      hashed_bucket_keys);
        }
      }
    }
  }
}

bool FixedSet::Contains(int number) const {
  auto front_buckets = this->GetFrontBuckets();
  auto front_hash_value = hash_function(number, front_buckets.size(),
                                        this->GetFrontFirstHashParameter(),
                                        this->GetFrontSecondHashParameter());
  if (!front_buckets[front_hash_value]) {
    return false;
  }
  auto front_bucket = this->GetHashTable()[front_hash_value];
  auto bucket_hash_value = hash_function(number, front_bucket.hashed_keys.size(),
                                         front_bucket.first_hash_parameter,
                                         front_bucket.second_hash_parameter);
  if (front_bucket.hashed_keys[bucket_hash_value] == number) {
    return true;
  }
  return false;
}


std::vector<int> ReadNumbers() {
  int size_of_numbers;
  std::cin >> size_of_numbers;
  std::vector<int> keys(size_of_numbers);
  for (int i = 0 ; i < size_of_numbers; ++i) {
    std::cin >> keys[i];
  }
  return keys;
}

std::vector<int> ReadQueries(int number_of_queries) {
  std::vector<int> queries(number_of_queries);
  for (auto &query: queries) {
    std::cin >> query;
  }
  return queries;
}

std::vector<bool> GetQueriesAnswers(const std::vector<int>& queries, const FixedSet& set) {
  std::vector<bool> answers;
  answers.reserve(queries.size());
  for (const auto &number: queries) {
    answers.push_back(set.Contains(number));
  }
  return answers;
}

void WriteAnswers(const std::vector<bool>& answers) {
  for (auto answer: answers) {
    answer ? std::cout << "Yes\n" : std::cout << "No\n";
  }
}

int main() {
  std::ios_base::sync_with_stdio(false);
  auto numbers = ReadNumbers();
  int number_of_queries;
  std::cin >> number_of_queries;
  auto queries = ReadQueries(number_of_queries);
  FixedSet set;
  set.Initialize(numbers);
  auto answers = GetQueriesAnswers(queries, set);
  WriteAnswers(answers);
}
