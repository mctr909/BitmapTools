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
worktable_create(TYPE_WORKTABLE* pTable, Bitmap& bmp, double lum_max) {
    const point bmp_size = { bmp.info_h.width, bmp.info_h.height };
    point pos;
    /*** パレットから最暗色と最明色を取得 ***/
    pTable->color_black = 0;
    pTable->color_white = 0;
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
    /*** ピクセルから塗っていない部分の輝度を取得 ***/
    double lum_nofill = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.pPixWork[index];
            auto color = bmp.pPalette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum <= lum_max && lum_nofill < lum) {
                lum_nofill = lum;
            }
        }
    }
    /*** ピクセルから塗部の輝度を取得 ***/
    double lum_filled = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.pPixWork[index];
            auto color = bmp.pPalette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum < lum_nofill && lum_filled < lum) {
                lum_filled = lum;
            }
        }
    }

    struct type_delta {
        int32 x;
        int32 y;
        bool enable;
    };
    type_delta delta_pos[9] = {
        /* 方向             x   y  enable */
        /* 0:BOTTOM_L */ { -1, -1, false },
        /* 1:BOTTOM   */ {  0, -1, false },
        /* 2:BOTTOM_R */ {  1, -1, false },
        /* 3:LEFT     */ { -1,  0, false },
        /* 4:CENTER   */ {  0,  0, false },
        /* 5:RIGHT    */ {  1,  0, false },
        /* 6:TOP_L    */ { -1,  1, false },
        /* 7:TOP      */ {  0,  1, false },
        /* 8:TOP_R    */ {  1,  1, false }
    };

    /*** ワークテーブルの初期化 ***/
    pTable->error = 0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto color = bmp.pPalette[bmp.pPixWork[index]];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);

            /*** ピクセル情報をセット***/
            pTable->pCells[index] = {
                lum < lum_nofill, // filled
                false,            // traced
                pos,
                {
                    INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
                    INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
                    INVALID_INDEX, INVALID_INDEX, INVALID_INDEX
                }
            };

            /* 左下, 左, 左上 */
            if (0 == pos.x) {
                delta_pos[0].enable = delta_pos[3].enable = delta_pos[6].enable = false;
            } else {
                delta_pos[0].enable = delta_pos[3].enable = delta_pos[6].enable = true;
            }
            /* 左下, 下, 右下 */
            if (0 == pos.y) {
                delta_pos[0].enable = delta_pos[1].enable = delta_pos[2].enable = false;
            } else {
                delta_pos[0].enable = delta_pos[1].enable = delta_pos[2].enable = true;
            }
            /* 右下, 右, 右上 */
            if (1 == (bmp_size.x - pos.x)) {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = true;
            }
            /* 左上, 上, 右上 */
            if (1 == (bmp_size.y - pos.y)) {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = true;
            }

            /*** 周囲ピクセルのインデックスを取得してセット ***/
            for (uint32 i = 0; i < 9; i++) {
                if (delta_pos[i].enable) {
                    point pos_around = { (pos.x + delta_pos[i].x), (pos.y + delta_pos[i].y) };
                    pTable->pCells[index].index_around[i] = bitmap_get_index(bmp, pos_around);
                }
            }
        }
    }

    return lum_filled;
}

