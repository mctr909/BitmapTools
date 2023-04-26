#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"

#pragma comment (lib, "CommonLib.lib")

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 3) {
        cout << "parameter format error..." << endl;
        cout << "[example] Outline.exe <line weight> <bitmap file path>" << endl;
        return (EXIT_FAILURE);
    }

    const auto line_weight = atoi(argv[1]);
    string bmp_file = argv[2];
    cout << "BMP FILE : " << bmp_file << endl;

    // get bitmap data
    auto pBmp = new Bitmap(bmp_file);
    if (pBmp->error != 0) {
        cout << "bmp reading error... (" << pBmp->error << ")" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    } else {
        pBmp->PrintHeader();
    }

    // palette chck
    if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_8BIT) {
        cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_8BIT << "bit colors)" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    }

    // backup input data
    pBmp->Backup();

    TYPE_WORKTABLE table;
    table.pCells = reinterpret_cast<TYPE_WORKCELL*>(malloc(sizeof(TYPE_WORKCELL) * pBmp->pixel_count));
    if (NULL == table.pCells) {
        delete pBmp;
        return (EXIT_FAILURE);
    }

    double layer_lum = 1.0;
    for (int layer = 1; ; layer++) {
        // write outline
        layer_lum = worktable_create(&table, *pBmp, layer_lum);
        worktable_write_outline(table, pBmp, line_weight);

        // save
        stringstream ss;
        ss << bmp_file.substr(0, bmp_file.size() - 4);
        ss << "_layer" << layer;
        if (1 < line_weight) {
            ss << "_thickness" << line_weight;
        }
        ss << ".bmp";

        pBmp->Save(ss.str());
        if (pBmp->error != 0) {
            cout << "bmp writing error..." << endl;
            break;
        }

        if (layer_lum <= 1.0 / 255.0) {
            break;
        }

        // set input data from backup
        memcpy_s(pBmp->pPixWork, pBmp->pixel_count, pBmp->pPixBackup, pBmp->pixel_count);
    }

    if (NULL != pBmp) {
        delete pBmp;
    }
    free(table.pCells);

    return (EXIT_SUCCESS);
}
