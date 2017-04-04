#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <iterator>
#include <chrono>
#include <boost/program_options.hpp>
#include <cstdlib>

namespace po = boost::program_options;

// Global variables, please don't kill me!
std::string file_a, file_b, file_out, file_timing;
size_t N;
std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds> timing_start;

int readCmdLineArgs(int ac, char* av[]) {
	try {
		size_t default_n = 1;
		if(const char* env_p = std::getenv("OMP_NUM_THREADS"))
			default_n = std::atoi(env_p);

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("file_a", po::value<std::string>(&file_a)->required(), "First matrix")
            ("file_b", po::value<std::string>(&file_b), "Second matrix, optional. If not given, the first matrix will be treated as second parameter")
            ("file_out", po::value<std::string>(&file_out)->default_value(""), "Output, optional")
            ("n", po::value<size_t>(&N)->default_value(default_n), "Number of parallel units, default 1")
            ("timing_output", po::value<std::string>(&file_timing)->default_value(""), "File to write timing values to")
        ;

        po::positional_options_description positionalOptions; 
    	positionalOptions.add("file_a", 1); 
    	positionalOptions.add("file_b", 1); 

        po::variables_map vm;        
		po::store(po::command_line_parser(ac, av).options(desc).positional(positionalOptions).run(), vm);
		if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
		po::notify(vm);

		if(!vm.count("file_b")) {
			file_b = file_a;
		}
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }
    return 0;
}

void start_timing_measure() {
	timing_start = std::chrono::steady_clock::now();
}

void end_timing_measure(std::string description = "Measurement ") {
	auto timing_end = std::chrono::steady_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(timing_end - timing_start).count();
	if(!file_timing.empty()) {
		std::fstream file;
		file.open(file_timing, std::ios::out | std::ios::app);
		file << description << " A=" << file_a << " B=" << file_b << " N=" << N << " \tTime=" << elapsed_seconds << "s\n";
		file.close();
	}
	std::cout << description << " A=" << file_a << " B=" << file_b << " N=" << N << " \tTime=" << elapsed_seconds << "s" << std::endl;
}

/**
* Chunks an area between start and end into samesized chunks
* The first variable of the pairs are the start, the second the end
* epsilon will be added to all starting points except for the first one
*/
template <typename T>
std::vector<std::pair<T, T>> chunk(std::pair<T, T> area, size_t n, T epsilon = 0) {
	std::vector<std::pair<T, T>> retval;
	T stepsize = (area.second - area.first) / n;

	if(n == 1)
		retval.push_back(area);
	else {
		T curstep = area.first;
		for(int i=0; i<n; i++) {
			T start = curstep;
			if(i != 0)
				start += epsilon;
			curstep += stepsize;
			T end = curstep;
			if(i == n-1)
				end = area.second;
			retval.push_back(std::pair<T, T>(start, end));
		}
	}
	return retval;
}

template <typename T>
class Matrix {
public:
	/**
	* Checks two matrices for equality
	*/
	bool equals(const Matrix<T>& rhs) {

		bool ret = true;
		for(auto x=0; x<getRows(); x++) {
			for(auto y=0; y<getCols(); y++) {
				if(get(x, y) != get(x, y)) {
					ret = false;
					break;
				}
			}
			if(!ret)
				break;
		}
		return ret;
	}

	T& get(size_t m, size_t n) {return data[m][n];};
	const T& get(size_t m, size_t n) const {return data[m][n];};

	T& getTransposed(size_t m, size_t n) {return data[n][m];};
	const T& getTransposed(size_t m, size_t n) const {return data[n][m];};

	std::vector<std::vector<T>>& getData() {return data;}
	const std::vector<std::vector<T>>& getData() const {return data;}

	size_t getRows() const {return rows;};
	size_t getCols() const {return cols;};

	void swap_rows(size_t a, size_t b) {std::iter_swap(data.begin() + a, data.begin() + b);}

	// Really inefficient transpose implementation
	Matrix<T> getTransposed() {
		Matrix<T> retval(getCols(), getRows());
		for(size_t m=0; m<getRows(); m++) {
			for(size_t n=0; n<getCols(); n++)
				retval.getTransposed(m, n) = get(m, n);
		}

		return retval;
	}


	/**
	* Outputs the matrix on stdout
	*/
	void output() const {
		std::cout << "-------" << std::endl;
		for(auto v : data) {
			for(auto i : v) {
				std::cout << i << " \t";
			}
			std::cout << std::endl;
		}
	}

