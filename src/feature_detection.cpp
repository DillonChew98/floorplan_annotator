#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

#include "floorplan_annotator/feature_detection.hpp"

namespace floorplan_annotator
{
FeatureDetection::FeatureDetection(const std::string & image_path)
{
    image_ = cv::imread(image_path);
    cv::cvtColor(image_, image_bgr_, cv::COLOR_RGB2BGR); // used for converting to QImage, may not need
    FeatureExtraction();
}

void FeatureDetection::FeatureExtraction()
{
    ExtractRooms();
    ExtractDoors();
    ExtractWalls();
}

FeatureDetection::~FeatureDetection()
{

}

void FeatureDetection::ExtractRooms()
{
    cv::Mat image_hsv;
    cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

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
    for (int i = 1; i < num_components; i++) // start at 1 as 0 will always contain the component of the entire canvas
    {
        Feature room;
        room.x_origin = stats.at<int>(i, cv::CC_STAT_LEFT);
        room.y_origin = stats.at<int>(i, cv::CC_STAT_TOP);
        room.width = stats.at<int>(i, cv::CC_STAT_WIDTH);
        room.height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        // int area = stats.at<int>(i, cv::CC_STAT_AREA);
        room.cx = centroids.at<double>(i, 0);
        room.cy = centroids.at<double>(i, 1);
        for (int y = room.y_origin; y < room.y_origin + room.height; y++) {
            for (int x = room.x_origin; x < room.x_origin + room.width; x++) {
                if (labels.at<int>(y, x) == i) {
                    auto index = y * room.x_origin + x;
                    room.pixels[std::pair<int,int>{x,y}] = index;
                }
            }
        }
        rooms.push_back(room);
    }
    features_.insert(std::pair<FeatureType, std::vector<Feature>>(FeatureType::ROOM, rooms));
}

void FeatureDetection::ExtractDoors()
{
    cv::Mat image_hsv;
    cv::cvtColor(image_, image_hsv, cv::COLOR_BGR2HSV);

    cv::Mat red_mask;
    cv::inRange(image_hsv, lower_red, upper_red, red_mask);

    cv::Mat labels, stats, centroids;
    int num_components = cv::connectedComponentsWithStats(red_mask, labels, stats, centroids);
    std::vector<Feature> doors;
    doors.reserve(num_components);
    for (int i = 1; i < num_components; i++) // start at 1 as 0 will always contain the component of the entire canvas
    {
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area < 20)
          continue;
        Feature door;
        door.x_origin = stats.at<int>(i, cv::CC_STAT_LEFT);
        door.y_origin = stats.at<int>(i, cv::CC_STAT_TOP);
        door.width = stats.at<int>(i, cv::CC_STAT_WIDTH);
        door.height = stats.at<int>(i, cv::CC_STAT_HEIGHT);
        door.cx = centroids.at<double>(i, 0);
        door.cy = centroids.at<double>(i, 1);
        for (int y = door.y_origin; y < door.y_origin + door.height; y++) {
            for (int x = door.x_origin; x < door.x_origin + door.width; x++) {
                if (labels.at<int>(y, x) == i) {
                    auto index = y * door.x_origin + x;
                    door.pixels[std::pair<int,int>{x,y}] = index;
                }
            }
        }
        doors.push_back(door);
    }
    features_.insert(std::pair<FeatureType, std::vector<Feature>>(FeatureType::DOOR, doors));
    // cv::imshow("blue_mask", red_mask);
    // cv::imshow("Display window", image_);
    // cv::waitKey(0);
}

void FeatureDetection::ExtractWalls()
{

}

QImage FeatureDetection::convert2QImage()
{
    return QImage((uchar*) image_bgr_.data, image_bgr_.cols, image_bgr_.rows, image_bgr_.step, QImage::Format_RGB888);
}

std::vector<Feature> FeatureDetection::GetWalls() const
{

}

std::vector<std::pair<double, double>> FeatureDetection::GetRoomVertices(double x_dist, double y_dist)
{
  std::vector<std::pair<double, double>> vertices;
  for (const auto& room : features_.at(FeatureType::ROOM))
  {
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
            if (room.pixels.find(std::pair<int,int>{v_x,v_y})!= room.pixels.end())
              vertices.push_back(std::make_pair(v_x,v_y));
        }
    }
  }
  return vertices;
}

std::vector<Door> FeatureDetection::GetDoors()
{
  // TODO take into account the possibility of doors that are diagonal in nature
  // Need to verify the x and y origin produced by opencv's connected components
  std::vector<Door> doors;
  for (const auto& door : features_.at(FeatureType::DOOR))
  {
    Door d;
    if (door.height > door.width) // vertical door
    {
      d.start = std::make_pair(door.cx,(door.cy - door.height/2));
      d.end = std::make_pair(door.cx, (door.cy + door.height/2));
    }
    else
    {
      d.start = std::make_pair((door.cx - door.width/2), door.cy);
      d.end = std::make_pair((door.cx + door.width/2), door.cy);
    }
    d.cx = door.cx;
    d.cy = door.cy;
    doors.push_back(d);
  }
  return doors;
}
}  // namespace floorplan_annotator