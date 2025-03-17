#include <QImage>
#include <opencv2/core.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

namespace floorplan_annotator
{
using Edge = int;

using IdxDistPair = std::pair<int,int>;

struct Node
{
    int x, y, index;
    std::map<std::pair<int,int>, Edge> edges; // pair of coordinate and length of an edge
};

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

struct Point
{
  int x, y, distance;
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

  std::vector<std::pair<int, int>> GetVertices(int x_dist, int y_dist);

  std::vector<Door> GetDoors();

  QImage convert2QImage();

  void UpdateIndex(const std::map<std::pair<int, int>, int> & reference);
  
  std::vector<std::pair<int, int>> Dijkstra(int start, int end);

  void setStartPoint(int p);

  void setEndPoint(int p);

  int getStartPoint();

  int getEndPoint();

  void Print();

private:
  void FeatureExtraction();

  void ExtractRooms();

  void ExtractDoors();

  void ExtractWalls();

  void TagDoortoRoom();

  // debug function
  void PrintDoortoRoom();

  Point CalculateClosestPoint(int x, int y, int room);

  cv::Mat image_;

  int start_point_;

  int end_point_;

  std::unordered_map<int, std::set<int>> room_to_doors_map_;  // used for connectivity map

  std::unordered_map<int, std::set<int>> doors_to_room_map_;  // reverse map

  std::map<FeatureType, std::vector<Feature>> features_;  // container for feature type containing pixels
  
  std::map<std::pair<int,int>, Node> node_map_; // adjacency list, coordinate of a point mapped to index and edges

  std::vector<std::map<std::pair<int,int>,int>> room_vertices_;  // maps coordinates to index of a vertex

  std::map<int, std::pair<int, int>> vertices_map_; // reverse node_map;

};
}  // namespace floorplan_annotator
