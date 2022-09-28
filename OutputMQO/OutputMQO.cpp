#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "OutputMQO.h"

void
fn_output_mqo_create_vertex(const point pos, type_mqo_object* pobj) {
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
fn_output_mqo_exec(Bitmap* pbmp) {
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

    auto lines = fn_worktable_outline(pbmp, &table);

#ifdef DEBUG_OUTPUT_MQO
    auto p = &pbmp->pPalette[1];
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
                pbmp->pPix[index] = static_cast<byte>(color + 1);
            }
            color = (color + 1) % 4;
        } else {
            for (uint32 j = 0; j < point_count; j++) {
                auto pos = line[j];
                auto index = bitmap_get_index(*pbmp, pos);
                pbmp->pPix[index] = 5;
            }
        }
    }
#endif

    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        uint32 id = obj.face.size();
        type_mqo_face face;
        face.material = INT16_MAX;
        face.id = id;
        auto line = lines[i];
        auto point_count = line.size();
        if (point_count < 3) {
            continue;
        }
        for (uint32 j = 0; j < point_count; j++) {
            auto pos = line[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            fn_output_mqo_create_vertex(pos, &obj);
            face.vertex.push_back(index_ofs + point_count - j - 1);
        }
        index_ofs += point_count;
        obj.face.push_back(face);
    }

    obj.error = 0;
    return (obj);
}
