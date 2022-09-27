#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "ExtPolyline.h"

void
fn_support_mqo_create_vertex(const point pos, type_mqo_object* pobj) {
    auto id = (*pobj).vertex.size();
    type_mqo_vertex vertex = {
        static_cast<double>(pos.x),
        0,
        static_cast<double>(pos.y),
        id
    };
    (*pobj).vertex.push_back(vertex);
}

vector<vector<point>>
fn_get_outline(Bitmap* pbmp, type_worktable* table) {
    const auto table_size = pbmp->size_max;
    const auto on = table->color_on;
    const auto off = table->color_off;
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

    vector<vector<point>> outlines;
    point cur_pos;
    while (true) { // アウトライン単位ループ
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
        if (!outline_found) {
            break;
        }
        int32 pre_dir = 0;
        while (true) { // 点単位ループ
            /*** トレースの進行方向を優先して周囲の点を検索 ***/
            /*** あればアウトラインの点として追加 ***/
            bool point_found = false;
            for (int32 i = -3; i <= 3; i++) {
                auto cur_dir = (i + pre_dir + 8) % 8;
                auto index = bitmap_get_index_ofs(*pbmp, cur_pos, trace_dir[cur_dir].x, trace_dir[cur_dir].y);
                if (UINT32_MAX == index) {
                    continue;
                }
                auto pCell = &table->pCells[index];
                if (pCell->enable) {
                    pCell->enable = false;
                    cur_pos = pCell->pos;
                    pre_dir = cur_dir;
                    outline.push_back(cur_pos);
                    point_found = true;
                    break;
                }
            }
            if (!point_found) { // アウトラインの終端
                if (outline.size() < 3) {
                    break;
                }
                /*** 直線上に存在する点を除いたアウトラインをリストに追加 ***/
                vector<point> tmp;
                point pos0 = outline[1];
                point pos1 = outline[0];
                point pos2;
                tmp.push_back(pos1);
                for (int32 i = 2; i < outline.size(); i++) {
                    pos2 = pos1;
                    pos1 = pos0;
                    pos0 = outline[i];
                    double abx = pos0.x - pos2.x;
                    double aby = pos0.y - pos2.y;
                    double px = pos1.x - pos2.x;
                    double py = pos1.y - pos2.y;
                    double dist = sqrt(abx * abx + aby * aby);
                    abx /= dist;
                    aby /= dist;
                    dist = sqrt(px * px + py * py);
                    px = abx - px / dist;
                    py = aby - py / dist;
                    if (0.1 < abs(px) || 0.1 < abs(py)) {
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

type_mqo_object
fn_convert_table_to_mqo(Bitmap* pbmp) {
    type_mqo_object obj;
    obj.error = -1;

    // Table
    const auto table_size = pbmp->size_max;
    type_worktable table;
    table.pCells = static_cast<type_workcell*>(calloc(table_size, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pbmp->error = -1;
        return (obj);
    }

    fn_worktable_create(&table, *pbmp);
    if (table.error != 0) {
        return (obj);
    }

    auto lines = fn_get_outline(pbmp, &table);

    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        uint32 id = obj.face.size();
        type_mqo_face face;
        face.material = INT16_MAX;
        face.id = id;
        auto line = lines[i];
        auto line_count = line.size();
        for (uint32 j = 0; j < line_count; j++) {
            auto pos = line[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            fn_support_mqo_create_vertex(pos, &obj);
            face.vertex.push_back(index_ofs + line_count - j - 1);
        }
        index_ofs += line_count;
        obj.face.push_back(face);
    }

    obj.error = 0;
    return (obj);
}
