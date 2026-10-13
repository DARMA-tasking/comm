/*
//@HEADER
// *****************************************************************************
//
//                              test_logging.cc
//                 DARMA/comm => Communicator
//
// Copyright 2019-2024 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// SPDX-License-Identifier: BSD-3-Clause
//
// *****************************************************************************
//@HEADER
*/

#include <comm/util/logging.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace comm::tests::unit {
namespace {

int testRankProvider() {
  return 7;
}

struct LoggingTest : ::testing::Test {
  void SetUp() override {
    previous_verbosity = util::getVerbosity();
    previous_color_enabled = util::getColorEnabled();
    previous_rank_provider = util::getRankProvider();

    util::setVerbosity(util::Verbosity::normal);
    util::setColorEnabled(false);
    util::clearRankProvider();
  }

  void TearDown() override {
    util::setVerbosity(previous_verbosity);
    util::setColorEnabled(previous_color_enabled);
    util::setRankProvider(previous_rank_provider);
  }

  util::Verbosity previous_verbosity = util::Verbosity::normal;
  bool previous_color_enabled = true;
  util::RankProvider previous_rank_provider = nullptr;
};

TEST_F(LoggingTest, registrationIsIdempotentAndFirstDefaultWins) {
  auto const first = util::registerComponent("test.registration", false);
  auto const second = util::registerComponent("test.registration", true);
  auto const found = util::findComponent("test.registration");

  EXPECT_EQ(first, second);
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(first, *found);
  EXPECT_EQ(util::componentName(first), "test.registration");
  EXPECT_FALSE(util::isEnabled(first));
}

TEST_F(LoggingTest, emptyAndUnknownNamesFailCleanly) {
  EXPECT_THROW(util::registerComponent(""), std::invalid_argument);
  EXPECT_FALSE(util::findComponent("test.unknown").has_value());
  EXPECT_FALSE(util::enable("test.unknown"));
  EXPECT_FALSE(util::disable("test.unknown"));
  EXPECT_FALSE(util::isEnabled("test.unknown"));
}

TEST_F(LoggingTest, componentCanBeTargetedByHandleOrName) {
  auto const component = util::registerComponent("test.targeting", false);

  EXPECT_FALSE(util::isEnabled(component));
  util::enable(component);
  EXPECT_TRUE(util::isEnabled("test.targeting"));
  EXPECT_TRUE(util::disable("test.targeting"));
  EXPECT_FALSE(util::isEnabled(component));
  EXPECT_TRUE(util::enable("test.targeting"));
  EXPECT_TRUE(util::isEnabled(component));
}

TEST_F(LoggingTest, registryIsNotLimitedToAFixedNumberOfComponents) {
  std::vector<util::Component> components;
  for (int i = 0; i < 32; ++i) {
    components.push_back(util::registerComponent("test.many." + std::to_string(i)));
  }

  for (auto const& component : components) {
    util::enable(component);
    EXPECT_TRUE(util::isEnabled(component));
    util::disable(component);
  }
}

TEST_F(LoggingTest, handleLoggingHonorsEnablementAndVerbosity) {
  auto const component = util::registerComponent("test.handle-output", false);

  testing::internal::CaptureStdout();
  COMM_LOG(component, normal, "hidden {}\n", 1);
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");

  util::enable(component);
  testing::internal::CaptureStdout();
  COMM_LOG(component, normal, "value={}\n", 42);
  EXPECT_EQ(
    testing::internal::GetCapturedStdout(),
    "COMM: (normal) test.handle-output: value=42\n"
  );

  testing::internal::CaptureStdout();
  COMM_LOG(component, verbose, "too detailed\n");
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
}

TEST_F(LoggingTest, namedLoggingUsesOnlyRegisteredComponents) {
  util::registerComponent("test.named-output", true);

  testing::internal::CaptureStdout();
  COMM_LOG("test.named-output", terse, "ready\n");
  EXPECT_EQ(
    testing::internal::GetCapturedStdout(),
    "COMM: (terse) test.named-output: ready\n"
  );

  testing::internal::CaptureStdout();
  COMM_LOG("test.misspelled", terse, "must not appear\n");
  EXPECT_EQ(testing::internal::GetCapturedStdout(), "");
}

TEST_F(LoggingTest, rankProviderIsIncludedInDynamicComponentOutput) {
  auto const component = util::registerComponent("test.rank-output", true);
  util::setRankProvider(&testRankProvider);

  testing::internal::CaptureStdout();
  COMM_LOG(component, normal, "ranked\n");
  EXPECT_EQ(
    testing::internal::GetCapturedStdout(),
    "COMM: [7] (normal) test.rank-output: ranked\n"
  );
}

TEST_F(LoggingTest, builtInsAreNormalRegistryEntries) {
  auto const found = util::findComponent("Communicator");
  auto const& communicator = util::communicatorComponent();

  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(*found, communicator);
}

} // anonymous namespace
} // namespace comm::tests::unit
