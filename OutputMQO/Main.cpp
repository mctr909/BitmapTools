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
    /*** ����q�ɂȂ��Ă���A�E�g���C�������� ***/
    for (uint32 iOut = 0; iOut < indexes.size(); iOut++) {
        if (indexes[iOut].size() < 3) {
            indexes[iOut].clear();
            continue;
        }
        for (uint32 iIn = iOut + 1; iIn < indexes.size(); iIn++) {
            if (indexes[iIn].size() < 3) {
                indexes[iIn].clear();
                continue;
            }
            if (nest_info[iOut].depth < nest_info[iIn].depth) {
                continue;
            }
            vector<surface> outer_surf;
            vector<surface> inner_surf;
            auto outer_area = worktable_create_polygon(verts, indexes[iOut], &outer_surf, order);
            auto inner_area = worktable_create_polygon(verts, indexes[iIn], &inner_surf, order);
            if (outer_area < inner_area) {
                continue;
            }
            if (worktable_inner_polygon(outer_surf, inner_surf, verts)) {
                nest_info[iIn].parent = iOut;
                nest_info[iIn].depth++;
            }
        }
    }
    /*** ���ɊY������A�E�g���C����e�̃A�E�g���C���Ƀ}�[�W ***/
    for (uint32 iNest = 0; iNest < nest_info.size(); iNest++) {
        auto nest = nest_info[iNest];
        /*** depth=0   : ��ԊO�� ***/
        /*** depth=����: ���ɊY�����Ȃ��A�E�g���C�� ***/
        if (0 == nest.depth % 2) {
            continue;
        }
        /*** ���ɊY������A�E�g���C���Ɛe�̃A�E�g���C���Ō݂��ɍł��߂��_������ ***/
        /*** �݂��ɍł��߂��_���}�[�W�J�n�ʒu�ɐݒ肷�� ***/
        uint32 insert_dst = 0, insert_src = 0;
        uint32 most_near = UINT32_MAX;
        auto index_p = indexes[nest.parent];
        auto index_c = indexes[iNest];
        for (uint32 c = 0; c < index_c.size(); c++) {
            for (uint32 n = 0; n < index_p.size(); n++) {
                auto ip = index_p[n];
                auto ic = index_c[c];
                auto sx = verts[ic].x - verts[ip].x;
                auto sy = verts[ic].y - verts[ip].y;
                auto dist = static_cast<uint32>(sx * sx + sy * sy);
                if (dist < most_near) {
                    insert_dst = n;
                    insert_src = c;
                    most_near = dist;
                }
            }
        }
        /*** �}�[�W ***/
        vector<uint32> temp;
        for (uint32 i = 0; i <= insert_dst; i++) {
            temp.push_back(index_p[i]);
        }
        auto inner_size = index_c.size();
        for (int32 i = 0; i < inner_size; i++) {
            auto im = (inner_size + insert_src - i) % inner_size;
            temp.push_back(index_c[im]);
        }
        temp.push_back(index_c[insert_src]);
        for (uint32 i = insert_dst; i < index_p.size(); i++) {
            temp.push_back(index_p[i]);
        }
        indexes[nest.parent] = temp;
        indexes[iNest].clear();
    }
}

type_mqo_object
output_mqo(Bitmap* pbmp, double thickness, double y_offset) {
    type_mqo_object obj;
    obj.error = -1;

    /*** ���[�N�e�[�u���쐬 ***/
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

    /*** �A�E�g���C�����擾 ***/
    auto outlines = worktable_create_polyline(&table, *pbmp);

    /*** �A�E�g���C�����璸�_�ƃC���f�b�N�X���擾 ***/
    vector<vector<uint32>> indexes_bottom, indexes_top;
    vector<point> verts;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < outlines.size(); i++) {
        auto outline = outlines[i];
        auto point_count = static_cast<uint32>(outline.size());
        if (0 == point_count) {
            continue;
        }
        /*** ��� ***/
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
        /*** ��� ***/
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

    /*** �������ɊY������A�E�g���C�����}�[�W ***/
    marge_outlines(indexes_bottom, verts, 1);
    marge_outlines(indexes_top, verts, -1);

    /*** �ʂ��o��(���) ***/
    for (uint32 i = 0; i < indexes_bottom.size(); i++) {
        auto index = indexes_bottom[i];
        if (0 == index.size()) {
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
    /*** �ʂ��o��(���) ***/
    for (uint32 i = 0; i < indexes_top.size(); i++) {
        auto index = indexes_top[i];
        if (0 == index.size()) {
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
    /*** �ʂ��o��(����) ***/
    for (uint32 i = 0; i < indexes_bottom.size(); i++) {
        auto index_bottom = indexes_bottom[i];
        auto index_top = indexes_top[i];
        if (0 == index_bottom.size() || 0 == index_top.size()) {
            continue;
        }
        auto point_count = static_cast<int32>(index_bottom.size());
        for (int32 ib = 0; ib < point_count; ib++) {
            auto idx0 = index_bottom[(ib + 1) % point_count];
            auto idx1 = index_bottom[ib];
            uint32 idx2;
            for (int32 it = 0; it < point_count; it++) {
                auto idx = index_top[it];
                if (idx == idx1) {
                    continue;
                }
                auto sx = verts[idx].x - verts[idx1].x;
                auto sy = verts[idx].y - verts[idx1].y;
                if (0 == sx * sx + sy * sy) {
                    idx2 = idx;
                    break;
                }
            }
            uint32 idx3;
            for (int32 it = 0; it < point_count; it++) {
                auto idx = index_top[it];
                if (idx == idx0) {
                    continue;
                }
                auto sx = verts[idx].x - verts[idx0].x;
                auto sy = verts[idx].y - verts[idx0].y;
                if (0 == sx * sx + sy * sy) {
                    idx3 = idx;
                    break;
                }
            }
            type_mqo_face faceA;
            faceA.material = INT16_MAX;
            faceA.id = static_cast<uint32>(obj.face.size());
            faceA.vertex.push_back(idx0);
            faceA.vertex.push_back(idx1);
            faceA.vertex.push_back(idx2);
            obj.face.push_back(faceA);
            type_mqo_face faceB;
            faceB.material = INT16_MAX;
            faceB.id = static_cast<uint32>(obj.face.size());
            faceB.vertex.push_back(idx0);
            faceB.vertex.push_back(idx2);
            faceB.vertex.push_back(idx3);
            obj.face.push_back(faceB);
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