	/**
	* Writes a matrix to a file
	*/
	void writeToFile(std::string filename) const {
		/**std::ofstream file(filename);
		file << rows << "|" << cols << '|' << '\n';
		for(auto v: data) {
			for(auto i: v) {
				file << i << '|';
			}
			file << '\n';
		}
		file.close();*/

		std::ofstream file;
		file.open(filename, std::ios::out | std::ios::binary);
		file.write(reinterpret_cast<const char*>(&rows), sizeof(size_t));
		file.write(reinterpret_cast<const char*>(&cols), sizeof(size_t));
		for(auto& i : data) {
			file.write(reinterpret_cast<const char*>(i.data()), cols*sizeof(T));
		}
		file.close();
	}

	/**
	* Reads the filecontents into the matrix
	* The file is assumed to be written by writeToFile, everything else will crash
	*/
	void readFromFile(std::string filename) {
		/**std::ifstream file(filename);
		data.clear();
		std::string line;

		// Read Metadata
		std::getline(file, line);
		std::vector<T> v = splitLine(line);
		rows = v[0];
		cols = v[1];

		// Read data
		while(std::getline(file, line)) {
			v = splitLine(line);
			if(v.size() != cols)
				throw std::runtime_error("Mismatching column size and metadata");
			data.push_back(v);
		}

		if(data.size() != rows)
			throw std::runtime_error("Mismatching row size and metadata");*/

		std::ifstream file;
		file.open(filename, std::ios::in | std::ios::binary);
		if(!file) {
			std::cerr << "Could not open " << filename << ", leaving matrix empty" << std::endl;
			return;
		}
		file.read(reinterpret_cast<char*>(&rows), sizeof(size_t));
		file.read(reinterpret_cast<char*>(&cols), sizeof(size_t));
		data.clear();
		for(int i=0; i<rows; i++) {
			std::vector<T> tmp(cols, 0);
			file.read(reinterpret_cast<char*>(tmp.data()), cols*sizeof(T));
			data.push_back(tmp);
		}
		file.close();
	};

	static std::pair<size_t, size_t> readMetadata(std::string filename) {
		size_t rows, cols;
		std::ifstream file;
		file.open(filename, std::ios::in | std::ios::binary);
		if(!file) {
			std::cerr << "Could not open " << filename << ", leaving matrix empty" << std::endl;
			return std::pair<size_t, size_t>(rows, cols);
		}
		file.read(reinterpret_cast<char*>(&rows), sizeof(size_t));
		file.read(reinterpret_cast<char*>(&cols), sizeof(size_t));
		file.close();
		return std::pair<size_t, size_t>(rows, cols);
	}

	static T* readCStyle(std::string filename, size_t trailing_zeroes = 0) {
		size_t rows, cols;
		std::ifstream file;
		file.open(filename, std::ios::in | std::ios::binary);
		if(!file) {
			std::cerr << "Could not open " << filename << ", leaving matrix empty" << std::endl;
			return NULL;
		}
		file.read(reinterpret_cast<char*>(&rows), sizeof(size_t));
		file.read(reinterpret_cast<char*>(&cols), sizeof(size_t));
		T* retval = new T[rows * cols + trailing_zeroes];
		file.read(reinterpret_cast<char*>(retval), sizeof(T) * rows * cols);
		file.close();
		return retval;
	}

	void addZeroRows(size_t count) {
		for(int i=0; i<count; i++)
			data.push_back(std::vector<T>(cols, 0));
		rows += count;
	}

	/**
	* Construct a mxn matrix filled with zeros
	*/
	Matrix(size_t m, size_t n)
		: data(m, std::vector<T>(n, 0)),
		rows(m),
		cols(n)
	{}

	/**
	* Read a matrix from file
	*/
	Matrix(std::string filename)
		: data(), rows(0), cols(0)
	{
		readFromFile(filename);
	}

	/**
	* Read a matrix from memory
	*/
	Matrix(T* raw_data, size_t m, size_t n)
		: data(), rows(m), cols(n)
	{
		for(int i=0; i<rows; i++) {
			data.push_back(std::vector<T>(raw_data + (i * n), raw_data + ((i+1) * n)));
		}
	}

	/**
	* Copy constructor
	*/
	Matrix(const Matrix& rhs)
		: rows(rhs.rows),
		cols(rhs.cols)
	{
		for(auto v : rhs.data) {
			std::vector<T> tmp;
			for(auto i : v) {
				tmp.push_back(i);
			}
			data.push_back(tmp);
		}
	}

private:

	// Splits a line in the format number | number | number ...
	static std::vector<T> splitLine(std::string line) {
		std::vector<T> res;
		std::stringstream tmp;
		for(auto c : line) {
			if(c != '|')
				tmp << c;
			else {
				T tmpval;
				tmp >> tmpval;
				tmp.clear();
				res.push_back(tmpval);
			}
		}
		return res;
	}

	Matrix() {}

	size_t rows;
	size_t cols;
	std::vector<std::vector<T>> data;
};

