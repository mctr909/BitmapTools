#ifndef __DECLEASE_H__
#define __DECLEASE_H__

#include "Bitmap.h"

#define DEFINE_HUE_RANGE        12
#define DEFINE_SATURATION_RANGE  5
#define DEFINE_LIGHTNESS_RANGE  24

#define DEFINE_AVG_CALC_DIST          7
#define DEFINE_AVG_WEIGHT_HUE        32
#define DEFINE_AVG_WEIGHT_SATURATION  2
#define DEFINE_AVG_WEIGHT_LIGHTNESS   8

class Declease {
private:
	struct HISTOGRAM;
public:
	/* 減色を行う
	24bitビットマップを8bitビットマップに変換
	*/
	static void Exec(Bitmap const &inBmp24, Bitmap *pOutBmp8);
private:
	/* ヒストグラムを計算
	インデックス指定のHSLが周辺HSLの平均値と近ければ平均値をヒストグラムに反映
	離れていればインデックス指定のHSLを反映
	*/
	static double CalcHistogram(Bitmap const &bmp, HISTOGRAM *pHistogram);
	/* ヒストグラム上位256個のHSL値を取得
	上位256個の中で近いHSL値を変換先に設定
	上位256個のHSL値をもとにパレットに色を設定
	*/
	static void GetTop256Colors(HISTOGRAM *pHistogram, Bitmap::pix32 *pPalette);
public:
	/* HSLエンコード */
	static void EncHSL(Bitmap::pix24 *pPix);
	/* HSLデコード */
	static void DecHSL(Bitmap::pix24 *pPix, uint8_t h, uint8_t s, uint8_t l);
	/* 周辺ピクセルの平均HSLを求める */
	static int32_t AvgHSL(Bitmap const &bmp, point const &pos, Bitmap::pix24 *pHsl);
};

inline void
Declease::EncHSL(Bitmap::pix24 *pPix) {
	int32_t h, s, l;
	const uint8_t rangeH = DEFINE_HUE_RANGE;
	const uint8_t rangeS = DEFINE_SATURATION_RANGE;
	const uint8_t rangeL = DEFINE_LIGHTNESS_RANGE;
	auto bmax = fmax(pPix->r, fmax(pPix->g, pPix->b));
	auto bmin = fmin(pPix->r, fmin(pPix->g, pPix->b));
	auto range = bmax - bmin;
	auto offsetH = rangeH * range / 3;
	if (0 == range) {
		h = 0;
	} else if (pPix->r == bmax) {
		h = static_cast<int32_t>(((pPix->g - pPix->b) * rangeH + offsetH) / (range * 6.0));
	} else if (pPix->g == bmax) {
		h = static_cast<int32_t>(((pPix->b - pPix->r) * rangeH + offsetH) / (range * 6.0) + rangeH / 3.0);
	} else if (pPix->b == bmax) {
		h = static_cast<int32_t>(((pPix->r - pPix->g) * rangeH + offsetH) / (range * 6.0) + 2 * rangeH / 3.0);
	} else {
		h = 0;
	}
	auto cnt = (bmax + bmin) / 2;
	if (0 == range || 0 == cnt) {
		s = 0;
	} else if (cnt <= 127) {
		s = static_cast<int32_t>(((cnt - bmin) * rangeS + rangeS) / cnt);
	} else {
		s = static_cast<int32_t>(((bmax - cnt) * rangeS + rangeS) / (255 - cnt));
	}
	l = static_cast<int32_t>(((bmax + bmin) * rangeL + rangeL) / 510);
	if (h < 0) {
		h += rangeH;
	}
	if (rangeH <= h) {
		h -= rangeH;
	}
	if (s < 0) {
		s = 0;
	}
	if (rangeS <= s) {
		s = rangeS - 1;
	}
	if (rangeL <= l) {
		l = rangeL - 1;
	}
	pPix->r = static_cast<uint8_t>(h);
	pPix->g = static_cast<uint8_t>(s);
	pPix->b = static_cast<uint8_t>(l);
}

