#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"
#include "../CommonLib/Mqo.h"

#pragma comment (lib, "CommonLib.lib")

void
create_vertex(const point pos, type_mqo_object* pobj, double y) {
    auto id = static_cast<uint32>((*pobj).vertex.size());
    type_mqo_vertex vertex = {
        static_cast<double>(pos.x),
        y,
        static_cast<double>(pos.y),
        id
    };
    (*pobj).vertex.push_back(vertex);
}

void
marge_outlines(vector<vector<uint32>>& indexes, vector<point>& verts, int32 order) {
    vector<type_nest_info> nest_info;
    for (uint32 i = 0; i < indexes.size(); i++) {
        type_nest_info nest;
        nest.parent = 0;
        nest.depth = 0;
        nest_info.push_back(nest);
    }
    for (uint32 i = 0; i < indexes.size(); i++) {
        for (uint32 j = 0; j < indexes.size(); j++) {
            if (i == j) {
                continue;
            }
            vector<surface> outer_surf;
            vector<surface> inner_surf;
            auto outer_area = worktable_create_polygon(verts, indexes[i], &outer_surf, order);
            auto inner_area = worktable_create_polygon(verts, indexes[j], &inner_surf, order);
            if (outer_area < inner_area) {
                continue;
            }
            auto a = worktable_inner_polygon(outer_surf, inner_surf, verts);
            if (!a) {
                continue;
            }
            nest_info[j].parent = i;
            nest_info[j].depth++;
        }
    }

    for (uint32 me_i = 0; me_i < nest_info.size(); me_i++) {
        auto nest = nest_info[me_i];
        if (0 == nest.depth || 0 == nest.depth % 2) {
            continue;
        }

        auto index_m = indexes[me_i];
        auto index_p = indexes[nest.parent];

        uint32 insert_pos = 0;
        uint32 insert_begin = 0;
        double most_near = UINT32_MAX;
        for (uint32 m = 0; m < index_m.size(); m++) {
            for (uint32 p = 0; p < index_p.size(); p++) {
                auto im = index_m[m];
                auto ip = index_p[p];
                auto sx = verts[im].x - verts[ip].x;
                auto sy = verts[im].y - verts[ip].y;
                auto dist = sqrt(sx * sx + sy * sy);
                if (dist < most_near) {
                    insert_pos = p;
                    insert_begin = m;
                    most_near = dist;
                }
            }
        }

        vector<uint32> tmp;
        for (uint32 i = 0; i <= insert_pos; i++) {
            tmp.push_back(index_p[i]);
        }

        auto inner_size = index_m.size();
        for (int32 i = 0; i < inner_size; i++) {
            auto idx = (inner_size + insert_begin - i) % inner_size;
            tmp.push_back(index_m[idx]);
        }
        tmp.push_back(index_m[insert_begin]);

        for (uint32 i = insert_pos; i < index_p.size(); i++) {
            tmp.push_back(index_p[i]);
        }

        indexes[nest.parent] = tmp;
        indexes[me_i].clear();
    }
}

