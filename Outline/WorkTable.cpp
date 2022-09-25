#include <string>

using namespace std;

#include "WorkTable.h"

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
                pos,
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

            /* ����, ��, ���� */
            if (pos.x < 1) {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 0; /* ���� */
            } else {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 1; /* �L�� */
            }
            /* ����, ��, �E�� */
            if (pos.y < 1) {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 0; /* ���� */
            } else {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 1; /* �L�� */
            }
            /* �E��, �E, �E�� */
            if (inv_pos.x < 2) {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 0; /* ���� */
            } else {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 1; /* �L�� */
            }
            /* ����, ��, �E�� */
            if (inv_pos.y < 2) {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 0; /* ���� */
            } else {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 1; /* �L�� */
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
        { ULONG_MAX, ULONG_MAX },
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
