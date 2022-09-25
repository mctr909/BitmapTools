#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"
#include "Declease.h"

Bitmap*
fn_exec_declease(Bitmap& const pbmp) {
    auto overwrite_bmp = &pbmp;
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const auto size_max = static_cast<uint32>(pbmp.info_h.width * pbmp.info_h.height);
    const auto hist_count = DEFINE_HUE_RANGE * DEFINE_SATURATION_RANGE * DEFINE_LIGHTNESS_RANGE;
    type_histogram histogram[hist_count];
    memset(&histogram[0], 0, sizeof(histogram));

    auto pPix = (Bitmap::pix24*)pbmp.pPix;
    for (uint32 i = 0; i < size_max; i++) {
        fn_rgb2hsl(&histogram[0], &pPix[i]);
    }

    fn_calc_histogram(&histogram[0]);

    for (uint32 i = 0; i < size_max; i++) {
        auto pix = pPix[i];
        auto hist_index
            = pix.r * s_range * l_range
            + pix.g * l_range
            + pix.b;
        auto hist = histogram[hist_index];
        fn_hsl2rgb(&pPix[i], hist.dst_h, hist.dst_s, hist.dst_l);
    }
    
    overwrite_bmp->error = 0;
    return overwrite_bmp;
}

void
fn_calc_histogram(type_histogram* pHistogram) {
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const auto hist_count = h_range * s_range * l_range;

    /*** ヒストグラム上位256個のHSL値を取得 ***/
    type_hsl_tops hsl_top256[256];
    for (int32 t = 0; t < 256; t++) {
        auto pTops = &hsl_top256[t];
        pTops->count = 0;
        for (int32 i = 0; i < hist_count; i++) {
            auto pHist = &pHistogram[i];
            if (pTops->count < pHist->count && UINT32_MAX != pHist->count) {
                pTops->count = pHist->count;
                pTops->index = i;
            }
        }
        if (0 == pTops->count) {
            pTops->h = 0;
            pTops->s = 0;
            pTops->l = 0;
            continue;
        }
        auto pTopHist = &pHistogram[pTops->index];
        pTopHist->count = UINT32_MAX;
        pTopHist->dst_h = pTopHist->src_h;
        pTopHist->dst_s = pTopHist->src_s;
        pTopHist->dst_l = pTopHist->src_l;
        pTops->h = pTopHist->src_h;
        pTops->s = pTopHist->src_s;
        pTops->l = pTopHist->src_l;
    }

    /*** ヒストグラム上位256個の中で近いHSL値を変換先に設定 ***/
    for (int32 i = 0; i < hist_count; i++) {
        auto pHist = &pHistogram[i];
        if (UINT32_MAX == pHist->count) {
            continue; /*** 上位256個 ***/
        }
        pHist->count = UINT32_MAX;
        for (int32 t = 0; t < 256; t++) {
            auto pTops = &hsl_top256[t];
            auto sh = (pHist->src_h - pTops->h) * 255 / h_range;
            auto ss = (pHist->src_s - pTops->s) * 255 / s_range;
            auto sl = (pHist->src_l - pTops->l) * 255 / l_range;
            auto dist2
                = static_cast<uint32>(sh * sh / 255) 
                + static_cast<uint32>(ss * ss / 255)
                + static_cast<uint32>(sl * sl / 255);
            if (dist2 < pHist->count) {
                pHist->count = dist2;
                pHist->dst_h = pTops->h;
                pHist->dst_s = pTops->s;
                pHist->dst_l = pTops->l;
            }
        }
    }
}

inline void
fn_rgb2hsl(type_histogram* pHistogram, Bitmap::pix24* pPix) {
    /*** HSLエンコード ***/
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    auto bmax = static_cast<byte>(fmax(pPix->r, fmax(pPix->g, pPix->b)));
    auto bmin = static_cast<byte>(fmin(pPix->r, fmin(pPix->g, pPix->b)));
    auto range = bmax - bmin;
    auto h_offset = h_range * range / 3;
    int32 h;
    if (0 == range) {
        h = 0;
    } else if (pPix->r == bmax) {
        h = ((pPix->g - pPix->b) * h_range + h_offset) / (range * 6);
    } else if (pPix->g == bmax) {
        h = ((pPix->b - pPix->r) * h_range + h_offset) / (range * 6) + h_range / 3;
    } else if (pPix->b == bmax) {
        h = ((pPix->r - pPix->g) * h_range + h_offset) / (range * 6) + 2 * h_range / 3;
    }
    int32 s;
    auto cnt = (bmax + bmin) / 2;
    if (0 == range || 0 == cnt) {
        s = 0;
    } else if (cnt <= 127) {
        s = (cnt - bmin) * s_range / cnt;
    } else {
        s = (bmax - cnt) * s_range / (255 - cnt);
    }
    auto l = cnt * l_range / 255;

    if (h < 0) {
        h += h_range;
    }
    if (h_range <= h) {
        h -= h_range;
    }
    if (s_range <= s) {
        s = s_range - 1;
    }
    if (l_range <= l) {
        l = l_range - 1;
    }

    pPix->r = h;
    pPix->g = s;
    pPix->b = l;

    /*** ヒストグラムカウント ***/
    auto hist_index
        = s_range * l_range * h
        + l_range * s
        + l;
    auto hist = &pHistogram[hist_index];
    hist->src_h = h;
    hist->src_s = s;
    hist->src_l = l;
    hist->count++;
}

inline void
fn_hsl2rgb(Bitmap::pix24* pPix, byte h, byte s, byte l) {
    /*** HSLデコード ***/
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    double dmax, dmin;
    if (l < l_range / 2) {
        dmax = (l + l * s / static_cast<double>(s_range)) * 255 / l_range;
        dmin = (l - l * s / static_cast<double>(s_range)) * 255 / l_range;
    } else {
        dmax = (l + (l_range - l) * s / static_cast<double>(s_range)) * 255 / l_range;
        dmin = (l - (l_range - l) * s / static_cast<double>(s_range)) * 255 / l_range;
    }
    auto range = dmax - dmin;
    auto bmax = static_cast<byte>(dmax);
    auto bmin = static_cast<byte>(dmin);
    if (h <= h_range / 6) {
        pPix->r = bmax;
        pPix->g = static_cast<byte>(h * range * 6 / h_range + dmin);
        pPix->b = bmin;
    } else if (h <= 2 * h_range / 6) {
        h = h_range / 3 - h;
        pPix->r = static_cast<byte>(h * range * 6 / h_range + dmin);
        pPix->g = bmax;
        pPix->b = bmin;
    } else if (h <= 3 * h_range / 6) {
        h -= h_range / 3;
        pPix->r = bmin;
        pPix->g = bmax;
        pPix->b = static_cast<byte>(h * range * 6 / h_range + dmin);
    } else if (h <= 4 * h_range / 6) {
        h = 2 * h_range / 3 - h;
        pPix->r = bmin;
        pPix->g = static_cast<byte>(h * range * 6 / h_range + dmin);
        pPix->b = bmax;
    } else if (h <= 5 * h_range / 6) {
        h -= 2 * h_range / 3;
        pPix->r = static_cast<byte>(h * range * 6 / h_range + dmin);
        pPix->g = bmin;
        pPix->b = bmax;
    } else {
        h = h_range - h;
        pPix->r = bmax;
        pPix->g = bmin;
        pPix->b = static_cast<byte>(h * range * 6 / h_range + dmin);
    }
}
