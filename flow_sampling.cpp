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
//	Pos pos;
};

class Map {
public:
	Map() {}

	Map(int w, int h, int d) : width_(w), height_(h), depth_(d) {
		data_.assign(w*h, 0);
	}

	~Map() {}
	unsigned int width_;
	unsigned int height_;
	unsigned int depth_;
	vector<uint64_t> data_;

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

	void normalize(void) {
		auto max = 0;
		for(int i=0; i<this->data_.size(); i++){
			max = max > data_[i] ? max : data_[i];
		}
		for(int i=0; i<this->data_.size(); i++){
			data_[i] = (int)(255 * data_[i] / max); 
		}
	
	}

	void setValue(int index, int value) {
		this->data_[index] = value;
	}

	void removeFixedObstacle(Map *fixed) {
		for(int i=0;i<fixed->data_.size();i++) {
			if(fixed->data_[i] > 128){
				this->data_[i] = 0;
			}
		}
	}

	int xyToIndex(int x, int y) {
		if (x < 0 || x >= this->width_) {
			return -10000;
		}
		if (y < 0 || y >= this->height_) {
			return -10000;
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
			while(tick >= accum) {
				j += 1;
				if (j >= this->data_.size()) {
					cerr << "Overflow at sampling" << endl;
					exit(1);
				}
				accum += this->data_[j];
			}
			//sample->push_back({j, toRandomXyPos(j)});
			sample->push_back({j});
			continue;
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
	double dx_;
	double dy_;
	double weight_;

	Pos move(Pos *from, double time) {
		Pos ans;
		ans.x = from->x + time*this->dx_;
		ans.y = from->y + time*this->dy_;
		return ans;
	}

	static void sampling(vector<Motion> *sample) {
		const int max_speed = 10;

		for(int ix=-max_speed; ix<=max_speed; ix++){
			for(int iy=-max_speed; iy<=max_speed; iy++){
				double dx = ix + uniform_rand() - 0.5;
				double dy = iy + uniform_rand() - 0.5;
				//double w = 1.0/sqrt(dx*dx + dy*dy + 1.0);
				double w = 1.0;///sqrt(dx*dx + dy*dy + 1.0);
				sample->push_back({dx, dy, w});
			}
		}
	}

	void print(void){
		cout << dx_ << " " << dy_ << " " << weight_ << endl;
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

void one_step(Map &origin) {
}

int main(int argc, char *argv[])
{
	ifstream fixed(argv[1]);
	ifstream origin(argv[2]);
	if (not fixed or not origin) {
		cerr << "Invalid files" << endl;
		return 1;
	}
        Map map_fixed, map_origin;
	if ( not map_fixed.load_from_pgm(&fixed) 
	  or not map_origin.load_from_pgm(&origin) ){
		cerr << "Cannot load image" << endl;
		return 1;
	}
	map_origin.removeFixedObstacle(&map_fixed);
	vector<Particle> sample_origin;
	map_origin.sampling(100, &sample_origin);


	for(int i=3;i<argc;i++) {
		ifstream ifs(argv[i]);
        	Map map_update;
		map_update.load_from_pgm(&ifs);
		map_update.removeFixedObstacle(&map_fixed);

	}	
	/*
	if(argc != 5) {
		cerr << "Invalid args" << endl;
		return 1;
	}

        Map map_before, map_current, map_verify, map_fixed;

	ifstream before(argv[1]);
	ifstream current(argv[2]);
	ifstream verify(argv[3]);
	ifstream fixed(argv[4]);
	if (not before or not current or not verify or not fixed) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not map_before.load_from_pgm(&before) 
	  or not map_current.load_from_pgm(&current) 
	  or not map_verify.load_from_pgm(&verify) 
	  or not map_fixed.load_from_pgm(&fixed)) {
		return 1;
	}

	map_before.removeFixedObstacle(&map_fixed);
	map_current.removeFixedObstacle(&map_fixed);

	vector<Particle> sample_before;
	map_before.sampling(100, &sample_before);

	vector<Motion> motions;
	Motion::sampling(&motions);

	vector<double> vote(map_before.width_*map_before.height_, 0.0);

	for(auto &from: sample_before) {
		vector<double> weights;
		for(auto &m: motions) {
			Pos current = m.move(&from.pos, 1.0);

			double w_center = map_current.xyToValue((int)floor(current.x), (int)floor(current.y));
			if(w_center == 0){
				weights.push_back(0.0);
				continue;
			}

			vector<double> before_neigh, current_neigh;
			map_before.getNeighborDistribution((int)floor(from.pos.x), (int)floor(from.pos.y), &before_neigh);
			map_current.getNeighborDistribution((int)floor(current.x), (int)floor(current.y), &current_neigh);

			double weight = (1.0 - rms(before_neigh, current_neigh))*w_center;
			weights.push_back(weight * m.weight_);
		}

		double sum = reduce(begin(weights), end(weights));
		if (sum <= 0.000001 ){
			for(int j=0; j<weights.size(); j++)
				weights[j] = 1.0;

			sum = (double)weights.size();
		}

		int i = 0;
		for(auto &m: motions) {
			Pos after = m.move(&from.pos, 2.0);
			int pos = map_before.xyToIndex((int)floor(after.x), (int)floor(after.y));
			weights[i] *= map_verify.data_[pos];
			i++;
		}


		for(double s=2.0; s<=10.0; s+=0.1){
			int i = 0;
			for(auto &m: motions) {
				Pos after = m.move(&from.pos, (double)s);
				int pos = map_before.xyToIndex((int)floor(after.x), (int)floor(after.y));
				if (pos >= 0 && pos < map_before.width_*map_before.height_){
					if (vote[pos] < weights[i]/sum)
						vote[pos] += weights[i]/sum;
				}
				i++;
			}
		}
	}

	Map ans(map_current.width_, map_current.height_, map_current.depth_);
	uint64_t all_weights = reduce(begin(map_current.data_), end(map_current.data_));
	double voted_weight = reduce(begin(vote), end(vote));
	double w = (double)all_weights/voted_weight;
	int i = 0;
	for(double v : vote) {
		ans.setValue(i++, (int)(10000*v));
	}

	ans.normalize();
	ans.print();

	*/
	return 0;
}
