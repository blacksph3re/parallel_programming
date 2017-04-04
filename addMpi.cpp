#include "mpi.h"
#include <iostream>
#include "matrix.hpp"


int main(int ac, char* av[]) {

  MPI_Init(&ac, &av);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


  std::pair<size_t, size_t> datasize;

  // Rank 0 initializes
  if(world_rank == 0) {
    if(readCmdLineArgs(ac, av) != 0) {
      MPI_Finalize();
      return 0;
    }
    N = world_size;

    auto a = Matrix<double>::readMetadata(file_a);
    auto b = Matrix<double>::readMetadata(file_b);

    if(a.first != b.first || a.second != b.second) {
      std::cout << "Dimension mismatch!" << std::endl;
      MPI_Finalize();
      return -1;
    }

    datasize = a;
  }
  // Broadcast the size of the data
  MPI_Bcast(&datasize, sizeof(datasize), MPI_BYTE, 0, MPI_COMM_WORLD);
  size_t total_data = datasize.first * datasize.second;
  size_t zeroes = total_data%world_size;
  total_data += zeroes;  
  size_t objects_per_proc = total_data/world_size;


  // This is a temporary class for rank 0 to read files into
  double* a_data, *b_data;

  // This is the real data, sorry for C-style 
  double* a = new double[objects_per_proc];
  double* b = new double[objects_per_proc];

  // Read actual data
  if(world_rank == 0) {
    // Add zeroes to pad data to make sure we are accessing valid memory
    // Those zeroes will be added but ignored upon output

    a_data = Matrix<double>::readCStyle(file_a, zeroes);
    b_data = Matrix<double>::readCStyle(file_b, zeroes);


    start_timing_measure();
  }

  // Scatter it out to all processes
  MPI_Scatter(a_data, objects_per_proc, MPI_DOUBLE, a, objects_per_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Scatter(b_data, objects_per_proc, MPI_DOUBLE, b, objects_per_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  for(size_t i=0; i<objects_per_proc; i++) {
    a[i] += std::sqrt(std::sqrt(std::sqrt(b[i])));
  }

  MPI_Gather(a, objects_per_proc, MPI_DOUBLE, a_data, objects_per_proc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  delete a;
  delete b;
/*
	start_timing_measure();
	for(size_t m=0; m<a.getRows(); m++) {
		for(size_t n=0; n<a.getCols(); n++)
			a.get(m, n) += std::sqrt(std::sqrt(std::sqrt(b.get(m, n))));
	}
	end_timing_measure("Add sequentially");
*/
  if(world_rank == 0) {
    end_timing_measure("Add Mpi");
    Matrix<double> res(a_data, datasize.first, datasize.second);
    delete a_data;
    delete b_data;

    if(!file_out.empty())
      res.writeToFile(file_out);
    else
      res.output();

  }




	MPI_Finalize();

	return 0;
}