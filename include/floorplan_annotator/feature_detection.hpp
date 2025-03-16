#include <QImage>
#include <opencv2/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

namespace floorplan_annotator
{
struct Feature
{
  std::map<std::pair<int, int>, int> pixels;
  int x_origin, y_origin, width, height, index;
  double cx, cy;
};

struct Door
{
  std::pair<double, double> start, end;
  double cx, cy;
};

enum class FeatureType
{
  ROOM,
  DOOR,
  WALL,
  FLOOR
};

class FeatureDetection
{
public:
  explicit FeatureDetection(const std::string & image_path);

  ~FeatureDetection();

  FeatureDetection() = delete;

  // Not copiable
  FeatureDetection(const FeatureDetection &) = delete;

  FeatureDetection & operator=(const FeatureDetection &) = delete;

  std::vector<Feature> GetWalls() const;

  std::vector<std::pair<int, int>> GetRoomVertices(int x_dist, int y_dist);

  std::vector<Door> GetDoors();

  QImage convert2QImage();

  void UpdateRoomIndex(const std::map<std::pair<int, int>, int> & reference);

  void Print();

private:
  void FeatureExtraction();

  void ExtractRooms();

  void ExtractDoors();

  void ExtractWalls();

  void TagDoortoRoom();

  // debug function
  void PrintDoortoRoom();

  cv::Mat image_;

  std::unordered_map<int, std::set<int>> room_to_doors_map_;  // used for connectivity map

  std::map<FeatureType, std::vector<Feature>> features_;  // container for feature type containing pixels

  std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> connectivity_map_;  // contains connectivity of vertices generated

  std::vector<std::map<std::pair<int,int>,int>> room_vertices_;  // maps coordinates to index of a vertex

};
}  // namespace floorplan_annotator
