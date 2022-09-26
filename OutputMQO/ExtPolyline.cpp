#include <string>
#include <vector>

using namespace std;

#include "../CommonLib/WorkTable.h"
#include "ExtPolyline.h"

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

    const auto on = table.color_on;
    const auto off = table.color_off;
    vector<type_mngmqo> mngs = { size_table,
                                 { ULONG_MAX,
                                   { ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX,
                                     ULONG_MAX } } };

    vector<E_DIRECTION> dir = { E_DIRECTION::RIGHT,
                                   E_DIRECTION::TOP_L,
                                   E_DIRECTION::TOP,
                                   E_DIRECTION::TOP_R };

    vector<E_DIRECTION> dir_o = { E_DIRECTION::LEFT,
                                     E_DIRECTION::BOTTOM_R,
                                     E_DIRECTION::BOTTOM,
                                     E_DIRECTION::BOTTOM_L };



    for (uint32 index_a = 0; index_a < size_table; index_a++) {
        auto cell_a = table.pCells[index_a];
        if (cell_a.data == on) {
            // Vertex
            if (mngs[index_a].index_vertex == ULONG_MAX) {
                mngs[index_a].index_vertex = fn_support_mqo_create_vertex(cell_a.pos, &obj);
            }

            for (int j = 0; j < static_cast<int>(dir.size()); j++) {
                auto cell_b = fn_worktable_get_data(index_a, dir[j], &table, size_table);
                if (cell_b.data == on) {
                    const auto index_b = cell_b.index_bmp;

                    // Vertex
                    if (mngs[index_b].index_vertex == ULONG_MAX) {
                        mngs[index_b].index_vertex = fn_support_mqo_create_vertex(cell_b.pos, &obj);
                    }

                    // Face
                    int dir_a = static_cast<int>(dir[j]);
                    int dir_b = static_cast<int>(dir_o[j]);
                    if ((mngs[index_a].index_face[dir_a] == ULONG_MAX) &&
                        (mngs[index_b].index_face[dir_b] == ULONG_MAX)) {
                        mngs[index_a].index_face[dir_a] = fn_support_mqo_create_face(mngs[index_a].index_vertex,
                            mngs[index_b].index_vertex,
                            &obj);
                        mngs[index_b].index_face[dir_b] = mngs[index_a].index_face[static_cast<int>(dir[j])];
                    }
                }
            }
        }
    }

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
