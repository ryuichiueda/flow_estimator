#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Image {
	unsigned int width;
	unsigned int height;
	vector<uint8_t> data;
};

bool read_image_file(Image *image_before, ifstream *ifs) {
	string buf;
	while(not ifs->eof()) {
		*ifs >> buf;
		cout << buf << endl;
	}
	return true;
}

int main(int argc, char *argv[]) {
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

	read_image_file(&image_before, &before);

	return 0;
}
