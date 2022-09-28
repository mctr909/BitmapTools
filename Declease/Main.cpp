#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "Declease.h"

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
    for (int32 fcount = 0; fcount < argc - 1; fcount++) {
        bmp_file = argv[fcount + 1];
        cout << "BMP FILE : " << bmp_file << endl;

        // get bitmap data
        auto bmp24 = new Bitmap(bmp_file);
        if (bmp24->error != 0) {
            cout << "bmp reading error... (" << bmp24->error << ")" << endl;
            delete bmp24;
            continue;
        } else {
            bmp24->PrintFileHeader();
            bmp24->PrintInfoHeader();
        }

        // bit chck
        if (bmp24->info_h.bits != DEFINE_SUPPORT_COLOR_24BIT) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_24BIT << " colors)" << endl;
            delete bmp24;
            continue;
        }

        auto bmp8 = new Bitmap(bmp24->info_h.width, bmp24->info_h.height, 8);
        fn_declease_exec(bmp24, bmp8);
        if (bmp24->error != 0) {
            cout << "bmp convert error... (" << bmp24->error << ")" << endl;
            delete bmp24;
            delete bmp8;
            continue;
        }

        // save
        stringstream ss;
        ss << bmp_file << ".declease.bmp";
        bmp8->Save(ss.str());
        if (bmp8->error != 0) {
            cout << "bmp writing error..." << endl;
            delete bmp24;
            delete bmp8;
            continue;
        }

        delete bmp24;
        delete bmp8;
    }

    return (EXIT_SUCCESS);
}
