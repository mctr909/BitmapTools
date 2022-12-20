#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "Declease.h"

#pragma pack(push, 4)
struct type_declease_histogram {
    byte src_h;
    byte src_s;
    byte src_l;
    byte dst_h;
    byte dst_s;
    byte dst_l;
    byte reserved;
    byte palette;
    uint32 count;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct type_declease_hsltop {
    byte h;
    byte s;
    byte l;
    byte reserved;
    uint32 count;
    uint32 index;
};
#pragma pack(pop)

/*
ヒストグラムを計算
インデックス指定のHSLが周辺HSLの平均値と近ければ平均値をヒストグラムに反映
離れていればインデックス指定のHSLを反映
*/
double
__declease_histogram(type_declease_histogram* pHistogram, Bitmap* pBmp) {
    const byte h_range = DEFINE_HUE_RANGE;
    const byte s_range = DEFINE_SATURATION_RANGE;
    const byte l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 h_weight = DEFINE_AVG_WEIGHT_HUE;
    const int32 s_weight = DEFINE_AVG_WEIGHT_SATURATION;
    const int32 l_weight = DEFINE_AVG_WEIGHT_LIGHTNESS;
    Bitmap::pix24 avg_hsl;
    point pos;
    uint32 origin_count = 0;
    uint32 avg_count = 0;
    auto pPix = reinterpret_cast<Bitmap::pix24*>(pBmp->pPix);
    for (pos.y = 0; pos.y < pBmp->info_h.height; pos.y++) {
        for (pos.x = 0; pos.x < pBmp->info_h.width; pos.x++) {
            auto index = bitmap_get_index(*pBmp, pos);
            if (UINT32_MAX != index) {
                auto hsl = pPix[index];
                auto hist_weight = declease_avghsl(pBmp, &avg_hsl, pos);
                auto sh = (hsl.r - avg_hsl.r) * h_weight / h_range;
                auto ss = (hsl.g - avg_hsl.g) * s_weight / s_range;
                auto sl = (hsl.b - avg_hsl.b) * l_weight / l_range;
                auto dist = sqrt(sh * sh + ss * ss + sl * sl);
                if (dist <= 1) {
                    hsl.r = avg_hsl.r;
                    hsl.g = avg_hsl.g;
                    hsl.b = avg_hsl.b;
                    hist_weight = 1;
                    avg_count++;
                } else {
                    origin_count++;
                }
                auto hist_index
                    = hsl.r * s_range * l_range
                    + hsl.g * l_range
                    + hsl.b;
                auto hist = &pHistogram[hist_index];
                hist->src_h = hsl.r;
                hist->src_s = hsl.g;
                hist->src_l = hsl.b;
                hist->count += hist_weight;
            }
        }
    }
    return static_cast<double>(origin_count) / avg_count;
}

/*
ヒストグラム上位256個のHSL値を取得
上位256個の中で近いHSL値を変換先に設定
上位256個のHSL値をもとにパレットに色を設定
*/
void
__declease_color_top256(type_declease_histogram* pHistogram, Bitmap::pix32* pPalette) {
    const byte h_range = DEFINE_HUE_RANGE;
    const byte s_range = DEFINE_SATURATION_RANGE;
    const byte l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 hist_count = h_range * s_range * l_range;
    /*** ヒストグラム上位256個のHSL値を取得 ***/
    type_declease_hsltop hsl_top256[256];
    for (int32 t = 0; t < 256; t++) {
        auto pTop = &hsl_top256[t];
        pTop->count = 0;
        for (int32 i = 0; i < hist_count; i++) {
            auto pHist = &pHistogram[i];
            if (pTop->count < pHist->count && UINT32_MAX != pHist->count) {
                pTop->count = pHist->count;
                pTop->index = i;
            }
        }
        if (0 == pTop->count) {
            pTop->h = 0;
            pTop->s = 0;
            pTop->l = 0;
            continue;
        }
        auto pTopHist = &pHistogram[pTop->index];
        pTopHist->count = UINT32_MAX;
        pTopHist->dst_h = pTopHist->src_h;
        pTopHist->dst_s = pTopHist->src_s;
        pTopHist->dst_l = pTopHist->src_l;
        pTopHist->palette = static_cast<byte>(t);
        pTop->h = pTopHist->src_h;
        pTop->s = pTopHist->src_s;
        pTop->l = pTopHist->src_l;
    }
    /*** 上位256個の中で近いHSL値を変換先に設定 ***/
    for (int32 i = 0; i < hist_count; i++) {
        auto pHist = &pHistogram[i];
        if (UINT32_MAX == pHist->count) {
            continue; /*** 上位256個 ***/
        }
        pHist->count = UINT32_MAX;
        for (int32 t = 0; t < 256; t++) {
            auto pTop = &hsl_top256[t];
            auto diff_h = (pHist->src_h - pTop->h) * 255 / h_range;
            auto diff_s = (pHist->src_s - pTop->s) * 255 / s_range;
            auto diff_l = (pHist->src_l - pTop->l) * 255 / l_range;
            auto dist2
                = static_cast<uint32>(diff_h * diff_h / 255)
                + static_cast<uint32>(diff_s * diff_s / 255)
                + static_cast<uint32>(diff_l * diff_l / 255);
            if (dist2 < pHist->count) {
                pHist->count = dist2;
                pHist->dst_h = pTop->h;
                pHist->dst_s = pTop->s;
                pHist->dst_l = pTop->l;
                pHist->palette = static_cast<byte>(t);
            }
        }
    }
    /*** 上位256個のHSL値をもとにパレットに色を設定 ***/
    for (int32 t = 0; t < 256; t++) {
        auto top = hsl_top256[t];
        declease_hsl2rgb(reinterpret_cast<Bitmap::pix24*>(&pPalette[t]), top.h, top.s, top.l);
    }
}

void
declease_exec(Bitmap* pInBmp24, Bitmap* pOutBmp8) {
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 hist_count = h_range * s_range * l_range;
    auto pHist = reinterpret_cast<type_declease_histogram*>(calloc(hist_count, sizeof(type_declease_histogram)));
    if (NULL == pHist) {
        pInBmp24->error = -1;
        return;
    }
    auto pInPix = reinterpret_cast<Bitmap::pix24*>(pInBmp24->pPix);
    point pos;
    for (pos.y = 0; pos.y < pInBmp24->info_h.height; pos.y++) {
        for (pos.x = 0; pos.x < pInBmp24->info_h.width; pos.x++) {
            auto index = bitmap_get_index(*pInBmp24, pos);
            if (UINT32_MAX != index) {
                declease_rgb2hsl(&pInPix[index]);
            }
        }
    }
    __declease_histogram(pHist, pInBmp24);
    __declease_color_top256(pHist, pOutBmp8->pPalette);
    for (pos.y = 0; pos.y < pInBmp24->info_h.height; pos.y++) {
        for (pos.x = 0; pos.x < pInBmp24->info_h.width; pos.x++) {
            auto index = bitmap_get_index(*pInBmp24, pos);
            if (UINT32_MAX != index) {
                auto pHsl = &pInPix[index];
                auto hist_index
                    = pHsl->r * s_range * l_range
                    + pHsl->g * l_range
                    + pHsl->b;
                auto hist = pHist[hist_index];
                pOutBmp8->pPix[index] = hist.palette;
            }
        }
    }
    free(pHist);
    pInBmp24->error = 0;
    return;
}