type_mqo_object
output_mqo(Bitmap* pbmp, double thickness, double y_offset) {
    type_mqo_object obj;
    obj.error = -1;

    /*** ワークテーブル作成 ***/
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

    /*** アウトラインを取得 ***/
    auto outlines = worktable_create_polyline(&table, *pbmp);

    /*** アウトラインから頂点とインデックスを取得 ***/
    vector<vector<uint32>> indexes_bottom, indexes_top;
    vector<point> verts;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < outlines.size(); i++) {
        auto outline = outlines[i];
        auto point_count = static_cast<uint32>(outline.size());
        if (point_count < 3) {
            continue;
        }
        /*** 底面 ***/
        vector<uint32> index_bottom;
        for (uint32 j = 0; j < point_count; j++) {
            auto pos = outline[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            verts.push_back(pos);
            index_bottom.push_back(index_ofs + j);
            create_vertex(pos, &obj, y_offset);
        }
        indexes_bottom.push_back(index_bottom);
        index_ofs += point_count;
        /*** 上面 ***/
        vector<uint32> index_top;
        for (uint32 j = 0; j < point_count; j++) {
            auto pos = outline[j];
            pos.y = pbmp->info_h.height - pos.y - 1;
            verts.push_back(pos);
            index_top.push_back(index_ofs + point_count - j - 1);
            create_vertex(pos, &obj, y_offset + thickness);
        }
        indexes_top.push_back(index_top);
        index_ofs += point_count;
    }

    /*** 穴部分に該当するアウトラインをマージ ***/
    marge_outlines(indexes_bottom, verts, 1);
    marge_outlines(indexes_top, verts, -1);

    /*** 面を出力(底面) ***/
    for (uint32 i = 0; i < indexes_bottom.size(); i++) {
        auto index = indexes_bottom[i];
        if (index.size() < 3) {
            continue;
        }
        vector<surface> surf;
        worktable_create_polygon(verts, index, &surf, 1);
        for (uint32 j = 0; j < surf.size(); j++) {
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = static_cast<uint32>(obj.face.size());
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
        if (index.size() < 3) {
            continue;
        }
        vector<surface> surf;
        worktable_create_polygon(verts, index, &surf, -1);
        for (uint32 j = 0; j < surf.size(); j++) {
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = static_cast<uint32>(obj.face.size());
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
        if (index_bottom.size() < 3 || index_top.size() < 3) {
            continue;
        }
        auto point_count = static_cast<int32>(index_bottom.size());
        for (int32 ib = 0, it = point_count - 1; ib < point_count; ib++, it--) {
            auto idx0 = index_bottom[(ib + 1) % point_count];
            auto idx1 = index_bottom[ib];
            auto idx2 = index_top[it];
            auto idx3 = index_top[(it + point_count - 1) % point_count];
            type_mqo_face face;
            face.material = INT16_MAX;
            face.id = static_cast<uint32>(obj.face.size());
            face.vertex.push_back(idx0);
            face.vertex.push_back(idx1);
            face.vertex.push_back(idx2);
            obj.face.push_back(face);
            face.material = INT16_MAX;
            face.id = static_cast<uint32>(obj.face.size());
            face.vertex.push_back(idx0);
            face.vertex.push_back(idx2);
            face.vertex.push_back(idx3);
            obj.face.push_back(face);
        }
    }
    obj.error = 0;
    return (obj);
}

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 4) {
        cout << "format error..." << endl;
        cout << "[format] OutputMQO.exe <thickness> <y offset> <BMP FILE1> <BMP FILE2> ..." << endl;
        return (EXIT_SUCCESS);
    }

    const auto thickness = atof(argv[1]);
    const auto y_offset = atof(argv[2]);

    string bmp_file;
    for (int32 fcount = 0; fcount < argc - 3; fcount++) {
        bmp_file = argv[fcount + 3];
        cout << "BMP FILE : " << bmp_file << endl;

        // get bitmap data
        auto pBmp = new Bitmap(bmp_file);
        if (pBmp->error != 0) {
            cout << "bmp reading error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        } else {
            pBmp->PrintHeader();
        }

        // palette chck
        if (pBmp->info_h.bits != DEFINE_SUPPORT_COLOR_8BIT) {
            cout << "bmp not support... (only " << DEFINE_SUPPORT_COLOR_8BIT << "bit colors)" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

        type_mqo mqo = fn_mqo_create_default_parameter();
        mqo.object = output_mqo(pBmp, thickness, y_offset);
        if (mqo.object.error != 0) {
            cout << "bmp convert error... (" << pBmp->error << ")" << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

        // save
        stringstream ss;
        ss << bmp_file.substr(0, bmp_file.size() - 4) << ".mqo";
        if (fn_mqo_write(&mqo, ss.str())) {
            cout << "bmp writing error..." << endl;
            delete pBmp;
            return (EXIT_SUCCESS);
        }

        delete pBmp;
    }
    return (EXIT_SUCCESS);
}
