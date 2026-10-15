#include <comm/comm/comm_traits.h>
#include <comm/comm/MPI/comm_mpi.h>

int main() {
  static_assert(comm::is_communicator_v<comm::CommMPI>);
  return 0;
}
