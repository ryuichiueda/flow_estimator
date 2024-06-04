#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdint>
#include <climits>
#include <cmath>
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

	Map(int w, int h, int d) : width_(w), height_(h), depth_(d) {
		data_.assign(w*h, 0);
	}

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

	void getNeighborDistribution(int x, int y, vector<double> *dist) {
		const int range = 2;
		for(int ix = -range+x; ix <= range+x ; ix++ ){
			if (ix < 0 || ix >= this->width_ ) {
				for(int iy = -range+y; iy <= range+y ; iy++ )
					dist->push_back(0.0);

				continue;
			}

			for(int iy = -range+y; iy <= range+y ; iy++ ){
				if (iy < 0 || iy >= this->height_)
					dist->push_back(0.0);
				else
					dist->push_back(this->data_[ix + iy*this->width_]);
			}

		}

		double sum = 0.0;
		for(auto &p: *dist)
			if(p > 0.0)
				sum += p;

		for(auto &p: *dist)
			if(p > 0.0)
				p /= sum;
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

double kld(vector<double> &before, vector<double> &current) {
	double ans = 0.0;
	for(int i=0; i<before.size(); i++) {
		if (before[i] < 0.0 || current[i] < 0.0)
			continue;
			
		ans += current[i]*(log(current[i]) - log(before[i]));
	}
	return ans;
}

double rms(vector<double> &before, vector<double> &current) {
	double square_sum = 0.0;
	for(int i=0; i<before.size(); i++) {
		double diff = current[i] - before[i];
		square_sum += diff*diff;
	}
	return sqrt(square_sum/before.size());
}

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

	vector<double> vote(map_before.width_*map_before.height_, 0.0);

	for(auto &from: sample_before) {
		for(auto &m: motions) {
			Pos current = m.move(&from.pos, 1.0);

			vector<double> before_neigh, current_neigh;
			map_before.getNeighborDistribution((int)from.pos.x, (int)from.pos.y, &before_neigh);
			map_current.getNeighborDistribution((int)current.x, (int)current.y, &current_neigh);

			cerr <<  rms(before_neigh, current_neigh) << " ";

			double weight = (1.0 - rms(before_neigh, current_neigh))
				* map_current.xyToValue((int)current.x, (int)current.y);

			//cerr << weight << " ";
			Pos after = m.move(&from.pos, 2.0);
			int pos = map_before.xyToIndex((int)after.x, (int)after.y);
			if (pos >= 0){
				vote[pos] += weight;
			}
		}
	}

	Map ans(map_current.width_, map_current.height_, map_current.depth_);
	int i = 0;
	for(double v : vote) {
		ans.setValue(i++, (int)v);
	}

	ans.print();

	return 0;
}
