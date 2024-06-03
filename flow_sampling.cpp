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

struct Particle {
	int index;
	Pos pos;
};

Pos toRandomXyPos(int pos_on_data, int width) {
       int x = pos_on_data % width;
       int y = pos_on_data / width;

       Pos ans;
       ans.x = (double)x + uniform_rand();
       ans.y = (double)y + uniform_rand();

       return ans;
}

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

	int xyToDataPos(int x, int y) {
		if (x < 0 || x >= this->width_) {
			return -1;
		}
		if (y < 0 || y >= this->height_) {
			return -1;
		}
	
		return x + y*this->width_;
	}

	Pos toRandomXyPos(int pos_on_data) {
		int x = pos_on_data % this->width_;
		int y = pos_on_data / this->width_;
	
		Pos ans;
		ans.x = (double)x + uniform_rand();
		ans.y = (double)y + uniform_rand();
	
		return ans;
	}

	void sampling(unsigned int num, vector<Particle> *sample)
	{
		uint64_t sum = reduce(begin(this->data_), end(this->data_));
		double step = (double)sum/num;
		double initial_shift = step*uniform_rand();
		uint64_t accum = this->data_[0];
		int j = 0;
		for(int i=0;i<num;i++){
			uint64_t tick = (uint64_t)(i*step + initial_shift);
	
			if(tick < accum) {
				sample->push_back({j, toRandomXyPos(j)});
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

class Motion {
public:
	double velocity;
	double direction;

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

	static void sampling(unsigned int num, vector<Motion> *sample) {
		double step = M_PI*2/num;
		double initial_shift = step*uniform_rand();

		const double max_speed = 3.0;
	
		for(int i=0; i<num; i++) {
			sample->push_back({max_speed*uniform_rand(), initial_shift + i*step});
		}
	}
};


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

        Image map_before, map_current;

	ifstream before(argv[1]);
	ifstream current(argv[2]);
	if (not before or not current) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not map_before.load(&before) 
	  or not map_current.load(&current)) {
		return 1;
	}

	vector<Particle> sample_before;
	map_before.sampling(50, &sample_before);

	vector<Motion> motions;
	Motion::sampling(50, &motions);

	vector<int> vote(map_before.width_*map_before.height_, 0);

	for(auto &from: sample_before) {
		for(auto &m: motions) {
			Pos current;
			current.x = from.pos.x + m.velocity*cos(m.direction);
			current.y = from.pos.y + m.velocity*sin(m.direction);

			int pos = map_current.xyToDataPos((int)current.x, (int)current.y);

			double weight = 0.0;
			if (0 <= current.x and current.x < map_current.width_ 
			and 0 <= current.y and current.y < map_current.height_ ) {
				weight = (double)map_current.data_[pos];
			}

			Pos after;
			after.x = from.pos.x + 2*m.velocity*cos(m.direction);
			after.y = from.pos.y + 2*m.velocity*sin(m.direction);
			int pos2 = xyToDataPos((int)after.x, (int)after.y,
				map_before.width_, map_before.height_);

			if (0 <= after.x and after.x < map_current.width_ 
			and 0 <= after.y and after.y < map_current.height_ ) {
				vote[pos2] += weight;
			}
		}
	}

	Image ans = map_before;
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
