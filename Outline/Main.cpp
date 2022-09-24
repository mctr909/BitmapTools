#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "Bitmap.h"
#include "ExtOutline.h"

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 2) {
        cout << "parameter format error..." << endl;
        cout << "[example] BmpOutline.exe <BMP FILE1> <BMP FILE2> ..." << endl;
        return (EXIT_SUCCESS);
    }

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
            bmp->print_fileheader();
            bmp->print_infoheader();
        }

        // palette chck
        if (bmp->info_h.pixel != DEFINE_SUPPORT_COLOR_256) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_256 << " colors)" << endl;
            delete bmp;
            continue;
        }

        auto conv_bmp = fn_ext_outline(*bmp);
        if (conv_bmp->error != 0) {
            cout << "bmp convert error... (" << conv_bmp->error << ")" << endl;
            delete bmp;
            continue;
        }

        // save
        stringstream ss;
        ss << bmp_file << ".outline.bmp";

        if (conv_bmp->copy_data_overwrite(bmp_file, ss.str())) {
            cout << "bmp writing error..." << endl;
            delete bmp;
            continue;
        }
    }

    return (EXIT_SUCCESS);
}
