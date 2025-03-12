#include "floorplan_annotator/feature_detection.hpp"
#include "gtest/gtest.h"

using namespace floorplan_annotator;  // NOLINT(build/namespaces)

class TESTREST : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }
};

TEST_F(TESTREST, DETECTIONTEST) {
  FeatureDetection("/home/dillon/rmf_ws/src/floorplan_annotator/test/output.png");
  
}
