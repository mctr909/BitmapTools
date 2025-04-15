#include <string>
#include <vector>

using namespace std;

#include "Outline.h"
#include "Bitmap.h"

#define ELIMINATE_THRESHOLD 0.9999

#define PREFER_DIRS 8i32
static const int8_t
PREFER_DIR[PREFER_DIRS] = {
	0, -1, 1, -2, 2, -3, 3, 4
};

#define TRACE_RADIUS 3i32
#define TRACE_DIRS   8i32
static const direction
TRACE_DIR[TRACE_RADIUS][TRACE_DIRS] = {
	{
		{  1,  0 },
		{  1,  1 },
		{  0,  1 },
		{ -1,  1 },
		{ -1,  0 },
		{ -1, -1 },
		{  0, -1 },
		{  1, -1 }
	}, {
		{  2,  1 },
		{  1,  2 },
		{ -1,  2 },
		{ -2,  1 },
		{ -2, -1 },
		{ -1, -2 },
		{  1, -2 },
		{  2, -1 }
	}, {
		{  2,  0 },
		{  2,  2 },
		{  0,  2 },
		{ -2,  2 },
		{ -2,  0 },
		{ -2, -2 },
		{  0, -2 },
		{  2, -2 }
	}
};

Outline::Outline(Bitmap *pBmp) {
	this->pBmp = pBmp;
	colorBlack = 0;
	colorWhite = 0;

	switch (pBmp->info.bits) {
	case Bitmap::Type::COLOR2:
	case Bitmap::Type::COLOR4:
	case Bitmap::Type::COLOR16:
	case Bitmap::Type::COLOR256:
		width = pBmp->info.width;
		height = pBmp->info.height;
		pixelCount = width * height;
		break;
	default:
		width = 0;
		height = 0;
		pixelCount = 0;
		return;
	}

	pCells = reinterpret_cast<CELL *>(malloc(sizeof(CELL) * pixelCount));
	if (nullptr == pCells) {
		return;
	}

	struct DELTA {
		direction dir;
		bool enable;
	} delta[9] = {
		/* 方向            dx  dy  enable */
		/* 0:BOTTOM_L */ { -1, -1, false },
		/* 1:BOTTOM   */ {  0, -1, false },
		/* 2:BOTTOM_R */ {  1, -1, false },
		/* 3:LEFT     */ { -1,  0, false },
		/* 4:CENTER   */ {  0,  0, false },
		/* 5:RIGHT    */ {  1,  0, false },
		/* 6:TOP_L    */ { -1,  1, false },
		/* 7:TOP      */ {  0,  1, false },
		/* 8:TOP_R    */ {  1,  1, false }
	};

	/*** ピクセルの初期化 ***/
	point pos;
	for (pos.y = 0; pos.y < height; pos.y++) {
		for (pos.x = 0; pos.x < width; pos.x++) {
			/*** ピクセル情報をセット***/
			auto index = GetIndex(pos);
			pCells[index] = {
				false, // filled
				false, // traced
				pos,
				{
					nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr
				}
			};

			/* 左下, 左, 左上 */
			if (0 == pos.x) {
				delta[0].enable = delta[3].enable = delta[6].enable = false;
			} else {
				delta[0].enable = delta[3].enable = delta[6].enable = true;
			}
			/* 左下, 下, 右下 */
			if (0 == pos.y) {
				delta[0].enable = delta[1].enable = delta[2].enable = false;
			} else {
				delta[0].enable = delta[1].enable = delta[2].enable = true;
			}
			/* 右下, 右, 右上 */
			if (1 == (width - pos.x)) {
				delta[2].enable = delta[5].enable = delta[8].enable = false;
			} else {
				delta[2].enable = delta[5].enable = delta[8].enable = true;
			}
			/* 左上, 上, 右上 */
			if (1 == (height - pos.y)) {
				delta[6].enable = delta[7].enable = delta[8].enable = false;
			} else {
				delta[6].enable = delta[7].enable = delta[8].enable = true;
			}
			/*** 周囲ピクセルのインデックスを取得してセット ***/
			for (int32_t i = 0; i < 9; i++) {
				if (delta[i].enable) {
					pCells[index].pFillAround[i] = &pCells[GetIndex(pos, delta[i].dir)].filled;
				}
			}
		}
	}

	/*** パレットから最暗色と最明色を取得 ***/
	double mostDark = 1.0;
	double mostLight = 0.0;
	for (uint32_t i = 0; i < pBmp->info.paletteColors; i++) {
		auto color = pBmp->pPalette[i];
		auto lum = Bitmap::GetLum(color);
		if (lum < mostDark) {
			mostDark = lum;
			colorBlack = static_cast<uint8_t>(i);
		}
		if (mostLight < lum) {
			mostLight = lum;
			colorWhite = static_cast<uint8_t>(i);
		}
	}
}