inline void
Declease::DecHSL(Bitmap::pix24 *pPix, uint8_t h, uint8_t s, uint8_t l) {
	const uint8_t rangeH = DEFINE_HUE_RANGE;
	const uint8_t rangeS = DEFINE_SATURATION_RANGE;
	const uint8_t rangeL = DEFINE_LIGHTNESS_RANGE;
	double dmax, dmin;
	if (l < rangeL / 2) {
		dmax = (l + l * static_cast<double>(s) / rangeS) * 255 / rangeL;
		dmin = (l - l * static_cast<double>(s) / rangeS) * 255 / rangeL;
	} else {
		dmax = (l + (rangeL - l) * static_cast<double>(s) / rangeS) * 255 / rangeL;
		dmin = (l - (rangeL - l) * static_cast<double>(s) / rangeS) * 255 / rangeL;
	}
	auto range = dmax - dmin;
	auto bmax = static_cast<uint8_t>(dmax);
	auto bmin = static_cast<uint8_t>(dmin);
	if (h <= rangeH / 6) {
		pPix->r = bmax;
		pPix->g = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
		pPix->b = bmin;
	} else if (h <= 2 * rangeH / 6) {
		h = rangeH / 3 - h;
		pPix->r = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
		pPix->g = bmax;
		pPix->b = bmin;
	} else if (h <= 3 * rangeH / 6) {
		h -= rangeH / 3;
		pPix->r = bmin;
		pPix->g = bmax;
		pPix->b = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
	} else if (h <= 4 * rangeH / 6) {
		h = 2 * rangeH / 3 - h;
		pPix->r = bmin;
		pPix->g = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
		pPix->b = bmax;
	} else if (h <= 5 * rangeH / 6) {
		h -= 2 * rangeH / 3;
		pPix->r = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
		pPix->g = bmin;
		pPix->b = bmax;
	} else {
		h = rangeH - h;
		pPix->r = bmax;
		pPix->g = bmin;
		pPix->b = static_cast<uint8_t>(h * range * 6 / rangeH + dmin);
	}
}

inline int32_t
Declease::AvgHSL(Bitmap const &bmp, point const &pos, Bitmap::pix24 *pHsl) {
	const uint8_t rangeH = DEFINE_HUE_RANGE;
	const uint8_t rangeS = DEFINE_SATURATION_RANGE;
	const uint8_t rangeL = DEFINE_LIGHTNESS_RANGE;
	const int32_t calcDist = DEFINE_AVG_CALC_DIST;
	double avgH = 0.0, avgS = 0.0, avgL = 0.0;
	int32_t calcCount = 0;
	auto pPix = reinterpret_cast<Bitmap::pix24 *>(bmp.pPix);
	for (int32_t dy = -calcDist; dy <= calcDist; dy++) {
		for (int32_t dx = -calcDist; dx <= calcDist; dx++) {
			auto dr = sqrt(dx * dx + dy * dy);
			if (calcDist < dr) {
				continue;
			}
			auto index = bmp.GetIndex(pos, dx, dy);
			if (INVALID_INDEX != index) {
				auto hsl = pPix[index];
				avgH += hsl.r;
				avgS += hsl.g;
				avgL += hsl.b;
				calcCount++;
			}
		}
	}
	avgH /= calcCount;
	avgS /= calcCount;
	avgL /= calcCount;
	if (rangeH <= avgH) {
		avgH -= rangeH;
	}
	if (rangeS <= avgS) {
		avgS = rangeS - 1;
	}
	if (rangeL <= avgL) {
		avgL = rangeL - 1;
	}
	pHsl->r = static_cast<uint8_t>(avgH);
	pHsl->g = static_cast<uint8_t>(avgS);
	pHsl->b = static_cast<uint8_t>(avgL);
	return calcCount;
}
#endif //__DECLEASE_H__
