#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "mqo.h"
#include "OutputMQO.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 4) {
        cout << "format error..." << endl;
        cout << "[format] BmpOutline.exe <thickness> <y offset> <BMP FILE1> <BMP FILE2> ..." << endl;
        return (EXIT_SUCCESS);
    }

    const auto thickness = atof(argv[1]);
    const auto y_offset = atof(argv[2]);

    string bmp_file;
    for (int32 fcount = 0; fcount < argc - 3; fcount++) {
        bmp_file = argv[fcount + 3];
        cout << "BMP FILE : " << bmp_file << endl;

        // get bitmap data
        auto pBmp = new Bitmap(bmp_file);
        if (pBmp->error != 0) {
            cout << "bmp reading error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        } else {
            pBmp->PrintHeader();
        }

        // palette chck
        if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_8BIT) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_8BIT << "bit colors)" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

        type_mqo mqo = fn_mqo_create_default_parameter();
        mqo.object = output_mqo_exec(pBmp, thickness, y_offset);
        if (mqo.object.error != 0) {
            cout << "bmp convert error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

        // save
        stringstream ss;
        ss << bmp_file << ".mqo";
        if (fn_mqo_write(&mqo, ss.str())) {
            cout << "bmp writing error..." << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

#ifdef DEBUG_OUTPUT_MQO
        stringstream so;
        so << bmp_file << ".traced.bmp";
        pBmp->Save(so.str());
#endif

        delete pBmp;
    }
    return (EXIT_SUCCESS);
}
