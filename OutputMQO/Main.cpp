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

#define DEBUG

string bmp_file_path;

struct INDEX {
    int value;
    bool join;
};

inline bool
has_intersect_line(point_d a, point_d b, point_d c, point_d d) {
    point_d ab = { b.x - a.x, b.y - a.y };
    point_d cd = { d.x - c.x, d.y - c.y };
    auto denom = ab.x * cd.y - ab.y * cd.x;
    if (abs(denom) < 1e-6) {
        return false;
    }
    point_d ac = { c.x - a.x, c.y - a.y };
    auto s = (ac.x * cd.y - ac.y * cd.x) / denom;
    auto t = (ac.x * ab.y - ac.y * ab.x) / denom;
    if (s < 0 || 1 < s || t < 0 || 1 < t) {
        return false;
    }
    return true;
}

inline bool
has_inner_point(point_d va, point_d vo, point_d vb, point_d p) {
    point_d oq, op;
    oq.x = va.x - vb.x;
    oq.y = va.y - vb.y;
    op.x = p.x - vb.x;
    op.y = p.y - vb.y;
    auto normal_abp = oq.x * op.y - oq.y * op.x;
    oq.x = vo.x - va.x;
    oq.y = vo.y - va.y;
    op.x = p.x - va.x;
    op.y = p.y - va.y;
    auto normal_oap = oq.x * op.y - oq.y * op.x;
    oq.x = vb.x - vo.x;
    oq.y = vb.y - vo.y;
    op.x = p.x - vo.x;
    op.y = p.y - vo.y;
    auto normal_bop = oq.x * op.y - oq.y * op.x;
    if (normal_abp < 0 && normal_oap < 0 && normal_bop < 0) {
        return true;
    }
    if (normal_abp > 0 && normal_oap > 0 && normal_bop > 0) {
        return true;
    }
    return false;
}

inline bool
has_inner_polygon(vector<surface>& outer_surf, vector<surface>& inner_surf, vector<point_d>& vert) {
    for (uint32 i = 0; i < outer_surf.size(); i++) {
        auto outer = outer_surf[i];
        auto outer_a = vert[outer.a];
        auto outer_o = vert[outer.o];
        auto outer_b = vert[outer.b];
        for (uint32 j = 0; j < inner_surf.size(); j++) {
            auto inner = inner_surf[j];
            auto inner_a = vert[inner.a];
            auto inner_o = vert[inner.o];
            auto inner_b = vert[inner.b];

            if (has_intersect_line(outer_o, outer_a, inner_o, inner_a)) {
                return true;
            }
            if (has_intersect_line(outer_o, outer_a, inner_o, inner_b)) {
                return true;
            }
            if (has_intersect_line(outer_o, outer_a, inner_a, inner_b)) {
                return true;
            }

            if (has_intersect_line(outer_o, outer_b, inner_o, inner_a)) {
                return true;
            }
            if (has_intersect_line(outer_o, outer_b, inner_o, inner_b)) {
                return true;
            }
            if (has_intersect_line(outer_o, outer_b, inner_a, inner_b)) {
                return true;
            }

            if (has_intersect_line(outer_a, outer_b, inner_o, inner_a)) {
                return true;
            }
            if (has_intersect_line(outer_a, outer_b, inner_o, inner_b)) {
                return true;
            }
            if (has_intersect_line(outer_a, outer_b, inner_a, inner_b)) {
                return true;
            }

            if (has_inner_point(outer_a, outer_o, outer_b, inner_a)) {
                return true;
            }
            if (has_inner_point(outer_a, outer_o, outer_b, inner_o)) {
                return true;
            }
            if (has_inner_point(outer_a, outer_o, outer_b, inner_b)) {
                return true;
            }
        }
    }
    return false;
}

void
create_vertex(const point_d pos, MQO::type_object* p_obj, double y) {
    auto id = static_cast<uint32>(p_obj->vertex.size());
    MQO::type_vertex vertex = { pos.x, y, pos.y, id };
    p_obj->vertex.push_back(vertex);
}

