#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <algorithm>
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
  // Print();
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

std::vector<std::pair<int, int>> FeatureDetection::GetRoomVertices(int x_dist, int y_dist)
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
          vertices.push_back({v_x, v_y});
          v.insert({{v_x,v_y}, -1});
        }
      }
    }
    room_vertices_.push_back(v); // we store the vertices into another container for easy reference for path planning
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
    std::vector<std::pair<int, int>> edges;
    for (const auto & [dx, dy] : directions) {
      auto neighbor_x = x + dx * x_dist;
      auto neighbor_y = y + dy * y_dist;
      if (std::find(
          vertices.begin(), vertices.end(),
          std::pair<int, int>{neighbor_x, neighbor_y}) != vertices.end())
      {
        edges.push_back(std::pair<int, int>{neighbor_x, neighbor_y});
      }
    }
    connectivity_map_.insert(std::make_pair(std::pair<int, int>{x, y}, edges));
  }
  return vertices;
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
}

void FeatureDetection::PrintDoortoRoom()
{
  for (const auto & [room, doors] : room_to_doors_map_) {
    std::cout << "Room:" << room;
    for (const auto & r : features_.at(FeatureType::ROOM)) {
      if (r.index == room) {
        std::cout << " cx:" << r.cx << " cy:" << r.cy << " ";
      }
    }

    std::cout << " | Doors:";
    for (const auto & door : doors) {
      std::cout << door << ",";
    }
    std::cout << "\n";
  }
}

void FeatureDetection::Print()
{
  for (const auto & [node, edges] : connectivity_map_) {
    std::cout << "node:(" << node.first << "," << node.second << ")" << "   edges:";
    for (const auto & edge : edges) {
      std::cout << "(" << edge.first << "," << edge.second << ") ";
    }
    std::cout << "\n";
  }
}
}  // namespace floorplan_annotator
