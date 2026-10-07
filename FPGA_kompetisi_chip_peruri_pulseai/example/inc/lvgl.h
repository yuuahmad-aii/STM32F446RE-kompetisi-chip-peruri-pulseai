/**
 * @file lvgl.h
 * Include all LVGL related headers
 */

#ifndef LVGL_H
#define LVGL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************
 * CURRENT VERSION OF LVGL
 ***************************/
#include "lv_version.h"

/*********************
 *      INCLUDES
 *********************/
#include "../Src/lvgl/lv_init.h"

#include "../Src/lvgl/stdlib/lv_mem.h"
#include "../Src/lvgl/stdlib/lv_sprintf.h"
#include "../Src/lvgl/stdlib/lv_string.h"


#include "../Src/lvgl/misc/lv_anim_timeline.h"
#include "../Src/lvgl/misc/lv_array.h"
#include "../Src/lvgl/misc/lv_async.h"
#include "../Src/lvgl/misc/lv_circle_buf.h"
#include "../Src/lvgl/misc/lv_iter.h"
#include "../Src/lvgl/misc/lv_log.h"
#include "../Src/lvgl/misc/lv_math.h"
#include "../Src/lvgl/misc/lv_profiler_builtin.h"
#include "../Src/lvgl/misc/lv_rb.h"
#include "../Src/lvgl/misc/lv_timer.h"
#include "../Src/lvgl/misc/lv_tree.h"
#include "../Src/lvgl/misc/lv_utils.h"


#include "../Src/lvgl/osal/lv_os.h"

#include "../Src/lvgl/tick/lv_tick.h"

#include "../Src/lvgl/core/lv_group.h"
#include "../Src/lvgl/core/lv_obj.h"
#include "../Src/lvgl/core/lv_observer.h"
#include "../Src/lvgl/core/lv_refr.h"
#include "../Src/lvgl/display/lv_display.h"
#include "../Src/lvgl/indev/lv_gridnav.h"
#include "../Src/lvgl/indev/lv_indev.h"
#include "../Src/lvgl/indev/lv_indev_gesture.h"


#include "../Src/lvgl/font/binfont_loader/lv_binfont_loader.h"
#include "../Src/lvgl/font/fmt_txt/lv_font_fmt_txt.h"
#include "../Src/lvgl/font/font_manager/lv_font_manager.h"
#include "../Src/lvgl/font/imgfont/lv_imgfont.h"
#include "../Src/lvgl/font/lv_font.h"


#include "../Src/lvgl/widgets/3dtexture/lv_3dtexture.h"
#include "../Src/lvgl/widgets/animimage/lv_animimage.h"
#include "../Src/lvgl/widgets/arc/lv_arc.h"
#include "../Src/lvgl/widgets/arclabel/lv_arclabel.h"
#include "../Src/lvgl/widgets/bar/lv_bar.h"
#include "../Src/lvgl/widgets/button/lv_button.h"
#include "../Src/lvgl/widgets/buttonmatrix/lv_buttonmatrix.h"
#include "../Src/lvgl/widgets/calendar/lv_calendar.h"
#include "../Src/lvgl/widgets/canvas/lv_canvas.h"
#include "../Src/lvgl/widgets/chart/lv_chart.h"
#include "../Src/lvgl/widgets/checkbox/lv_checkbox.h"
#include "../Src/lvgl/widgets/dropdown/lv_dropdown.h"
#include "../Src/lvgl/widgets/gif/lv_gif.h"
#include "../Src/lvgl/widgets/image/lv_image.h"
#include "../Src/lvgl/widgets/imagebutton/lv_imagebutton.h"
#include "../Src/lvgl/widgets/ime/lv_ime_pinyin.h"
#include "../Src/lvgl/widgets/keyboard/lv_keyboard.h"
#include "../Src/lvgl/widgets/label/lv_label.h"
#include "../Src/lvgl/widgets/led/lv_led.h"
#include "../Src/lvgl/widgets/line/lv_line.h"
#include "../Src/lvgl/widgets/list/lv_list.h"
#include "../Src/lvgl/widgets/lottie/lv_lottie.h"
#include "../Src/lvgl/widgets/menu/lv_menu.h"
#include "../Src/lvgl/widgets/msgbox/lv_msgbox.h"
#include "../Src/lvgl/widgets/roller/lv_roller.h"
#include "../Src/lvgl/widgets/scale/lv_scale.h"
#include "../Src/lvgl/widgets/slider/lv_slider.h"
#include "../Src/lvgl/widgets/span/lv_span.h"
#include "../Src/lvgl/widgets/spinbox/lv_spinbox.h"
#include "../Src/lvgl/widgets/spinner/lv_spinner.h"
#include "../Src/lvgl/widgets/switch/lv_switch.h"
#include "../Src/lvgl/widgets/table/lv_table.h"
#include "../Src/lvgl/widgets/tabview/lv_tabview.h"
#include "../Src/lvgl/widgets/textarea/lv_textarea.h"
#include "../Src/lvgl/widgets/tileview/lv_tileview.h"
#include "../Src/lvgl/widgets/win/lv_win.h"


