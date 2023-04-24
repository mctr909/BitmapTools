#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"

#pragma comment (lib, "CommonLib.lib")

bool
write_outline(Bitmap* pBmp, int32 weight, double* layer_lum) {
    const auto size_max = pBmp->size_max;
    type_worktable table;
    table.pCells = reinterpret_cast<type_workcell*>(calloc(size_max, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pBmp->error = -1;
        return false;
    }

    *layer_lum = worktable_create(&table, *pBmp, *layer_lum);
    if (table.error != 0) {
        pBmp->error = -1;
        return false;
    }

    worktable_write_outline(table, pBmp);

    auto pTempPix = reinterpret_cast<byte*>(malloc(size_max * sizeof(byte)));
    if (NULL == pTempPix) {
        pBmp->error = -1;
        return false;
    }
    memset(pTempPix, table.color_white, size_max);

    double fill_radius = weight / 2.0;
    bool has_outline = false;
    for (uint32 i = 0; i < size_max; i++) {
        if (table.color_filled != pBmp->pPix[i]) {
            continue;
        }
        auto pos = table.pCells[i].pos;
        for (double dy = -fill_radius; dy <= fill_radius; dy++) {
            for (double dx = -fill_radius; dx <= fill_radius; dx++) {
                auto r = sqrt(dx * dx + dy * dy);
                if (r <= weight) {
                    auto arownd = bitmap_get_index_ofs(*pBmp, pos, static_cast<int32>(dx), static_cast<int32>(dy));
                    if (ULONG_MAX != arownd) {
                        pTempPix[arownd] = table.color_black;
                        has_outline = true;
                    }
                }
            }
        }
    }
    memcpy_s(pBmp->pPix, size_max, pTempPix, size_max);

    free(pTempPix);
    free(table.pCells);

    pBmp->error = 0;
    return has_outline;
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

        double layer_lum = 1.0;
        for (int layer = 1; ; layer++) {
            // get bitmap data
            auto pBmp = new Bitmap(bmp_file);
            if (pBmp->error != 0) {
                cout << "bmp reading error... (" << pBmp->error << ")" << endl;
                delete pBmp;
                break;
            } else {
                pBmp->PrintHeader();
            }

            // palette chck
            if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_8BIT) {
                cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_8BIT << "bit colors)" << endl;
                delete pBmp;
                break;
            }

            // write outline
            auto has_outline = write_outline(pBmp, thickness, &layer_lum);
            if (pBmp->error != 0) {
                cout << "bmp convert error... (" << pBmp->error << ")" << endl;
                delete pBmp;
                break;
            }
            if (!has_outline) {
                delete pBmp;
                break;
            }

            // save
            stringstream ss;
            ss << bmp_file.substr(0, bmp_file.size() - 4);
            ss << "_layer" << layer;
            if (1 < thickness) {
                ss << "_thickness" << thickness;
            }
            ss << ".bmp";

            pBmp->Save(ss.str());
            if (pBmp->error != 0) {
                cout << "bmp writing error..." << endl;
                delete pBmp;
                break;
            }

            delete pBmp;

            if (0.0 == layer_lum) {
                break;
            }
        }
    }

    return (EXIT_SUCCESS);
}
