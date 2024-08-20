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

struct PosIndex {
	int x;
	int y;
};

class Trajectory {
public:
	vector<PosIndex> indexes;

	void print(void) {
		for(auto &p : indexes ){
			cout << "(" << p.x << ", " << p.y << ") ->";
		}
		cout << endl;
	}
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

		if(max == 0)
			return;
		
		for(int i=0; i<this->data_.size(); i++){
			/*
			if(data_[i] < 3){
				data_[i] = 0;
				continue; 
			}*/

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

	PosIndex chooseNextPos(PosIndex index, int skip) {
		int window = 3*skip;
		vector<int> cands;
		vector<int> cands_all;
		for(int iy=-window+index.y;iy<=index.y+window;iy++){
			for(int ix=-window+index.x;ix<=index.x+window;ix++){
				int i = xyToIndex(ix, iy);

				if(i < 0)
					continue;
				
				cands_all.push_back(i);
				if(this->data_[i] > 0)
					cands.push_back(i);
			}
		}

		int chosen;
		vector<int> result;
		if(cands.size() == 0){
			return { -1, -1};
		}else{
			int pos = (int)(uniform_rand()*cands.size());
			chosen = cands[pos];
		}

		return { (int)(chosen % this->width_), (int)(chosen / this->width_)};
	}

	Pos toRandomXyPos(int pos_on_data) {
		int x = pos_on_data % this->width_;
		int y = pos_on_data / this->width_;
	
		Pos ans;
		ans.x = (double)x + uniform_rand();
		ans.y = (double)y + uniform_rand();
	
		return ans;
	}

	/*
	void sampling(unsigned int num, vector<int> *sample)
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
			sample->push_back(j);
			continue;
		}
	}*/

	bool samplingXY(unsigned int num, vector<PosIndex> *sample)
	{
		uint64_t sum = reduce(begin(this->data_), end(this->data_));
		if (sum == 0) {
			return false;
		}

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

			int x = j  % this->width_;
			int y = j  / this->width_;
			sample->push_back({x, y});
			continue;
		}

		return true;
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

	/*
	static void sampling(vector<Motion> *sample, int skip) {
		const int max_speed = 3*skip;

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
	*/

	void print(void){
		cout << dx_ << " " << dy_ << " " << weight_ << endl;
	}
};

void one_step(Map &map, vector<Trajectory> &particles, int skip) {
	for(auto &p : particles){
		PosIndex after = map.chooseNextPos(p.indexes.back(), skip);

		if (after.x >= 0)
			p.indexes.push_back(after);
	}
}

int main(int argc, char *argv[])
{
	int target_time = atoi(argv[1]);
	int skip = atoi(argv[2]);
	ifstream fixed(argv[3]);
	ifstream origin(argv[4]);
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
	vector<PosIndex> particles;
	if (not map_origin.samplingXY(100, &particles)) {
		cerr << "No obstacle" << endl;
		return 1;
	}

	vector<Trajectory> trajs;
	for(auto &p: particles){
		Trajectory tmp;
		tmp.indexes.push_back(p);
		trajs.push_back(tmp);
	}

	for(int i=5;i<argc;i++) {
		ifstream ifs(argv[i]);
        	Map map_update;
		map_update.load_from_pgm(&ifs);
		map_update.removeFixedObstacle(&map_fixed);

		vector<int> new_partciles;
		one_step(map_update, trajs, skip);
	}	

	Map ans(map_origin.width_, map_origin.height_, map_origin.depth_);

	for(auto &t: trajs) {
	       int len = t.indexes.size();
	       PosIndex *org = &t.indexes[0];
	       PosIndex *last = &t.indexes.back();

		for(double time=2.0; time<10.0; time+=0.2) {
	       double org_x = org->x + uniform_rand() - 0.5;
	       double org_y = org->y + uniform_rand() - 0.5;
	       double last_x = last->x + uniform_rand() - 0.5;
	       double last_y = last->y + uniform_rand() - 0.5;

	       //int dx = (int)((last_x - org_x)*target_time/(4*skip));
	       //int dy = (int)((last_y - org_y)*target_time/(4*skip));
	       int dx = (int)((last_x - org_x)*10*time/(4*skip));
	       int dy = (int)((last_y - org_y)*10*time/(4*skip));

	       int new_x = last->x + dx;
	       int new_y = last->y + dy;

	       int index = ans.xyToIndex(new_x, new_y);
	       if(index < 0)
		       continue;

	       ans.data_[index] += 1;
		}
	}
	ans.normalize();
	ans.print();

	return 0;
}
