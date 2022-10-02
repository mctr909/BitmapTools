#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "OutputMQO.h"

void
__output_mqo_create_vertex(const point pos, type_mqo_object* pobj) {
    auto id = (*pobj).vertex.size();
    type_mqo_vertex vertex = {
        static_cast<double>(pos.x),
        0,
        static_cast<double>(pos.y),
        id
    };
    (*pobj).vertex.push_back(vertex);
}

type_mqo_object
output_mqo_exec(Bitmap* pbmp) {
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

    worktable_create(&table, *pbmp);
    if (table.error != 0) {
        return (obj);
    }

    worktable_write_outline(table, pbmp);

    worktable_create(&table, *pbmp);
    if (table.error != 0) {
        return (obj);
    }

    auto lines = worktable_create_polyline(&table, *pbmp);

#ifdef DEBUG_OUTPUT_MQO
    auto p = &pbmp->pPalette[0];
    p->r = 239;
    p->g = 239;
    p->b = 239;
    p = &pbmp->pPalette[1];
    p->r = 0;
    p->g = 211;
    p->b = 0;
    p = &pbmp->pPalette[2];
    p->r = 0;
    p->g = 191;
    p->b = 191;
    p = &pbmp->pPalette[3];
    p->r = 0;
    p->g = 0;
    p->b = 255;
    p = &pbmp->pPalette[4];
    p->r = 211;
    p->g = 0;
    p->b = 211;
    p = &pbmp->pPalette[5];
    p->r = 255;
    p->g = 0;
    p->b = 0;
    int32 color = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        auto line = lines[i];
        auto point_count = line.size();
        if (3 <= point_count) {
            for (uint32 j = 0; j < point_count; j++) {
                auto pos = line[j];
                auto index = bitmap_get_index(*pbmp, pos);
                if (UINT32_MAX == index) {
                    continue;
                }
                pbmp->pPix[index] = static_cast<byte>(color + 1);
            }
            color = (color + 1) % 4;
        } else {
            for (uint32 j = 0; j < point_count; j++) {
                auto pos = line[j];
                auto index = bitmap_get_index(*pbmp, pos);
                if (UINT32_MAX == index) {
                    continue;
                }
                pbmp->pPix[index] = 5;
            }
        }
    }
#endif
    vector<vector<uint32>> index_list;
    vector<point> verts;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        auto line = lines[i];
        auto point_count = line.size();
        if (point_count < 3) {
            continue;
        }
        vector<uint32> index;
        for (uint32 j = 0; j < point_count; j++) {
            auto pos = line[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            verts.push_back(pos);
            index.push_back(index_ofs + point_count - j - 1);
            __output_mqo_create_vertex(pos, &obj);
        }
        index_list.push_back(index);
        index_ofs += point_count;
    }

    for (uint32 i = 0; i < index_list.size(); i++) {
        auto index = index_list[i];

        vector<surface> surf;
        worktable_create_polygon(verts, &index, &surf);

        for (uint32 j = 0; j < surf.size(); j++) {
            uint32 id = obj.face.size();
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = id;
            auto idx = surf[j];
            face.vertex.push_back(idx.a);
            face.vertex.push_back(idx.o);
            face.vertex.push_back(idx.b);
            obj.face.push_back(face);
        }
    }

    obj.error = 0;
    return (obj);
}
