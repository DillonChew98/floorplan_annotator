#include <QImage>
#include <opencv2/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace floorplan_annotator
{
struct Feature
{
  std::map<std::pair<int,int>, int> pixels;
  int x_origin, y_origin, width, height;
  double cx, cy;

};

// TODO change to only having colours for walls, rooms and doors
cv::Scalar lower_white = cv::Scalar(0, 0, 255);  // walls
cv::Scalar upper_white = cv::Scalar(0, 0, 255);  // walls
cv::Scalar lower_blue = cv::Scalar(90, 50, 150);  // kitchen/living/dining room
cv::Scalar upper_blue = cv::Scalar(90, 100, 255); // kitchen/living/dining room
cv::Scalar lower_purple = cv::Scalar(120,35,120);  // rooms
cv::Scalar upper_purple = cv::Scalar(120,45,255);  // rooms
cv::Scalar lower_yellow = cv::Scalar(22,120,120);  // rooms
cv::Scalar upper_yellow = cv::Scalar(24,150,255);  // rooms
cv::Scalar lower_green = cv::Scalar(44,60,255);  // rooms
cv::Scalar upper_green = cv::Scalar(45,160,255);  // rooms
cv::Scalar lower_orange = cv::Scalar(10,155,255);  // rooms
cv::Scalar upper_orange = cv::Scalar(40,160,255);  // rooms
cv::Scalar lower_red = cv::Scalar(169, 170, 120);  // doors
cv::Scalar upper_red = cv::Scalar(170, 200, 255);  // doors

class FeatureDetection
{
public:
  explicit FeatureDetection(const std::string & image_path);
  ~FeatureDetection();

  FeatureDetection() = delete;

  // Not copiable
  FeatureDetection(const FeatureDetection &) = delete;

  FeatureDetection& operator=(const FeatureDetection &) = delete;

  std::vector<Feature> GetWalls() const;

  std::vector<std::pair<double, double>> GetRoomVertices(double x_dist, double y_dist);

  std::vector<Feature> GetDoors() const;

  QImage convert2QImage();

private:
  void FeatureExtraction();

  void ExtractRooms();

  void ExtractDoors();

  void ExtractWalls();

  cv::Mat image_;

  cv::Mat image_bgr_;

  std::unordered_map<std::string, std::vector<Feature>> features_;
  
};
}
