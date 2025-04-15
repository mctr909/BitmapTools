#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "Declease.h"

#pragma pack(push, 4)
struct Declease::HISTOGRAM {
	uint8_t srcH;
	uint8_t srcS;
	uint8_t srcL;
	uint8_t dstH;
	uint8_t dstS;
	uint8_t dstL;
	uint8_t reserved;
	uint8_t palette;
	uint32_t count;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct HSL_TOP {
	uint8_t h;
	uint8_t s;
	uint8_t l;
	uint8_t reserved;
	uint32_t count;
	uint32_t index;
};
#pragma pack(pop)

double
Declease::CalcHistogram(Bitmap const &bmp, HISTOGRAM *pHistogram) {
	const uint8_t rangeH = DEFINE_HUE_RANGE;
	const uint8_t rangeS = DEFINE_SATURATION_RANGE;
	const uint8_t rangeL = DEFINE_LIGHTNESS_RANGE;
	const int32_t weightH = DEFINE_AVG_WEIGHT_HUE;
	const int32_t weightS = DEFINE_AVG_WEIGHT_SATURATION;
	const int32_t weightL = DEFINE_AVG_WEIGHT_LIGHTNESS;
	Bitmap::pix24 avgHSL;
	point pos;
	uint32_t originCount = 0;
	uint32_t avgCount = 0;
	auto pPix = reinterpret_cast<Bitmap::pix24 *>(bmp.pPix);
	for (pos.y = 0; pos.y < bmp.info.height; pos.y++) {
		for (pos.x = 0; pos.x < bmp.info.width; pos.x++) {
			auto index = bmp.GetIndex(pos);
			if (INVALID_INDEX != index) {
				auto hsl = pPix[index];
				auto histWeight = AvgHSL(bmp, pos, &avgHSL);
				auto sh = (hsl.r - avgHSL.r) * weightH / rangeH;
				auto ss = (hsl.g - avgHSL.g) * weightS / rangeS;
				auto sl = (hsl.b - avgHSL.b) * weightL / rangeL;
				auto dist = sqrt(sh * sh + ss * ss + sl * sl);
				if (dist <= 1) {
					hsl.r = avgHSL.r;
					hsl.g = avgHSL.g;
					hsl.b = avgHSL.b;
					histWeight = 1;
					avgCount++;
				} else {
					originCount++;
				}
				auto histIndex
					= hsl.r * rangeS * rangeL
					+ hsl.g * rangeL
					+ hsl.b;
				auto pHist = pHistogram + histIndex;
				pHist->srcH = hsl.r;
				pHist->srcS = hsl.g;
				pHist->srcL = hsl.b;
				pHist->count += histWeight;
			}
		}
	}
	return static_cast<double>(originCount) / avgCount;
}

void
Declease::GetTop256Colors(HISTOGRAM *pHistogram, Bitmap::pix32 *pPalette) {
	const uint8_t rangeH = DEFINE_HUE_RANGE;
	const uint8_t rangeS = DEFINE_SATURATION_RANGE;
	const uint8_t rangeL = DEFINE_LIGHTNESS_RANGE;
	const int32_t histCount = rangeH * rangeS * rangeL;
	/*** ヒストグラム上位256個のHSL値を取得 ***/
	HSL_TOP top256[256];
	for (int32_t t = 0; t < 256; t++) {
		auto pTop = top256 + t;
		pTop->count = 0;
		for (int32_t i = 0; i < histCount; i++) {
			auto hist = pHistogram[i];
			if (pTop->count < hist.count && UINT32_MAX != hist.count) {
				pTop->count = hist.count;
				pTop->index = i;
			}
		}
		if (0 == pTop->count) {
			pTop->h = 0;
			pTop->s = 0;
			pTop->l = 0;
			continue;
		}
		auto pTopHist = pHistogram + pTop->index;
		pTopHist->count = UINT32_MAX;
		pTopHist->dstH = pTopHist->srcH;
		pTopHist->dstS = pTopHist->srcS;
		pTopHist->dstL = pTopHist->srcL;
		pTopHist->palette = static_cast<uint8_t>(t);
		pTop->h = pTopHist->srcH;
		pTop->s = pTopHist->srcS;
		pTop->l = pTopHist->srcL;
	}
	/*** 上位256個の中で近いHSL値を変換先に設定 ***/
	for (int32_t i = 0; i < histCount; i++) {
		auto pHist = &pHistogram[i];
		if (UINT32_MAX == pHist->count) {
			continue; /*** 上位256個 ***/
		}
		pHist->count = UINT32_MAX;
		for (int32_t t = 0; t < 256; t++) {
			auto pTop = &top256[t];
			auto diffH = (pHist->srcH - pTop->h) * 255 / rangeH;
			auto diffS = (pHist->srcS - pTop->s) * 255 / rangeS;
			auto diffL = (pHist->srcL - pTop->l) * 255 / rangeL;
			auto dist2
				= static_cast<uint32_t>(diffH * diffH / 255)
				+ static_cast<uint32_t>(diffS * diffS / 255)
				+ static_cast<uint32_t>(diffL * diffL / 255);
			if (dist2 < pHist->count) {
				pHist->count = dist2;
				pHist->dstH = pTop->h;
				pHist->dstS = pTop->s;
				pHist->dstL = pTop->l;
				pHist->palette = static_cast<uint8_t>(t);
			}
		}
	}
	/*** 上位256個のHSL値をもとにパレットに色を設定 ***/
	for (int32_t t = 0; t < 256; t++) {
		auto top = top256[t];
		DecHSL(reinterpret_cast<Bitmap::pix24 *>(&pPalette[t]), top.h, top.s, top.l);
	}
}

void
Declease::Exec(Bitmap const &inBmp24, Bitmap *pOutBmp8) {
	const int32_t rangeH = DEFINE_HUE_RANGE;
	const int32_t rangeS = DEFINE_SATURATION_RANGE;
	const int32_t rangeL = DEFINE_LIGHTNESS_RANGE;
	const int32_t histCount = rangeH * rangeS * rangeL;
	auto pHist = reinterpret_cast<HISTOGRAM *>(calloc(histCount, sizeof(HISTOGRAM)));
	if (nullptr == pHist) {
		return;
	}
	auto pInPix = reinterpret_cast<Bitmap::pix24 *>(inBmp24.pPix);
	point pos;
	for (pos.y = 0; pos.y < inBmp24.info.height; pos.y++) {
		for (pos.x = 0; pos.x < inBmp24.info.width; pos.x++) {
			auto index = inBmp24.GetIndex(pos);
			if (INVALID_INDEX != index) {
				EncHSL(pInPix + index);
			}
		}
	}
	CalcHistogram(inBmp24, pHist);
	GetTop256Colors(pHist, pOutBmp8->pPalette);
	for (pos.y = 0; pos.y < inBmp24.info.height; pos.y++) {
		for (pos.x = 0; pos.x < inBmp24.info.width; pos.x++) {
			auto index = inBmp24.GetIndex(pos);
			if (INVALID_INDEX != index) {
				auto hsl = pInPix[index];
				auto histIndex
					= hsl.r * rangeS * rangeL
					+ hsl.g * rangeL
					+ hsl.b;
				auto hist = pHist[histIndex];
				pOutBmp8->pPix[index] = hist.palette;
			}
		}
	}
	free(pHist);
	return;
}
