#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "ExtPolyline.h"

uint32 fn_support_mqo_create_vertex(
    const point position,
    type_mqo_object* pobj,
    int32 height) {
    uint32 id = (*pobj).vertex.size();
    type_mqo_vertex vertex = {
        static_cast<double>(position.x),
        0,
        static_cast<double>(height - position.y - 1),
        id
    };
    (*pobj).vertex.push_back(vertex);

    return (id);
}

vector<vector<point>>
get_polyline(Bitmap* pbmp, type_worktable* table) {
    const auto size_table = static_cast<uint32>(pbmp->info_h.width * pbmp->info_h.height);
    const auto on = table->color_on;
    const auto off = table->color_off;
    vector<vector<point>> lines;

    auto pPix = pbmp->pPix;
    auto pr = &pbmp->pPalette[1];
    auto pg = &pbmp->pPalette[2];
    auto pb = &pbmp->pPalette[3];
    auto ps = &pbmp->pPalette[4];
    pr->r = 255;
    pr->g = 0;
    pr->b = 0;
    pg->r = 0;
    pg->g = 255;
    pg->b = 0;
    pb->r = 0;
    pb->g = 0;
    pb->b = 255;
    ps->r = 0;
    ps->g = 255;
    ps->b = 255;

    point trace_dir[] = {
        {  1,  0 },
        {  1,  1 },
        {  0,  1 },
        { -1,  1 },
        { -1,  0 },
        { -1, -1 },
        {  0, -1 },
        {  1, -1 }
    };

    /*** ポリラインループ ***/
    point pos;
    int32 off_index = 0;
    while (true) {
        bool line_found = false;
        vector<point> line;
        for (uint32 i = 0; i < size_table; i++) {
            auto pCell = &table->pCells[i];
            if (pCell->enable) {
                pCell->enable = false;
                pPix[i] = off_index + 1;
                pos = pCell->pos;
                line.push_back(pCell->pos);
                line_found = true;
                break;
            }
        }
        if (!line_found) {
            break;
        }

        /*** 点トレースループ ***/
        int32 pre_dir = 0;
        while (true) {
            bool point_found = false;
            for (int32 i = -3; i <= 3; i++) {
                auto dir = (i + pre_dir + 8) % 8;
                auto index = bitmap_get_index_ofs(*pbmp, pos, trace_dir[dir].x, trace_dir[dir].y);
                if (UINT32_MAX == index) {
                    continue;
                }
                auto pCell = &table->pCells[index];
                if (pCell->enable) {
                    pCell->enable = false;
                    pPix[index] = off_index + 1;
                    pos = pCell->pos;
                    pre_dir = dir;
                    line.push_back(pos);
                    point_found = true;
                    break;
                }
            }
            if (!point_found) {
                if (3 <= line.size()) {
                    lines.push_back(line);
                    off_index = (off_index + 1) % 4;
                }
                break;
            }
        }
    }

    return lines;
}

type_mqo_object fn_convert_table_to_mqo(Bitmap* pbmp) {
    type_mqo_object obj;
    obj.error = -1;

    // Table
    const auto size_table = static_cast<uint32>(pbmp->info_h.width * pbmp->info_h.height);
    type_worktable table;
    table.pCells = static_cast<type_workcell*>(calloc(size_table, sizeof(type_workcell)));
    if (NULL == table.pCells) {
        pbmp->error = -1;
        return (obj);
    }

    fn_worktable_create(&table, *pbmp);
    if (table.error != 0) {
        return (obj);
    }

    auto lines = get_polyline(pbmp, &table);
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        uint32 id = obj.face.size();
        type_mqo_face face;
        face.material = INT16_MAX;
        face.id = id;
        auto line = lines[i];
        for (uint32 j = 0; j < line.size(); j++) {
            auto pos = line[j];
            fn_support_mqo_create_vertex(pos, &obj, pbmp->info_h.height);
            face.vertex.push_back(index_ofs + j);
        }
        index_ofs += line.size();
        obj.face.push_back(face);
    }

    obj.error = 0;
    return (obj);
}
