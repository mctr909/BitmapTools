#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "../CommonLib/Outline.h"
#include "Mqo.h"

#pragma comment (lib, "CommonLib.lib")

#define DEBUG_OUTLINE
#define DEBUG_SURFACE

typedef vector<uint32_t> INDEX;
typedef vector<vec3> VERT;
typedef vector<surface> SURF;

string bmp_file_path;

inline bool
has_intersect_line(vec3 &a, vec3 &b, vec3 &c, vec3 &d) {
	auto ab = b - a;
	auto cd = d - c;
	auto denom = (ab * cd).z;
	if (fabs(denom) < 1e-6) {
		return false;
	}
	auto ac = c - a;
	auto s = (ac * cd / denom).z;
	auto t = (ac * ab / denom).z;
	if (s < 0 || 1 < s || t < 0 || 1 < t) {
		return false;
	}
	return true;
}

inline bool
has_inner_point(vec3 &a, vec3 &o, vec3 &b, vec3 &p) {
	auto op = p - a;
	auto oq = o - a;
	auto oapNormal = (oq * op).z;
	op = p - o;
	oq = b - o;
	auto bopNormal = (oq * op).z;
	op = p - b;
	oq = a - b;
	auto abpNormal = (oq * op).z;
	if (oapNormal > 0 && bopNormal > 0 && abpNormal > 0) {
		return true;
	}
	if (oapNormal < 0 && bopNormal < 0 && abpNormal < 0) {
		return true;
	}
	if (oapNormal == 0 && (bopNormal > 0 && abpNormal > 0 || abpNormal < 0 && bopNormal < 0)) {
		return true;
	}
	if (bopNormal == 0 && (abpNormal > 0 && oapNormal > 0 || oapNormal < 0 && abpNormal < 0)) {
		return true;
	}
	if (abpNormal == 0 && (oapNormal > 0 && bopNormal > 0 || bopNormal < 0 && oapNormal < 0)) {
		return true;
	}
	return false;
}

inline bool
has_inner_polygon(SURF &outerSurf, SURF &innerSurf, VERT &vert) {
	for (int32_t i = 0; i < outerSurf.size(); i++) {
		auto outer = outerSurf[i];
		auto outerA = vert[outer.a];
		auto outerO = vert[outer.o];
		auto outerB = vert[outer.b];
		auto innerCount = innerSurf.size();
		for (int32_t j = 0; j < innerCount; j++) {
			auto inner = innerSurf[j];
			auto innerA = vert[inner.a];
			auto innerO = vert[inner.o];
			auto innerB = vert[inner.b];
			if (has_inner_point(outerA, outerO, outerB, innerA)) {
				return true;
			}
			if (has_inner_point(outerA, outerO, outerB, innerO)) {
				return true;
			}
			if (has_inner_point(outerA, outerO, outerB, innerB)) {
				return true;
			}
		}
	}
	return false;
}

void
create_vertex(const vec3 pos, MQO::type_object *pObj, double y) {
	auto id = static_cast<uint32_t>(pObj->vertex.size());
	MQO::type_vertex vertex = {pos.x, y, pos.y, id};
	pObj->vertex.push_back(vertex);
}

