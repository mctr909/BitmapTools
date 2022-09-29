#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"
#include "Outline.h"

void
fn_outline_exec(Bitmap* pBmp) {
    const auto size_max = pBmp->size_max;
    type_worktable table;
    table.pCells = reinterpret_cast<type_workcell*>(calloc(size_max, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pBmp->error = -1;
        return;
    }

    fn_worktable_create(&table, *pBmp);
    if (table.error != 0) {
        pBmp->error = -1;
        return;
    }

    for (uint32 i = 0; i < size_max; i++) {
        if (table.pCells[i].enable) {
            type_workcell tmp;
            bool flg = false;
            tmp = fn_worktable_get_data(i, E_DIRECTION::BOTTOM, &table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::RIGHT, &table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::LEFT, &table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = fn_worktable_get_data(i, E_DIRECTION::TOP, &table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            auto index = table.pCells[i].index_bmp;
            if (flg) {
                pBmp->pPix[index] = table.color_on;
            } else {
                pBmp->pPix[index] = table.color_off;
            }
        }
    }

    free(table.pCells);
    pBmp->error = 0;
}

void
fn_outline_thickness(Bitmap* pBmp, int32 weight) {
    const auto size_max = pBmp->size_max;
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