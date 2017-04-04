#include "matrix.hpp"
#include <thread>


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


	auto calc = [&](std::pair<int, int> area) {
		for(size_t n=area.first; n<area.second; n++) {
			for(size_t m=0; m<res.getRows(); m++) {
				double tmp=0;
				for(size_t k=0; k<a.getCols(); k++)
					tmp += a.get(m, k)*b.getTransposed(k, n);
				res.get(m, n) = tmp;
			}
		}
	};


	start_timing_measure();
	std::vector<std::thread> threads;
	auto chunking = chunk<int>(std::pair<int, int>(0, res.getCols()), N);
	for(auto i : chunking) {
		threads.push_back(std::thread(calc, i));
	}

	for(auto& i : threads)
		i.join();

	end_timing_measure("Mult threads");


	if(!file_out.empty())
		res.writeToFile(file_out);
	else
		res.output();

	return 0;
}