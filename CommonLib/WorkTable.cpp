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
    const point trace_dir[] = {
        {  1,  0 },
        {  1,  1 },
        {  0,  1 },
        { -1,  1 },
        { -1,  0 },
        { -1, -1 },
        {  0, -1 },
        {  1, -1 }
    };
    const auto trace_dirs = static_cast<int32>(sizeof(trace_dir) / sizeof(point));

    vector<vector<point>> outlines;
    point cur_pos;
    while (true) { // アウトライン検索ループ
        /*** アウトラインの始点を検索 ***/
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
        if (!outline_found) { // 残っているアウトラインなし
            break;
        }
        int32 prev_dir = 0;
        while (true) { // 点検索ループ
            /*** トレースの進行方向を優先して検索半径を広げながら周囲の点を検索 ***/
            /*** あればアウトラインの点として追加 ***/
            bool point_found = false;
            for (int32 radius = 1; radius <= search_radius; radius++) {
                for (int32 d = 0; d < sizeof(prefer_dir); d++) {
                    auto curr_dir = (prefer_dir[d] + prev_dir + trace_dirs) % trace_dirs;
                    auto index = bitmap_get_index_ofs(*pbmp,
                        cur_pos, 
                        trace_dir[curr_dir].x * radius, 
                        trace_dir[curr_dir].y * radius
                    );
                    if (UINT32_MAX == index) {
                        continue;
                    }
                    auto pCell = &table->pCells[index];
                    if (pCell->enable) { // 点を発見、アウトラインの点として追加
                        pCell->enable = false;
                        cur_pos = pCell->pos;
                        prev_dir = curr_dir;
                        outline.push_back(cur_pos);
                        point_found = true;
                        break;
                    }
                }
                if (point_found) { // 次の点を検索
                    break;
                }
            }
            if (!point_found) { // アウトラインの終端
                if (outline.size() < 4) {
                    outlines.push_back(outline);
                    break;
                }
                /*** 直線上に存在する点を除いたアウトラインをリストに追加 ***/
                vector<point> tmp;
                point pos3;
                point pos2 = outline[0];
                point pos1 = outline[1];
                point pos0 = outline[2];
                tmp.push_back(pos2);
                for (int32 i = 3; i < outline.size(); i++) {
                    pos3 = pos2;
                    pos2 = pos1;
                    pos1 = pos0;
                    pos0 = outline[i];
                    double abx = pos0.x - pos2.x;
                    double aby = pos0.y - pos2.y;
                    double l = sqrt(abx * abx + aby * aby);
                    abx /= l;
                    aby /= l;
                    double px = pos1.x - pos2.x;
                    double py = pos1.y - pos2.y;
                    l = sqrt(px * px + py * py);
                    px = px / l - abx;
                    py = py / l - aby;
                    if (abs(atan2(py, px)) < 1e-3) {
                        continue;
                    }
                    abx = pos0.x - pos3.x;
                    aby = pos0.y - pos3.y;
                    l = sqrt(abx * abx + aby * aby);
                    abx /= l;
                    aby /= l;
                    px = (pos1.x + pos2.x) / 2.0 - pos3.x;
                    py = (pos1.y + pos2.y) / 2.0 - pos3.y;
                    l = sqrt(px * px + py * py);
                    px = px / l - abx;
                    py = py / l - aby;
                    if (1e-3 < abs(atan2(py, px))) {
                        tmp.push_back(pos1);
                    }
                }
                outlines.push_back(tmp);
                break;
            }
        }
    }
    return outlines;
}
