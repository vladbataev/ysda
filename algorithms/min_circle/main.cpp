#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

constexpr double MAX_RADIUS = 2000;
constexpr double PRECISION = 0.001;
constexpr double MAX_BORDER = 4000;

struct Point {
  int x_coordinate;
  int y_coordinate;
  Point(int x_coordinate_, int y_coordinate_)
      : x_coordinate(x_coordinate_),
        y_coordinate(y_coordinate_)
  {}
  Point() {}
};

struct Segment {
  double begin;
  double end;
  Segment(double begin_, double end_): begin(begin_), end(end_) {}
  Segment() {}
};

struct BorderPoint {
  double coordinate = 0;
  std::string type;
  BorderPoint() {}
  BorderPoint(double coordinate_, std::string type_): coordinate(coordinate_), type(type_) {}
};

class BorderPointComparator {
 public:
  bool operator()(const BorderPoint& first, const BorderPoint& second) {
    if (first.coordinate == second.coordinate && first.type == second.type) {
      return false;
    }
    return first.coordinate <= second.coordinate;
  }
};

std::vector<Point> read_points(int number_of_points, std::istream& input_stream = std::cin) {
  std::vector<Point> points;
  for (int i = 0 ; i < number_of_points; ++i) {
    int x_coordinate, y_coordinate;
    input_stream >> x_coordinate;
    input_stream >> y_coordinate;
    points.emplace_back(Point(x_coordinate, y_coordinate));
  }
  return points;
}

Segment intersect_circle_with_axes(Point center, double radius) {
  double temp = radius * radius - center.y_coordinate * center.y_coordinate;
  if (temp < 0) {
    return Segment(MAX_BORDER, MAX_BORDER);
  }
  temp = std::pow(temp, 0.5);
  return Segment(center.x_coordinate - temp, center.x_coordinate + temp);
}

double find_minimal_covering_radius(const std::vector<Point>& points,
                                    int number_of_covered_points) {
  double insufficient_radius = 0;
  double approved_radius = MAX_RADIUS;
  while (approved_radius - insufficient_radius >= PRECISION) {
    auto current_radius = (approved_radius - insufficient_radius) / 2 + insufficient_radius;
    std::vector<BorderPoint> border_points;
    for (auto &point: points) {
      auto segment = intersect_circle_with_axes(point, current_radius);
      if (segment.begin != MAX_BORDER) {
        border_points.emplace_back(BorderPoint(segment.begin, "left"));
      }
      if (segment.end != MAX_BORDER) {
        border_points.emplace_back(BorderPoint(segment.end, "right"));
      }
    }
    std::sort(border_points.begin(), border_points.end(), BorderPointComparator());
    int intersection_counter = 0;
    bool find_success = false;
    for (auto &border_point: border_points) {
      if (border_point.type == "left") {
        intersection_counter += 1;
      }
      if (border_point.type == "right") {
        intersection_counter -= 1;
      }
      if (intersection_counter >= number_of_covered_points) {
        approved_radius = current_radius;
        find_success = true;
        break;
      }
    }
    if (!find_success) {
      insufficient_radius = current_radius;
    }
  }
  return approved_radius;
}

int main() {
  std::ios_base::sync_with_stdio(false);

  int number_of_points;
  int number_of_covered_points;
  std::cin >> number_of_points;
  std::cin >> number_of_covered_points;
  auto points = read_points(number_of_points);

  auto minimal_covering_radius = find_minimal_covering_radius(points, number_of_covered_points);

  std::cout.precision(7);
  std::cout << minimal_covering_radius << std::endl;
  return 0;
}
