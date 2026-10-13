#include <comm/util/logging.h>

#include <cstdio>

int main() {
  namespace logging = comm::util;

  logging::setColorEnabled(false);
  logging::setVerbosity(logging::Verbosity::verbose);

  auto const scheduler = logging::registerComponent("Scheduler", true);
  COMM_LOG(scheduler, normal, "logging through a retained handle\n");
  COMM_LOG("Scheduler", verbose, "logging through the registered name\n");
  logging::disable("Scheduler");
  COMM_LOG(scheduler, normal, "this disabled message is not printed\n");

  auto const communicator = logging::registerComponent("Communicator");
  logging::enable(communicator);
  COMM_LOG(communicator, terse, "built-in component enabled by handle\n");

  if (!logging::enable("MisspelledComponent")) {
    std::puts("Unknown component was rejected");
  }

  return 0;
}
