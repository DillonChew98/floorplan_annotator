#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <algorithm>
#include <cmath>
#include <queue>
#include <climits>
#include <iostream>

#include "floorplan_annotator/feature_detection.hpp"

namespace floorplan_annotator
{
FeatureDetection::FeatureDetection(const std::string & image_path)
{
  image_ = cv::imread(image_path);
  FeatureExtraction();
}

void FeatureDetection::FeatureExtraction()
{
  ExtractRooms();
  ExtractDoors();
  ExtractWalls();
  TagDoortoRoom();
  // PrintDoortoRoom();
}

FeatureDetection::~FeatureDetection()
{

}

void FeatureDetection::ExtractRooms()
{
  cv::Mat image_hsv;
  cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

  cv::Scalar lower_blue = cv::Scalar(90, 50, 150);    // kitchen/living/dining room
  cv::Scalar upper_blue = cv::Scalar(90, 100, 255);   // kitchen/living/dining room
  cv::Scalar lower_purple = cv::Scalar(120, 35, 120);  // rooms
  cv::Scalar upper_purple = cv::Scalar(120, 45, 255);  // rooms
  cv::Scalar lower_yellow = cv::Scalar(22, 120, 120);  // rooms
  cv::Scalar upper_yellow = cv::Scalar(24, 150, 255);  // rooms
  cv::Scalar lower_green = cv::Scalar(44, 60, 255);  // rooms
  cv::Scalar upper_green = cv::Scalar(45, 160, 255);  // rooms
  cv::Scalar lower_orange = cv::Scalar(10, 155, 255);  // rooms
  cv::Scalar upper_orange = cv::Scalar(40, 160, 255);  // rooms

  cv::Mat orange_mask, green_mask, purple_mask, blue_mask, yellow_mask;
  cv::Mat room_mask;
  cv::inRange(image_hsv, lower_orange, upper_orange, orange_mask);
  cv::inRange(image_hsv, lower_green, upper_green, green_mask);
  cv::inRange(image_hsv, lower_blue, upper_blue, blue_mask);
  cv::inRange(image_hsv, lower_purple, upper_purple, purple_mask);
  cv::inRange(image_hsv, lower_yellow, upper_yellow, yellow_mask);
  cv::bitwise_or(yellow_mask, orange_mask, room_mask);
  cv::bitwise_or(green_mask, room_mask, room_mask);
  cv::bitwise_or(blue_mask, room_mask, room_mask);
  cv::bitwise_or(purple_mask, room_mask, room_mask);

  cv::Mat labels, stats, centroids;
  int num_components = cv::connectedComponentsWithStats(room_mask, labels, stats, centroids);
  std::vector<Feature> rooms;
  rooms.reserve(num_components);
  for (int i = 1; i < num_components; i++) { // start at 1 as 0 will always contain the component of the entire canvas
    Feature room;
    room.x_origin = stats.at<int>(i, cv::CC_STAT_LEFT);
    room.y_origin = stats.at<int>(i, cv::CC_STAT_TOP);
    room.width = stats.at<int>(i, cv::CC_STAT_WIDTH);
    room.height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
    // int area = stats.at<int>(i, cv::CC_STAT_AREA);
    room.cx = centroids.at<double>(i, 0);
    room.cy = centroids.at<double>(i, 1);
    room.index = i;
    for (int y = room.y_origin; y < room.y_origin + room.height; y++) {
      for (int x = room.x_origin; x < room.x_origin + room.width; x++) {
        if (labels.at<int>(y, x) == i) {
          auto index = y * room.x_origin + x;
          room.pixels[std::pair<int, int>{x, y}] = index;
        }
      }
    }
    rooms.push_back(room);
  }
  features_.insert({FeatureType::ROOM, rooms});
}

void FeatureDetection::ExtractDoors()
{
  cv::Mat image_hsv;
  cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

  cv::Scalar lower_red = cv::Scalar(169, 170, 120);    // doors
  cv::Scalar upper_red = cv::Scalar(170, 200, 255);    // doors
  cv::Mat red_mask;
  cv::inRange(image_hsv, lower_red, upper_red, red_mask);

  cv::Mat labels, stats, centroids;
  int num_components = cv::connectedComponentsWithStats(red_mask, labels, stats, centroids);
  std::vector<Feature> doors;
  doors.reserve(num_components);
  for (int i = 1; i < num_components; i++) { // start at 1 as 0 will always contain the component of the entire canvas
    int area = stats.at<int>(i, cv::CC_STAT_AREA);
    if (area < 20) {
      continue;
    }
    Feature door;
    door.x_origin = stats.at<int>(i, cv::CC_STAT_LEFT);
    door.y_origin = stats.at<int>(i, cv::CC_STAT_TOP);
    door.width = stats.at<int>(i, cv::CC_STAT_WIDTH);
    door.height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
    door.cx = centroids.at<double>(i, 0);
    door.cy = centroids.at<double>(i, 1);
    door.index = i;
    for (int y = door.y_origin; y < door.y_origin + door.height; y++) {
      for (int x = door.x_origin; x < door.x_origin + door.width; x++) {
        if (labels.at<int>(y, x) == i) {
          auto index = y * door.x_origin + x;
          door.pixels[std::pair<int, int>{x, y}] = index;
        }
      }
    }
    doors.push_back(door);
  }
  features_.insert({FeatureType::DOOR, doors});
}

void FeatureDetection::ExtractWalls()
{
  cv::Mat image_hsv;
  cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

  cv::Mat white_mask;
  cv::Scalar white = cv::Scalar(0, 0, 255);    // walls
  cv::inRange(image_hsv, white, white, white_mask);

  cv::Mat labels, stats, centroids;
  int num_components = cv::connectedComponentsWithStats(white_mask, labels, stats, centroids);
  std::vector<Feature> walls;
  walls.reserve(num_components);
  for (int i = 1; i < num_components; i++) { // start at 1 as 0 will always contain the component of the entire canvas
    int area = stats.at<int>(i, cv::CC_STAT_AREA);
    if (area < 20) {
      continue;
    }
    Feature wall;
    wall.x_origin = stats.at<int>(i, cv::CC_STAT_LEFT);
    wall.y_origin = stats.at<int>(i, cv::CC_STAT_TOP);
    wall.width = stats.at<int>(i, cv::CC_STAT_WIDTH);
    wall.height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
    wall.cx = centroids.at<double>(i, 0);
    wall.cy = centroids.at<double>(i, 1);
    wall.index = i;
    for (int y = wall.y_origin; y < wall.y_origin + wall.height; y++) {
      for (int x = wall.x_origin; x < wall.x_origin + wall.width; x++) {
        if (labels.at<int>(y, x) == i) {
          auto index = y * wall.x_origin + x;
          wall.pixels[std::pair<int, int>{x, y}] = index;
        }
      }
    }
    walls.push_back(wall);
  }
  features_.insert({FeatureType::WALL, walls});
}


QImage FeatureDetection::convert2QImage()
{
  cv::Mat image_bgr;
  cv::cvtColor(image_, image_bgr, cv::COLOR_RGB2BGR); // used for converting to QImage, may not need
  return QImage(
    (uchar *) image_bgr.data, image_bgr.cols, image_bgr.rows, image_bgr.step,
    QImage::Format_RGB888);
}

std::vector<Feature> FeatureDetection::GetWalls() const
{
  return features_.at(FeatureType::WALL);
}

std::vector<std::pair<int, int>> FeatureDetection::GetVertices(int x_dist, int y_dist)
{
  std::vector<std::pair<int, int>> vertices;
  for (const auto & room : features_.at(FeatureType::ROOM)) {
    std::map<std::pair<int,int>,int> v;
    // padding the corners of the detected room, casting to int as we don't need the precision for rooms
    int x_o = room.x_origin + x_dist;
    int y_o = room.y_origin + y_dist;
    int x_max = room.x_origin + room.width;
    int y_max = room.y_origin + room.height;

    // compute number of rows and columns based on height/width over pixel_dist
    auto h = y_max - y_o;
    auto w = x_max - x_o;
    int rows = w / x_dist;
    int cols = h / y_dist;
    for (int y = 0; y < cols; ++y) {
      for (int x = 0; x < rows; ++x) {
        int v_x = x_o + x * x_dist;
        int v_y = y_o + y * y_dist;
        // check if generated vertex is a valid part of the room
        if (room.pixels.find(std::pair<int, int>{v_x, v_y}) != room.pixels.end()) {
          Node node;
          node.x = v_x;
          node.y = v_y;
          node.index = -1;
          node_map_.insert({{v_x, v_y}, node});
          vertices.push_back({v_x, v_y});
          v.insert({{v_x,v_y}, -1});          
        }
      }
    }
    room_vertices_.push_back(v); // we store the point in nother container for coordinate to index mapping
  }
  // we generate vertices for doors too
  for (const auto & door : features_.at(FeatureType::DOOR)) {
    Node node;
    node.x = door.cx;
    node.y = door.cy;
    node.index = -1;
    vertices.push_back({door.cx, door.cy});
    auto rooms = doors_to_room_map_.at(door.index);
    for (const auto & room : rooms)
    {
      auto point = CalculateClosestPoint(door.cx, door.cy, room);
      node.edges.insert({{point.x, point.y}, point.distance});
      node_map_.at({point.x, point.y}).edges.insert({{node.x, node.y}, point.distance});
    }
    node_map_.insert({{door.cx, door.cy}, node});
  }

  std::vector<std::pair<int, int>> directions = {
    {-1, 0},
    {1, 0},
    {0, -1},
    {0, 1},
    {-1, -1},
    {-1, 1},
    {1, -1},
    {1, 1}
  };
  for (auto [x, y] : vertices) {
    // Getting all neighboring pixels. Need to redo this... crazy inefficient.
    for (const auto & [dx, dy] : directions) {
      auto neighbor_x = x + dx * x_dist;
      auto neighbor_y = y + dy * y_dist;
      if (std::find(
          vertices.begin(), vertices.end(),
          std::pair<int, int>{neighbor_x, neighbor_y}) != vertices.end())
      {
        // inputting neighboring cells as edges into node_map 
        auto dist = sqrt(pow(neighbor_x-x,2) + pow(neighbor_y-y,2));
        node_map_.at({x,y}).edges.insert({{neighbor_x, neighbor_y},dist});
      }
    }
  }
  return vertices;
}

// Used to calculate distance between a door vertex to all the points in a room 
Point FeatureDetection::CalculateClosestPoint(int x, int y, int room)
{
  std::vector<Point> room_points;
  auto calc_2_points = [&x, &y](const std::pair<int, int> & point) -> int {
    auto dx = x - point.first;
    auto dy = y - point.second;
    return (sqrt(pow(dx, 2) + pow(dy,2)));
  };
  for (const auto & [point, index] : room_vertices_[(room-1)])
  {
    Point p;
    auto dist = calc_2_points(point);
    p.distance = dist;
    p.x = point.first;
    p.y = point.second;
    room_points.push_back(p);
  }
  std::sort(room_points.begin(), room_points.end(), [&](const Point & lhs, const Point & rhs) -> bool {
    return lhs.distance < rhs.distance;
  });
  return room_points[0];
}

std::vector<Door> FeatureDetection::GetDoors()
{
  // TODO take into account the possibility of doors that are diagonal in nature
  // Need to verify the x and y origin produced by opencv's connected components
  std::vector<Door> doors;
  for (const auto & door : features_.at(FeatureType::DOOR)) {
    Door d;
    if (door.height > door.width) { // vertical door
      d.start = std::make_pair(door.cx, (door.cy - door.height / 2));
      d.end = std::make_pair(door.cx, (door.cy + door.height / 2));
    } else { // horizontal door
      d.start = std::make_pair((door.cx - door.width / 2), door.cy);
      d.end = std::make_pair((door.cx + door.width / 2), door.cy);
    }
    d.cx = door.cx;
    d.cy = door.cy;
    doors.push_back(d);
  }
  return doors;
}

void FeatureDetection::TagDoortoRoom()
{
  cv::Mat image_hsv;
  cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

  cv::Scalar lower_red = cv::Scalar(169, 170, 120);  // doors
  cv::Scalar upper_red = cv::Scalar(170, 200, 255);  // doors
  cv::Mat red_mask;
  cv::inRange(image_hsv, lower_red, upper_red, red_mask);

  std::vector<std::pair<int, int>> directions = {
    {-1, 0},
    {1, 0},
    {0, -1},
    {0, 1},
    {-1, -1},
    {-1, 1},
    {1, -1},
    {1, 1}
  };

  for (auto & room : features_.at(FeatureType::ROOM)) {
    std::set<int> doors;
    for (const auto & [point, index] : room.pixels) {
      for (const auto & [dx, dy] : directions) {
        if (red_mask.at<uchar>(point.second + dy, point.first + dx) == 255) {
          for (const auto & door : features_.at(FeatureType::DOOR)) {
            if (door.pixels.find(
                std::pair<int, int>(
                  point.first + dx,
                  point.second + dy)) != door.pixels.end())
            {
              doors.insert(door.index);
            }
          }
        }
      }
    }
    room_to_doors_map_.insert({room.index, doors});
  }

  for (const auto & door : features_.at(FeatureType::DOOR))
  {
    std::set<int> rooms;
    for (const auto & [room, doors] : room_to_doors_map_)
    {
      if (doors.find(door.index) != doors.end())
      {
        rooms.insert(room);
      }
    }
    doors_to_room_map_.insert({door.index, rooms});
  }
}

void FeatureDetection::PrintDoortoRoom()
{
  for (const auto & [room, doors] : room_to_doors_map_) {
    std::cout << "Room:" << room;

    std::cout << " | Doors:";
    for (const auto & door : doors) {
      std::cout << door << ",";
    }
    std::cout << "\n";
  }

  for (const auto & [door, rooms] : doors_to_room_map_) {
    std::cout << "Door:" << door;

    std::cout << " | Rooms:";
    for (const auto & room : rooms) {
      std::cout << room << ",";
    }
    std::cout << "\n";
  }
}

void FeatureDetection::Print()
{
  for (const auto & [coord, node] : node_map_) {
    std::cout << "Node:" << node.index << " | edges: ";
    for ( const auto & [edge, length] : node.edges) 
    {
      std::cout << node_map_[{edge.first, edge.second}].index << ",length:" << length << " | ";
    }
    std::cout << "\n";
  }

  // for (const auto & room : room_vertices_)
  // {
  //   for (const auto & [coord, idx] : room)
  //   {
  //     std::cout << coord.first << "," << coord.second << "index: " << idx << "\n";
  //   }
  // }
}

void FeatureDetection::UpdateIndex(const std::map<std::pair<int, int>, int> & reference)
{
  for (const auto & [coordinate, idx] : reference)
  {
    if (node_map_.find(coordinate)!= node_map_.end())
    {
      node_map_.at(coordinate).index = idx;
    }
    for (auto & room : room_vertices_)
    {
      for (auto & [coord, index] : room)
      {
        if (coordinate == coord)
        {
          index = idx;
        }
      }
    }
  }
  for (const auto & [coord, node] : node_map_)
  {
    vertices_map_[node.index] = coord;
  }
}

void FeatureDetection::setStartPoint(int p)
{
  start_point_ = p;
}

void FeatureDetection::setEndPoint(int p)
{
  end_point_ = p;
}

int FeatureDetection::getStartPoint()
{
  // auto node = node_map_.at(start_point_);
  // std::cout << "index:" << node.index <<" | " << node.x <<","<< node.y << std::endl; 
  return start_point_;
}

int FeatureDetection::getEndPoint()
{
  // auto node = node_map_.at(end_point_);
  // std::cout << "index:" << node.index <<" | " << node.x <<","<< node.y << std::endl; 
  return end_point_;
}

std::vector<std::pair<int,int>> FeatureDetection::Dijkstra(int start, int end)
{
  std::priority_queue<IdxDistPair, std::vector<IdxDistPair>, std::greater<IdxDistPair>> queue;
  std::vector<int> distance(node_map_.size(), INT_MAX);
  std::vector<int> parent (node_map_.size(), -1);

  distance[start] = 0;
  queue.push({0,start});

  while (!queue.empty())
  {
    int node_idx = queue.top().second;
    int dist = queue.top().first;
    queue.pop();

    if (node_idx == end)
      break;
    if (dist > distance[node_idx])
      continue;
    for (const auto & [vertex,length] : node_map_.at(vertices_map_.at(node_idx)).edges)
    {
      if (distance[node_idx] + length < distance[node_map_.at(vertex).index])
      {
        distance[node_map_.at(vertex).index] = distance[node_idx] + length;
        parent[node_map_.at(vertex).index] = node_idx;
        queue.push({distance[node_map_.at(vertex).index], node_map_.at(vertex).index});
      }
    }
  }
  std::vector<int> path;
  std::vector<std::pair<int,int>> path_coordinates;
  if (distance[end] == INT_MAX)
    return path_coordinates;
  
  for (int v = end; v != -1; v = parent[v])
  {
    path.push_back(v);
  }
  std::reverse(path.begin(), path.end());

  for (const auto & index : path)
  {
    path_coordinates.push_back(vertices_map_.at(index));
  }
  return path_coordinates;
}

}  // namespace floorplan_annotator
