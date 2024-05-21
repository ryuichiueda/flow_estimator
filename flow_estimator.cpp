#include <iostream>
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
	vector<uint8_t> data_;

	void print()
	{
		for(int y=0;y<height_;y++) {
			for(int x=0;x<width_;x++) {
				cout << (int)data_[x + y*width_] << " ";
			}
			cout << endl;
		}
	}
};

bool read_image_file(Image *image, ifstream *ifs)
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
			image->width_ = stoi(buf);
		}else if(counter == 2) {
			image->height_ = stoi(buf);
		}else{
			image->data_.push_back(stoi(buf));
		}
		counter++;
	}
	return true;
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

	if (read_image_file(&image_before, &before)) {
		image_before.print();
	}

	return 0;
}
