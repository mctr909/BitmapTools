#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/WorkTable.h"
#include "mqo.h"

#pragma comment (lib, "CommonLib.lib")

void
create_vertex(const point pos, type_mqo_object* pobj, double y) {
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
output_mqo(Bitmap* pbmp, double thickness, double y_offset) {
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

    vector<point> verts;
    vector<vector<uint32>> indexes;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < lines.size(); i++) {
        auto line = lines[i];
        auto point_count = line.size();
        if (point_count < 3) {
            continue;
        }
        vector<uint32> index;
        for (int32 j = 0; j < point_count; j++) {
            verts.push_back(line[j]);
            index.push_back(index_ofs + j);
        }
        indexes.push_back(index);
        index_ofs += point_count;
    }
    for (uint32 i = 0; i < indexes.size(); i++) {
        auto index = indexes[i];
        vector<surface> surf;
        worktable_create_polygon(verts, index, &surf, 1);
    }

    /*** 頂点とインデックスを取得 ***/
    vector<vector<uint32>> indexes_bottom, indexes_top;
    verts.clear();
    index_ofs = 0;
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
            create_vertex(pos, &obj, y_offset);
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
            create_vertex(pos, &obj, y_offset + thickness);
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

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 4) {
        cout << "format error..." << endl;
        cout << "[format] BmpOutline.exe <thickness> <y offset> <BMP FILE1> <BMP FILE2> ..." << endl;
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
