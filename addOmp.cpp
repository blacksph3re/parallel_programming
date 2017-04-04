#include "matrix.hpp"
#include <cmath>
#include <omp.h>

int main(int ac, char* av[]) {

    if(readCmdLineArgs(ac, av) != 0)
        return 0;

    Matrix<double> a(file_a);
    Matrix<double> b(file_b);

    if(a.getRows() != b.getRows() || a.getCols() != b.getCols()) {
        std::cout << "Dimension mismatch!" << std::endl;
        return -1;
    }

    start_timing_measure();
    #pragma omp parallel for collapse(2)
    for(size_t m=0; m<a.getRows(); m++) {
        for(size_t n=0; n<a.getCols(); n++)
            a.get(m, n) += std::sqrt(std::sqrt(std::sqrt(b.get(m, n))));
    }
    end_timing_measure("Add openmp");

    if(!file_out.empty())
        a.writeToFile(file_out);
    else
        a.output();

    return 0;
}