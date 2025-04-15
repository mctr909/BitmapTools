#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/Declease.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 2) {
        cout << "parameter format error..." << endl;
        cout << "[example] Declease.exe <BMP FILE1> <BMP FILE2> ..." << endl;
        return (EXIT_SUCCESS);
    }

    const auto thickness = atoi(argv[1]);

    string bmp_file;
    for (int32_t fcount = 0; fcount < argc - 1; fcount++) {
        bmp_file = argv[fcount + 1];
        cout << "BMP FILE : " << bmp_file << endl;

        // get bitmap data
        auto bmp24 = new Bitmap(bmp_file);
        bmp24->PrintInfo();

        // bit chck
        if (bmp24->info.bits != Bitmap::Type::COLOR24BIT) {
            cout << "bmp not support... (only 24bit colors)" << endl;
            delete bmp24;
            continue;
        }

        auto bmp8 = new Bitmap(bmp24->info.width, bmp24->info.height, Bitmap::Type::COLOR256);
        Declease::Exec(*bmp24, bmp8);

        // save
        stringstream ss;
        ss << bmp_file.substr(0, bmp_file.size() - 4) << "_declease.bmp";
        if (!bmp8->Save(ss.str())) {
            cout << "bmp writing error..." << endl;
        }
        delete bmp24;
        delete bmp8;
    }

    return (EXIT_SUCCESS);
}
