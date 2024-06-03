#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdint>
#include <climits>
#include <numeric>

using namespace std;

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

	double initial_shift = step*((double)rand()/RAND_MAX);
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
	ans.x = (double)x + (double)rand()/RAND_MAX;
	ans.y = (double)y + (double)rand()/RAND_MAX;

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
	srand(time(NULL));
	Image image_before, image_after;

	if(argc != 3) {
		cerr << "Invalid args" << endl;
		return 1;
	}

	ifstream before(argv[1]);
	ifstream after(argv[2]);
	if (not before or not after) {
		cerr << "Invalid files" << endl;
		return 1;
	}

	if ( not image_before.load(&before) 
	  or not image_after.load(&after)) {
		return 1;
	}

	//image_before.print();

	const int sample_num = 50;
	vector<int> sample_before, sample_after;
	vector<Pos> sample_before_xy, sample_after_xy;

	sampling(&image_before, sample_num, &sample_before);
	for(auto p : sample_before){
		sample_before_xy.push_back(toRandomXyPos(p, image_before.width_));
	}

	sampling(&image_after, sample_num, &sample_after);
	for(auto p : sample_after){
		sample_after_xy.push_back(toRandomXyPos(p, image_after.width_));
	}

	uint64_t pixel_sum = reduce(begin(image_after.data_), end(image_after.data_));
	double sample_weight = (double)pixel_sum/sample_num;

	vector<double> vote(image_before.width_*image_before.height_, 0.0);
	for(int i=0;i<sample_num;i++){
		double new_x = 2*sample_after_xy[i].x - sample_before_xy[i].x;
		double new_y = 2*sample_after_xy[i].y - sample_before_xy[i].y;

		int pos = xyToDataPos((int)new_x, (int)new_y,
				image_before.width_, image_before.height_);

		vote[pos] += sample_weight;
	}

	Image ans = image_before;
	ans.data_.clear();
	for(double v : vote) {
		if (v > image_after.depth_) {
			ans.data_.push_back(image_after.depth_);
		}else{
			ans.data_.push_back((int)v);
		}
	}

	ans.print();

	return 0;
}