void
worktable_write_outline(TYPE_WORKTABLE& table, Bitmap* pBmp, int32 weight) {
    const auto FILL_RADIUS = weight / 2;
    const auto PIXEL_COUNT = pBmp->pixel_count;
    memset(pBmp->pPixWork, table.color_white, PIXEL_COUNT);
    for (uint32 i = 0; i < PIXEL_COUNT; i++) {
        if (!table.pCells[i].filled) {
            continue;
        }
        bool nofill_bottom = !worktable_get_data(i, E_DIRECTION::BOTTOM, table, PIXEL_COUNT).filled;
        bool nofill_right = !worktable_get_data(i, E_DIRECTION::RIGHT, table, PIXEL_COUNT).filled;
        bool nofill_left = !worktable_get_data(i, E_DIRECTION::LEFT, table, PIXEL_COUNT).filled;
        bool nofill_top = !worktable_get_data(i, E_DIRECTION::TOP, table, PIXEL_COUNT).filled;
        if (nofill_bottom || nofill_right || nofill_left || nofill_top) {
            auto pos = table.pCells[i].pos;
            for (int32 dy = -FILL_RADIUS; dy <= FILL_RADIUS; dy++) {
                for (int32 dx = -FILL_RADIUS; dx <= FILL_RADIUS; dx++) {
                    auto r = sqrt(dx * dx + dy * dy);
                    if (r <= FILL_RADIUS) {
                        auto arownd = bitmap_get_index_ofs(*pBmp, pos, dx, dy);
                        if (INVALID_INDEX != arownd) {
                            pBmp->pPixWork[arownd] = table.color_black;
                        }
                    }
                }
            }
        }
    }
}

