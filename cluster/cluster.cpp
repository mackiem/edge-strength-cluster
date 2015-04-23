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
#include <numeric>

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
    //std::cout << mat << std::endl;
}

struct Color {
    int i;
    float strength;

    bool operator<(const Color& other) const {
        return i < other.i;
    };

    bool operator==(const Color& other) const {
        return i == other.i;
    };

};

//struct Cluster {
//	std::set<Color> colors;
//};

typedef std::set<Color> Cluster;

typedef std::vector<Cluster> Clusters;

Cluster adjust_edge_strength(const Cluster& cluster, const cv::Mat& edges) {
    Cluster adjusted_cluster;
    for (auto& color : cluster) {
        std::vector<float> accum;
        cv::Mat row = edges.row(color.i);
        for (auto& inner_color : cluster) {
            if (color.i != inner_color.i) {
                accum.push_back(row.at<float>(0, inner_color.i));
            }
        }
        float sum = std::accumulate(std::begin(accum), std::end(accum), 0.f);
        Color adjusted_color = color;
        adjusted_color.strength = sum;
        adjusted_cluster.insert(adjusted_color);
    }
    return adjusted_cluster;
}

void initialize(Clusters& clusters, int k, const cv::Mat& edges) {
    std::priority_queue<Color> edge_sums;
    for (int i = 0; i < edges.rows; ++i) {
        Color color;
        color.i = i;
        color.strength = cv::sum(edges.row(i))[0];
        edge_sums.push(color);
    }

    clusters.resize(k);

    for (int i = 0; i < edges.rows; ++i) {
        clusters[i % k].insert(edge_sums.top());
        edge_sums.pop();
    }

    for (auto& cluster : clusters) {
        cluster = adjust_edge_strength(cluster, edges);
    }
}

void mean_std_dev(std::vector<float> v, float& mean, float& std_dev) {
    float sum = std::accumulate(std::begin(v), std::end(v), 0.f);

	if (v.size() > 0) {
		mean = sum / v.size();

		float accum = 0.0;
		std::for_each(std::begin(v), std::end(v), [&](const double d) {
			accum += (d - mean) * (d - mean);
		});

		std_dev = sqrt(accum / (v.size()));
	}
	else {
		mean = 0.f;
		std_dev = 0.f;
	}
}

float calc_score(float mean, float stddev) {
	//std::cout << "mean : " << mean << " std dev : " << stddev << std::endl;
    const float alpha = 0.5f;
	const float max_mean = 255.f;
	const float max_stddev = 5.f;
    float mean_part = alpha * (max_mean - mean) / max_mean;
    float std_dev_part = (1 - alpha) * stddev / max_stddev;
	std::cout << "mean part: " << mean_part << " std dev part : " << std_dev_part << std::endl;
    float score = mean_part + std_dev_part;
    return score;
}

float eval_color_score(const Cluster& cluster, const cv::Mat& edges) {
    std::vector<float> color_mean_accum;
    for (auto& color : cluster) {
        std::vector<float> accum;
        cv::Mat row = edges.row(color.i);
        for (auto& inner_color : cluster) {
            if (color.i != inner_color.i) {
                accum.push_back(row.at<float>(0, inner_color.i));
            }
        }
        float mean, stddev;
        mean_std_dev(accum, mean, stddev);
        color_mean_accum.push_back(mean);
    }
    float cluster_mean, cluster_stddev;
    mean_std_dev(color_mean_accum, cluster_mean, cluster_stddev);
    return calc_score(cluster_mean, cluster_stddev);
}

float eval_score(const Clusters& clusters, const cv::Mat& edges) {
    float score = 0.f;
    for (auto& cluster : clusters) {
        score += eval_color_score(cluster, edges);
    }
    return score;
}

Color remove_color(Cluster& cluster) {
	auto min_itr = std::min_element(cluster.begin(), cluster.end(), [&](const Color& left, const Color& right) {
		return left.strength < right.strength;
	});
	auto color = *min_itr;
	cluster.erase(color);
	return color;
}

unsigned int total_cluster_size(const Clusters& clusters, bool print = false) {
	int total = 0;
	if (print) std::cout << "Cluster sizes -> total : ";
	for (auto& inner_cluster_r : clusters) {
		if (print) std::cout << inner_cluster_r.size() << ", ";
		total += inner_cluster_r.size();
	}
	if (print) std::cout << " -> " << total << std::endl;
	return total;
}

Clusters cluster(cv::Mat& edges, int k) {
    Clusters clusters;
    initialize(clusters, k, edges);

	int init_total = total_cluster_size(clusters, true);
	

    // for a number of iter do
    // eval score and store in curr_score
    // naive: pick lowest and put it in another cluster
    // eval score and if greater than curr_score
    int iter_num = 100;
    for (int i = 0; i < iter_num; ++i) {
        float score = eval_score(clusters, edges);
		std::cout << "init score : " << score << std::endl;
        float min_score = 1e6;
        Clusters min_clusters = clusters;

        // pick the lowest, and shuffle it around
		for (auto clusterItr = clusters.begin(); clusterItr != clusters.end(); ) {
			auto& cluster = *clusterItr;
            min_score = eval_score(clusters, edges);
            float curr_score = min_score;
			auto lowest_color = remove_color(cluster);

            for (auto& inner_cluster : clusters) {
				//std::cout << "after removing color : "; total_cluster_size(clusters, true);
                if (cluster != inner_cluster) {
                    auto iterator_pair = inner_cluster.insert(lowest_color);
					if (!iterator_pair.second) {
						assert(!iterator_pair.second);
					}
                    auto color_pos = *iterator_pair.first;
                    //auto color_pos = iterator_pair;
					float new_score = eval_score(clusters, edges);
					if (cluster.size() == 1) {
						eval_score(clusters, edges);
					}
					//std::cout << "new score : " << new_score << " color used : " << lowest_color.i << std::endl;
					if (new_score < 0 && new_score > 1000 && new_score != new_score) {
						assert(new_score > 0);
					}
                    if (new_score < min_score) {
                        min_clusters = clusters;
                        min_score = new_score;
                    }

					inner_cluster.erase(color_pos);
					//std::cout << "after color shuffling : ";
					//int total = total_cluster_size(clusters, true);
				}
			}
			if (min_score == curr_score) {
				// TODO : more robust check, check if removed color is present in another cluster
				cluster.insert(lowest_color);
				++clusterItr;
			}
			else {
				clusters = min_clusters;
				clusterItr = clusters.begin();
			}
			int total = total_cluster_size(clusters, false);
			if (total != init_total) {
				assert(total == init_total);
			}
        }
        clusters = min_clusters;
        std::cout << score << std::endl;
    }

	total_cluster_size(clusters, true);
    return clusters;
}

int _tmain(int argc, _TCHAR* argv[])
{
    cv::Mat edges(27, 27, CV_32FC1);
    read_csv("edge-strength-matrix.csv", 1, edges);
    auto clusters = cluster(edges, 3);
	int i = 1;
	for (auto& cluster : clusters) {
		adjust_edge_strength(cluster, edges);
		std::cout << "Cluster " << i << " : " << std::endl;
		for (auto& color : cluster) {
			std::cout << color.i << ", " << color.strength << std::endl;
		}
		std::cout << std::endl;
		++i;
	}
    //std::cout << edges << std::endl;
    return 0;
}

