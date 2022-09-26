#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "ExtPolyline.h"

vector<vector<Bitmap::position>>
get_polyline(Bitmap* pbmp, type_worktable* table) {
    const auto size_table = static_cast<uint32>(pbmp->info_h.width * pbmp->info_h.height);
    const auto on = table->color_on;
    const auto off = table->color_off;
    vector<vector<Bitmap::position>> lines;

    Bitmap::position delta_pos[] = {
        {  1, 0 },
        {  1, 1 },
        {  0, 1 },
        { -1, 1 },
        { -1, 0 },
        { -1, -1},
        {  0, -1},
        {  1, -1}
    };

    /*** ポリラインループ ***/
    while (true) {
        bool line_found = false;
        Bitmap::position pos;
        vector<Bitmap::position> line;
        for (uint32 i = 0; i < size_table; i++) {
            auto pCell = &table->pCells[i];
            if (pCell->data == on) {
                pCell->data = off;
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
        while (true) {
            bool point_found = false;
            for (uint32 i = 0; i < 8; i++) {
                auto index = bitmap_get_index_ofs(*pbmp, pos, delta_pos[i].x, delta_pos[i].y);
                if (UINT32_MAX == index) {
                    continue;
                }
                auto pCell = &table->pCells[index];
                if (pCell->data == on) {
                    pCell->data = off;
                    pos = pCell->pos;
                    line.push_back(pCell->pos);
                    point_found = true;
                    break;
                }
            }
            if (!point_found) {
                lines.push_back(line);
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

    obj.error = 0;
    return (obj);
}

uint32 fn_support_mqo_create_vertex(
    const Bitmap::position position,
    type_mqo_object* pobj) {
    uint32 id = (*pobj).vertex.size();
    type_mqo_vertex vertex = {
        static_cast<double>(position.x),
        0,
        static_cast<double>(position.y),
        id
    };
    (*pobj).vertex.push_back(vertex);

    return (id);
}

uint32 fn_support_mqo_create_face(
    const uint32 vertex_a,
    const uint32 vertex_b,
    type_mqo_object* pobj) {
    uint32 id = (*pobj).face.size();

    type_mqo_face face;
    face.vertex.push_back(vertex_a);
    face.vertex.push_back(vertex_b);
    face.material = INT16_MAX;
    face.id = id;
    (*pobj).face.push_back(face);

    return (id);
}
