#include <string>
#include <vector>

using namespace std;

#include "WorkTable.h"
#include "Bitmap.h"

const sbyte
WorkTable::PREFER_DIR[PREFER_DIRS] = {
    0, -1, 1, -2, 2, -3, 3, 4
};

const point_b
WorkTable::TRACE_DIR[TRACE_RADIUS][TRACE_DIRS] = {
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

const WorkTable::Cell
WorkTable::DEFAULT_CELL = {
    false,
    false,
    { INVALID_POS, INVALID_POS },
    {
        INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
        INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
        INVALID_INDEX, INVALID_INDEX, INVALID_INDEX
    }
};

WorkTable::WorkTable(int32 width, int32 height) {
    m_color_black = 0;
    m_color_white = 0;
    m_width = width;
    m_height = height;
    m_stride = ((width + 3) >> 2) << 2;
    m_pixel_count = m_stride * height;

    mp_cells = reinterpret_cast<Cell*>(malloc(sizeof(Cell) * m_pixel_count));
    if (NULL == mp_cells) {
        return;
    }

    struct type_delta {
        sbyte x;
        sbyte y;
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
    point pos;
    for (pos.y = 0; pos.y < height; pos.y++) {
        for (pos.x = 0; pos.x < width; pos.x++) {
            /*** ピクセル情報をセット***/
            auto index = get_index(pos);
            mp_cells[index] = {
                false, // filled
                false, // traced
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
            if (1 == (width - pos.x)) {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = true;
            }
            /* 左上, 上, 右上 */
            if (1 == (height - pos.y)) {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = true;
            }
            /*** 周囲ピクセルのインデックスを取得してセット ***/
            for (uint32 i = 0; i < 9; i++) {
                if (delta_pos[i].enable) {
                    mp_cells[index].index_around[i] = get_index_ofs(
                        pos, delta_pos[i].x, delta_pos[i].y
                    );
                }
            }
        }
    }
}

WorkTable::~WorkTable() {
    free(mp_cells);
}

double
WorkTable::Setup(Bitmap& bmp, double lum_limit) {
    const point bmp_size = { bmp.m_info.width, bmp.m_info.height };
    point pos;
    /*** パレットから最暗色と最明色を取得 ***/
    m_color_black = 0;
    m_color_white = 0;
    double most_dark = 1.0;
    double most_light = 0.0;
    for (uint32 i = 0; i < 256; i++) {
        auto color = bmp.mp_palette[i];
        auto lum = bitmap_get_lum(color.r, color.g, color.b);
        if (lum < most_dark) {
            most_dark = lum;
            m_color_black = i;
        }
        if (most_light < lum) {
            most_light = lum;
            m_color_white = i;
        }
    }
    /*** ピクセルから塗っていない部分の輝度を取得 ***/
    double lum_nofill = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.mp_pix[index];
            auto color = bmp.mp_palette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum <= lum_limit && lum_nofill < lum) {
                lum_nofill = lum;
            }
        }
    }
    /*** ピクセルから塗部の輝度を取得 ***/
    double lum_filled = 0.0;
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto index = bitmap_get_index(bmp, pos);
            auto pix = bmp.mp_pix[index];
            auto color = bmp.mp_palette[pix];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            if (lum < lum_nofill && lum_filled < lum) {
                lum_filled = lum;
            }
        }
    }
    /*** 塗部であるかをセット/トレース状態のクリア ***/
    for (pos.y = 0; pos.y < bmp_size.y; pos.y++) {
        for (pos.x = 0; pos.x < bmp_size.x; pos.x++) {
            auto p_index = bitmap_get_index(bmp, pos);
            auto color = bmp.mp_palette[bmp.mp_pix[p_index]];
            auto lum = bitmap_get_lum(color.r, color.g, color.b);
            auto index = get_index(pos);
            mp_cells[index].filled = lum < lum_nofill;
            mp_cells[index].traced = false;
        }
    }
    return lum_filled;
}

