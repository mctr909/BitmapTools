#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "Bitmap.h"
#include "Declease.h"

#define DEFINE_HUE_RANGE        (12)
#define DEFINE_LIGHTNESS_RANGE  (32)
#define DEFINE_SATURATION_RANGE (3)

#define DEFINE_HUE_WEIGHT       (4)
#define DEFINE_LIGHTNESS_WEIGHT (2)

#pragma pack(push, 4)
struct type_histogram {
    byte src_h;
    byte src_s;
    byte src_l;
    byte dst_h;
    byte dst_s;
    byte dst_l;
    uint16 reserved;
    uint32 count;
};
#pragma pack(pop)

#pragma pack(push, 4)
struct type_hsl_top {
    byte h;
    byte s;
    byte l;
    byte reserved;
    uint32 count;
    uint32 index;
};
#pragma pack(pop)

inline int32
fn_calc_hsl_avg(Bitmap *pBmp, Bitmap::position pos, Bitmap::pix24* pHsl) {
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 calc_dist = 5;
    const uint32 calc_cells = (calc_dist * 2 + 1) * (calc_dist * 2 + 1);
    auto pPix = reinterpret_cast<Bitmap::pix24*>(pBmp->pPix);
    double avg_h = 0.0;
    double avg_s = 0.0;
    double avg_l = 0.0;
    int32 calc_count = 0;
    /*** 周辺HSLの平均を求める ***/
    for (int32 dy = -calc_dist; dy <= calc_dist; dy++) {
        for (int32 dx = -calc_dist; dx <= calc_dist; dx++) {
            auto r = sqrt(dx * dx + dy * dy);
            if (calc_dist < r) {
                continue;
            }
            auto index = bitmap_pix_ofs_index(*pBmp, pos, dx, dy);
            if (UINT32_MAX != index) {
                auto hsl = pPix[index];
                avg_h += hsl.r;
                avg_s += hsl.g;
                avg_l += hsl.b;
                calc_count++;
            }
        }
    }
    avg_h /= calc_count;
    avg_s /= calc_count;
    avg_l /= calc_count;
    if (h_range <= avg_h) {
        avg_h -= h_range;
    }
    if (s_range <= avg_s) {
        avg_s = s_range - 1;
    }
    if (l_range <= avg_l) {
        avg_l = l_range - 1;
    }
    pHsl->r = static_cast<byte>(avg_h);
    pHsl->g = static_cast<byte>(avg_s);
    pHsl->b = static_cast<byte>(avg_l);
    return calc_count;
}

void
fn_calc_histogram(type_histogram* pHistogram, Bitmap* pBmp) {
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 h_weight = DEFINE_HUE_WEIGHT;
    const int32 l_weight = DEFINE_LIGHTNESS_WEIGHT;
    const auto size_max = static_cast<uint32>(pBmp->info_h.width * pBmp->info_h.height);
    auto pPix = reinterpret_cast<Bitmap::pix24*>(pBmp->pPix);
    Bitmap::position pos;
    Bitmap::pix24 rms_hsl;
    /*** 周辺HSLの平均値と近ければ平均値をヒストグラムに反映 ***/
    /*** 離れていればインデックスで指定されたHSLを反映 ***/
    for (uint32 i = 0; i < size_max; i++) {
        auto hsl = pPix[i];
        bitmap_pix_pos(*pBmp, &pos, i);
        auto weight = fn_calc_hsl_avg(pBmp, pos, &rms_hsl);        
        auto sh = (hsl.r - rms_hsl.r) * h_weight;
        auto ss = (hsl.g - rms_hsl.g);
        auto sl = (hsl.b - rms_hsl.b) * l_weight;
        auto dist = sqrt(sh * sh + ss * ss + sl * sl);
        if (dist < DEFINE_HUE_WEIGHT) {
            hsl.r = rms_hsl.r;
            hsl.g = rms_hsl.g;
            hsl.b = rms_hsl.b;
            weight = 1;
        }
        auto hist_index
            = hsl.r * s_range * l_range
            + hsl.g * l_range
            + hsl.b;
        auto hist = &pHistogram[hist_index];
        hist->src_h = hsl.r;
        hist->src_s = hsl.g;
        hist->src_l = hsl.b;
        hist->count += weight;
    }
}