double
create_polygon(VERT &vert, INDEX &index, SURF *pSurfList, int32_t order) {
	const auto INDEX_COUNT = static_cast<int32_t>(index.size());
	const auto INDEX_NEXT = INDEX_COUNT + order;
	const auto INDEX_RIGHT = 1;
	const auto INDEX_LEFT = INDEX_COUNT - 1;
	struct VERT_INFO {
		double distance;
		bool deleted;
	};
	/*** 頂点情報を作成、原点からの距離と削除フラグを設定 ***/
	auto pVertInfo = (VERT_INFO *)calloc(INDEX_COUNT, sizeof(VERT_INFO));
	auto origin = vec3{INT32_MIN, INT32_MIN, 0};
	for (int32_t i = 0; i < INDEX_COUNT; i++) {
		auto ov = vert[index[i]] - origin;
		pVertInfo[i].distance = sqrt(ov & ov);
		pVertInfo[i].deleted = false;
	}
	double area = 0.0;
	int32_t reverseCount = 0;
	int32_t vertCount = 0;
	do { // 最も遠くにある頂点(vo)の取得ループ
		/*** 最も遠くにある頂点(vo)を取得 ***/
		vec3 vo; int32_t io = 0;
		double distMax = 0.0;
		vertCount = 0;
		for (int32_t i = 0; i < INDEX_COUNT; i++) {
			auto info = pVertInfo[i];
			if (info.deleted) {
				continue;
			}
			if (distMax < info.distance) {
				distMax = info.distance;
				io = i;
			}
			vertCount++;
		}
		reverseCount = 0;
		vo = vert[index[io]];
		while (true) { // 頂点(vo)の移動ループ
			/*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
			vec3 va; int32_t ia;
			ia = (io + INDEX_LEFT) % INDEX_COUNT;
			for (int32_t i = 0; i < INDEX_COUNT; i++) {
				if (pVertInfo[ia].deleted) {
					ia = (ia + INDEX_LEFT) % INDEX_COUNT;
				} else {
					break;
				}
			}
			va = vert[index[ia]];
			/*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
			vec3 vb; int32_t ib;
			ib = (io + INDEX_RIGHT) % INDEX_COUNT;
			for (int32_t i = 0; i < INDEX_COUNT; i++) {
				if (pVertInfo[ib].deleted) {
					ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
				} else {
					break;
				}
			}
			vb = vert[index[ib]];
			/*** 三角形(va vo vb)の表裏を確認 ***/
			double aobNormal; {
				auto oa = va - vo;
				auto ob = vb - vo;
				aobNormal = (oa * ob).z * order;
			}
			if (aobNormal < 0) {
				/*** 裏の場合 ***/
				reverseCount++;
				if (INDEX_COUNT < reverseCount) {
					/*** 表になる三角形(va vo vb)がない場合 ***/
					/*** 頂点(vo)を検索対象から削除 ***/
					pVertInfo[io].deleted = true;
					/*** 次の最も遠くにある頂点(vo)を取得 ***/
					break;
				}
				/*** 頂点(vo)を隣に移動 ***/
				io = (io + INDEX_NEXT) % INDEX_COUNT;
				for (int32_t i = 0; i < INDEX_COUNT; i++) {
					if (pVertInfo[io].deleted) {
						io = (io + INDEX_NEXT) % INDEX_COUNT;
					} else {
						break;
					}
				}
				vo = vert[index[io]];
				continue;
			}
			/*** 三角形(va vo vb)の内側にva vo vb以外の頂点がないか確認 ***/
			auto pointInTriangle = false;
			for (int32_t i = 0; i < INDEX_COUNT; i++) {
				if (i == ia || i == io || i == ib || pVertInfo[i].deleted) {
					continue;
				}
				auto p = vert[index[i]];
				if (has_inner_point(va, vo, vb, p)) {
					pointInTriangle = true;
					break;
				}
			}
			if (pointInTriangle) {
				/*** 内側に他の頂点がある場合 ***/
				/*** 頂点(vo)を隣に移動 ***/
				io = (io + INDEX_NEXT) % INDEX_COUNT;
				for (int32_t i = 0; i < INDEX_COUNT; i++) {
					if (pVertInfo[io].deleted) {
						io = (io + INDEX_NEXT) % INDEX_COUNT;
					} else {
						break;
					}
				}
				vo = vert[index[io]];
			} else {
				/*** 内側に他の頂点がない場合 ***/
				/*** 三角形(va vo vb)を面リストに追加 ***/
				surface surf;
				surf.a = index[ia];
				surf.o = index[io];
				surf.b = index[ib];
				pSurfList->push_back(surf);
				/*** 三角形の面積を加算 ***/
				area += fabs(aobNormal) / 2.0;
				/*** 頂点(vo)を検索対象から削除 ***/
				pVertInfo[io].deleted = true;
				/*** 次の最も遠くにある頂点(vo)を取得 ***/
				break;
			}
		} // 頂点(vo)の移動ループ
	} while (3 < vertCount); // 最も遠くにある頂点(vo)の取得ループ
	free(pVertInfo);
	return area;
}

