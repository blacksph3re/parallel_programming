#include "matrix.hpp"


int main(int ac, char* av[]) {

  if(readCmdLineArgs(ac, av) != 0)
    return 0;

  Matrix<double> a(file_a);

  Matrix<double> b_file(file_b); // Just a vector in our case, but I am too lazy to implement a separate datastructure for that.

  if(a.getCols() != a.getRows() || b_file.getCols() != 1 || b_file.getRows() != a.getRows()) {
    std::cout << "Dimension mismatch!" << std::endl;
    return -1;
  }
  int N = b_file.getRows();

  // Read column vector from b
  double* b = new double[N];
  double* x = new double[N];
  double* s = new double[N];

  for(int i=0; i<N; i++) {
    b[i] = b_file.get(0, i);
    x[i] = 0;
    s[i] = 0;
  }



  start_timing_measure();

  for(size_t k=N-1; k>0; k--) {
    x[k] = b[k] - s[k];

    for(int i=k-1; i>0; i--) {
      s[i] += x[k] * a.get(k, i);
    }

  }
  

  end_timing_measure("Gauss sequentially");

  if(!file_out.empty())
    a.writeToFile(file_out);
  else {
    a.output();
  }

  return 0;
}