#include "dummy.h"

namespace comm::dummy {

  /// Computes something dummy.
  void Dummy::addDummy() {
    if(is_dummy_) {
      dummies_.insert(2);
    }
  }

  int Dummy::sum(int a, int b) const {
    return a + b;
  }

} // namespace comm::dummy
