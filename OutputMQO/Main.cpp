#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "mqo.h"
#include "ExtPolyline.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
    string bmp_file;

    // check parameter
    if (argc < 2) {
        cout << "format error..." << endl;
        cout << "[format] BmpOutline.exe <BMP FILE>" << endl;
        return (EXIT_SUCCESS);
    } else {
        bmp_file = argv[1];
        cout << "BMP FILE : " << bmp_file << endl;
    }

    // get bitmap data
    auto pBmp = new Bitmap(bmp_file);
    if (pBmp->error != 0) {
        cout << "bmp reading error... (" << pBmp->error << ")" << endl;
        return (EXIT_SUCCESS);
    } else {
        pBmp->PrintFileHeader();
        pBmp->PrintInfoHeader();
    }

    // palette chck
    if (pBmp->info_h.pixel != DEFINE_SUPPORT_COLOR_256) {
        cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_256 << " colors)" << endl;
        return (EXIT_SUCCESS);
    }

    type_mqo mqo = fn_mqo_create_default_parameter();
    mqo.object = fn_convert_table_to_mqo(pBmp);
    if (mqo.object.error != 0) {
        cout << "bmp convert error... (" << pBmp->error << ")" << endl;
        return (EXIT_SUCCESS);
    }

    // save
    stringstream ss;
    ss << bmp_file << ".mqo";
    if (fn_mqo_write(&mqo, ss.str())) {
        cout << "bmp writing error..." << endl;
        return (EXIT_SUCCESS);
    }

    return (EXIT_SUCCESS);
}
