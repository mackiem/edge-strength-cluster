// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <stdexcept>
#include <fstream>
#include <set>

void read_csv(const char* filename, int row_offset, cv::Mat& mat) {
	std::ifstream file(filename);
	if (!file.good()) {
		throw std::runtime_error("File does not exist");
	}

	std::string line;
	int no_of_rows = 27;
	
	int row = row_offset;
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string val;
		int col = 0;
		while (std::getline(ss, val, ',')) {
			mat.at<float>(row, col++) = std::stof(val);
		}
		row++;
	}
}

struct Cluster {
	std::set<int> colors;
};

void cluster(cv::Mat& mat, int k) {

}

int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

