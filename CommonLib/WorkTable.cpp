#include <string>
#include <vector>

using namespace std;

#include "WorkTable.h"

void
fn_worktable_create(type_worktable* output, Bitmap& const pbmp) {
    const point bmp_size = { pbmp.info_h.width, pbmp.info_h.height };
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
    point pos;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(pbmp, pos);

            if (index == UINT32_MAX) {
                output->error = -1;
                return;
            }

            type_workcell cell = {
                false,
                pos,
                index,
                {
                    UINT32_MAX, UINT32_MAX, UINT32_MAX,
                    UINT32_MAX, UINT32_MAX, UINT32_MAX,
                    UINT32_MAX, UINT32_MAX, UINT32_MAX
                }
            };

            if (pbmp.pPix[index] == on) {
                cell.enable = true;
            }

            point inv_pos = { (bmp_size.x - pos.x), (bmp_size.y - pos.y) };

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
                    point pos_dir = { (pos.x + delta_pos[i][1]), (pos.y + delta_pos[i][2]) };
                    cell.index_dir[i] = bitmap_get_index(pbmp, pos_dir);
                }
            }

            output->pCells[cell_index++] = cell;
        }
    }
}

vector<vector<point>>
fn_worktable_outline(Bitmap* pbmp, type_worktable* table) {
    const auto table_size = pbmp->size_max;
    const auto on = table->color_on;
    const auto off = table->color_off;
    const int32 search_radius = 3;
    const sbyte prefer_dir[] = {
        -3, -2, -1, 0, 1, 2, 3
    };
    const point trace_dir[search_radius][8] = {
        {
            {  1,  0 },
            {  1,  1 },
            {  0,  1 },
            { -1,  1 },
            { -1,  0 },
            { -1, -1 },
            {  0, -1 },
            {  1, -1 }
        }, {
            {  2,  1 },
            {  1,  2 },
            { -1,  2 },
            { -2,  1 },
            { -2, -1 },
            { -1, -2 },
            {  1, -2 },
            {  2, -1 }
        }, {
            {  2,  0 },
            {  2,  2 },
            {  0,  2 },
            { -2,  2 },
            { -2,  0 },
            { -2, -2 },
            {  0, -2 },
            {  2, -2 }
        }
    };
    const auto trace_dirs = static_cast<int32>(sizeof(trace_dir[0]) / sizeof(point));

    vector<vector<point>> outlines;
    point cur_pos;
    while (true) { // �A�E�g���C���������[�v
        /*** �A�E�g���C���̎n�_������ ***/
        bool outline_found = false;
        vector<point> outline;
        for (uint32 i = 0; i < table_size; i++) {
            auto pCell = &table->pCells[i];
            if (pCell->enable) {
                pCell->enable = false;
                cur_pos = pCell->pos;
                outline.push_back(pCell->pos);
                outline_found = true;
                break;
            }
        }
        if (!outline_found) { // �c���Ă���A�E�g���C���Ȃ�
            break;
        }
        int32 prev_dir = 0;
        while (true) { // �_�������[�v
            /*** �g���[�X�̐i�s������D�悵�Č������a���L���Ȃ�����͂̓_������ ***/
            /*** ����΃A�E�g���C���̓_�Ƃ��Ēǉ� ***/
            bool point_found = false;
            for (int32 r = 0; r < search_radius; r++) {
                for (int32 d = 0; d < sizeof(prefer_dir); d++) {
                    auto curr_dir = (prefer_dir[d] + prev_dir + trace_dirs) % trace_dirs;
                    auto index = bitmap_get_index_ofs(*pbmp,
                        cur_pos, 
                        trace_dir[r][curr_dir].x, 
                        trace_dir[r][curr_dir].y
                    );
                    if (UINT32_MAX == index) {
                        continue;
                    }
                    auto pCell = &table->pCells[index];
                    if (pCell->enable) { // �_�𔭌��A�A�E�g���C���̓_�Ƃ��Ēǉ�
                        pCell->enable = false;
                        cur_pos = pCell->pos;
                        prev_dir = curr_dir;
                        outline.push_back(cur_pos);
                        point_found = true;
                        break;
                    }
                }
                if (point_found) { // ���̓_������
                    break;
                }
            }
            if (!point_found) { // �A�E�g���C���̏I�[
                if (outline.size() < 3) {
                    outlines.push_back(outline);
                    break;
                }
                /*** ������ɑ��݂���_���������A�E�g���C�������X�g�ɒǉ� ***/
                vector<point> tmp;
                point pos_o;
                point pos_p;
                point pos_q;
                point_d op;
                point_d oq;
                double l;
                tmp.push_back(outline[0]);
                pos_q = outline[0];
                pos_p = outline[1];
                for (int32 i = 2; i < outline.size(); i++) {
                    pos_o = pos_q;
                    pos_q = pos_p;
                    pos_p = outline[i];
                    op.x = pos_p.x - pos_o.x;
                    op.y = pos_p.y - pos_o.y;
                    oq.x = pos_q.x - pos_o.x;
                    oq.y = pos_q.y - pos_o.y;
                    l = sqrt(op.x * op.x + op.y * op.y);
                    op.x /= l;
                    op.y /= l;
                    l = sqrt(oq.x * oq.x + oq.y * oq.y);
                    oq.x /= l;
                    oq.y /= l;
                    if (1e-3 < abs(oq.x - op.x) || 1e-3 < abs(oq.y - op.y)) {
                        tmp.push_back(pos_q);
                    }
                }
                outlines.push_back(tmp);
                break;
            }
        }
    }
    return outlines;
}
