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
        auto bmp = new Bitmap(bmp_file);
        if (bmp->error != 0) {
            cout << "bmp reading error... (" << bmp->error << ")" << endl;
            delete bmp;
            continue;
        } else {
            bmp->PrintFileHeader();
            bmp->PrintInfoHeader();
        }

        // palette chck
        if (bmp->info_h.pixel != DEFINE_SUPPORT_COLOR_24BIT) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_24BIT << " colors)" << endl;
            delete bmp;
            continue;
        }

        auto out_bmp = new Bitmap(bmp->info_h.width, bmp->info_h.height, 8);
        fn_exec_declease(bmp, out_bmp);
        if (bmp->error != 0) {
            cout << "bmp convert error... (" << bmp->error << ")" << endl;
            delete bmp;
            continue;
        }

        // save
        stringstream ss;
        ss << bmp_file << ".declease.bmp";
        out_bmp->Save(ss.str());
        if (out_bmp->error != 0) {
            cout << "bmp writing error..." << endl;
            delete bmp;
            continue;
        }
    }

    return (EXIT_SUCCESS);
}