void
marge_outlines(vector<INDEX> &indexes, VERT &vert, int32_t order) {
	struct NEST_INFO {
		uint32_t parent;
		uint32_t depth;
	};
	vector<NEST_INFO> nestInfo;
	for (int32_t i = 0; i < indexes.size(); i++) {
		NEST_INFO nest;
		nest.parent = -1;
		nest.depth = 0;
		nestInfo.push_back(nest);
	}
	/*** 入れ子になっているアウトラインを検索 ***/
	for (int32_t idxOuter = 0; idxOuter < indexes.size(); idxOuter++) {
		if (indexes[idxOuter].size() < 3) {
			indexes[idxOuter].clear();
			continue;
		}
		auto innerCount = indexes.size();
		for (int32_t idxInner = 0; idxInner < innerCount; idxInner++) {
			if (indexes[idxInner].size() < 3) {
				indexes[idxInner].clear();
				continue;
			}
			if (idxInner == idxOuter) {
				continue;
			}
			auto inner = &nestInfo[idxInner];
			if (nestInfo[idxOuter].depth < inner->depth) {
				continue;
			}
			SURF outerSurf;
			auto outerArea = create_polygon(vert, indexes[idxOuter], &outerSurf, order);
			SURF innerSurf;
			auto innerArea = create_polygon(vert, indexes[idxInner], &innerSurf, order);
			if (innerArea < outerArea && has_inner_polygon(outerSurf, innerSurf, vert)) {
				inner->parent = idxOuter;
				inner->depth++;
			}
		}
	}
	/*** 穴に該当するアウトラインを親のアウトラインにマージ ***/
	while (true) {
		double mostNear = 1e20;
		int32_t idxInner;
		NEST_INFO *inner = NULL;
		auto nestCount = nestInfo.size();
		for (int32_t i = 0; i < nestCount; i++) {
			auto innerTemp = &nestInfo[i];
			if (0 == innerTemp->depth % 2) {
				/* depth=偶数: 穴に該当しないアウトライン */
				continue;
			}
			if (i == innerTemp->parent) {
				continue;
			}
			if (indexes[i].size() < 3) {
				continue;
			}
			auto pos = vert[indexes[i][0]];
			double ox, oy;
			if (order < 0) {
				ox = pos.x - INT32_MAX;
				oy = pos.y - INT32_MAX;
			} else {
				ox = pos.x - INT32_MIN;
				oy = pos.y - INT32_MIN;
			}
			auto dist = ox * ox + oy * oy;
			if (dist < mostNear) {
				/* 原点から近いアウトラインを優先してマージする */
				idxInner = i;
				inner = innerTemp;
				mostNear = dist;
			}
		}
		if (NULL == inner) {
			break;
		}
		/*** 穴に該当するアウトラインと親のアウトラインで互いに最も近い点を検索 ***/
		/*** 互いに最も近い点をマージ開始位置に設定する ***/
		int32_t insertDst = 0, insertSrc = 0;
		mostNear = UINT32_MAX;
		auto indexP = indexes[inner->parent];
		auto indexC = indexes[idxInner];
		auto parentCount = indexP.size();
		for (int32_t c = 0; c < indexC.size(); c++) {
			for (int32_t p = 0; p < parentCount; p++) {
				auto ip = indexP[p];
				auto ic = indexC[c];
				auto pc = vert[ic] - vert[ip];
				auto dist = pc & pc;
				if (dist < mostNear) {
					insertDst = p;
					insertSrc = c;
					mostNear = dist;
				}
			}
		}
		/*** マージ ***/
		INDEX temp;
		for (int32_t i = 0; i <= insertDst && i < indexP.size(); i++) {
			temp.push_back(indexP[i]);
		}
		auto innerSize = indexC.size();
		for (int32_t i = 0; i < innerSize; i++) {
			auto im = (innerSize + insertSrc - i) % innerSize;
			temp.push_back(indexC[im]);
		}
		temp.push_back(indexC[insertSrc]);
		for (int32_t i = insertDst; i < indexP.size(); i++) {
			temp.push_back(indexP[i]);
		}
		indexes[inner->parent] = temp;
		indexes[idxInner].clear();
	}
}

