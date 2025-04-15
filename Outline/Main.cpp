#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/Outline.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
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
    if (pBmp->error != 0) {
        cout << "bitmap reading error... (" << pBmp->error << ")" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    } else {
        pBmp->PrintInfo();
    }

    // check format(8bit palette only)
    if (Bitmap::Type::COLOR8 != pBmp->info.bits) {
        cout << "bitmap not support... (8bit palette only)" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    }

    // allocate worktable
    auto pTable = new Outline(pBmp->info.width, pBmp->info.height);

    // backup input data
    pBmp->Backup();

    double layer_lum = 1.0;
    for (int32_t layer = 1; ; layer++) {
        // write outline
        layer_lum = pTable->Read(*pBmp, layer_lum);
        pTable->Write(pBmp);

        // save file
        stringstream ss;
        ss << bmp_file.substr(0, bmp_file.size() - 4);
        ss << "_layer" << layer << ".bmp";
        pBmp->Save(ss.str());
        if (pBmp->error != 0) {
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
    if (NULL != pTable) {
        delete pTable;
    }

    return (EXIT_SUCCESS);
}
