#include <string>
#include <vector>

using namespace std;

#include "WorkTable.h"

/**
ポリラインから直線上の点を除く
*/
void
__worktable_eliminate_points_on_straightline(vector<point>* pPolyline) {
    if (pPolyline->size() < 3) {
        return;
    }

    point pos_a;
    point pos_b;
    point pos_c;
    point pos_o;
    point_d oa;
    point_d og;
    double len;

    /*** 3点の直線チェック ***/
    vector<point> line_3p;
    line_3p.push_back((*pPolyline)[0]);
    pos_b = (*pPolyline)[0];
    pos_a = (*pPolyline)[1];
    for (int32 i = 2; i < pPolyline->size(); i++) {
        pos_o = pos_b;
        pos_b = pos_a;
        pos_a = (*pPolyline)[i];
        oa.x = pos_a.x - pos_o.x;
        oa.y = pos_a.y - pos_o.y;
        og.x = pos_b.x - pos_o.x;
        og.y = pos_b.y - pos_o.y;
        len = sqrt(oa.x * oa.x + oa.y * oa.y);
        oa.x /= len;
        oa.y /= len;
        len = sqrt(og.x * og.x + og.y * og.y);
        og.x /= len;
        og.y /= len;
        if (1e-6 < abs(og.x - oa.x) || 1e-6 < abs(og.y - oa.y)) {
            line_3p.push_back(pos_b);
        }
    }

    /*** 4点の直線チェック ***/
    pPolyline->clear();
    pPolyline->push_back(line_3p[0]);
    pPolyline->push_back(line_3p[1]);
    pos_c = line_3p[0];
    pos_b = line_3p[1];
    pos_a = line_3p[2];
    for (int32 i = 3; i < line_3p.size(); i++) {
        pos_o = pos_c;
        pos_c = pos_b;
        pos_b = pos_a;
        pos_a = line_3p[i];
        oa.x = pos_a.x - pos_o.x;
        oa.y = pos_a.y - pos_o.y;
        og.x = (pos_b.x + pos_c.x) / 2.0 - pos_o.x;
        og.y = (pos_b.y + pos_c.y) / 2.0 - pos_o.y;
        len = sqrt(oa.x * oa.x + oa.y * oa.y);
        oa.x /= len;
        oa.y /= len;
        len = sqrt(og.x * og.x + og.y * og.y);
        og.x /= len;
        og.y /= len;
        if (1e-6 < abs(og.x - oa.x) || 1e-6 < abs(og.y - oa.y)) {
            pPolyline->push_back(pos_b);
        }
    }
    pPolyline->push_back(pos_a);
}

double
worktable_create(type_worktable* pTable, Bitmap& bmp, double lum_max) {
    const point bmp_size = { bmp.info_h.width, bmp.info_h.height };
    point pos;
    /*** パレットから最暗色と最明色を取得 ***/
    double most_dark = 1.0;
    double most_light = 0.0;
    for (uint32 i = 0; i < 256; i++) {
        auto color = bmp.pPalette[i];
        auto lum = bitmap_get_lum(color.r, color.g, color.b);
        if (lum < most_dark) {
            most_dark = lum;
            pTable->color_black = i;
        }
        if (most_light < lum) {
            most_light = lum;
            pTable->color_white = i;
        }
    }
    /*** ピクセルから塗っていない部分を取得 ***/
    double lum_off = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.pPix[index];
            auto color = bmp.pPalette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum <= lum_max && lum_off < lum) {
                lum_off = lum;
                pTable->color_off = pix;
            }
        }
    }
    /*** ピクセルから塗部を取得 ***/
    double lum_on = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.pPix[index];
            auto color = bmp.pPalette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum < lum_off && lum_on < lum) {
                lum_on = lum;
                pTable->color_on = pix;
            }
        }
    }

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
    
    /*** ワークテーブルの初期化 ***/
    pTable->error = 0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.pPalette[bmp.pPix[index]];
            auto lum = bitmap_get_lum(pix.r, pix.g, pix.b);

            /*** ピクセル情報の初期化 ***/
            type_workcell cell = {
                lum < lum_off, // enable: lum < lum_off
                false,         // traced: false
                pos,
                {
                    UINT32_MAX, UINT32_MAX, UINT32_MAX,
                    UINT32_MAX, UINT32_MAX, UINT32_MAX,
                    UINT32_MAX, UINT32_MAX, UINT32_MAX
                }
            };

            /* 左下, 左, 左上 */
            if (0 == pos.x) {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 0; /* 無効 */
            } else {
                delta_pos[0][0] = delta_pos[3][0] = delta_pos[6][0] = 1; /* 有効 */
            }
            /* 左下, 下, 右下 */
            if (0 == pos.y) {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 0; /* 無効 */
            } else {
                delta_pos[0][0] = delta_pos[1][0] = delta_pos[2][0] = 1; /* 有効 */
            }
            /* 右下, 右, 右上 */
            if (1 == (bmp_size.x - pos.x)) {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 0; /* 無効 */
            } else {
                delta_pos[2][0] = delta_pos[5][0] = delta_pos[8][0] = 1; /* 有効 */
            }
            /* 左上, 上, 右上 */
            if (1 == (bmp_size.y - pos.y)) {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 0; /* 無効 */
            } else {
                delta_pos[6][0] = delta_pos[7][0] = delta_pos[8][0] = 1; /* 有効 */
            }

            /*** 周囲ピクセルのインデックスを取得してセット ***/
            for (uint32 i = 0; i < 9; i++) {
                if (delta_pos[i][0] != 0) {
                    point pos_around = { (pos.x + delta_pos[i][1]), (pos.y + delta_pos[i][2]) };
                    cell.index_around[i] = bitmap_get_index(bmp, pos_around);
                }
            }

            /*** ピクセル情報をワークテーブルにセット ***/
            pTable->pCells[index] = cell;
        }
    }

    return lum_on;
}