Outline::~Outline() {
	free(pCells);
}

double
Outline::Read(const double lowerLum) {
	point pos;
	/*** ピクセルから塗っていない部分の輝度を取得 ***/
	double lumUnfill = 0.0;
	for (pos.y = 0; pos.y < height; pos.y++) {
		for (pos.x = 0; pos.x < width; pos.x++) {
			auto color = pBmp->GetIndexedColor(pos);
			auto lum = Bitmap::GetLum(color);
			if (lum <= lowerLum && lumUnfill < lum) {
				lumUnfill = lum;
			}
		}
	}
	/*** ピクセルから塗部の輝度を取得 ***/
	double lumFilled = 0.0;
	for (pos.y = 0; pos.y < height; pos.y++) {
		for (pos.x = 0; pos.x < width; pos.x++) {
			auto color = pBmp->GetIndexedColor(pos);
			auto lum = Bitmap::GetLum(color);
			if (lum < lumUnfill && lumFilled < lum) {
				lumFilled = lum;
			}
		}
	}
	/*** 塗部であるかの設定とトレース状態のクリア ***/
	for (pos.y = 0; pos.y < height; pos.y++) {
		for (pos.x = 0; pos.x < width; pos.x++) {
			auto color = pBmp->GetIndexedColor(pos);
			auto lum = Bitmap::GetLum(color);
			auto index = GetIndex(pos);
			pCells[index].filled = lum < lumUnfill;
			pCells[index].traced = false;
		}
	}
	return lumFilled;
}

void
Outline::Write() const {
	uint8_t defaultColor = 0;
	auto shiftUnit = static_cast<int32_t>(pBmp->info.bits);
	for (int32_t shift = 8 - shiftUnit; shift >= 0; shift -= shiftUnit) {
		defaultColor |= colorWhite << shift;
	}
	memset(pBmp->pPix, defaultColor, pBmp->info.imageSize);
	for (int32_t i = 0; i < pixelCount; i++) {
		auto pCell = pCells + i;
		if (!pCell->filled) {
			continue;
		}
		auto ppAround = pCell->pFillAround;
		auto fillB = Outline::HasFilledCell(ppAround, E_DIRECTION::BOTTOM);
		auto fillR = Outline::HasFilledCell(ppAround, E_DIRECTION::RIGHT);
		auto fillL = Outline::HasFilledCell(ppAround, E_DIRECTION::LEFT);
		auto fillT = Outline::HasFilledCell(ppAround, E_DIRECTION::TOP);
		int32_t hvFillCount = fillB;
		hvFillCount += fillR;
		hvFillCount += fillL;
		hvFillCount += fillT;
		if (0 == hvFillCount || 4 == hvFillCount) {
			continue;
		}
		if (1 == hvFillCount) {
			E_DIRECTION dirA;
			E_DIRECTION dirB;
			if (fillB) {
				dirA = E_DIRECTION::BOTTOM_L;
				dirB = E_DIRECTION::BOTTOM_R;
			}
			if (fillR) {
				dirA = E_DIRECTION::TOP_R;
				dirB = E_DIRECTION::BOTTOM_R;
			}
			if (fillL) {
				dirA = E_DIRECTION::TOP_L;
				dirB = E_DIRECTION::BOTTOM_L;
			}
			if (fillT) {
				dirA = E_DIRECTION::TOP_L;
				dirB = E_DIRECTION::TOP_R;
			}
			auto fillA = Outline::HasFilledCell(ppAround, dirA);
			auto fillB = Outline::HasFilledCell(ppAround, dirB);
			if (fillA ^ fillB) {
				continue;
			}
		} else {
			int32_t crossFillCount = Outline::HasFilledCell(ppAround, E_DIRECTION::BOTTOM_L);
			crossFillCount += Outline::HasFilledCell(ppAround, E_DIRECTION::BOTTOM_R);
			crossFillCount += Outline::HasFilledCell(ppAround, E_DIRECTION::TOP_L);
			crossFillCount += Outline::HasFilledCell(ppAround, E_DIRECTION::TOP_R);
			if (0 == crossFillCount && !((fillL && fillR) || (fillB && fillT))) {
				continue;
			}
		}
		auto pixIndex = pBmp->GetIndex(pCell->pos);
		if (INVALID_INDEX != pixIndex) {
			pBmp->SetIndexedColor(pCell->pos, colorBlack);
		}
	}
}