void
WorkTable::WriteOutline(Bitmap* pBmp, int32 line_weight) {
    const auto FILL_RADIUS = line_weight / 2;
    memset(pBmp->mp_pix, m_color_white, pBmp->m_info.imagesize);
    for (uint32 i = 0; i < m_pixel_count; i++) {
        if (!mp_cells[i].filled) {
            continue;
        }
        bool fill_b = get_cell(i, E_DIRECTION::BOTTOM).filled;
        bool fill_r = get_cell(i, E_DIRECTION::RIGHT).filled;
        bool fill_l = get_cell(i, E_DIRECTION::LEFT).filled;
        bool fill_t = get_cell(i, E_DIRECTION::TOP).filled;
        int fill_count = fill_b ? 1 : 0;
        fill_count += fill_r ? 1 : 0;
        fill_count += fill_l ? 1 : 0;
        fill_count += fill_t ? 1 : 0;
        if (0 == fill_count) {
            continue;
        }
        if (1 == fill_count) {
            E_DIRECTION dir_a;
            E_DIRECTION dir_b;
            if (fill_b) {
                dir_a = E_DIRECTION::BOTTOM_L;
                dir_b = E_DIRECTION::BOTTOM_R;
            }
            if (fill_r) {
                dir_a = E_DIRECTION::TOP_R;
                dir_b = E_DIRECTION::BOTTOM_R;
            }
            if (fill_l) {
                dir_a = E_DIRECTION::TOP_L;
                dir_b = E_DIRECTION::BOTTOM_L;
            }
            if (fill_t) {
                dir_a = E_DIRECTION::TOP_L;
                dir_b = E_DIRECTION::TOP_R;
            }
            bool fill_a = get_cell(i, dir_a).filled;
            bool fill_b = get_cell(i, dir_b).filled;
            if (fill_a ^ fill_b) {
                continue;
            }
        }
        if (4 == fill_count) {
            bool fill_bl = get_cell(i, E_DIRECTION::BOTTOM_L).filled;
            bool fill_br = get_cell(i, E_DIRECTION::BOTTOM_R).filled;
            bool fill_tl = get_cell(i, E_DIRECTION::TOP_L).filled;
            bool fill_tr = get_cell(i, E_DIRECTION::TOP_R).filled;
            fill_count = fill_bl ? 1 : 0;
            fill_count += fill_br ? 1 : 0;
            fill_count += fill_tl ? 1 : 0;
            fill_count += fill_tr ? 1 : 0;
            if (3 == fill_count) {
                int dx, dy;
                if (!fill_bl) {
                    dx = -2;
                    dy = -2;
                }
                if (!fill_br) {
                    dx = 2;
                    dy = -2;
                }
                if (!fill_tl) {
                    dx = -2;
                    dy = 2;
                }
                if (!fill_tr) {
                    dx = 2;
                    dy = 2;
                }
                auto pos = mp_cells[i].pos;
                auto idx_dx = bitmap_get_index_ofs(*pBmp, pos, dx, 0);
                auto idx_dy = bitmap_get_index_ofs(*pBmp, pos, 0, dy);
                if (INVALID_INDEX == idx_dx) {
                    idx_dx = i;
                }
                if (INVALID_INDEX == idx_dy) {
                    idx_dy = i;
                }
                if (mp_cells[idx_dx].filled && mp_cells[idx_dy].filled) {
                    pBmp->mp_pix[i] = m_color_black;
                }
            }
            continue;
        }
        auto pos = mp_cells[i].pos;
        for (int32 dy = -FILL_RADIUS; dy <= FILL_RADIUS; dy++) {
            for (int32 dx = -FILL_RADIUS; dx <= FILL_RADIUS; dx++) {
                auto r = sqrt(dx * dx + dy * dy);
                if (r <= FILL_RADIUS) {
                    auto arownd = bitmap_get_index_ofs(*pBmp, pos, dx, dy);
                    if (INVALID_INDEX != arownd) {
                        pBmp->mp_pix[arownd] = m_color_black;
                    }
                }
            }
        }
    }
}

