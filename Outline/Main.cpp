#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/Outline.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char *argv[]) {
	// check parameter
	if (argc < 2) {
		cout << "parameter format error..." << endl
			<< "[example] Outline.exe <bitmap file path>" << endl;
		return (EXIT_FAILURE);
	}

	string bmp_file = argv[1];
	cout << "BMP FILE : " << bmp_file << endl;

	// get bitmap data
	auto pBmp = new Bitmap(bmp_file);
	pBmp->PrintInfo();

	// check format(index color only)
	switch (pBmp->info.bits) {
	case Bitmap::Type::COLOR2:
	case Bitmap::Type::COLOR4:
	case Bitmap::Type::COLOR16:
	case Bitmap::Type::COLOR256:
		break;
	default:
		cout << "bitmap not support... (1bit/2bit/4bit/8bit index color)" << endl;
		delete pBmp;
		return (EXIT_FAILURE);
	}

	// allocate outline
	auto pOutline = new Outline(pBmp);

	// backup input data
	pBmp->Backup();

	double layer_lum = 1.0;
	for (int32_t layer = 1; ; layer++) {
		// write outline
		layer_lum = pOutline->Read(layer_lum);
		pOutline->Write();

		// save file
		stringstream ss;
		ss << bmp_file.substr(0, bmp_file.size() - 4);
		ss << "_layer" << layer << ".bmp";
		if (!pBmp->Save(ss.str())) {
			cout << "bmp writing error..." << endl;
			break;
		}

		if (layer_lum <= 1.0 / 255.0) {
			break;
		}

		// set input data from backup
		pBmp->Rollback();
	}

	if (NULL != pBmp) {
		delete pBmp;
	}
	if (NULL != pOutline) {
		delete pOutline;
	}

	return (EXIT_SUCCESS);
}