inline void
fn_rgb2hsl(Bitmap::pix24* pPix) {
    /*** HSLエンコード ***/
    int32 h, s, l;
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    auto bmax = max(pPix->r, max(pPix->g, pPix->b));
    auto bmin = min(pPix->r, min(pPix->g, pPix->b));
    auto range = bmax - bmin;
    auto h_offset = h_range * range / 3;
    if (0 == range) {
        h = 0;
    } else if (pPix->r == bmax) {
        h = ((pPix->g - pPix->b) * h_range + h_offset) / (range * 6);
    } else if (pPix->g == bmax) {
        h = ((pPix->b - pPix->r) * h_range + h_offset) / (range * 6) + h_range / 3;
    } else if (pPix->b == bmax) {
        h = ((pPix->r - pPix->g) * h_range + h_offset) / (range * 6) + 2 * h_range / 3;
    } else {
        h = 0;
    }
    auto cnt = (bmax + bmin) / 2;
    if (0 == range || 0 == cnt) {
        s = 0;
    } else if (cnt <= 127) {
        s = ((cnt - bmin) * s_range + s_range) / cnt;
    } else {
        s = ((bmax - cnt) * s_range + s_range) / (255 - cnt);
    }
    l = ((bmax + bmin) * l_range + l_range) / 510;

    if (h < 0) {
        h += h_range;
    }
    if (h_range <= h) {
        h -= h_range;
    }
    if (s < 0) {
        s = 0;
    }
    if (s_range <= s) {
        s = s_range - 1;
    }
    if (l_range <= l) {
        l = l_range - 1;
    }
    pPix->r = static_cast<byte>(h);
    pPix->g = static_cast<byte>(s);
    pPix->b = static_cast<byte>(l);
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

void
fn_set_dst_hsl(type_histogram* pHistogram) {
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 h_weight = DEFINE_HUE_WEIGHT;
    const int32 l_weight = DEFINE_LIGHTNESS_WEIGHT;
    const auto hist_count = h_range * s_range * l_range;

    /*** ヒストグラム上位256個のHSL値を取得 ***/
    type_hsl_top hsl_top256[256];
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
        pTop->h = pTopHist->src_h;
        pTop->s = pTopHist->src_s;
        pTop->l = pTopHist->src_l;
    }

    /*** ヒストグラム上位256個の中で近いHSL値を変換先に設定 ***/
    for (int32 i = 0; i < hist_count; i++) {
        auto pHist = &pHistogram[i];
        if (UINT32_MAX == pHist->count) {
            continue; /*** 上位256個 ***/
        }
        pHist->count = UINT32_MAX;
        for (int32 t = 0; t < 256; t++) {
            auto pTop = &hsl_top256[t];
            auto diff_h = ((pHist->src_h - pTop->h) * 255 / h_range) * h_weight;
            auto diff_s = ((pHist->src_s - pTop->s) * 255 / s_range);
            auto diff_l = ((pHist->src_l - pTop->l) * 255 / l_range) * l_weight;
            auto dist2
                = static_cast<uint32>(diff_h * diff_h / 255)
                + static_cast<uint32>(diff_s * diff_s / 255)
                + static_cast<uint32>(diff_l * diff_l / 255);
            if (dist2 < pHist->count) {
                pHist->count = dist2;
                pHist->dst_h = pTop->h;
                pHist->dst_s = pTop->s;
                pHist->dst_l = pTop->l;
            }
        }
    }
}

void
fn_exec_declease(Bitmap* pBmp) {
    const int32 h_range = DEFINE_HUE_RANGE;
    const int32 s_range = DEFINE_SATURATION_RANGE;
    const int32 l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 hist_count = h_range * s_range * l_range;
    const auto size_max = static_cast<uint32>(pBmp->info_h.width * pBmp->info_h.height);

    auto pHist = reinterpret_cast<type_histogram*>(calloc(hist_count, sizeof(type_histogram)));
    if (NULL == pHist) {
        pBmp->error = -1;
        return;
    }

    auto pPix = reinterpret_cast<Bitmap::pix24*>(pBmp->pPix);
    for (uint32 i = 0; i < size_max; i++) {
        fn_rgb2hsl(&pPix[i]);
    }

    fn_calc_histogram(pHist, pBmp);
    fn_set_dst_hsl(pHist);

    for (uint32 i = 0; i < size_max; i++) {
        auto pHsl = &pPix[i];
        auto hist_index
            = pHsl->r * s_range * l_range
            + pHsl->g * l_range
            + pHsl->b;
        auto hist = pHist[hist_index];
        fn_hsl2rgb(pHsl, hist.dst_h, hist.dst_s, hist.dst_l);
    }

    free(pHist);
    pBmp->error = 0;
    return;
}