MQO::type_object
create_object(Bitmap *pbmp, double height, double y_offset, double scale) {
	MQO::type_object obj;

	/*** アウトラインを取得 ***/
	auto pOutline = new Outline(pbmp);
	pOutline->Read(1.0);
	auto outlines = pOutline->CreatePolyline();
	delete pOutline;

	/*** スケール反映 ***/
	for (int32_t i = 0; i < outlines.size(); i++) {
		for (int32_t j = 0; j < outlines[i].size(); j++) {
			outlines[i][j] *= scale;
		}
	}

	auto swidth = static_cast<int32_t>(pbmp->info.width * scale);
	auto sheight = static_cast<int32_t>(pbmp->info.height * scale);

#ifdef DEBUG_OUTLINE
	string svgLinePath = pbmp->filePath + "_Line.svg";
	ofstream svgLine(svgLinePath, ios::out);
	svgLine << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
	svgLine << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
	svgLine << "<svg width=\"" << swidth << "\" height=\"" << sheight << "\""
		<< " viewBox=\"0 0 " << swidth << " " << sheight << "\""
		<< " version=\"1.1\""
		<< " xmlns=\"http://www.w3.org/2000/svg\""
		<< " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
		<< " xmlns:serif=\"http://www.serif.com/\""
		<< " xml:space=\"preserve\""
		<< ">" << endl;
	for (int32_t i = 0; i < outlines.size(); i++) {
		auto outline = outlines[i];
		svgLine << "\t<polygon points=\"";
		for (int32_t j = 0; j < outline.size(); j++) {
			svgLine << outline[j].x << "," << (sheight - outline[j].y) << " ";
		}
		svgLine << "\" fill=\"none\" stroke=\"black\" id=\"line" << (i + 1) << "\"/>" << endl;
	}
	for (int32_t i = 0; i < outlines.size(); i++) {
		auto outline = outlines[i];
		svgLine << "\t<g id=\"dots" << (i + 1) << "\">" << endl;
		for (int32_t j = 0; j < outline.size(); j++) {
			svgLine << "\t\t<circle cx=\"" << outline[j].x
				<< "\" cy=\"" << (sheight - outline[j].y) << "\" r=\"1\""
				<< " stroke=\"black\" stroke-width=\"0.25\" fill=\"cyan\"/>" << endl;
		}
		svgLine << "\t</g>" << endl;
	}
	svgLine << "</svg>" << endl;
	svgLine.close();
#endif

	/*** アウトラインから頂点とインデックスを取得 ***/
	vector<INDEX> indexes_bottom, indexes_top;
	VERT verts;
	uint32_t index_ofs = 0;
	for (int32_t i = 0; i < outlines.size(); i++) {
		auto outline = outlines[i];
		auto point_count = static_cast<int32_t>(outline.size());
		if (0 == point_count) {
			continue;
		}
		/*** 底面 ***/
		INDEX index_bottom;
		for (int32_t j = 0; j < point_count; j++) {
			vec3 pos = {outline[j].x, sheight - outline[j].y - 1, 0.0};
			verts.push_back(pos);
			index_bottom.push_back(index_ofs + j);
			create_vertex(pos, &obj, y_offset);
		}
		indexes_bottom.push_back(index_bottom);
		index_ofs += point_count;
		/*** 上面 ***/
		INDEX index_top;
		for (int32_t j = 0; j < point_count; j++) {
			vec3 pos = {outline[j].x, sheight - outline[j].y - 1, 0.0};
			verts.push_back(pos);
			index_top.push_back(index_ofs + point_count - j - 1);
			create_vertex(pos, &obj, y_offset + height);
		}
		indexes_top.push_back(index_top);
		index_ofs += point_count;
	}

	/*** 穴部分に該当するアウトラインをマージ ***/
	marge_outlines(indexes_bottom, verts, 1);
	marge_outlines(indexes_top, verts, -1);

#ifdef DEBUG_SURFACE
	string svgSurfPath = pbmp->filePath + "_Surf.svg";
	ofstream svgSurf(svgSurfPath, ios::out);
	svgSurf << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
	svgSurf << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
	svgSurf << "<svg width=\"" << swidth << "\" height=\"" << sheight << "\""
		<< " viewBox=\"0 0 " << swidth << " " << sheight << "\""
		<< " version=\"1.1\""
		<< " xmlns=\"http://www.w3.org/2000/svg\""
		<< " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
		<< " xmlns:serif=\"http://www.serif.com/\""
		<< " xml:space=\"preserve\""
		<< ">" << endl;
	for (int32_t i = 0; i < indexes_top.size(); i++) {
		auto index = indexes_top[i];
		if (0 == index.size()) {
			continue;
		}
		svgSurf << "\t<polygon points=\"";
		for (int32_t j = 0; j < index.size(); j++) {
			auto v = verts[index[j]];
			svgSurf << v.x << "," << v.y << " ";
		}
		svgSurf << "\" style=\"fill:#000000; fill-rule:nonzero; stroke-width:1.0\"/>" << endl;
	}
	svgSurf << "</svg>" << endl;
	svgSurf.close();
#endif

	/*** 面を出力(底面) ***/
	for (int32_t i = 0; i < indexes_bottom.size(); i++) {
		auto index = indexes_bottom[i];
		if (0 == index.size()) {
			continue;
		}
		SURF surf;
		create_polygon(verts, index, &surf, 1);
		for (int32_t j = 0; j < surf.size(); j++) {
			MQO::type_face face;
			face.material = INT16_MAX;
			face.id = static_cast<uint32_t>(obj.face.size());
			auto idx = surf[j];
			face.vertex.push_back(idx.a);
			face.vertex.push_back(idx.o);
			face.vertex.push_back(idx.b);
			obj.face.push_back(face);
		}
	}
	/*** 面を出力(上面) ***/
	for (int32_t i = 0; i < indexes_top.size(); i++) {
		auto index = indexes_top[i];
		if (0 == index.size()) {
			continue;
		}
		SURF surf;
		create_polygon(verts, index, &surf, -1);
		for (int32_t j = 0; j < surf.size(); j++) {
			MQO::type_face face;
			face.material = INT16_MAX;
			face.id = static_cast<uint32_t>(obj.face.size());
			auto idx = surf[j];
			face.vertex.push_back(idx.a);
			face.vertex.push_back(idx.o);
			face.vertex.push_back(idx.b);
			obj.face.push_back(face);
		}
	}
	/*** 面を出力(側面) ***/
	for (int32_t i = 0; i < indexes_bottom.size(); i++) {
		auto index_bottom = indexes_bottom[i];
		auto index_top = indexes_top[i];
		if (0 == index_bottom.size() || 0 == index_top.size()) {
			continue;
		}
		auto point_count = static_cast<int32_t>(index_bottom.size());
		for (int32_t ib = 0; ib < point_count; ib++) {
			auto idx0 = index_bottom[(ib + 1) % point_count];
			auto idx1 = index_bottom[ib];
			uint32_t idx2;
			for (int32_t it = 0; it < point_count; it++) {
				auto idx = index_top[it];
				auto sx = verts[idx].x - verts[idx1].x;
				auto sy = verts[idx].y - verts[idx1].y;
				if (0 == sx * sx + sy * sy) {
					idx2 = idx;
					break;
				}
			}
			uint32_t idx3;
			for (int32_t it = 0; it < point_count; it++) {
				auto idx = index_top[it];
				auto sx = verts[idx].x - verts[idx0].x;
				auto sy = verts[idx].y - verts[idx0].y;
				if (0 == sx * sx + sy * sy) {
					idx3 = idx;
					break;
				}
			}
			MQO::type_face faceA;
			faceA.material = INT16_MAX;
			faceA.id = static_cast<uint32_t>(obj.face.size());
			faceA.vertex.push_back(idx0);
			faceA.vertex.push_back(idx1);
			faceA.vertex.push_back(idx2);
			obj.face.push_back(faceA);
			MQO::type_face faceB;
			faceB.material = INT16_MAX;
			faceB.id = static_cast<uint32_t>(obj.face.size());
			faceB.vertex.push_back(idx0);
			faceB.vertex.push_back(idx2);
			faceB.vertex.push_back(idx3);
			obj.face.push_back(faceB);
		}
	}
	return (obj);
}