#include "../Src/lvgl/debugging/monkey/lv_monkey.h"
#include "../Src/lvgl/debugging/sysmon/lv_sysmon.h"
#include "../Src/lvgl/debugging/test/lv_test.h"


#include "../Src/lvgl/others/file_explorer/lv_file_explorer.h"
#include "../Src/lvgl/others/fragment/lv_fragment.h"
#include "../Src/lvgl/others/translation/lv_translation.h"


#include "../Src/lvgl/libs/barcode/lv_barcode.h"
#include "../Src/lvgl/libs/bin_decoder/lv_bin_decoder.h"
#include "../Src/lvgl/libs/bmp/lv_bmp.h"
#include "../Src/lvgl/libs/ffmpeg/lv_ffmpeg.h"
#include "../Src/lvgl/libs/freetype/lv_freetype.h"
#include "../Src/lvgl/libs/fsdrv/lv_fsdrv.h"
#include "../Src/lvgl/libs/gltf/gltf_data/lv_gltf_model.h"
#include "../Src/lvgl/libs/gltf/gltf_view/lv_gltf.h"
#include "../Src/lvgl/libs/gstreamer/lv_gstreamer.h"
#include "../Src/lvgl/libs/libjpeg_turbo/lv_libjpeg_turbo.h"
#include "../Src/lvgl/libs/libpng/lv_libpng.h"
#include "../Src/lvgl/libs/libwebp/lv_libwebp.h"
#include "../Src/lvgl/libs/lodepng/lv_lodepng.h"
#include "../Src/lvgl/libs/qrcode/lv_qrcode.h"
#include "../Src/lvgl/libs/rle/lv_rle.h"
#include "../Src/lvgl/libs/rlottie/lv_rlottie.h"
#include "../Src/lvgl/libs/svg/lv_svg.h"
#include "../Src/lvgl/libs/svg/lv_svg_render.h"
#include "../Src/lvgl/libs/tiny_ttf/lv_tiny_ttf.h"
#include "../Src/lvgl/libs/tjpgd/lv_tjpgd.h"


#include "../Src/lvgl/layouts/lv_layout.h"

#include "../Src/lvgl/draw/eve/lv_draw_eve_target.h"
#include "../Src/lvgl/draw/lv_draw_buf.h"
#include "../Src/lvgl/draw/lv_draw_vector.h"
#include "../Src/lvgl/draw/snapshot/lv_snapshot.h"
#include "../Src/lvgl/draw/sw/lv_draw_sw_utils.h"


#include "../Src/lvgl/themes/lv_theme.h"

#include "../Src/lvgl/drivers/lv_drivers.h"

/* Define LV_DISABLE_API_MAPPING using a compiler option
 * to make sure your application is not using deprecated names */
#ifndef LV_DISABLE_API_MAPPING
#include "../Src/lvgl/lv_api_map_v8.h"
#include "../Src/lvgl/lv_api_map_v9_0.h"
#include "../Src/lvgl/lv_api_map_v9_1.h"
#include "../Src/lvgl/lv_api_map_v9_2.h"
#include "../Src/lvgl/lv_api_map_v9_3.h"
#include "../Src/lvgl/lv_api_map_v9_4.h"
#endif /*LV_DISABLE_API_MAPPING*/

#if LV_USE_PRIVATE_API
#include "../Src/lvgl/lvgl_private.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/** Gives 1 if the x.y.z version is supported in the current version
 * Usage:
 *
 * - Require v6
 * #if LV_VERSION_CHECK(6,0,0)
 *   new_func_in_v6();
 * #endif
 *
 *
 * - Require at least v5.3
 * #if LV_VERSION_CHECK(5,3,0)
 *   new_feature_from_v5_3();
 * #endif
 *
 *
 * - Require v5.3.2 bugfixes
 * #if LV_VERSION_CHECK(5,3,2)
 *   bugfix_in_v5_3_2();
 * #endif
 *
 */
#define LV_VERSION_CHECK(x, y, z)                                              \
  (x == LVGL_VERSION_MAJOR &&                                                  \
   (y < LVGL_VERSION_MINOR ||                                                  \
    (y == LVGL_VERSION_MINOR && z <= LVGL_VERSION_PATCH)))

/**
 * Wrapper functions for VERSION macros
 */

static inline int lv_version_major(void) { return LVGL_VERSION_MAJOR; }

static inline int lv_version_minor(void) { return LVGL_VERSION_MINOR; }

static inline int lv_version_patch(void) { return LVGL_VERSION_PATCH; }

static inline const char *lv_version_info(void) { return LVGL_VERSION_INFO; }

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVGL_H*/