vector<vector<point>>
worktable_create_polyline(TYPE_WORKTABLE* pTable, Bitmap& bmp) {
    const int32 TRACE_RADIUS = 3;
    const int32 TRACE_DIRS = 8;
    const int32 PREFER_DIRS = 8;
    const sbyte PREFER_DIR[PREFER_DIRS] = {
        0, -1, 1, -2, 2, -3, 3, 4
    };
    const point TRACE_DIR[TRACE_RADIUS][TRACE_DIRS] = {
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

    const auto PIXEL_COUNT = bmp.pixel_count;

    vector<vector<point>> polyline_list;
    uint32 start_index = 0;
    while (true) { // ポリライン取得ループ
        vector<point> point_list; // 点リスト
        point current_pos;        // 現在の位置
        int32 current_dir = 0;    // 現在の進行方向
        /*** ポリライン始点を検索 ***/
        bool polyline_found = false;
        for (uint32 i = start_index; i < PIXEL_COUNT; i++) {
            auto pCell = &pTable->pCells[i];
            if (pCell->filled && !pCell->traced) {
                // ポリライン始点を発見
                polyline_found = true;
                pCell->traced = true;
                // ポリラインの始点として点リストに追加
                current_pos = pCell->pos;
                point_list.push_back(current_pos);
                // 次のポリライン始点の検索開始インデックスをセット
                start_index = i;
                // ポリライン始点の検索終了
                break;
            }
        }
        if (!polyline_found) { // 残っているポリラインなし
            break;
        }
        while (true) { // 点トレースループ
            /*** 現在の進行方向を優先して半径を広げながら周囲の点を検索 ***/
            /*** あればポリラインの点として点リストに追加 ***/
            bool point_found = false;
            for (int32 r = 0; r < TRACE_RADIUS; r++) {
                for (int32 p = 0; p < PREFER_DIRS; p++) {
                    auto trace_dir = (current_dir + PREFER_DIR[p] + TRACE_DIRS) % TRACE_DIRS;
                    auto index = bitmap_get_index_ofs(bmp,
                        current_pos,
                        TRACE_DIR[r][trace_dir].x,
                        TRACE_DIR[r][trace_dir].y
                    );
                    if (INVALID_INDEX == index) {
                        continue;
                    }
                    auto pCell = &pTable->pCells[index];
                    if (pCell->filled && !pCell->traced) {
                        // 点を発見
                        point_found = true;
                        pCell->traced = true;
                        // トレース方向を現在の進行方向とする
                        current_dir = trace_dir;
                        // ポリラインの点として点リストに追加
                        current_pos = pCell->pos;
                        point_list.push_back(current_pos);
                        // 次の点を検索
                        break;
                    }
                }
                if (point_found) { // 次の点を検索
                    break;
                }
            }
            if (!point_found) { // ポリラインの終端
                // 直線上にある点を点リストから除外する
                __worktable_eliminate_points_on_straightline(&point_list);
                // 点リストをポリラインリストに追加
                polyline_list.push_back(point_list);
                break;
            }
        }
    }
    return polyline_list;
}

double
worktable_create_polygon(vector<point>& vert_list, vector<uint32>& index_list, vector<surface>* pSurf_list, int32 order) {
    const auto INDEX_COUNT = static_cast<uint32>(index_list.size());
    const auto INDEX_NEXT = INDEX_COUNT + order;
    const auto INDEX_RIGHT = 1;
    const auto INDEX_LEFT = INDEX_COUNT - 1;
    struct type_vert_info {
        double distance;
        bool deleted;
    };
    /*** 頂点情報を作成、原点からの距離と削除フラグを設定 ***/
    auto p_vert_info = (type_vert_info*)calloc(INDEX_COUNT, sizeof(type_vert_info));
    for (uint32 i = 0; i < INDEX_COUNT; i++) {
        auto x = static_cast<int64>(vert_list[index_list[i]].x) - INT32_MIN;
        auto y = static_cast<int64>(vert_list[index_list[i]].y) - INT32_MIN;
        p_vert_info[i].distance = sqrt(x * x + y * y);
        p_vert_info[i].deleted = false;
    }
    double area = 0.0;
    uint32 reverse_count = 0;
    uint32 vert_count = 0;
    do { // 最も遠くにある頂点(vo)の取得ループ
        /*** 最も遠くにある頂点(vo)を取得 ***/
        point vo; uint32 io = 0;
        double dist_max = 0.0;
        vert_count = 0;
        for (uint32 i = 0; i < INDEX_COUNT; i++) {
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
        vo = vert_list[index_list[io]];
        while (true) { // 頂点(vo)の移動ループ
            /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
            point va; uint32 ia;
            ia = (io + INDEX_LEFT) % INDEX_COUNT;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (p_vert_info[ia].deleted) {
                    ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            va = vert_list[index_list[ia]];
            /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
            point vb; uint32 ib;
            ib = (io + INDEX_RIGHT) % INDEX_COUNT;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (p_vert_info[ib].deleted) {
                    ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            vb = vert_list[index_list[ib]];
            /*** 三角形(va vo vb)の表裏を確認 ***/
            int32 aob_normal;
            auto oa_x = va.x - vo.x;
            auto oa_y = va.y - vo.y;
            auto ob_x = vb.x - vo.x;
            auto ob_y = vb.y - vo.y;
            aob_normal = (oa_x * ob_y - oa_y * ob_x) * order;
            if (aob_normal < 0) {
                /*** 裏の場合 ***/
                reverse_count++;
                if (INDEX_COUNT < reverse_count) {
                    /*** 表になる三角形(va vo vb)がない場合 ***/
                    /*** 頂点(vo)を検索対象から削除 ***/
                    p_vert_info[io].deleted = true;
                    /*** 次の最も遠くにある頂点(vo)を取得 ***/
                    break;
                }
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (uint32 i = 0; i < INDEX_COUNT; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = vert_list[index_list[io]];
                continue;
            }
            /*** 三角形(va vo vb)の内側にva vo vb以外の頂点がないか確認 ***/
            bool point_in_triangle = false;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (i == ia || i == io || i == ib || p_vert_info[i].deleted) {
                    continue;
                }
                auto p = vert_list[index_list[i]];
                if (worktable_inner_triangle(va, vo, vb, p)) {
                    point_in_triangle = true;
                    break;
                }
            }
            if (point_in_triangle) {
                /*** 内側に他の頂点がある場合 ***/
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (uint32 i = 0; i < INDEX_COUNT; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = vert_list[index_list[io]];
            } else {
                /*** 内側に他の頂点がない場合 ***/
                /*** 三角形(va vo vb)を面リストに追加 ***/
                surface surf;
                surf.a = index_list[ia];
                surf.o = index_list[io];
                surf.b = index_list[ib];
                pSurf_list->push_back(surf);
                /*** 三角形の面積を加算 ***/
                area += abs(aob_normal) / 2.0;
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
