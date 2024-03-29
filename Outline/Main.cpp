﻿#include <iostream>
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
        cout << "bitmap reading error... (" << pBmp->error << ")" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    } else {
        pBmp->PrintHeader();
    }

    // check format(8bit palette only)
    if (BITMAP_COLOR_8BIT != pBmp->m_info.bits) {
        cout << "bitmap not support... (8bit palette only)" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    }

    // allocate worktable
    auto pTable = new WorkTable(pBmp->m_info.width, pBmp->m_info.height);

    // backup input data
    pBmp->Backup();

    double layer_lum = 1.0;
    for (int layer = 1; ; layer++) {
        // write outline
        layer_lum = pTable->Setup(*pBmp, layer_lum);
        pTable->WriteOutline(pBmp, line_weight);

        // save file
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