double
create_polygon(vector<point_d>& vert_list, vector<INDEX>& index_list, vector<surface>* pSurf_list, int32 order) {
    const auto INDEX_COUNT = static_cast<uint32>(index_list.size());
    const auto INDEX_NEXT = INDEX_COUNT + order;
    const auto INDEX_RIGHT = 1;
    const auto INDEX_LEFT = INDEX_COUNT - 1;
    struct type_vert_info {
        double distance;
        bool deleted;
    };
    /*** 頂点情報を作成、原点からの距離と削除フラグを設定 ***/
    auto p_vert_info = (type_vert_info*)calloc(INDEX_COUNT, sizeof(type_vert_info));
    for (uint32 i = 0; i < INDEX_COUNT; i++) {
        auto x = vert_list[index_list[i].value].x - INT32_MIN;
        auto y = vert_list[index_list[i].value].y - INT32_MIN;
        p_vert_info[i].distance = sqrt(x * x + y * y);
        p_vert_info[i].deleted = false;
    }
    double area = 0.0;
    uint32 reverse_count = 0;
    uint32 vert_count = 0;
    do { // 最も遠くにある頂点(vo)の取得ループ
        /*** 最も遠くにある頂点(vo)を取得 ***/
        point_d vo; uint32 io = 0;
        double dist_max = 0.0;
        vert_count = 0;
        for (uint32 i = 0; i < INDEX_COUNT; i++) {
            auto info = p_vert_info[i];
            if (info.deleted) {
                continue;
            }
            if (dist_max < info.distance) {
                dist_max = info.distance;
                io = i;
            }
            vert_count++;
        }
        reverse_count = 0;
        vo = vert_list[index_list[io].value];
        while (true) { // 頂点(vo)の移動ループ
            /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
            point_d va; uint32 ia;
            ia = (io + INDEX_LEFT) % INDEX_COUNT;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (p_vert_info[ia].deleted) {
                    ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            va = vert_list[index_list[ia].value];
            /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
            point_d vb; uint32 ib;
            ib = (io + INDEX_RIGHT) % INDEX_COUNT;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (p_vert_info[ib].deleted) {
                    ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            vb = vert_list[index_list[ib].value];
            /*** 三角形(va vo vb)の表裏を確認 ***/
            double aob_normal;
            auto oa_x = va.x - vo.x;
            auto oa_y = va.y - vo.y;
            auto ob_x = vb.x - vo.x;
            auto ob_y = vb.y - vo.y;
            aob_normal = (oa_x * ob_y - oa_y * ob_x) * order;
            if (aob_normal < 0) {
                /*** 裏の場合 ***/
                reverse_count++;
                if (INDEX_COUNT < reverse_count) {
                    /*** 表になる三角形(va vo vb)がない場合 ***/
                    /*** 頂点(vo)を検索対象から削除 ***/
                    p_vert_info[io].deleted = true;
                    /*** 次の最も遠くにある頂点(vo)を取得 ***/
                    break;
                }
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (uint32 i = 0; i < INDEX_COUNT; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = vert_list[index_list[io].value];
                continue;
            }
            /*** 三角形(va vo vb)の内側にva vo vb以外の頂点がないか確認 ***/
            bool point_in_triangle = false;
            for (uint32 i = 0; i < INDEX_COUNT; i++) {
                if (i == ia || i == io || i == ib || p_vert_info[i].deleted) {
                    continue;
                }
                auto p = vert_list[index_list[i].value];
                if (has_inner_point(va, vo, vb, p)) {
                    point_in_triangle = true;
                    break;
                }
            }
            if (point_in_triangle) {
                /*** 内側に他の頂点がある場合 ***/
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (uint32 i = 0; i < INDEX_COUNT; i++) {
                    if (p_vert_info[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = vert_list[index_list[io].value];
            } else {
                /*** 内側に他の頂点がない場合 ***/
                /*** 三角形(va vo vb)を面リストに追加 ***/
                surface surf;
                surf.a = index_list[ia].value;
                surf.o = index_list[io].value;
                surf.b = index_list[ib].value;
                pSurf_list->push_back(surf);
                /*** 三角形の面積を加算 ***/
                area += abs(aob_normal) / 2.0;
                /*** 頂点(vo)を検索対象から削除 ***/
                p_vert_info[io].deleted = true;
                /*** 次の最も遠くにある頂点(vo)を取得 ***/
                break;
            }
        } // 頂点(vo)の移動ループ
    } while (3 < vert_count); // 最も遠くにある頂点(vo)の取得ループ
    free(p_vert_info);
    return area;
}

void
marge_outlines(vector<vector<INDEX>>& indexes, vector<point_d>& verts, int32 order) {
    struct type_nest_info {
        uint32 parent;
        uint32 depth;
        double s;
    };
    vector<type_nest_info> nest_info;
    for (uint32 i = 0; i < indexes.size(); i++) {
        type_nest_info nest;
        nest.parent = 0;
        nest.depth = 0;
        nest.s = 0;
        nest_info.push_back(nest);
    }
    /*** 入れ子になっているアウトラインを検索 ***/
    for (uint32 iOut = 0; iOut < indexes.size(); iOut++) {
        if (indexes[iOut].size() < 3) {
            indexes[iOut].clear();
            continue;
        }
        for (uint32 iIn = 0; iIn < indexes.size(); iIn++) {
            auto inner = &nest_info[iIn];
            if (indexes[iIn].size() < 3) {
                indexes[iIn].clear();
                continue;
            }
            if (nest_info[iOut].depth < inner->depth) {
                continue;
            }
            vector<surface> outer_surf;
            auto outer_area = create_polygon(verts, indexes[iOut], &outer_surf, order);
            if (iOut == iIn) {
                inner->s = outer_area;
                continue;
            }
            vector<surface> inner_surf;
            auto inner_area = create_polygon(verts, indexes[iIn], &inner_surf, order);
            if (inner_area < outer_area && has_inner_polygon(outer_surf, inner_surf, verts)) {
                inner->parent = iOut;
                inner->depth++;
                inner->s = inner_area;
            }
        }
    }
    /*** 穴に該当するアウトラインを親のアウトラインにマージ ***/
    for (uint32 iIn = 0; iIn < nest_info.size(); iIn++) {
        auto inner = &nest_info[iIn];
        printf_s("index:%d\n\tparent:%d\n\tdepth:%d\n\tarea:%f\n", iIn, inner->parent, inner->depth, (float)inner->s);
        /*** depth=0   : 一番外側 ***/
        /*** depth=偶数: 穴に該当しないアウトライン ***/
        if (0 == inner->depth % 2) {
            continue;
        }
        if (iIn == inner->parent) {
            continue;
        }
        /*** 穴に該当するアウトラインと親のアウトラインで互いに最も近い点を検索 ***/
        /*** 互いに最も近い点をマージ開始位置に設定する ***/
        uint32 insert_dst = 0, insert_src = 0;
        double most_near = UINT32_MAX;
        auto index_p = indexes[inner->parent];
        auto index_c = indexes[iIn];
        for (uint32 c = 0; c < index_c.size(); c++) {
            for (uint32 n = 0; n < index_p.size(); n++) {
                auto ip = index_p[n].value;
                auto ic = index_c[c].value;
                auto sx = verts[ic].x - verts[ip].x;
                auto sy = verts[ic].y - verts[ip].y;
                auto dist = sx * sx + sy * sy;
                if (dist < most_near) {
                    insert_dst = n;
                    insert_src = c;
                    most_near = dist;
                }
            }
        }
        /*** マージ ***/
        vector<INDEX> temp;
        for (uint32 i = 0; i <= insert_dst && i < index_p.size(); i++) {
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
        indexes[inner->parent] = temp;
        indexes[iIn].clear();
    }
    printf_s("\n");
}

MQO::type_object
create_object(Bitmap* pbmp, double height, double y_offset) {
    MQO::type_object obj;

    /*** ワークテーブル作成 ***/
    auto pTable = new WorkTable(pbmp->m_info.width, pbmp->m_info.height);
    pTable->Setup(*pbmp, 1.0);

    /*** アウトラインを取得 ***/
    auto outlines = pTable->CreatePolyline();

    delete pTable;

#ifdef DEBUG
    auto p = &pbmp->mp_palette[0];
    p->r = 223;
    p->g = 223;
    p->b = 223;
    p = &pbmp->mp_palette[1];
    p->r = 255;
    p->g = 0;
    p->b = 0;
    p = &pbmp->mp_palette[2];
    p->r = 0;
    p->g = 191;
    p->b = 0;
    p = &pbmp->mp_palette[3];
    p->r = 0;
    p->g = 191;
    p->b = 255;
    p = &pbmp->mp_palette[4];
    p->r = 0;
    p->g = 0;
    p->b = 255;
    byte color = 0;
    for (uint32 i = 0; i < outlines.size(); i++) {
        auto outline = outlines[i];
        for (uint32 j = 0; j < outline.size(); j++) {
            point p = { (int)outline[j].x, (int)outline[j].y };
            auto index = bitmap_get_index(*pbmp, p);
            if (UINT32_MAX != index) {
                pbmp->mp_pix[index] = color + 1;
            }
        }
        color = (color + 1) % 4;
    }
#endif

#ifdef DEBUG_TSV
    string ssdebug = bmp_file_path.substr(0, bmp_file_path.size() - 4) + "_debug.tsv";
    FILE *fp_tsv = nullptr;
    fopen_s(&fp_tsv, ssdebug.c_str(), "w");
    if (nullptr != fp_tsv) {
        fprintf_s(fp_tsv, "outline\tx\ty\tr\tθ\n");
        for (uint32 i = 0; i < outlines.size(); i++) {
            auto outline = outlines[i];
            int bx = 0;
            int by = 0;
            for (uint32 j = 0; j < outline.size(); j++) {
                point p = { (int)outline[j].x, (int)outline[j].y };
                auto index = bitmap_get_index(*pbmp, p);
                if (UINT32_MAX != index) {
                    double dx = p.x - bx;
                    double dy = p.y - by;
                    bx = p.x;
                    by = p.y;
                    auto dr = sqrt(dx * dx + dy * dy);
                    if (0.0 < dr) {
                        dx /= dr;
                        dy /= dr;
                    }
                    auto dtheta = atan2(dy, dx) * 180 / 3.141592;
                    if (dtheta < 0.0) {
                        dtheta += 360;
                    }
                    fprintf_s(fp_tsv, "%d\t%4d\t%4d\t%4.1f\t%3.0f\n", i, p.x, p.y, dr, dtheta);
                }
            }
        }
        fclose(fp_tsv);
    }
#endif

    /*** アウトラインから頂点とインデックスを取得 ***/
    vector<vector<INDEX>> indexes_bottom, indexes_top;
    vector<point_d> verts;
    uint32 index_ofs = 0;
    for (uint32 i = 0; i < outlines.size(); i++) {
        auto outline = outlines[i];
        auto point_count = static_cast<uint32>(outline.size());
        if (0 == point_count) {
            continue;
        }
        /*** 底面 ***/
        vector<INDEX> index_bottom;
        for (uint32 j = 0; j < point_count; j++) {
            point_d pos = { outline[j].x, pbmp->m_info.height - outline[j].y - 1 };
            verts.push_back(pos);
            INDEX idx = { index_ofs + j, false };
            index_bottom.push_back(idx);
            create_vertex(pos, &obj, y_offset);
        }
        indexes_bottom.push_back(index_bottom);
        index_ofs += point_count;
        /*** 上面 ***/
        vector<INDEX> index_top;
        for (uint32 j = 0; j < point_count; j++) {
            point_d pos = { outline[j].x, pbmp->m_info.height - outline[j].y - 1 };
            verts.push_back(pos);
            INDEX idx = { index_ofs + point_count - j - 1, false };
            index_top.push_back(idx);
            create_vertex(pos, &obj, y_offset + height);
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
        if (0 == index.size()) {
            continue;
        }
        vector<surface> surf;
        create_polygon(verts, index, &surf, 1);
        for (uint32 j = 0; j < surf.size(); j++) {
            MQO::type_face face;
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
        if (0 == index.size()) {
            continue;
        }
        vector<surface> surf;
        create_polygon(verts, index, &surf, -1);
        for (uint32 j = 0; j < surf.size(); j++) {
            MQO::type_face face;
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
        if (0 == index_bottom.size() || 0 == index_top.size()) {
            continue;
        }
        auto point_count = static_cast<int32>(index_bottom.size());
        for (int32 ib = 0; ib < point_count; ib++) {
            auto idx0 = index_bottom[(ib + 1) % point_count].value;
            auto idx1 = index_bottom[ib].value;
            uint32 idx2;
            for (int32 it = 0; it < point_count; it++) {
                auto idx = index_top[it].value;
                auto sx = verts[idx].x - verts[idx1].x;
                auto sy = verts[idx].y - verts[idx1].y;
                if (0 == sx * sx + sy * sy) {
                    idx2 = idx;
                    break;
                }
            }
            uint32 idx3;
            for (int32 it = 0; it < point_count; it++) {
                auto idx = index_top[it].value;
                auto sx = verts[idx].x - verts[idx0].x;
                auto sy = verts[idx].y - verts[idx0].y;
                if (0 == sx * sx + sy * sy) {
                    idx3 = idx;
                    break;
                }
            }
            MQO::type_face faceA;
            faceA.material = INT16_MAX;
            faceA.id = static_cast<uint32>(obj.face.size());
            faceA.vertex.push_back(idx0);
            faceA.vertex.push_back(idx1);
            faceA.vertex.push_back(idx2);
            obj.face.push_back(faceA);
            MQO::type_face faceB;
            faceB.material = INT16_MAX;
            faceB.id = static_cast<uint32>(obj.face.size());
            faceB.vertex.push_back(idx0);
            faceB.vertex.push_back(idx2);
            faceB.vertex.push_back(idx3);
            obj.face.push_back(faceB);
        }
    }
    return (obj);
}

int main(int argc, char* argv[]) {
    // check parameter
    if (argc < 5) {
        cout << "format error..." << endl;
        cout << "[format] OutputMQO.exe <height> <y offset> <obj name> <bitmap file path>" << endl;
        return (EXIT_FAILURE);
    }

    const auto height = atof(argv[1]);
    const auto y_offset = atof(argv[2]);
    const auto obj_name = string(argv[3]);

    bmp_file_path = argv[4];
    cout << "BMP FILE : " << bmp_file_path << endl;

    string marge_from;
    if (argc < 6) {
        marge_from = "";
    } else {
        marge_from = string(argv[5]);
    }

    // get bitmap data
    auto pBmp = new Bitmap(bmp_file_path);
    if (pBmp->error != 0) {
        cout << "bitmap reading error... (" << pBmp->error << ")" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    } else {
        pBmp->PrintHeader();
    }

    // check format(8bit palette only)
    if (BITMAP_COLOR_8BIT != pBmp->m_info.bits) {
        cout << "bitmap not support... (8bit palette only)" << endl;
        delete pBmp;
        return (EXIT_FAILURE);
    }

    auto pMqo = new MQO();
    pMqo->m_object = create_object(pBmp, height, y_offset);
    pMqo->m_object.name = obj_name;

    // save file
    stringstream ss;
    ss << bmp_file_path.substr(0, bmp_file_path.size() - 4) << ".mqo";
    pMqo->Write(ss.str(), marge_from);
    //pMqo->WriteStl(ss.str(), marge_from);

#ifdef DEBUG
    stringstream ssdebug;
    ssdebug << bmp_file_path.substr(0, bmp_file_path.size() - 4) << "_debug.bmp";
    pBmp->Save(ssdebug.str());
#endif

    delete pBmp;
    delete pMqo;
    return (EXIT_SUCCESS);
}
