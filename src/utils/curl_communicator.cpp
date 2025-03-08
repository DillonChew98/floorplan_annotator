#include "floorplan_annotator/utils/curl_communicator.hpp"

namespace floorplan_annotator
{
namespace utils
{

CurlCommunicator::CurlCommunicator()
: verbose_{false}
{
  curl_handle_ = curl_easy_init();
}

void CurlCommunicator::post_request(
  const std::string & url, const std::string & infilepath, const std::string & outfilepath,
  int colorize, int postprocess)
{
  curl_httppost * formpost = NULL;
  curl_httppost * lastptr = NULL;
  curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, verbose_ ? 1L : 0L);

  FILE * fp = fopen(outfilepath.c_str(), "wb");
  if (!fp) {
    printf("!!! Failed to create file on the disk\n");
    return;
  }

  curl_formadd(
    &formpost, &lastptr,
    CURLFORM_COPYNAME, "file",
    CURLFORM_FILE, infilepath.c_str(),
    CURLFORM_END);

  curl_formadd(
    &formpost, &lastptr,
    CURLFORM_COPYNAME, "postprocess",
    CURLFORM_COPYCONTENTS, postprocess,
    CURLFORM_END);

  curl_formadd(
    &formpost, &lastptr,
    CURLFORM_COPYNAME, "colorize",
    CURLFORM_COPYCONTENTS, colorize,
    CURLFORM_END);

  curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle_, CURLOPT_HTTPPOST, formpost);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1);

  CURLcode res = curl_easy_perform(curl_handle_);

  if (res) {
    printf("!!! Failed to download: %s\n", url);
    return;
  }

  int res_code = 0;
  curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &res_code);
  if (!((res_code == 200 || res_code == 201) && res != CURLE_ABORTED_BY_CALLBACK)) {
    printf("!!! Response code: %i\n", res_code);
    return;
  }

  fclose(fp);
}

static size_t write_data(void * ptr, size_t size, size_t nmemb, void * data)
{
  FILE * stream = reinterpret_cast<FILE *>(data);
  if (!stream) {
    printf("!!! No stream\n");
    return 0;
  }
  size_t written = fwrite(reinterpret_cast<FILE *>(ptr), size, nmemb, stream);
  return written;
}
CurlCommunicator::~CurlCommunicator()
{
  curl_easy_cleanup(curl_handle_);
}

void CurlCommunicator::set_verbose(bool cond)
{
  verbose_ = cond;
}

}  // namespace utils
}  // namespace floorplan_annotator
