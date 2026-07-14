#include <comm/comm/comm_traits.h>
#include <comm/comm/MPI/comm_mpi.h>

int main() {
  static_assert(comm::Communicator<comm::CommMPI>);
  return 0;
}
