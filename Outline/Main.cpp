#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"

#pragma comment (lib, "CommonLib.lib")

void
write_outline(Bitmap* pBmp, int32 weight) {
    const auto size_max = pBmp->size_max;
    type_worktable table;
    table.pCells = reinterpret_cast<type_workcell*>(calloc(size_max, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pBmp->error = -1;
        return;
    }

    worktable_create(&table, *pBmp);
    if (table.error != 0) {
        pBmp->error = -1;
        return;
    }

    worktable_write_outline(table, pBmp);

    free(table.pCells);
    pBmp->error = 0;

    const auto radius = weight / 2;
    auto pTempPix = reinterpret_cast<byte*>(malloc(size_max * sizeof(byte)));
    if (NULL == pTempPix) {
        pBmp->error = -1;
        return;
    }
    memset(pTempPix, DEFINE_COLOR_OFF, size_max);
    point pos;
    for (uint32 i = 0; i < size_max; i++) {
        if (DEFINE_COLOR_ON != pBmp->pPix[i]) {
            continue;
        }
        pTempPix[i] = DEFINE_COLOR_ON;
        bitmap_get_pos(*pBmp, &pos, i);
        for (int32 dy = -radius; dy <= radius; dy++) {
            for (int32 dx = -radius; dx <= radius; dx++) {
                auto r = sqrt(dx * dx + dy * dy);
                if (r <= weight / 2.0) {
                    auto arownd = bitmap_get_index_ofs(*pBmp, pos, dx, dy);
                    if (ULONG_MAX != arownd) {
                        pTempPix[arownd] = DEFINE_COLOR_ON;
                    }
                }
            }
        }
    }
    memcpy_s(pBmp->pPix, size_max, pTempPix, size_max);
    free(pTempPix);
}

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
            pBmp->PrintHeader();
        }

        // palette chck
        if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_8BIT) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_8BIT << "bit colors)" << endl;
            delete pBmp;
            continue;
        }

        write_outline(pBmp, thickness);
        if (pBmp->error != 0) {
            cout << "bmp convert error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            continue;
        }

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
