#include "matrix.hpp"
#include <thread>

int main(int ac, char* av[]) {

	if(readCmdLineArgs(ac, av) != 0)
		return 0;

	Matrix<double> a(file_a);
	Matrix<double> b(file_b);

	if(a.getRows() != b.getRows() || a.getCols() != b.getCols()) {
		std::cout << "Dimension mismatch!" << std::endl;
		return -1;
	}

	auto calc = [&](std::pair<int, int> area){
		for(size_t m=area.first; m<area.second; m++) {
			for(size_t n=0; n<a.getCols(); n++)
				a.get(m, n) += std::sqrt(std::sqrt(std::sqrt(b.get(m, n))));
		}
	};

	start_timing_measure();
	std::vector<std::thread> threads;
	auto chunking = chunk<int>(std::pair<int, int>(0, a.getRows()), N);
	for(auto i : chunking) {
		threads.push_back(std::thread(calc, i));
	}

	for(auto& i : threads)
		i.join();
	end_timing_measure("Add threads");

	if(!file_out.empty())
		a.writeToFile(file_out);
	else
		a.output();

	return 0;
}