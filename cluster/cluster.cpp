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

	bool operator<(const Color& other) const {
		return strength > other.strength;
	};

};

struct Cluster {
	std::set<Color> colors;
};

typedef std::vector<Cluster> Clusters;


void initialize(Clusters& clusters, int k, const cv::Mat& edges) {
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

Clusters cluster(cv::Mat& edges, int k) {
	Clusters clusters;
	initialize(clusters, k, edges);
	return clusters;

	// for a number of iter do
	// eval score and store in curr_score
	// naive: pick lowest and put it in another cluster
	// eval score and if greater than curr_score
}

int _tmain(int argc, _TCHAR* argv[])
{
	cv::Mat edges(27, 27, CV_32FC1);
	read_csv("edge-strength-matrix.csv", 1, edges);
	auto clusters = cluster(edges, 3);
	//std::cout << edges << std::endl;
	return 0;
}

