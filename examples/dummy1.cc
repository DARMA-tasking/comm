#include <iostream>
#include <comm/dummy.h>

int main() {
  comm::dummy::Dummy dummy;

  std::cout << dummy.sum(2, 8) << '\n';

  return 0;
}