vector<vector<vec3>>
Outline::CreatePolyline() {
	vector<vector<vec3>> polylineList;
	int32_t startIndex = 0;
	while (true) { // ポリライン取得ループ
		vector<point> pointList; // 点リスト
		point currentPos;        // 現在の位置
		int32_t currentDir = 0;  // 現在の進行方向
		/*** ポリライン始点を検索 ***/
		auto polylineFound = false;
		for (int32_t i = startIndex; i < pixelCount; i++) {
			auto pCell = &pCells[i];
			if (pCell->filled && !pCell->traced) {
				// ポリライン始点を発見
				polylineFound = true;
				pCell->traced = true;
				// ポリラインの始点として点リストに追加
				currentPos = pCell->pos;
				pointList.push_back(pCell->pos);
				// 次のポリライン始点の検索開始インデックスをセット
				startIndex = i;
				// ポリライン始点の検索終了
				break;
			}
		}
		if (!polylineFound) { // 残っているポリラインなし
			break;
		}
		while (true) { // 点トレースループ
			/*** 現在の進行方向を優先して半径を広げながら周囲の点を検索 ***/
			/*** あればポリラインの点として点リストに追加 ***/
			auto pointFound = false;
			for (int32_t r = 0; r < TRACE_RADIUS; r++) {
				for (int32_t p = 0; p < PREFER_DIRS; p++) {
					auto traceDir = (currentDir + PREFER_DIR[p] + TRACE_DIRS) % TRACE_DIRS;
					auto index = GetIndex(currentPos, TRACE_DIR[r][traceDir]);
					if (INVALID_INDEX == index) {
						continue;
					}
					auto pCell = &pCells[index];
					if (pCell->filled && !pCell->traced) {
						// 点を発見
						pointFound = true;
						pCell->traced = true;
						// トレース方向を現在の進行方向とする
						currentDir = traceDir;
						// ポリラインの点として点リストに追加
						currentPos = pCell->pos;
						pointList.push_back(pCell->pos);
						// 次の点を検索
						break;
					}
				}
				if (pointFound) { // 次の点を検索
					break;
				}
			}
			if (!pointFound) { // ポリラインの終端
				// 直線上にある点を点リストから除外する
				auto polyline = EliminatePointsOnStraightLine(pointList);
				if (3 <= polyline.size()) {
					// 点リストをポリラインリストに追加
					polylineList.push_back(polyline);
				}
				break;
			}
		}
	}
	return polylineList;
}

vector<vec3>
Outline::EliminatePointsOnStraightLine(vector<point> polyline) const {
	if (polyline.size() < 3) {
		return vector<vec3>();
	}

	vec3 a, b, c, o;
	vec3 oa, og;

	// 直線上に存在する点の除外(3点チェック)
	vector<vec3> line3p;
	auto polylineCount = polyline.size();
	b = vec3(polyline[polylineCount - 2].x, polyline[polylineCount - 2].y, 0);
	a = vec3(polyline[polylineCount - 1].x, polyline[polylineCount - 1].y, 0);
	for (int32_t i = 0; i < polylineCount; i++) {
		o = b;
		b = a;
		a = vec3(polyline[i].x, polyline[i].y, 0);
		oa = a - o;
		og = b - o;
		auto oaLen = sqrt(oa & oa);
		auto ogLen = sqrt(og & og);
		oa /= oaLen;
		og /= ogLen;
		if ((oa & og) < ELIMINATE_THRESHOLD) {
			line3p.push_back(b);
		}
	}
	line3p.push_back(line3p[0]);

	// 直線上に存在する点の除外(4点チェック)
	vector<vec3> line4p;
	auto line3pCount = line3p.size();
	c = line3p[line3pCount - 3];
	b = line3p[line3pCount - 2];
	a = line3p[line3pCount - 1];
	for (int32_t i = 0; i < line3pCount; i++) {
		o = c;
		c = b;
		b = a;
		a = line3p[i];
		oa = a - o;
		og = (b + c) / 2.0 - o;
		auto oaLen = sqrt(oa & oa);
		auto ogLen = sqrt(og & og);
		oa /= oaLen;
		og /= ogLen;
		if ((oa & og) < ELIMINATE_THRESHOLD) {
			line4p.push_back(b);
		}
	}
	line4p.push_back(line4p[0]);

	// リストに入れて返す(近接する点は重心を1つの点とする)
	vector<vec3> ret;
	int32_t sumCount = 1;
	vec3 sum = line4p[line4p.size() - 1];
	vec3 avg = sum;
	for (int32_t i = 0; i < line4p.size(); i++) {
		auto cur = line4p[i];
		auto diff = cur - avg;
		if ((diff & diff) <= 2.0) {
			sum += cur;
			sumCount++;
			avg = sum / sumCount;
		} else {
			ret.push_back(avg);
			sumCount = 1;
			sum = cur;
			avg = cur;
		}
	}
	return ret;
}
