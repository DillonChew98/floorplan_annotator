#ifndef FLOORPLAN_ANNOTATOR__UTILS__CURL_COMMUNICATOR_HPP_
#define FLOORPLAN_ANNOTATOR__UTILS__CURL_COMMUNICATOR_HPP_

#include <curl/curl.h>

#include <memory>
#include <string>

namespace floorplan_annotator
{
namespace utils
{
static size_t write_data(void * ptr, size_t size, size_t nmemb, void * data);

class CurlCommunicator
{
public:
  CurlCommunicator();

  void post_request(
    const std::string & url, const std::string & infilepath, const std::string & outfilepath,
    int colorize, int postprocess);

  ~CurlCommunicator();

  void set_verbose(bool cond);

private:
  CURL * curl_handle_{nullptr};

  bool verbose_;
};

}  // namespace utils
}  // namespace floorplan_annotator
#endif  // FLOORPLAN_ANNOTATOR__UTILS__CURL_COMMUNICATOR_HPP_