vector<vector<point_d>>
WorkTable::CreatePolyline() {
    vector<vector<point_d>> polyline_list;
    uint32 start_index = 0;
    while (true) { // ポリライン取得ループ
        vector<point> point_list; // 点リスト
        point current_pos;        // 現在の位置
        int32 current_dir = 0;    // 現在の進行方向
        /*** ポリライン始点を検索 ***/
        bool polyline_found = false;
        for (uint32 i = start_index; i < m_pixel_count; i++) {
            auto pCell = &mp_cells[i];
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
                    auto index = get_index_ofs(
                        current_pos,
                        TRACE_DIR[r][trace_dir].x,
                        TRACE_DIR[r][trace_dir].y
                    );
                    if (INVALID_INDEX == index) {
                        continue;
                    }
                    auto pCell = &mp_cells[index];
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
                auto polyline = eliminatePointsOnStraightLine(point_list);
                if (3 <= polyline.size()) {
                    // 点リストをポリラインリストに追加
                    polyline_list.push_back(polyline);
                }
                break;
            }
        }
    }
    return polyline_list;
}

vector<point_d>
WorkTable::eliminatePointsOnStraightLine(vector<point> polyline) {
    if (polyline.size() < 3) {
        return vector<point_d>();
    }

    point pos_a;
    point pos_b;
    point pos_c;
    point pos_o;
    point_d oa;
    point_d og;

    /*** 3点の直線チェック ***/
    vector<point> line_3p;
    line_3p.push_back(polyline[0]);
    pos_b = polyline[0];
    pos_a = polyline[1];
    for (int32 i = 2; i < polyline.size(); i++) {
        pos_o = pos_b;
        pos_b = pos_a;
        pos_a = polyline[i];
        oa.x = pos_a.x - pos_o.x;
        oa.y = pos_a.y - pos_o.y;
        og.x = pos_b.x - pos_o.x;
        og.y = pos_b.y - pos_o.y;
        auto oa_len = sqrt(oa.x * oa.x + oa.y * oa.y);
        auto og_len = sqrt(og.x * og.x + og.y * og.y);
        oa.x /= oa_len;
        oa.y /= oa_len;
        og.x /= og_len;
        og.y /= og_len;
        if (1e-6 < abs(og.x - oa.x) || 1e-6 < abs(og.y - oa.y)) {
            line_3p.push_back(pos_b);
        }
    }
    line_3p.push_back(line_3p[0]);

    /*** 4点の直線チェック ***/
    vector<point> line_4p;
    line_4p.push_back(line_3p[0]);
    line_4p.push_back(line_3p[1]);
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
        auto oa_len = sqrt(oa.x * oa.x + oa.y * oa.y);
        auto og_len = sqrt(og.x * og.x + og.y * og.y);
        oa.x /= oa_len;
        oa.y /= oa_len;
        og.x /= og_len;
        og.y /= og_len;
        auto limit = 1.0 / (oa_len * 5.0);
        if (limit < abs(og.x - oa.x) || limit < abs(og.y - oa.y)) {
            line_4p.push_back(pos_b);
        }
    }
    line_4p.push_back(line_4p[0]);

    /*** 隣り合う点の重心をリスト入れて返す ***/
    vector<point_d> ret;
    point_d pre = { line_4p[0].x, line_4p[0].y };
    point_d avg = pre;
    int avg_count = 1;
    for (int32 i = 1; i < line_4p.size(); i++) {
        auto cur = line_4p[i];
        auto sx = cur.x - pre.x;
        auto sy = cur.y - pre.y;
        if ((sx * sx + sy * sy) < 4) {
            avg.x += cur.x;
            avg.y += cur.y;
            avg_count++;
            pre.x = avg.x / avg_count;
            pre.y = avg.y / avg_count;
        } else {
            ret.push_back(pre);
            avg.x = cur.x;
            avg.y = cur.y;
            avg_count = 1;
            pre.x = cur.x;
            pre.y = cur.y;
        }
    }
    return ret;
}