int main(int argc, char *argv[]) {
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

	double scale;
	if (argc < 6) {
		scale = 1;
	} else {
		scale = atof(argv[5]);
	}

	string marge_from;
	if (argc < 7) {
		marge_from = "";
	} else {
		marge_from = string(argv[6]);
	}

	// get bitmap data
	auto pBmp = new Bitmap(bmp_file_path);
	pBmp->PrintInfo();

	// check format(index color only)
	switch (pBmp->info.bits) {
	case Bitmap::Type::COLOR2:
	case Bitmap::Type::COLOR4:
	case Bitmap::Type::COLOR16:
	case Bitmap::Type::COLOR256:
		break;
	default:
		cout << "bitmap not support... (1bit/2bit/4bit/8bit index color)" << endl;
		delete pBmp;
		return (EXIT_FAILURE);
	}

	auto pMqo = new MQO();
	pMqo->m_object = create_object(pBmp, height, y_offset, scale);
	pMqo->m_object.name = obj_name;

	// save file
	stringstream ss;
	ss << bmp_file_path.substr(0, bmp_file_path.size() - 4) << ".mqo";
	pMqo->Write(ss.str(), marge_from);
	//pMqo->WriteStl(ss.str(), marge_from);

	delete pBmp;
	delete pMqo;
	return (EXIT_SUCCESS);
}
