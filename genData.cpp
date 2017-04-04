#include "matrix.hpp"
#include <cstdlib>
#include <random>


int main() {
	size_t rows = 1000;
	size_t cols = 1000;
	bool gen_integers = false;


	if(const char* env_p = std::getenv("ROWS")) {
		std::stringstream s;
		s << env_p;
		s >> rows;
	}

	if(const char* env_p = std::getenv("COLS")) {
		std::stringstream s;
		s << env_p;
		s >> cols;
	}

	if(const char* env_p = std::getenv("GENINT")) {
		gen_integers=true;
	}

	std::cout << "Creating a " << rows << "x" << cols << " matrix..." << std::flush;

	Matrix<double> m(rows, cols);

	auto lower_bound = -100;
	auto upper_bound = 100;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> realdistribution(lower_bound, upper_bound);
	std::uniform_int_distribution<int> intdistribution(lower_bound, upper_bound);
	for(auto& v : m.getData()) {
		for(auto& i : v) {
			i = static_cast<double>(gen_integers ? intdistribution(generator) : realdistribution(generator));
		}
	}
	

	std::stringstream filename;
	filename << "data/" << rows << "x" << cols << ".matrix";
	m.writeToFile(filename.str());

	std::cout << " done" << std::endl;

	return 0;
}