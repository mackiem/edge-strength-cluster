
class OSLOptimizer {
public:
	void read_csv(const char* filename, int row_offset, cv::Mat& mat);
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
	typedef std::set<Color> Cluster;
	typedef std::vector<Cluster> Clusters;
	void mean_std_dev(std::vector<float> v, float& mean, float& std_dev);
	Cluster adjust_edge_strength(const Cluster& cluster, const cv::Mat& edges);
	void initialize(Clusters& clusters, int k, const cv::Mat& edges);
	float calc_score(float mean, float stddev);
	float eval_color_score(const Cluster& cluster, const cv::Mat& edges);
	float eval_score(const Clusters& clusters, const cv::Mat& edges);
	Color remove_color(Cluster& cluster);
	unsigned int total_cluster_size(const Clusters& clusters, bool print = false);
	Clusters cluster(cv::Mat& edges, int k);
};