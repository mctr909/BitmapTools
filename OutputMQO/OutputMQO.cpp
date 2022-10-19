#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "OutputMQO.h"

void
__output_mqo_create_vertex(const point pos, type_mqo_object* pobj, double y) {
    auto id = (*pobj).vertex.size();
    type_mqo_vertex vertex = {
        static_cast<double>(pos.x),
        y,
        static_cast<double>(pos.y),
        id
    };
    (*pobj).vertex.push_back(vertex);
}

type_mqo_object
output_mqo_exec(Bitmap* pbmp, double thickness, double y_offset) {
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

    /*** 頂点とインデックスを取得 ***/
    vector<point> verts;
    vector<vector<uint32>> indexes_bottom;
    vector<vector<uint32>> indexes_top;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        auto line = lines[i];
        auto point_count = line.size();
        if (point_count < 3) {
            continue;
        }
        /*** 底面 ***/
        vector<uint32> index_bottom;
        for (int32 j = 0; j < point_count; j++) {
            auto pos = line[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            verts.push_back(pos);
            index_bottom.push_back(index_ofs + j);
            __output_mqo_create_vertex(pos, &obj, y_offset);
        }
        indexes_bottom.push_back(index_bottom);
        index_ofs += point_count;
        /*** 上面 ***/
        vector<uint32> index_top;
        for (int32 j = 0; j < point_count; j++) {
            auto pos = line[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            verts.push_back(pos);
            index_top.push_back(index_ofs + point_count - j - 1);
            __output_mqo_create_vertex(pos, &obj, y_offset + thickness);
        }
        indexes_top.push_back(index_top);
        index_ofs += point_count;
    }
    /*** 面を出力(底面) ***/
    for (uint32 i = 0; i < indexes_bottom.size(); i++) {
        auto index = indexes_bottom[i];
        vector<surface> surf;
        worktable_create_polygon(verts, index, &surf, 1);
        for (uint32 j = 0; j < surf.size(); j++) {
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = obj.face.size();
            auto idx = surf[j];
            face.vertex.push_back(idx.a);
            face.vertex.push_back(idx.o);
            face.vertex.push_back(idx.b);
            obj.face.push_back(face);
        }
    }
    /*** 面を出力(上面) ***/
    for (uint32 i = 0; i < indexes_top.size(); i++) {
        auto index = indexes_top[i];
        vector<surface> surf;
        worktable_create_polygon(verts, index, &surf, -1);
        for (uint32 j = 0; j < surf.size(); j++) {
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = obj.face.size();
            auto idx = surf[j];
            face.vertex.push_back(idx.a);
            face.vertex.push_back(idx.o);
            face.vertex.push_back(idx.b);
            obj.face.push_back(face);
        }
    }
    /*** 面を出力(側面) ***/
    for (uint32 i = 0; i < indexes_bottom.size(); i++) {
        auto index_bottom = indexes_bottom[i];
        auto index_top = indexes_top[i];
        auto point_count = index_bottom.size();
        for (int32 ib = 0, it = point_count - 1; ib < point_count; ib++, it--) {
            auto idx0 = index_bottom[(ib + 1) % point_count];
            auto idx1 = index_bottom[ib];
            auto idx2 = index_top[it];
            auto idx3 = index_top[(it + point_count - 1) % point_count];
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = obj.face.size();
            face.vertex.push_back(idx0);
            face.vertex.push_back(idx1);
            face.vertex.push_back(idx2);
            obj.face.push_back(face);
            face.material = INT16_MAX;
            face.id = obj.face.size();
            face.vertex.push_back(idx0);
            face.vertex.push_back(idx2);
            face.vertex.push_back(idx3);
            obj.face.push_back(face);
        }
    }
    obj.error = 0;
    return (obj);
}
