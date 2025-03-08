#ifndef FLOORPLAN_ANNOTATOR__UTILS__REST_COMMUNICATOR_HPP_
#define FLOORPLAN_ANNOTATOR__UTILS__REST_COMMUNICATOR_HPP_

#include <memory>
#include <string>

#include "cpprest/http_client.h"

#include "floorplan_annotator/utils/rest_communicator_base.hpp"
#include "nlohmann/json.hpp"

namespace floorplan_annotator
{
namespace utils
{
class RestCommunicator : public RestCommunicatorBase
{
public:
  RestCommunicator();

  void init(const std::string & url) override;

  bool is_connected() const override;

  bool send_update(
    const std::string & filepath,
    int colorize, int postprocess, const std::string & outputfilepath) override;

  ~RestCommunicator();

private:
  class Implementation;
  std::unique_ptr<Implementation> p_;
};

}  // namespace utils
}  // namespace floorplan_annotator
#endif  // FLOORPLAN_ANNOTATOR__UTILS__REST_COMMUNICATOR_HPP_
