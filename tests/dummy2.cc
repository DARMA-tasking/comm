#include <gtest/gtest.h>
#include <comm/dummy.h>

TEST(TestSource, Dummy2) {
  comm::dummy::Dummy dummy;
  int gold = 5;
  EXPECT_EQ(dummy.sum(2, 3), gold);
}
