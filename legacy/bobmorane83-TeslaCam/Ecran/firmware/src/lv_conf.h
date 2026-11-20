/**
 * LVGL configuration for TeslaCam Dashboard
 * JC3636W518C - ST77916 360x360 round display - ESP32-S3
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* ─── Color ─── */
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0
#define LV_COLOR_CHROMA_KEY lv_color_hex(0xFF00FF) /* magenta (not green, we use green for glow) */

/* ─── Memory ─── */
#define LV_MEM_CUSTOM      1
#if LV_MEM_CUSTOM == 1
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif

/* ─── HAL ─── */
#define LV_DISP_DEF_REFR_PERIOD  100   /* 10 Hz dashboard refresh */
#define LV_INDEV_DEF_READ_PERIOD 50
#define LV_TICK_CUSTOM            1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif
#define LV_DPI_DEF 130

/* ─── Draw ─── */
#define LV_DRAW_COMPLEX 1
#define LV_SHADOW_CACHE_SIZE 0
#define LV_CIRCLE_CACHE_SIZE 4

/* ─── GPU ─── */
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP     0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL         0

/* ─── Logging ─── */
#define LV_USE_LOG 0

/* ─── Asserts ─── */
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/* ─── Others ─── */
#define LV_USE_PERF_MONITOR  0
#define LV_USE_MEM_MONITOR   0
#define LV_USE_REFR_DEBUG    0
#define LV_SPRINTF_CUSTOM    0
#define LV_USE_USER_DATA     1
#define LV_ATTRIBUTE_FAST_MEM

/* ─── Font ─── */
#define LV_FONT_MONTSERRAT_8   0
#define LV_FONT_MONTSERRAT_10  0
#define LV_FONT_MONTSERRAT_12  1
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  0
#define LV_FONT_MONTSERRAT_18  1
#define LV_FONT_MONTSERRAT_20  0
#define LV_FONT_MONTSERRAT_22  0
#define LV_FONT_MONTSERRAT_24  1
#define LV_FONT_MONTSERRAT_26  0
#define LV_FONT_MONTSERRAT_28  0
#define LV_FONT_MONTSERRAT_30  0
#define LV_FONT_MONTSERRAT_32  0
#define LV_FONT_MONTSERRAT_34  0
#define LV_FONT_MONTSERRAT_36  0
#define LV_FONT_MONTSERRAT_38  0
#define LV_FONT_MONTSERRAT_40  0
#define LV_FONT_MONTSERRAT_42  0
#define LV_FONT_MONTSERRAT_44  0
#define LV_FONT_MONTSERRAT_46  0
#define LV_FONT_MONTSERRAT_48  1
#define LV_FONT_MONTSERRAT_28_COMPRESSED  0
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW  0
#define LV_FONT_SIMSUN_16_CJK            0
#define LV_FONT_UNSCII_8   0
#define LV_FONT_UNSCII_16  0
#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_FONT_FMT_TXT_LARGE 0
#define LV_USE_FONT_COMPRESSED 0
#define LV_USE_FONT_SUBPX     0

/* ─── Text ─── */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/* ─── Widgets ─── */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  0
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0

/* ─── Extra widgets ─── */
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_USE_CHART      0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      0
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/* ─── Layouts ─── */
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/* ─── Themes ─── */
#define LV_USE_THEME_DEFAULT 0
#define LV_USE_THEME_BASIC   0
#define LV_USE_THEME_MONO    0

/* ─── Others ─── */
#define LV_USE_FS_STDIO  0
#define LV_USE_FS_POSIX  0
#define LV_USE_FS_WIN32  0
#define LV_USE_FS_FATFS  0
#define LV_USE_PNG       0
#define LV_USE_BMP       0
#define LV_USE_SJPG      0
#define LV_USE_GIF       0
#define LV_USE_QRCODE    0
#define LV_USE_FREETYPE  0
#define LV_USE_RLOTTIE   0
#define LV_USE_FFMPEG    0
#define LV_USE_SNAPSHOT  0

#endif /* LV_CONF_H */
