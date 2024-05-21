#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdint>

using namespace std;

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

	uint64_t sum(void) 
	{
		uint64_t ans = 0;
		for(auto d : data_) {
			ans += d;
		}
		return ans;
	}
};

void sampling(Image *image, int num, vector<int> *sample)
{
	uint64_t sum = image->sum();
	cerr << sum << endl;
}

int main(int argc, char *argv[])
{
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

	image_before.print();

	vector<int> sample;
	sampling(&image_before, 50, &sample);

	return 0;
}
