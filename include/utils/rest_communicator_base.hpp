#ifndef UTILS__REST_COMMUNICATOR_BASE_HPP_
#define UTILS__REST_COMMUNICATOR_BASE_HPP_

#include <string>

#include <nlohmann/json.hpp>

namespace floorplan_annotator
{
namespace utils
{
class RestCommunicatorBase
{
public:
  virtual void init(const std::string & url) = 0;

  virtual bool send_update(
    const std::string & filepath,
    int colorize, int postprocess, const std::string & outputfilepath) = 0;

  virtual bool is_connected() const = 0;

  virtual ~RestCommunicatorBase() {}
};
}  // namespace utils
}  // namespace floorplan_annotator
#endif  // UTILS__REST_COMMUNICATOR_BASE_HPP_
