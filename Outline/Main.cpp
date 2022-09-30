#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "Outline.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 3) {
        cout << "parameter format error..." << endl;
        cout << "[example] Outline.exe <thickness> <BMP FILE1> <BMP FILE2> ..." << endl;
        return (EXIT_SUCCESS);
    }

    const auto thickness = atoi(argv[1]);

    string bmp_file;
    for (int32 fcount = 0; fcount < argc - 2; fcount++) {
        bmp_file = argv[fcount + 2];
        cout << "BMP FILE : " << bmp_file << endl;

        // get bitmap data
        auto pBmp = new Bitmap(bmp_file);
        if (pBmp->error != 0) {
            cout << "bmp reading error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            continue;
        } else {
            pBmp->PrintFileHeader();
            pBmp->PrintInfoHeader();
        }

        // palette chck
        if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_256) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_256 << " colors)" << endl;
            delete pBmp;
            continue;
        }

        outline_exec(pBmp);
        if (pBmp->error != 0) {
            cout << "bmp convert error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            continue;
        }

        outline_thickness(pBmp, thickness);

        // save
        stringstream ss;
        ss << bmp_file << ".outline.bmp";
        pBmp->Save(ss.str());
        if (pBmp->error != 0) {
            cout << "bmp writing error..." << endl;
            delete pBmp;
            continue;
        }

        delete pBmp;
    }

    return (EXIT_SUCCESS);
}