void
worktable_write_outline(type_worktable& table, Bitmap* pBmp) {
    const auto size_max = pBmp->size_max;
    for (uint32 i = 0; i < size_max; i++) {
        if (table.pCells[i].enable) {
            type_workcell tmp;
            bool flg = false;
            tmp = worktable_get_data(i, E_DIRECTION::BOTTOM, table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = worktable_get_data(i, E_DIRECTION::RIGHT, table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = worktable_get_data(i, E_DIRECTION::LEFT, table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            tmp = worktable_get_data(i, E_DIRECTION::TOP, table, size_max);
            if (!flg && !tmp.enable) {
                flg = true;
            }
            if (flg) {
                pBmp->pPix[i] = table.color_on;
            } else {
                pBmp->pPix[i] = table.color_off;
            }
        }
    }
}

vector<vector<point>>
worktable_create_polyline(type_worktable* pTable, Bitmap& bmp) {
    const auto table_size = bmp.size_max;
    const auto on = pTable->color_on;
    const auto off = pTable->color_off;
    const int32 search_radius = 3;
    const sbyte prefer_dir[] = {
        0, -1, 1, -2, 2, -3, 3, 4
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

    vector<vector<point>> polyline_list;
    uint32 index_ofs = 0;
    while (true) { // ポリライン検索ループ
        /*** ポリラインの始点を検索 ***/
        bool polyline_found = false;
        point curr_pos;
        vector<point> polyline;
        for (uint32 i = index_ofs; i < table_size; i++) {
            auto pCell = &pTable->pCells[i];
            if (pCell->enable && !pCell->traced) { // 点を発見、ポリラインの始点として追加
                pCell->traced = true;
                curr_pos = pCell->pos;
                polyline.push_back(pCell->pos);
                polyline_found = true;
                index_ofs = i;
                break;
            }
        }
        if (!polyline_found) { // 残っているポリラインなし
            break;
        }
        int32 prev_dir = 0;
        while (true) { // 点検索ループ
            /*** トレースの進行方向を優先して検索半径を広げながら周囲の点を検索 ***/
            /*** あればポリラインの点として追加 ***/
            bool point_found = false;
            for (int32 r = 0; r < search_radius; r++) {
                for (int32 d = 0; d < sizeof(prefer_dir); d++) {
                    auto curr_dir = (prefer_dir[d] + prev_dir + trace_dirs) % trace_dirs;
                    auto index = bitmap_get_index_ofs(bmp,
                        curr_pos, 
                        trace_dir[r][curr_dir].x, 
                        trace_dir[r][curr_dir].y
                    );
                    if (UINT32_MAX == index) {
                        continue;
                    }
                    auto pCell = &pTable->pCells[index];
                    if (pCell->enable && !pCell->traced) { // 点を発見、ポリラインの点として追加
                        pCell->traced = true;
                        curr_pos = pCell->pos;
                        prev_dir = curr_dir;
                        polyline.push_back(curr_pos);
                        point_found = true;
                        break;
                    }
                }
                if (point_found) { // 次の点を検索
                    break;
                }
            }
            if (!point_found) { // ポリラインの終端、ポリラインをリストに追加
                __worktable_eliminate_points_on_straightline(&polyline);
                polyline_list.push_back(polyline);
                break;
            }
        }
    }
    return polyline_list;
}

double
worktable_create_polygon(vector<point>& vert, vector<uint32>& index, vector<surface>* pSurf_list, int32 order) {
    const auto index_size = static_cast<uint32>(index.size());
    type_worktable_vert_info* p_vert_info = (type_worktable_vert_info*)calloc(index_size, sizeof(type_worktable_vert_info));
    double area = 0.0;
    /*** 頂点情報を作成、原点からの距離と削除フラグを設定 ***/
    for (uint32 i = 0; i < index_size; i++) {
        auto x = static_cast<int64>(vert[index[i]].x) - INT32_MIN;
        auto y = static_cast<int64>(vert[index[i]].y) - INT32_MIN;
        p_vert_info[i].distance = sqrt(x * x + y * y);
        p_vert_info[i].deleted = false;
    }
    uint32 reverse_count = 0;
    uint32 vert_count = 0;
    do { // 最も遠くにある頂点(vo)の取得ループ
        /*** 最も遠くにある頂点(vo)を取得 ***/
        point vo; uint32 io = 0;
        double dist_max = 0.0;
        vert_count = 0;
        for (uint32 i = 0; i < index_size; i++) {
            auto info = p_vert_info[i];
            if (info.deleted) {
                continue;
            }
            if (dist_max < info.distance) {
                dist_max = info.distance;
                io = i;
            }
            vert_count++;
        }
        reverse_count = 0;
        vo = vert[index[io]];
        while (true) { // 頂点(vo)の移動ループ
            /*** 頂点(vo)の隣にある頂点(va)を取得 ***/
            point va; uint32 ia;
            ia = (io + index_size - 1) % index_size;
            for (uint32 i = 0; i < index_size; i++) {
                if (p_vert_info[ia].deleted) {
                    ia = (ia + index_size - 1) % index_size;
                } else {
                    break;
                }
            }
            va = vert[index[ia]];
            /*** 頂点(vo)の隣にある頂点(vb)を取得 ***/
            point vb; uint32 ib;
            ib = (io + 1) % index_size;
            for (uint32 i = 0; i < index_size; i++) {
                if (p_vert_info[ib].deleted) {
                    ib = (ib + 1) % index_size;
                } else {
                    break;
                }
            }
            vb = vert[index[ib]];
            /*** 三角形(va vo vb)の表裏を確認 ***/
            int32 normal_aob; point oa, ob;
            oa.x = va.x - vo.x;
            oa.y = va.y - vo.y;
            ob.x = vb.x - vo.x;
            ob.y = vb.y - vo.y;
            normal_aob = oa.x * ob.y - oa.y * ob.x;
            if (normal_aob * order < 0) {
                /*** 裏の場合 ***/
                reverse_count++;
                if (index_size < reverse_count) {
                    /*** 表になる三角形(va vo vb)がない場合 ***/
                    /*** 頂点(vo)を検索対象から削除 ***/
                    p_vert_info[io].deleted = true;
                    /*** 次の最も遠くにある頂点(vo)を取得 ***/
                    break;
                }
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + index_size + order) % index_size;
                for (uint32 i = 0; i < index_size; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + index_size + order) % index_size;
                    } else {
                        break;
                    }
                }
                vo = vert[index[io]];
                continue;
            }
            /*** 三角形(va vo vb)の内側にva vo vb以外の頂点がないか確認 ***/
            bool point_in_triangle = false;
            for (uint32 i = 0; i < index_size; i++) {
                if (i == ia || i == io || i == ib || p_vert_info[i].deleted) {
                    continue;
                }
                auto p = vert[index[i]];
                if (worktable_inner_triangle(va, vo, vb, p)) {
                    point_in_triangle = true;
                    break;
                }
            }
            if (point_in_triangle) {
                /*** 内側に他の頂点がある場合 ***/
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + index_size + order) % index_size;
                for (uint32 i = 0; i < index_size; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + index_size + order) % index_size;
                    } else {
                        break;
                    }
                }
                vo = vert[index[io]];
            } else {
                /*** 内側に他の頂点がない場合 ***/
                /*** 三角形(va vo vb)を面リストに追加 ***/
                surface surf;
                surf.a = index[ia];
                surf.o = index[io];
                surf.b = index[ib];
                pSurf_list->push_back(surf);
                /*** 三角形の面積を加算 ***/
                area += abs(normal_aob) / 2.0;
                /*** 頂点(vo)を検索対象から削除 ***/
                p_vert_info[io].deleted = true;
                /*** 次の最も遠くにある頂点(vo)を取得 ***/
                break;
            }
        } // 頂点(vo)の移動ループ
    } while (3 < vert_count); // 最も遠くにある頂点(vo)の取得ループ
    free(p_vert_info);
    return area;
}

bool
worktable_inner_polygon(vector<surface>& outer_surf, vector<surface>& inner_surf, vector<point>& vert) {
    for (uint32 i = 0; i < outer_surf.size(); i++) {
        auto outer = outer_surf[i];
        auto outer_a = vert[outer.a];
        auto outer_o = vert[outer.o];
        auto outer_b = vert[outer.b];
        for (uint32 j = 0; j < inner_surf.size(); j++) {
            auto inner = inner_surf[j];
            auto inner_a = vert[inner.a];
            auto inner_o = vert[inner.o];
            auto inner_b = vert[inner.b];
            if (worktable_inner_triangle(outer_a, outer_o, outer_b, inner_a)) {
                return true;
            }
            if (worktable_inner_triangle(outer_a, outer_o, outer_b, inner_o)) {
                return true;
            }
            if (worktable_inner_triangle(outer_a, outer_o, outer_b, inner_b)) {
                return true;
            }
        }
    }
    return false;
}
