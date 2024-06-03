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

	void sampling(int num, vector<int> *sample)
	{
		if (num <= 0) {
			cerr << "Invalid sample num" << endl;
			exit(1);
		}
	
		uint64_t sum = reduce(begin(this->data_), end(this->data_));
		double step = (double)sum/num;
		//cerr << "STEP: " << step << endl;
	
		double initial_shift = step*uniform_rand();
		//cerr << initial_shift << endl;
	
		uint64_t accum = this->data_[0];
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
				if (j >= this->data_.size()) {
					cerr << "Overflow at sampling" << endl;
					exit(1);
				}
				accum += this->data_[j];
			}
		}
	}
};

void direction_sampling(int num, vector<double> *sample) {
	if (num <= 0) {
		cerr << "Invalid sample num (in direction_sampling)" << endl;
		exit(1);
	}

	double step = M_PI*2/num;
	double initial_shift = step*uniform_rand();

	for(int i=0; i<num; i++) {
		sample->push_back(initial_shift + i*step);
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

        Image distribution_before, distribution_current;

	ifstream before(argv[1]);
	ifstream current(argv[2]);
	if (not before or not current) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not distribution_before.load(&before) 
	  or not distribution_current.load(&current)) {
		return 1;
	}

	const int before_pos_sample_num = 50;
	vector<int> sample_index_before;
	vector<Pos> sample_xy_before;

	distribution_before.sampling(before_pos_sample_num, &sample_index_before);
	for(auto &p: sample_index_before){
		sample_xy_before.push_back(toRandomXyPos(p, distribution_before.width_));
	}

	/*
	for(auto &p: sample_xy_before) {
		cout << p.x << "\t" << p.y << endl;
	}*/

	vector<double> sampled_motions, sampled_directions;

	const int motion_sample_num = 50;
	const double max_speed = 3.0;
	direction_sampling(motion_sample_num, &sampled_directions);
	for(int i=0; i<motion_sample_num; i++){
		sampled_motions.push_back(uniform_rand()*max_speed);

	//	cout << sampled_motions[i] << " " << sampled_directions[i]*180/M_PI << endl;
	}

	vector<int> vote(distribution_before.width_*distribution_before.height_, 0);

	for(auto &from: sample_xy_before) {
		for(int i=0; i<motion_sample_num; i++){
			Pos current;
			double move = sampled_motions[i];
			double theta = sampled_directions[i];
			current.x = from.x + move*cos(theta);
			current.y = from.y + move*sin(theta);

			int pos = xyToDataPos((int)current.x, (int)current.y,
				distribution_before.width_, distribution_before.height_);

			double weight = 0.0;
			if (0 <= current.x and current.x < distribution_current.width_ 
			and 0 <= current.y and current.y < distribution_current.height_ ) {
				weight = (double)distribution_current.data_[pos];
			}

			Pos after;
			after.x = from.x + 2*move*cos(theta);
			after.y = from.y + 2*move*sin(theta);
			int pos2 = xyToDataPos((int)after.x, (int)after.y,
				distribution_before.width_, distribution_before.height_);

			if (0 <= after.x and after.x < distribution_current.width_ 
			and 0 <= after.y and after.y < distribution_current.height_ ) {
				vote[pos2] += weight;
			}
		}
	}

	Image ans = distribution_before;
	ans.data_.clear();
	for(double v : vote) {
//		if (v > distribution_after.depth_) {
//			ans.data_.push_back(distribution_after.depth_);
//		}else{
			ans.data_.push_back((int)v);
//		}
	}

	ans.print();

	return 0;
}
