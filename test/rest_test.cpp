#include <memory>

#include "floorplan_annotator/utils/curl_communicator.hpp"
#include "gtest/gtest.h"


using namespace floorplan_annotator;  // NOLINT(build/namespaces)
using namespace floorplan_annotator::utils;  // NOLINT(build/namespaces)

class TESTREST : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
};

TEST_F(TESTREST, UPLOADTEST) {
  auto http_ptr = std::make_unique<CurlCommunicator>();
  http_ptr->set_verbose(true);
  http_ptr->post_request(
    "/home/dillon/30939153.jpg",
    "/home/dillon/rmf_ws/src/floorplan_annotator/test/output.png",
    1, 1);
}
