#include "matrix.hpp"


int main(int ac, char* av[]) {

	if(readCmdLineArgs(ac, av) != 0)
		return 0;

	Matrix<double> a(file_a);
	Matrix<double> b(file_b);

	if(a.getCols() != b.getRows()) {
		std::cout << "Dimension mismatch!" << std::endl;
		return -1;
	}

	Matrix<double> res(a.getRows(), b.getCols());

	b = b.getTransposed();

	start_timing_measure();
	#pragma omp parallel for collapse(2)
	for(size_t n=0; n<res.getCols(); n++) {
		for(size_t m=0; m<res.getRows(); m++) {
			double tmp=0;
			for(size_t k=0; k<a.getCols(); k++)
				tmp += a.get(m, k)*b.getTransposed(k, n);
			res.get(m, n) = tmp;
		}
	}
	end_timing_measure("Mult openmp");

	if(!file_out.empty())
		res.writeToFile(file_out);
	else
		res.output();

	return 0;
}