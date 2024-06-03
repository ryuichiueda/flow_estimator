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

class Map {
private:
	vector<uint8_t> data_;
public:
	Map() {}
	~Map() {}
	unsigned int width_;
	unsigned int height_;
	unsigned int depth_;

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

	bool load_from_pgm(ifstream *ifs)
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

	void setValue(int index, int value) {
		this->data_[index] = value;
	}

	int xyToIndex(int x, int y) {
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

	int xyToValue(int x, int y) {
		int index = xyToIndex(x, y);
		if (index < 0) {
			return 0;
		}else {
			return this->data_[index];
		}
	}
};

class Motion {
public:
	double velocity;
	double direction;

	Pos move(Pos *from, double time) {
		Pos ans;
		ans.x = from->x + time*this->velocity*cos(this->direction);
		ans.y = from->y + time*this->velocity*sin(this->direction);
		return ans;
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

int main(int argc, char *argv[])
{
	if(argc != 3) {
		cerr << "Invalid args" << endl;
		return 1;
	}

        Map map_before, map_current;

	ifstream before(argv[1]);
	ifstream current(argv[2]);
	if (not before or not current) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not map_before.load_from_pgm(&before) 
	  or not map_current.load_from_pgm(&current)) {
		return 1;
	}

	vector<Particle> sample_before;
	map_before.sampling(50, &sample_before);

	vector<Motion> motions;
	Motion::sampling(50, &motions);

	vector<int> vote(map_before.width_*map_before.height_, 0);

	for(auto &from: sample_before) {
		for(auto &m: motions) {
			Pos current = m.move(&from.pos, 1.0);

			//int pos = map_current.xyToIndex((int)current.x, (int)current.y);

			double weight = map_current.xyToValue((int)current.x, (int)current.y);

			Pos after = m.move(&from.pos, 2.0);
			int pos2 = map_before.xyToIndex((int)after.x, (int)after.y);
			if (pos2 >= 0){
				vote[pos2] += weight;
			}
		}
	}

	Map ans = map_before;
	//ans.data_.clear();
	int i = 0;
	for(double v : vote) {
//		if (v > distribution_after.depth_) {
//			ans.data_.push_back(distribution_after.depth_);
//		}else{
			ans.setValue(i++, (int)v);
		//	ans.data_.push_back((int)v);
//		}
	}

	ans.print();

	return 0;
}
