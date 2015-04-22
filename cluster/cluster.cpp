// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <stdexcept>
#include <fstream>
#include <set>
#include <vector>
#include <iostream>
#include <queue>

void read_csv(const char* filename, int row_offset, cv::Mat& mat) {
	std::ifstream file(filename);
	if (!file.good()) {
		throw std::runtime_error("File does not exist");
	}

	std::string line;
	int no_of_rows = 27;
	
	int row = 0;
	int line_no = 0;
	while (std::getline(file, line)) {
		if (line_no++ < row_offset) {
			continue;
		}

		std::stringstream ss(line);
		std::string val;
		int col = 0;
		while (std::getline(ss, val, ',')) {
			float fval = std::stof(val);
			mat.at<float>(row, col++) = fval;
		}
		row++;
	}
	std::cout << mat << std::endl;
}

struct Color {
	int i;
	float strength;

	bool operator()(const Color& left, const Color& right) {
		return left.strength > right.strength;
	};

};

struct Cluster {
	std::set<Color> colors;
};

typedef std::vector<Cluster> Clusters;


void initialize(Clusters clusters, int k, cv::Mat& edges) {
	std::priority_queue<Color> edge_sums;
	for (int i = 0; i < edges.rows; ++i) {
		Color color;
		color.i = 0;
		color.strength = cv::sum(edges.row(i))[0];
		edge_sums.push(color);
	}

	clusters.resize(k);

	for (int i = 0; i < edges.rows; ++i) {
		clusters[i % k].colors.insert(edge_sums.top());
		edge_sums.pop();
	}
}

void cluster(cv::Mat& edges, int k) {
	Clusters clusters;
	initialize(clusters, k, edges);

	// for a number of iter do
	// eval 
}

int _tmain(int argc, _TCHAR* argv[])
{
	cv::Mat edges(27, 27, CV_32FC1);
	read_csv("edge-strength-matrix.csv", 1, edges);
	//std::cout << edges << std::endl;
	return 0;
}

