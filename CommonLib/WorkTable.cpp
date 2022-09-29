#include <string>
#include <vector>

using namespace std;

#include "WorkTable.h"

/**
アウトラインから直線上の点を除く
*/
void
fn_worktable_eliminate_points_on_line(vector<point> *pList) {
    point pos_o2;
    point pos_o;
    point pos_p;
    point pos_q;
    point_d op;
    point_d oq;
    double l;

    vector<point> tmp;
    tmp.push_back((*pList)[0]);

    pos_p = (*pList)[0];
    pos_q = (*pList)[1];
    for (int32 i = 2; i < pList->size(); i++) {
        pos_o = pos_p;
        pos_p = pos_q;
        pos_q = (*pList)[i];
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
        if (1e-3 < abs(op.x - oq.x) || 1e-3 < abs(op.y - oq.y)) {
            tmp.push_back(pos_p);
        }
    }

    pList->clear();
    pList->push_back(tmp[0]);
    pList->push_back(tmp[1]);
    pos_o = tmp[0];
    pos_p = tmp[1];
    pos_q = tmp[2];
    for (int32 i = 3; i < tmp.size(); i++) {
        pos_o2 = pos_o;
        pos_o = pos_p;
        pos_p = pos_q;
        pos_q = tmp[i];
        op.x = (pos_q.x + pos_p.x + pos_o.x + pos_o2.x) / 4.0 - pos_o2.x;
        op.y = (pos_q.y + pos_p.y + pos_o.y + pos_o2.y) / 4.0 - pos_o2.y;
        oq.x = pos_q.x - pos_o2.x;
        oq.y = pos_q.y - pos_o2.y;
        l = sqrt(op.x * op.x + op.y * op.y);
        op.x /= l;
        op.y /= l;
        l = sqrt(oq.x * oq.x + oq.y * oq.y);
        oq.x /= l;
        oq.y /= l;
        if (1e-3 < abs(op.x - oq.x) || 1e-3 < abs(op.y - oq.y)) {
            pList->push_back(pos_p);
        }
    }
    pList->push_back(pos_q);
}

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
                if (outline.size() < 3) {
                    outlines.push_back(outline);
                    break;
                }
                fn_worktable_eliminate_points_on_line(&outline);
                outlines.push_back(outline);
                break;
            }
        }
    }
    return outlines;
}

void
fn_worktable_create_polygon(vector<point> vert, vector<uint32>* pIndex, vector<vector<uint32>>* pSurf) {
    vector<point> vert_tmp;
    for (int32 i = 0; i < vert.size(); i++) {
        vert_tmp.push_back(vert[i]);
    }
    vector<uint32> index_far;
    for (int32 j = 0; j < vert_tmp.size(); j++) {
        double dist_max = 0;
        uint32 index_max = 0;
        for (uint32 i = 0; i < vert_tmp.size(); i++) {
            auto pos = &vert_tmp[i];
            auto dist = sqrt(pos->x * pos->x + pos->y * pos->y);
            if (dist_max < dist) {
                dist_max = dist;
                index_max = i;
            }
        }
        auto tmp = &vert_tmp[index_max];
        tmp->x = 0;
        tmp->y = 0;
        index_far.push_back(index_max);
    }
    vert_tmp.clear();

    vector<uint32> index_near;
    while (0 < index_far.size()) {
        index_near.push_back(index_far.back());
        index_far.pop_back();
    }

    const size_t size = pIndex->size();
    uint32 index;
    point va, vo, vb;
    uint32 ia, io, ib;
    point oa, ob;
    int32 norm_a, norm_b;

    while (3 <= index_near.size()) {
        /*** 最も遠い三角形(va vo vb)を取り出す ***/
        index = index_near.back();
        ia = (index - 1 + size) % size;
        io = index;
        ib = (index + 1) % size;
        va = vert[ia];
        vo = vert[io];
        vb = vert[ib];
        /*** 三角形(va vo vb)の内部に他の頂点があるかを調べる ***/
        bool innner_triangle = false;
        for (int32 i = size - 1; 0 <= i; i--) {
            innner_triangle = fn_worktable_inner_triangle(va, vo, vb, vert[(*pIndex)[i]]);
            if (innner_triangle) {
                break;
            }
        }
        if (!innner_triangle) {
            /*** 内部に頂点がなければ三角形(va vo vb)をリストに追加、 ***/
            /*** 頂点インデックス(va)を削除して次の頂点(va vo vb)に移動 ***/
            vector<uint32> index;
            index.push_back(ia);
            index.push_back(io);
            index.push_back(ib);
            pSurf->push_back(index);
            index_near.pop_back();
            continue;
        }

        /*** 三角形(va vo vb)の法線(norm_a)を計算 ***/
        oa.x = va.x - vo.x;
        oa.y = va.y - vo.y;
        ob.x = vb.x - vo.x;
        ob.y = vb.y - vo.y;
        norm_a = oa.x * ob.y - oa.y * ob.x;
        if (norm_a < 0) {
            norm_a = -1;
        } else {
            norm_a = 1;
        }

        index_near.pop_back();
        while (3 <= index_near.size()) {
            /*** 次の三角形(va vo vb)の法線(norm_b)を計算 ***/
            index = index_near.back();
            ia = (index - 1 + size) % size;
            io = index;
            ib = (index + 1) % size;
            index_near.pop_back();
            va = vert[ia];
            vo = vert[io];
            vb = vert[ib];
            oa.x = va.x - vo.x;
            oa.y = va.y - vo.y;
            ob.x = vb.x - vo.x;
            ob.y = vb.y - vo.y;
            norm_b = oa.x * ob.y - oa.y * ob.x;
            if (norm_b < 0) {
                norm_b = -1;
            } else {
                norm_b = 1;
            }
            innner_triangle = false;
            if (norm_a == norm_b) {
                /*** 法線(norm_a)と法線(norm_b)が同じ場合 ***/
                /*** 三角形(va vo vb)の内部に他の頂点があるかを調べる ***/
                for (int32 i = size - 1; 0 <= i; i--) {
                    innner_triangle = fn_worktable_inner_triangle(vo, va, vb, vert[(*pIndex)[i]]);
                    if (innner_triangle) {
                        break;
                    }
                }
            }
            if (!innner_triangle) {
                /*** 内部に頂点がなければ三角形(va vo vb)をリストに追加、 ***/
                /*** 次の頂点(va vo vb)に移動 ***/
                vector<uint32> index;
                index.push_back(ia);
                index.push_back(io);
                index.push_back(ib);
                pSurf->push_back(index);
                break;
            }
        }
    }
}
