#include <atomic>
#include <fstream>
#include <string>
#include <sstream>

#include "floorplan_annotator/utils/rest_communicator.hpp"

namespace floorplan_annotator
{
namespace utils
{
class RestCommunicator::Implementation
{
public:
  using http_request = web::http::http_request;
  using http_response = web::http::http_response;
  using http_client = web::http::client::http_client;

  void init(const std::string & url)
  {
    client_ptr_ = std::make_unique<http_client>(U(url));
  }

  bool send_update(
    const std::string & filepath,
    bool colorize, bool postprocess, const std::string & outputfilepath)
  {
    bool update_status = false;
    web::http::uri_builder builder(U("upload"));
    // builder.append_query()    //uncomment this if queries are needed
    try {
      http_request request(web::http::methods::POST);
      std::pair<std::ifstream, std::string> data = std::make_pair(
        std::ifstream{filepath,
          std::ios::binary}, filepath);
      auto p = generate_multiform_data(data, colorize, postprocess);
      // request.headers().add(U("Content-Type"), U("application/json"));
      request.set_body(p.second, "multipart/form-data; boundary=" + p.first);
      client_ptr_->request(web::http::methods::POST, builder.to_string()).then(
        [&](http_response response)
        {
          if (response.status_code() < 400) {
            update_status_ = true;

          } else {
            update_status_ = false;
          }
        }).wait();
    } catch (const std::exception & e) {
      connected_ = false;
    }
    return update_status;
  }

  bool is_connected()
  {
    return connected_;
  }

  std::pair<std::string, std::string> generate_multiform_data(
    std::pair<std::ifstream, std::string> & file_data,
    int colorize, int postprocess)
  {
    std::stringstream data;

    std::string boundary {};
    for (int i = 0; i < 50; ++i) {
      boundary += (rand() % 26) + 'A';
    }
    std::string filename = "file";
    data << "--" << boundary << "\r\n";
    data << "Content-Disposition: form-data; name=\"file\"; filename=\""
         << file_data.second << "\"\r\nContent-Type: application/octet-stream\r\n\r\n"
         << read_entire_file(file_data.first) << "\r\n\r\n";
    data << "--" << boundary << "\r\nContent-Disposition: form-data; name=\"postprocess\"\r\n\r\n"
         << postprocess << "\r\n";
    data << "--" << boundary << "\r\nContent-Disposition: form-data; name=\"colorize\"\r\n\r\n"
         << colorize << "\r\n";
    data << "--" << boundary << "--";

    return {boundary, data.str()};
  }

  std::string read_entire_file(std::ifstream & ifs)
  {
    return std::string(
      (std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>()));
  }

private:
  std::atomic_bool connected_ = false;
  std::atomic_bool update_status_ = false;
  std::unique_ptr<http_client> client_ptr_;
};

RestCommunicator::RestCommunicator()
: p_(std::make_unique<Implementation>())
{}

RestCommunicator::~RestCommunicator() {}

void RestCommunicator::init(const std::string & url)
{
  p_->init(url);
}

bool RestCommunicator::is_connected() const
{
  return p_->is_connected();
}

bool RestCommunicator::send_update(
  const std::string & filepath,
  int colorize, int postprocess, const std::string & outputfilepath)
{
  return p_->send_update(filepath, colorize, postprocess, outputfilepath);
}

}  // namespace utils
}  // namespace floorplan_annotator
