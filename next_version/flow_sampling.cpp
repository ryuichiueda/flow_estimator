#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdint>
#include <climits>
#include <numeric>
#include <random>

using namespace std;

const uint64_t rand_max = mt19937_64::max();
random_device seed_gen;
mt19937_64 rand_u64(seed_gen());

double uniform_rand(){
	return (double)rand_u64()/rand_max;
}

struct Pos {
	double x;
	double y;
};

class Image {
public:
	Image() {}
	~Image() {}
	unsigned int width_;
	unsigned int height_;
	unsigned int depth_;
	vector<uint8_t> data_;

	void print()
	{
		cout << "P2" << endl;
		cout << width_ << " " << height_ << endl;
		cout << depth_ << endl;
		for(int y=0;y<height_;y++) {
			for(int x=0;x<width_;x++) {
				cout << setw(4) << (int)data_[x + y*width_];
			}
			cout << endl;
		}
	}

	bool load(ifstream *ifs)
	{
		string buf;
		int counter = 0;
	
		while(not ifs->eof()) {
			*ifs >> buf;
	
			if(counter == 0){
				if(buf != "P2") {
					cerr << "Not supprted format" << endl;
					return false;
				}
			}else if(counter == 1) {
				width_ = stoi(buf);
			}else if(counter == 2) {
				height_ = stoi(buf);
			}else if(counter == 3) {
				depth_ = stoi(buf);
			}else{
				data_.push_back(stoi(buf));
			}
			counter++;
		}
		return true;
	}
};

void sampling(Image *image, int num, vector<int> *sample)
{
	if (num <= 0) {
		cerr << "Invalid sample num" << endl;
		exit(1);
	}

	uint64_t sum = reduce(begin(image->data_), end(image->data_));
	double step = (double)sum/num;
	//cerr << "STEP: " << step << endl;

	double initial_shift = step*uniform_rand();
	//cerr << initial_shift << endl;

	uint64_t accum = image->data_[0];
	int j = 0;
	for(int i=0;i<num;i++){
		uint64_t tick = (uint64_t)(i*step + initial_shift);
		//cerr << "tick " << tick << " accum " << accum << endl;

		if(tick < accum) {
			sample->push_back(j);
			continue;
		}

		while(tick >= accum) {
			j += 1;
			if (j >= image->data_.size()) {
				cerr << "Overflow at sampling" << endl;
				exit(1);
			}
			accum += image->data_[j];
		}
	}
}


Pos toRandomXyPos(int pos_on_data, int width) {
	int x = pos_on_data % width;
	int y = pos_on_data / width;

	Pos ans;
	ans.x = (double)x + uniform_rand();
	ans.y = (double)y + uniform_rand();

	return ans;
}

int xyToDataPos(int x, int y, int width, int height) {
	if (x < 0 || x >= width) {
		return -1;
	}
	if (y < 0 || y >= height) {
		return -1;
	}

	return x + y*width;
}

int main(int argc, char *argv[])
{
	if(argc != 3) {
		cerr << "Invalid args" << endl;
		return 1;
	}

        Image distribution_before, distribution_after;

	ifstream before(argv[1]);
	ifstream after(argv[2]);
	if (not before or not after) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not distribution_before.load(&before) 
	  or not distribution_after.load(&after)) {
		return 1;
	}

	const int sample_num = 50;
	vector<int> sample_index_before;
	vector<Pos> sample_xy_before;

	sampling(&distribution_before, sample_num, &sample_index_before);
	for(auto p: sample_index_before){
		sample_xy_before.push_back(toRandomXyPos(p, distribution_before.width_));
	}

	for(auto p: sample_xy_before) {
		cout << p.x << "\t" << p.y << endl;
	}
	//distribution_before.print();

	/*
	const int sample_num = 50;
	vector<int> sample_before, sample_after;
	vector<Pos> sample_before_xy, sample_after_xy;

	sampling(&distribution_before, sample_num, &sample_before);
	for(auto p : sample_before){
		sample_before_xy.push_back(toRandomXyPos(p, distribution_before.width_));
	}

	sampling(&distribution_after, sample_num, &sample_after);
	for(auto p : sample_after){
		sample_after_xy.push_back(toRandomXyPos(p, distribution_after.width_));
	}

	uint64_t pixel_sum = reduce(begin(distribution_after.data_), end(distribution_after.data_));
	double sample_weight = (double)pixel_sum/sample_num;

	vector<double> vote(distribution_before.width_*distribution_before.height_, 0.0);
	for(int i=0;i<sample_num;i++){
		double new_x = 2*sample_after_xy[i].x - sample_before_xy[i].x;
		double new_y = 2*sample_after_xy[i].y - sample_before_xy[i].y;

		int pos = xyToDataPos((int)new_x, (int)new_y,
				distribution_before.width_, distribution_before.height_);

		vote[pos] += sample_weight;
	}

	Image ans = distribution_before;
	ans.data_.clear();
	for(double v : vote) {
		if (v > distribution_after.depth_) {
			ans.data_.push_back(distribution_after.depth_);
		}else{
			ans.data_.push_back((int)v);
		}
	}

	ans.print();
	*/

	return 0;
}
