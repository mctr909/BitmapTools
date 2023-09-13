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
    m_pixel_count = width * height;

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
        bool nofill_b = !get_cell(i, E_DIRECTION::BOTTOM).filled;
        bool nofill_r = !get_cell(i, E_DIRECTION::RIGHT).filled;
        bool nofill_l = !get_cell(i, E_DIRECTION::LEFT).filled;
        bool nofill_t = !get_cell(i, E_DIRECTION::TOP).filled;
        int nofill_count = nofill_b ? 1 : 0;
        nofill_count += nofill_r ? 1 : 0;
        nofill_count += nofill_l ? 1 : 0;
        nofill_count += nofill_t ? 1 : 0;
        if (3 <= nofill_count) {
            E_DIRECTION dir_a;
            E_DIRECTION dir_b;
            if (!nofill_b) {
                dir_a = E_DIRECTION::BOTTOM_L;
                dir_b = E_DIRECTION::BOTTOM_R;
            }
            if (!nofill_r) {
                dir_a = E_DIRECTION::TOP_R;
                dir_b = E_DIRECTION::BOTTOM_R;
            }
            if (!nofill_l) {
                dir_a = E_DIRECTION::TOP_L;
                dir_b = E_DIRECTION::BOTTOM_L;
            }
            if (!nofill_t) {
                dir_a = E_DIRECTION::TOP_L;
                dir_b = E_DIRECTION::TOP_R;
            }
            bool fill_a = get_cell(i, dir_a).filled;
            bool fill_b = get_cell(i, dir_b).filled;
            if (fill_a ^ fill_b) {
                continue;
            }
        }
        if (nofill_b || nofill_r || nofill_l || nofill_t) {
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
    for (uint32 i = 0; i < m_pixel_count; i++) {
        auto idx = i;
        while (true) {
            auto pos = mp_cells[idx].pos;
            auto idx_r = bitmap_get_index_ofs(*pBmp, pos, 1, 0);
            auto idx_t = bitmap_get_index_ofs(*pBmp, pos, 0, 1);
            auto idx_l = bitmap_get_index_ofs(*pBmp, pos, -1, 0);
            auto idx_b = bitmap_get_index_ofs(*pBmp, pos, 0, -1);
            bool fill_r, fill_t, fill_l, fill_b;
            if (INVALID_INDEX == idx_r) {
                fill_r = false;
            } else {
                fill_r = pBmp->mp_pix[idx_r] == m_color_black;
            }
            if (INVALID_INDEX == idx_t) {
                fill_t = false;
            } else {
                fill_t = pBmp->mp_pix[idx_t] == m_color_black;
            }
            if (INVALID_INDEX == idx_l) {
                fill_l = false;
            } else {
                fill_l = pBmp->mp_pix[idx_l] == m_color_black;
            }
            if (INVALID_INDEX == idx_b) {
                fill_b = false;
            } else {
                fill_b = pBmp->mp_pix[idx_b] == m_color_black;
            }
            int fill_count = fill_r ? 1 : 0;
            fill_count += fill_t ? 1 : 0;
            fill_count += fill_l ? 1 : 0;
            fill_count += fill_b ? 1 : 0;
            if (fill_count < 2) {
                auto idx_tl = bitmap_get_index_ofs(*pBmp, pos, -1, 1);
                auto idx_tr = bitmap_get_index_ofs(*pBmp, pos, 1, 1);
                auto idx_bl = bitmap_get_index_ofs(*pBmp, pos, -1, -1);
                auto idx_br = bitmap_get_index_ofs(*pBmp, pos, 1, -1);
                if (INVALID_INDEX == idx_tl || INVALID_INDEX == idx_tr ||
                    INVALID_INDEX == idx_bl || INVALID_INDEX == idx_br) {
                    break;
                }
                if (pBmp->mp_pix[idx_tl] == m_color_black || pBmp->mp_pix[idx_tr] == m_color_black ||
                    pBmp->mp_pix[idx_bl] == m_color_black || pBmp->mp_pix[idx_br] == m_color_black) {
                    break;
                }
                pBmp->mp_pix[idx] = m_color_white;
                idx = INVALID_INDEX;
                if (fill_r) {
                    idx = idx_r;
                }
                if (fill_t) {
                    idx = idx_t;
                }
                if (fill_l) {
                    idx = idx_l;
                }
                if (fill_b) {
                    idx = idx_b;
                }
                if (INVALID_INDEX == idx) {
                    break;
                }
            } else {
                break;
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
                // 点リストをポリラインリストに追加
                polyline_list.push_back(eliminatePointsOnStraightLine(point_list));
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
    double len;

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
    {
        pos_o = pos_b;
        pos_b = pos_a;
        pos_a = line_3p[0];
        oa.x = pos_a.x - pos_o.x;
        oa.y = pos_a.y - pos_o.y;
        og.x = pos_b.x - pos_o.x;
        og.y = pos_b.y - pos_o.y;
        auto len = sqrt(oa.x * oa.x + oa.y * oa.y);
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
    polyline.clear();
    polyline.push_back(line_3p[0]);
    polyline.push_back(line_3p[1]);
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
        auto len = sqrt(oa.x * oa.x + oa.y * oa.y);
        oa.x /= len;
        oa.y /= len;
        len = sqrt(og.x * og.x + og.y * og.y);
        og.x /= len;
        og.y /= len;
        auto limit = 1 / (len * 10);
        if (limit < abs(og.x - oa.x) || limit < abs(og.y - oa.y)) {
            polyline.push_back(pos_b);
        }
    }
    {
        pos_o = pos_b;
        pos_b = pos_a;
        pos_a = polyline[0];
        oa.x = pos_a.x - pos_o.x;
        oa.y = pos_a.y - pos_o.y;
        og.x = pos_b.x - pos_o.x;
        og.y = pos_b.y - pos_o.y;
        auto len = sqrt(oa.x * oa.x + oa.y * oa.y);
        oa.x /= len;
        oa.y /= len;
        len = sqrt(og.x * og.x + og.y * og.y);
        og.x /= len;
        og.y /= len;
        auto limit = 1 / (len * 10);
        if (limit < abs(og.x - oa.x) || limit < abs(og.y - oa.y)) {
            polyline.push_back(pos_b);
        }
    }

    vector<point_d> ret;
    for (int32 i = 0; i < polyline.size(); i++) {
        point_d tmp = { polyline[i].x, polyline[i].y };
        ret.push_back(tmp);
    }
    return ret;
}
