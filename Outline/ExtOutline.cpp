#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"
#include "ExtOutline.h"

void
fn_ext_outline(Bitmap* pbmp) {
    const auto size_max = static_cast<uint32>(pbmp->info_h.width * pbmp->info_h.height);
    type_worktable table;
    table.pCells = static_cast<type_workcell*>(calloc(size_max, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pbmp->error = -1;
        return;
    }

    fn_worktable_create(&table, *pbmp);

    if (table.error != 0) {
        pbmp->error = -1;
        return;
    }

    for (uint32 i = 0; i < size_max; i++) {
        if (table.pCells[i].data == table.color_on) {
            type_workcell tmp;
            bool flg = false;
            tmp = fn_worktable_get_data(i, E_DIRECTION::BOTTOM, &table, size_max);
            if ((flg == false) && (tmp.data != table.color_on)) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::LEFT, &table, size_max);
            if ((flg == false) && (tmp.data != table.color_on)) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::RIGHT, &table, size_max);
            if ((flg == false) && (tmp.data != table.color_on)) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::TOP, &table, size_max);
            if ((flg == false) && (tmp.data != table.color_on)) {
                flg = true;
            }

            auto new_color = table.color_off;
            if (flg == true) {
                new_color = table.color_on;
            }
            auto index = table.pCells[i].index_bmp;
            pbmp->pPix[index] = new_color;
        }
    }

    free(table.pCells);
    pbmp->error = 0;
}

void
fn_thickness(Bitmap* pbmp, int32 weight) {
    const auto radius = weight / 2;
    const auto index_count = pbmp->info_h.width * pbmp->info_h.height;
    auto pTemp = new byte[index_count];
    memset(pTemp, DEFINE_COLOR_OFF, index_count);

    Bitmap::position pos;
    for (uint32 i = 0; i < index_count; i++) {
        if (DEFINE_COLOR_OFF == pbmp->pPix[i]) {
            continue;
        }
        pTemp[i] = DEFINE_COLOR_ON;
        bitmap_get_pos(*pbmp, &pos, i);
        for (int32 dy = -radius; dy <= radius; dy++) {
            for (int32 dx = -radius; dx <= radius; dx++) {
                auto r = sqrt(dx * dx + dy * dy);
                if (r <= weight / 2.0) {
                    auto arownd = bitmap_get_index_ofs(*pbmp, pos, dx, dy);
                    if (ULONG_MAX != arownd) {
                        pTemp[arownd] = DEFINE_COLOR_ON;
                    }
                }
            }
        }
    }

    memcpy_s(pbmp->pPix, index_count, pTemp, index_count);

    delete pTemp;
}
