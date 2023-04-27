#include <string>
#include <vector>

using namespace std;

#include "WorkTable.h"
#include "Bitmap.h"

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
        int32 x;
        int32 y;
        bool enable;
    };
    type_delta delta_pos[9] = {
        /* ����             x   y  enable */
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

    /*** ���[�N�e�[�u���̏����� ***/
    point pos;
    for (pos.y = 0; pos.y < height; pos.y++) {
        for (pos.x = 0; pos.x < width; pos.x++) {
            /*** �s�N�Z�������Z�b�g***/
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

            /* ����, ��, ���� */
            if (0 == pos.x) {
                delta_pos[0].enable = delta_pos[3].enable = delta_pos[6].enable = false;
            } else {
                delta_pos[0].enable = delta_pos[3].enable = delta_pos[6].enable = true;
            }
            /* ����, ��, �E�� */
            if (0 == pos.y) {
                delta_pos[0].enable = delta_pos[1].enable = delta_pos[2].enable = false;
            } else {
                delta_pos[0].enable = delta_pos[1].enable = delta_pos[2].enable = true;
            }
            /* �E��, �E, �E�� */
            if (1 == (width - pos.x)) {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[2].enable = delta_pos[5].enable = delta_pos[8].enable = true;
            }
            /* ����, ��, �E�� */
            if (1 == (height - pos.y)) {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = false;
            } else {
                delta_pos[6].enable = delta_pos[7].enable = delta_pos[8].enable = true;
            }
            /*** ���̓s�N�Z���̃C���f�b�N�X���擾���ăZ�b�g ***/
            for (uint32 i = 0; i < 9; i++) {
                if (delta_pos[i].enable) {
                    mp_cells[index].index_around[i] = get_index_ofs(
                        pos,
                        delta_pos[i].x, delta_pos[i].y
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
    /*** �p���b�g����ňÐF�ƍŖ��F���擾 ***/
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
    /*** �s�N�Z������h���Ă��Ȃ������̋P�x���擾 ***/
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
    /*** �s�N�Z������h���̋P�x���擾 ***/
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
    /*** �h���ł��邩���Z�b�g/�g���[�X��Ԃ̃N���A ***/
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
        bool nofill_bottom = !get_cell(i, E_DIRECTION::BOTTOM).filled;
        bool nofill_right = !get_cell(i, E_DIRECTION::RIGHT).filled;
        bool nofill_left = !get_cell(i, E_DIRECTION::LEFT).filled;
        bool nofill_top = !get_cell(i, E_DIRECTION::TOP).filled;
        if (nofill_bottom || nofill_right || nofill_left || nofill_top) {
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
}

vector<vector<point>>
WorkTable::CreatePolyline() {
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

    vector<vector<point>> polyline_list;
    uint32 start_index = 0;
    while (true) { // �|�����C���擾���[�v
        vector<point> point_list; // �_���X�g
        point current_pos;        // ���݂̈ʒu
        int32 current_dir = 0;    // ���݂̐i�s����
        /*** �|�����C���n�_������ ***/
        bool polyline_found = false;
        for (uint32 i = start_index; i < m_pixel_count; i++) {
            auto pCell = &mp_cells[i];
            if (pCell->filled && !pCell->traced) {
                // �|�����C���n�_�𔭌�
                polyline_found = true;
                pCell->traced = true;
                // �|�����C���̎n�_�Ƃ��ē_���X�g�ɒǉ�
                current_pos = pCell->pos;
                point_list.push_back(current_pos);
                // ���̃|�����C���n�_�̌����J�n�C���f�b�N�X���Z�b�g
                start_index = i;
                // �|�����C���n�_�̌����I��
                break;
            }
        }
        if (!polyline_found) { // �c���Ă���|�����C���Ȃ�
            break;
        }
        while (true) { // �_�g���[�X���[�v
            /*** ���݂̐i�s������D�悵�Ĕ��a���L���Ȃ�����͂̓_������ ***/
            /*** ����΃|�����C���̓_�Ƃ��ē_���X�g�ɒǉ� ***/
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
                        // �_�𔭌�
                        point_found = true;
                        pCell->traced = true;
                        // �g���[�X���������݂̐i�s�����Ƃ���
                        current_dir = trace_dir;
                        // �|�����C���̓_�Ƃ��ē_���X�g�ɒǉ�
                        current_pos = pCell->pos;
                        point_list.push_back(current_pos);
                        // ���̓_������
                        break;
                    }
                }
                if (point_found) { // ���̓_������
                    break;
                }
            }
            if (!point_found) { // �|�����C���̏I�[
                // ������ɂ���_��_���X�g���珜�O����
                eliminatePointsOnStraightLine(&point_list);
                // �_���X�g���|�����C�����X�g�ɒǉ�
                polyline_list.push_back(point_list);
                break;
            }
        }
    }
    return polyline_list;
}

void
WorkTable::eliminatePointsOnStraightLine(vector<point>* pPolyline) {
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

    /*** 3�_�̒����`�F�b�N ***/
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

    /*** 4�_�̒����`�F�b�N ***/
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
