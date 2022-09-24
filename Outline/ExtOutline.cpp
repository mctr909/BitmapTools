#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"
#include "ExtOutline.h"

Bitmap*
fn_ext_outline(Bitmap& const pbmp) {
    auto overwrite_bmp = &pbmp;

    const auto size_max = static_cast<uint32>(pbmp.info_h.width * pbmp.info_h.height);
    type_worktable table;
    table.pCells = static_cast<type_workcell*>(calloc(size_max, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        overwrite_bmp->error = -1;
        return overwrite_bmp;
    }

    fn_worktable_create(&table, pbmp);

    if (table.error != 0) {
        overwrite_bmp->error = -1;
        return overwrite_bmp;
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
            overwrite_bmp->pPix[index] = new_color;
        }
    }

    free(table.pCells);
    overwrite_bmp->error = 0;
    return overwrite_bmp;
}

void
fn_worktable_create(type_worktable* output, Bitmap& const pbmp) {
    const Bitmap::position bmp_size = { pbmp.info_h.width, pbmp.info_h.height };
    const byte on = DEFINE_COLOR_ON;
    const byte off = DEFINE_COLOR_OFF;

    output->color_on = on;
    output->color_off = off;
    output->error = 0;

    sbyte delta_pos[9][3] = {
        /* DIRECTION     f   x   y */
        /* BOTTOM_L */ { 1, -1, -1},
        /* BOTTOM   */ { 1,  0, -1},
        /* BOTTOM_R */ { 1,  1, -1},
        /* LEFT     */ { 1, -1,  0},
        /* CENTER   */ { 1,  0,  0},
        /* RIGHT    */ { 1,  1,  0},
        /* TOP_L    */ { 1, -1,  1},
        /* TOP      */ { 1,  0,  1},
        /* TOP_R    */ { 1,  1,  1}
    };

    uint32 cell_index = 0;
    Bitmap::position pos;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_pix_index(pbmp, pos);

            if (index == ULONG_MAX) {
                output->error = -1;
                return;
            }

            type_workcell cell = {
                off,
                pos.x,
                pos.y,
                index,
                {
                    ULONG_MAX, ULONG_MAX, ULONG_MAX,
                    ULONG_MAX, ULONG_MAX, ULONG_MAX,
                    ULONG_MAX, ULONG_MAX, ULONG_MAX
                }
            };

            if (pbmp.pPix[index] == on) {
                cell.data = on;
            }

            Bitmap::position inv_pos = { (bmp_size.x - pos.x), (bmp_size.y - pos.y) };

            /* 左下, 左, 左上 */
            if (pos.x < 1) {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 0; /* 無効 */
            } else {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 1; /* 有効 */
            }
            /* 左下, 下, 右下 */
            if (pos.y < 1) {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 0; /* 無効 */
            } else {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 1; /* 有効 */
            }
            /* 右下, 右, 右上 */
            if (inv_pos.x < 2) {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 0; /* 無効 */
            } else {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 1; /* 有効 */
            }
            /* 左上, 上, 右上 */
            if (inv_pos.y < 2) {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 0; /* 無効 */
            } else {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 1; /* 有効 */
            }

            for (int i = 0; i < 9; i++) {
                if (delta_pos[i][0] != 0) {
                    Bitmap::position pos_dir = { (pos.x + delta_pos[i][1]), (pos.y + delta_pos[i][2]) };
                    cell.index_dir[i] = bitmap_pix_index(pbmp, pos_dir);
                }
            }

            output->pCells[cell_index++] = cell;
        }
    }
}

inline type_workcell
fn_worktable_get_data(
    uint32      center_index,
    E_DIRECTION direction,
    const type_worktable* ptable,
    uint32 size_max
) {
    type_workcell ret = {
        (*ptable).color_off,
        ULONG_MAX,
        ULONG_MAX,
        ULONG_MAX,
        {
            ULONG_MAX, ULONG_MAX, ULONG_MAX,
            ULONG_MAX, ULONG_MAX, ULONG_MAX,
            ULONG_MAX, ULONG_MAX, ULONG_MAX
        }
    };

    if (center_index >= size_max) {
        return (ret);
    }

    auto index = (*ptable).pCells[center_index].index_dir[static_cast<unsigned long>(direction)];
    if (index < size_max) {
        ret = (*ptable).pCells[index];
    }

    return (ret);
}
