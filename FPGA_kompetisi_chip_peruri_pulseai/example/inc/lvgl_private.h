/**
 * @file lvgl_private.h
 *
 */

#ifndef LVGL_PRIVATE_H
#define LVGL_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../Src/lvgl/core/lv_global.h"

#include "../Src/lvgl/display/lv_display_private.h"
#include "../Src/lvgl/indev/lv_indev_private.h"
#include "../Src/lvgl/misc/lv_text_private.h"
#include "../Src/lvgl/misc/cache/lv_cache.h"
#include "../Src/lvgl/misc/cache/lv_cache_entry_private.h"
#include "../Src/lvgl/misc/cache/lv_cache_private.h"
#include "../Src/lvgl/layouts/lv_layout_private.h"
#include "../Src/lvgl/stdlib/lv_mem_private.h"
#include "../Src/lvgl/others/file_explorer/lv_file_explorer_private.h"
#include "../Src/lvgl/others/fragment/lv_fragment_private.h"
#include "../Src/lvgl/others/translation/lv_translation_private.h"
#include "../Src/lvgl/libs/qrcode/lv_qrcode_private.h"
#include "../Src/lvgl/libs/barcode/lv_barcode_private.h"
#include "../Src/lvgl/draw/lv_draw_triangle_private.h"
#include "../Src/lvgl/draw/lv_draw_private.h"
#include "../Src/lvgl/draw/lv_draw_rect_private.h"
#include "../Src/lvgl/draw/lv_draw_image_private.h"
#include "../Src/lvgl/draw/lv_image_decoder_private.h"
#include "../Src/lvgl/draw/lv_draw_label_private.h"
#include "../Src/lvgl/draw/lv_draw_vector_private.h"
#include "../Src/lvgl/draw/lv_draw_buf_private.h"
#include "../Src/lvgl/draw/sw/lv_draw_sw_private.h"
#include "../Src/lvgl/draw/sw/lv_draw_sw_mask_private.h"
#include "../Src/lvgl/draw/sw/blend/lv_draw_sw_blend_private.h"
#include "../Src/lvgl/drivers/libinput/lv_xkb_private.h"
#include "../Src/lvgl/drivers/libinput/lv_libinput_private.h"
#include "../Src/lvgl/drivers/evdev/lv_evdev_private.h"
#include "../Src/lvgl/themes/lv_theme_private.h"
#include "../Src/lvgl/core/lv_refr_private.h"
#include "../Src/lvgl/core/lv_obj_style_private.h"
#include "../Src/lvgl/core/lv_obj_private.h"
#include "../Src/lvgl/core/lv_obj_scroll_private.h"
#include "../Src/lvgl/core/lv_obj_draw_private.h"
#include "../Src/lvgl/core/lv_obj_class_private.h"
#include "../Src/lvgl/core/lv_group_private.h"
#include "../Src/lvgl/core/lv_obj_event_private.h"
#include "../Src/lvgl/core/lv_observer_private.h"

#include "../Src/lvgl/debugging/sysmon/lv_sysmon_private.h"
#include "../Src/lvgl/debugging/monkey/lv_monkey_private.h"

#include "../Src/lvgl/font/fmt_txt/lv_font_fmt_txt_private.h"

#include "../Src/lvgl/misc/lv_timer_private.h"
#include "../Src/lvgl/misc/lv_area_private.h"
#include "../Src/lvgl/misc/lv_fs_private.h"
#include "../Src/lvgl/misc/lv_profiler_builtin_private.h"
#include "../Src/lvgl/misc/lv_event_private.h"
#include "../Src/lvgl/misc/lv_bidi_private.h"
#include "../Src/lvgl/misc/lv_rb_private.h"
#include "../Src/lvgl/misc/lv_style_private.h"
#include "../Src/lvgl/misc/lv_color_op_private.h"
#include "../Src/lvgl/misc/lv_anim_private.h"
#include "../Src/lvgl/misc/lv_anim_timeline_private.h"

#include "../Src/lvgl/widgets/msgbox/lv_msgbox_private.h"
#include "../Src/lvgl/widgets/buttonmatrix/lv_buttonmatrix_private.h"
#include "../Src/lvgl/widgets/slider/lv_slider_private.h"
#include "../Src/lvgl/widgets/switch/lv_switch_private.h"
#include "../Src/lvgl/widgets/calendar/lv_calendar_private.h"
#include "../Src/lvgl/widgets/imagebutton/lv_imagebutton_private.h"
#include "../Src/lvgl/widgets/bar/lv_bar_private.h"
#include "../Src/lvgl/widgets/image/lv_image_private.h"
#include "../Src/lvgl/widgets/textarea/lv_textarea_private.h"
#include "../Src/lvgl/widgets/table/lv_table_private.h"
#include "../Src/lvgl/widgets/checkbox/lv_checkbox_private.h"
#include "../Src/lvgl/widgets/roller/lv_roller_private.h"
#include "../Src/lvgl/widgets/win/lv_win_private.h"
#include "../Src/lvgl/widgets/keyboard/lv_keyboard_private.h"
#include "../Src/lvgl/widgets/line/lv_line_private.h"
#include "../Src/lvgl/widgets/animimage/lv_animimage_private.h"
#include "../Src/lvgl/widgets/dropdown/lv_dropdown_private.h"
#include "../Src/lvgl/widgets/menu/lv_menu_private.h"
#include "../Src/lvgl/widgets/chart/lv_chart_private.h"
#include "../Src/lvgl/widgets/button/lv_button_private.h"
#include "../Src/lvgl/widgets/scale/lv_scale_private.h"
#include "../Src/lvgl/widgets/led/lv_led_private.h"
#include "../Src/lvgl/widgets/arc/lv_arc_private.h"
#include "../Src/lvgl/widgets/tileview/lv_tileview_private.h"
#include "../Src/lvgl/widgets/spinbox/lv_spinbox_private.h"
#include "../Src/lvgl/widgets/span/lv_span_private.h"
#include "../Src/lvgl/widgets/label/lv_label_private.h"
#include "../Src/lvgl/widgets/canvas/lv_canvas_private.h"
#include "../Src/lvgl/widgets/tabview/lv_tabview_private.h"
#include "../Src/lvgl/widgets/3dtexture/lv_3dtexture_private.h"
#include "../Src/lvgl/widgets/ime/lv_ime_pinyin_private.h"

#include "../Src/lvgl/tick/lv_tick_private.h"
#include "../Src/lvgl/stdlib/builtin/lv_tlsf_private.h"
#include "../Src/lvgl/libs/rlottie/lv_rlottie_private.h"
#include "../Src/lvgl/libs/ffmpeg/lv_ffmpeg_private.h"
#include "../Src/lvgl/widgets/lottie/lv_lottie_private.h"
#include "../Src/lvgl/osal/lv_os_private.h"

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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVGL_PRIVATE_H*/
