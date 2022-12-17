#ifndef __DECLEASE_H__
#define __DECLEASE_H__

#include "../CommonLib/Bitmap.h"

#define DEFINE_HUE_RANGE        (12)
#define DEFINE_SATURATION_RANGE (5)
#define DEFINE_LIGHTNESS_RANGE  (32)

#define DEFINE_AVG_CALC_DIST         (7)
#define DEFINE_AVG_WEIGHT_HUE        (32)
#define DEFINE_AVG_WEIGHT_SATURATION (4)
#define DEFINE_AVG_WEIGHT_LIGHTNESS  (32)

/*
減色を行う
24bitビットマップを8bitビットマップに変換
*/
void
declease_exec(Bitmap* pInBmp24, Bitmap* pOutBmp8);

/*
HSLエンコード
*/
inline void
declease_rgb2hsl(Bitmap::pix24* pPix) {
    int32 h, s, l;
    const byte h_range = DEFINE_HUE_RANGE;
    const byte s_range = DEFINE_SATURATION_RANGE;
    const byte l_range = DEFINE_LIGHTNESS_RANGE;
    auto bmax = fmax(pPix->r, fmax(pPix->g, pPix->b));
    auto bmin = fmin(pPix->r, fmin(pPix->g, pPix->b));
    auto range = bmax - bmin;
    auto h_offset = h_range * range / 3;
    if (0 == range) {
        h = 0;
    } else if (pPix->r == bmax) {
        h = static_cast<int32>(((pPix->g - pPix->b) * h_range + h_offset) / (range * 6.0));
    } else if (pPix->g == bmax) {
        h = static_cast<int32>(((pPix->b - pPix->r) * h_range + h_offset) / (range * 6.0) + h_range / 3.0);
    } else if (pPix->b == bmax) {
        h = static_cast<int32>(((pPix->r - pPix->g) * h_range + h_offset) / (range * 6.0) + 2 * h_range / 3.0);
    } else {
        h = 0;
    }
    auto cnt = (bmax + bmin) / 2;
    if (0 == range || 0 == cnt) {
        s = 0;
    } else if (cnt <= 127) {
        s = static_cast<int32>(((cnt - bmin) * s_range + s_range) / cnt);
    } else {
        s = static_cast<int32>(((bmax - cnt) * s_range + s_range) / (255 - cnt));
    }
    l = static_cast<int32>(((bmax + bmin) * l_range + l_range) / 510);
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

/*
HSLデコード
*/
inline void
declease_hsl2rgb(Bitmap::pix24* pPix, byte h, byte s, byte l) {
    const byte h_range = DEFINE_HUE_RANGE;
    const byte s_range = DEFINE_SATURATION_RANGE;
    const byte l_range = DEFINE_LIGHTNESS_RANGE;
    double dmax, dmin;
    if (l < l_range / 2) {
        dmax = (l + l * static_cast<double>(s) / s_range) * 255 / l_range;
        dmin = (l - l * static_cast<double>(s) / s_range) * 255 / l_range;
    } else {
        dmax = (l + (l_range - l) * static_cast<double>(s) / s_range) * 255 / l_range;
        dmin = (l - (l_range - l) * static_cast<double>(s) / s_range) * 255 / l_range;
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

/*
周辺HSLの平均を求める
*/
inline int32
declease_avghsl(Bitmap* pBmp, Bitmap::pix24* pHsl, point pos) {
    const byte h_range = DEFINE_HUE_RANGE;
    const byte s_range = DEFINE_SATURATION_RANGE;
    const byte l_range = DEFINE_LIGHTNESS_RANGE;
    const int32 calc_dist = DEFINE_AVG_CALC_DIST;
    double avg_h = 0.0, avg_s = 0.0, avg_l = 0.0;
    int32 calc_count = 0;
    auto pPix = reinterpret_cast<Bitmap::pix24*>(pBmp->pPix);
    for (int32 dy = -calc_dist; dy <= calc_dist; dy++) {
        for (int32 dx = -calc_dist; dx <= calc_dist; dx++) {
            auto dr = sqrt(dx * dx + dy * dy);
            if (calc_dist < dr) {
                continue;
            }
            auto index = bitmap_get_index_ofs(*pBmp, pos, dx, dy);
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

#endif //__DECLEASE_H__
