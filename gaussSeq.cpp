#include "matrix.hpp"


int main(int ac, char* av[]) {

	if(readCmdLineArgs(ac, av) != 0)
		return 0;

	Matrix<double> a(file_a);
	// Transpose a for better access performance
	a = a.getTransposed();
	Matrix<double> b(file_b); // Just a vector in our case, but I am too lazy to implement a separate datastructure for that.

	if(a.getCols() != a.getRows() || b.getCols() != 1 || b.getRows() != a.getRows()) {
		std::cout << "Dimension mismatch!" << std::endl;
		return -1;
	}


	start_timing_measure();


	// Create the echelon form
	for(size_t k = 0; k < a.getRows(); k++) {

		// Find pivot, for some magic reason the highest absolute
		size_t pivot;
		double max = -1;
		for(size_t i=k; i<a.getRows(); i++) {
			if(std::abs(a.getTransposed(i, k)) > max) {
				max = std::abs(a.getTransposed(i, k));
				pivot = i;
			}
		}

		if(a.getTransposed(pivot, k) == 0) {
			std::cout << "Matrix is not inversible" << std::endl;
			return -1;
		}

		a.swap_rows(pivot, k);

		for (size_t row = k+1; row < a.getRows(); row++) {
			auto multiplier = a.getTransposed(row, k) / a.getTransposed(k, k);
			for (size_t col = k; col < a.getRows(); col++) {
				a.getTransposed(row, col) -= a.getTransposed(k, col) * multiplier;
			}
			a.getTransposed(row, k) = 0;
		}
	}
	

	end_timing_measure("Gauss sequentially");

	a = a.getTransposed();
	if(!file_out.empty())
		a.writeToFile(file_out);
	else {
		a.output();
	}

	return 0;
}