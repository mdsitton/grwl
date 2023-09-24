//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#if defined(_GRWL_WAYLAND)
    #include <wayland-client-core.h>
    #include <xkbcommon/xkbcommon.h>
    #include <xkbcommon/xkbcommon-compose.h>

    #include <cstdbool>

typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;

typedef struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType sType;
    const void* pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display* display;
    struct wl_surface* surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef VkResult(APIENTRY* PFN_vkCreateWaylandSurfaceKHR)(VkInstance, const VkWaylandSurfaceCreateInfoKHR*,
                                                          const VkAllocationCallbacks*, VkSurfaceKHR*);
typedef VkBool32(APIENTRY* PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice, uint32_t,
                                                                                 struct wl_display*);

    #include "xkb_unicode.h"
    #include "posix_poll.h"

typedef int (*PFN_wl_display_flush)(struct wl_display* display);
typedef void (*PFN_wl_display_cancel_read)(struct wl_display* display);
typedef int (*PFN_wl_display_dispatch_pending)(struct wl_display* display);
typedef int (*PFN_wl_display_read_events)(struct wl_display* display);
typedef struct wl_display* (*PFN_wl_display_connect)(const char*);
typedef void (*PFN_wl_display_disconnect)(struct wl_display*);
typedef int (*PFN_wl_display_roundtrip)(struct wl_display*);
typedef int (*PFN_wl_display_get_fd)(struct wl_display*);
typedef int (*PFN_wl_display_prepare_read)(struct wl_display*);
typedef void (*PFN_wl_proxy_marshal)(struct wl_proxy*, uint32_t, ...);
typedef int (*PFN_wl_proxy_add_listener)(struct wl_proxy*, void (**)(), void*);
typedef void (*PFN_wl_proxy_destroy)(struct wl_proxy*);
typedef struct wl_proxy* (*PFN_wl_proxy_marshal_constructor)(struct wl_proxy*, uint32_t, const struct wl_interface*,
                                                             ...);
typedef struct wl_proxy* (*PFN_wl_proxy_marshal_constructor_versioned)(struct wl_proxy*, uint32_t,
                                                                       const struct wl_interface*, uint32_t, ...);
typedef void* (*PFN_wl_proxy_get_user_data)(struct wl_proxy*);
typedef void (*PFN_wl_proxy_set_user_data)(struct wl_proxy*, void*);
typedef void (*PFN_wl_proxy_set_tag)(struct wl_proxy*, const char* const*);
typedef const char* const* (*PFN_wl_proxy_get_tag)(struct wl_proxy*);
typedef uint32_t (*PFN_wl_proxy_get_version)(struct wl_proxy*);
typedef struct wl_proxy* (*PFN_wl_proxy_marshal_flags)(struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t,
                                                       uint32_t, ...);
    #define wl_display_flush _grwl.wl.client.display_flush
    #define wl_display_cancel_read _grwl.wl.client.display_cancel_read
    #define wl_display_dispatch_pending _grwl.wl.client.display_dispatch_pending
    #define wl_display_read_events _grwl.wl.client.display_read_events
    #define wl_display_disconnect _grwl.wl.client.display_disconnect
    #define wl_display_roundtrip _grwl.wl.client.display_roundtrip
    #define wl_display_get_fd _grwl.wl.client.display_get_fd
    #define wl_display_prepare_read _grwl.wl.client.display_prepare_read
    #define wl_proxy_marshal _grwl.wl.client.proxy_marshal
    #define wl_proxy_add_listener _grwl.wl.client.proxy_add_listener
    #define wl_proxy_destroy _grwl.wl.client.proxy_destroy
    #define wl_proxy_marshal_constructor _grwl.wl.client.proxy_marshal_constructor
    #define wl_proxy_marshal_constructor_versioned _grwl.wl.client.proxy_marshal_constructor_versioned
    #define wl_proxy_get_user_data _grwl.wl.client.proxy_get_user_data
    #define wl_proxy_set_user_data _grwl.wl.client.proxy_set_user_data
    #define wl_proxy_get_tag _grwl.wl.client.proxy_get_tag
    #define wl_proxy_set_tag _grwl.wl.client.proxy_set_tag
    #define wl_proxy_get_version _grwl.wl.client.proxy_get_version
    #define wl_proxy_marshal_flags _grwl.wl.client.proxy_marshal_flags

struct wl_shm;
struct wl_output;

    #define wl_display_interface _grwl_wl_display_interface
    #define wl_subcompositor_interface _grwl_wl_subcompositor_interface
    #define wl_compositor_interface _grwl_wl_compositor_interface
    #define wl_shm_interface _grwl_wl_shm_interface
    #define wl_data_device_manager_interface _grwl_wl_data_device_manager_interface
    #define wl_shell_interface _grwl_wl_shell_interface
    #define wl_buffer_interface _grwl_wl_buffer_interface
    #define wl_callback_interface _grwl_wl_callback_interface
    #define wl_data_device_interface _grwl_wl_data_device_interface
    #define wl_data_offer_interface _grwl_wl_data_offer_interface
    #define wl_data_source_interface _grwl_wl_data_source_interface
    #define wl_keyboard_interface _grwl_wl_keyboard_interface
    #define wl_output_interface _grwl_wl_output_interface
    #define wl_pointer_interface _grwl_wl_pointer_interface
    #define wl_region_interface _grwl_wl_region_interface
    #define wl_registry_interface _grwl_wl_registry_interface
    #define wl_seat_interface _grwl_wl_seat_interface
    #define wl_shell_surface_interface _grwl_wl_shell_surface_interface
    #define wl_shm_pool_interface _grwl_wl_shm_pool_interface
    #define wl_subsurface_interface _grwl_wl_subsurface_interface
    #define wl_surface_interface _grwl_wl_surface_interface
    #define wl_touch_interface _grwl_wl_touch_interface
    #define zwp_idle_inhibitor_v1_interface _grwl_zwp_idle_inhibitor_v1_interface
    #define zwp_idle_inhibit_manager_v1_interface _grwl_zwp_idle_inhibit_manager_v1_interface
    #define zwp_confined_pointer_v1_interface _grwl_zwp_confined_pointer_v1_interface
    #define zwp_locked_pointer_v1_interface _grwl_zwp_locked_pointer_v1_interface
    #define zwp_pointer_constraints_v1_interface _grwl_zwp_pointer_constraints_v1_interface
    #define zwp_relative_pointer_v1_interface _grwl_zwp_relative_pointer_v1_interface
    #define zwp_relative_pointer_manager_v1_interface _grwl_zwp_relative_pointer_manager_v1_interface
    #define wp_viewport_interface _grwl_wp_viewport_interface
    #define wp_viewporter_interface _grwl_wp_viewporter_interface
    #define xdg_toplevel_interface _grwl_xdg_toplevel_interface
    #define zxdg_toplevel_decoration_v1_interface _grwl_zxdg_toplevel_decoration_v1_interface
    #define zxdg_decoration_manager_v1_interface _grwl_zxdg_decoration_manager_v1_interface
    #define xdg_popup_interface _grwl_xdg_popup_interface
    #define xdg_positioner_interface _grwl_xdg_positioner_interface
    #define xdg_surface_interface _grwl_xdg_surface_interface
    #define xdg_toplevel_interface _grwl_xdg_toplevel_interface
    #define xdg_wm_base_interface _grwl_xdg_wm_base_interface

    #define GRWL_WAYLAND_WINDOW_STATE _GRWLwindowWayland wl;
    #define GRWL_WAYLAND_LIBRARY_WINDOW_STATE _GRWLlibraryWayland wl;
    #define GRWL_WAYLAND_MONITOR_STATE _GRWLmonitorWayland wl;
    #define GRWL_WAYLAND_CURSOR_STATE _GRWLcursorWayland wl;

struct wl_cursor_image
{
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};

struct wl_cursor
{
    unsigned int image_count;
    struct wl_cursor_image** images;
    char* name;
};
typedef struct wl_cursor_theme* (*PFN_wl_cursor_theme_load)(const char*, int, struct wl_shm*);
typedef void (*PFN_wl_cursor_theme_destroy)(struct wl_cursor_theme*);
typedef struct wl_cursor* (*PFN_wl_cursor_theme_get_cursor)(struct wl_cursor_theme*, const char*);
typedef struct wl_buffer* (*PFN_wl_cursor_image_get_buffer)(struct wl_cursor_image*);
    #define wl_cursor_theme_load _grwl.wl.cursor.theme_load
    #define wl_cursor_theme_destroy _grwl.wl.cursor.theme_destroy
    #define wl_cursor_theme_get_cursor _grwl.wl.cursor.theme_get_cursor
    #define wl_cursor_image_get_buffer _grwl.wl.cursor.image_get_buffer

typedef struct wl_egl_window* (*PFN_wl_egl_window_create)(struct wl_surface*, int, int);
typedef void (*PFN_wl_egl_window_destroy)(struct wl_egl_window*);
typedef void (*PFN_wl_egl_window_resize)(struct wl_egl_window*, int, int, int, int);
    #define wl_egl_window_create _grwl.wl.egl.window_create
    #define wl_egl_window_destroy _grwl.wl.egl.window_destroy
    #define wl_egl_window_resize _grwl.wl.egl.window_resize

typedef struct xkb_context* (*PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (*PFN_xkb_context_unref)(struct xkb_context*);
typedef struct xkb_keymap* (*PFN_xkb_keymap_new_from_string)(struct xkb_context*, const char*, enum xkb_keymap_format,
                                                             enum xkb_keymap_compile_flags);
typedef void (*PFN_xkb_keymap_unref)(struct xkb_keymap*);
typedef xkb_mod_index_t (*PFN_xkb_keymap_mod_get_index)(struct xkb_keymap*, const char*);
typedef int (*PFN_xkb_keymap_key_repeats)(struct xkb_keymap*, xkb_keycode_t);
typedef int (*PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap*, xkb_keycode_t, xkb_layout_index_t,
                                                    xkb_level_index_t, const xkb_keysym_t**);
typedef struct xkb_state* (*PFN_xkb_state_new)(struct xkb_keymap*);
typedef void (*PFN_xkb_state_unref)(struct xkb_state*);
typedef int (*PFN_xkb_state_key_get_syms)(struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**);
typedef enum xkb_state_component (*PFN_xkb_state_update_mask)(struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t,
                                                              xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t,
                                                              xkb_layout_index_t);
typedef xkb_layout_index_t (*PFN_xkb_state_key_get_layout)(struct xkb_state*, xkb_keycode_t);
typedef int (*PFN_xkb_state_mod_index_is_active)(struct xkb_state*, xkb_mod_index_t, enum xkb_state_component);
typedef xkb_mod_mask_t (*PFN_xkb_state_serialize_mods)(struct xkb_state*, enum xkb_state_component);
typedef const char* (*PFN_xkb_keymap_layout_get_name)(struct xkb_keymap*, xkb_layout_index_t);
    #define xkb_context_new _grwl.wl.xkb.context_new
    #define xkb_context_unref _grwl.wl.xkb.context_unref
    #define xkb_keymap_new_from_string _grwl.wl.xkb.keymap_new_from_string
    #define xkb_keymap_unref _grwl.wl.xkb.keymap_unref
    #define xkb_keymap_mod_get_index _grwl.wl.xkb.keymap_mod_get_index
    #define xkb_keymap_key_repeats _grwl.wl.xkb.keymap_key_repeats
    #define xkb_keymap_key_get_syms_by_level _grwl.wl.xkb.keymap_key_get_syms_by_level
    #define xkb_state_new _grwl.wl.xkb.state_new
    #define xkb_state_unref _grwl.wl.xkb.state_unref
    #define xkb_state_key_get_syms _grwl.wl.xkb.state_key_get_syms
    #define xkb_state_update_mask _grwl.wl.xkb.state_update_mask
    #define xkb_state_key_get_layout _grwl.wl.xkb.state_key_get_layout
    #define xkb_state_mod_index_is_active _grwl.wl.xkb.state_mod_index_is_active
    #define xkb_state_serialize_mods _grwl.wl.xkb.state_serialize_mods
    #define xkb_keymap_layout_get_name _grwl.wl.xkb.keymap_layout_get_name

typedef struct xkb_compose_table* (*PFN_xkb_compose_table_new_from_locale)(struct xkb_context*, const char*,
                                                                           enum xkb_compose_compile_flags);
typedef void (*PFN_xkb_compose_table_unref)(struct xkb_compose_table*);
typedef struct xkb_compose_state* (*PFN_xkb_compose_state_new)(struct xkb_compose_table*, enum xkb_compose_state_flags);
typedef void (*PFN_xkb_compose_state_unref)(struct xkb_compose_state*);
typedef enum xkb_compose_feed_result (*PFN_xkb_compose_state_feed)(struct xkb_compose_state*, xkb_keysym_t);
typedef enum xkb_compose_status (*PFN_xkb_compose_state_get_status)(struct xkb_compose_state*);
typedef xkb_keysym_t (*PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state*);
    #define xkb_compose_table_new_from_locale _grwl.wl.xkb.compose_table_new_from_locale
    #define xkb_compose_table_unref _grwl.wl.xkb.compose_table_unref
    #define xkb_compose_state_new _grwl.wl.xkb.compose_state_new
    #define xkb_compose_state_unref _grwl.wl.xkb.compose_state_unref
    #define xkb_compose_state_feed _grwl.wl.xkb.compose_state_feed
    #define xkb_compose_state_get_status _grwl.wl.xkb.compose_state_get_status
    #define xkb_compose_state_get_one_sym _grwl.wl.xkb.compose_state_get_one_sym

struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;

enum libdecor_error
{
    LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
    LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
};

enum libdecor_window_state
{
    LIBDECOR_WINDOW_STATE_NONE = 0,
    LIBDECOR_WINDOW_STATE_ACTIVE = 1,
    LIBDECOR_WINDOW_STATE_MAXIMIZED = 2,
    LIBDECOR_WINDOW_STATE_FULLSCREEN = 4,
    LIBDECOR_WINDOW_STATE_TILED_LEFT = 8,
    LIBDECOR_WINDOW_STATE_TILED_RIGHT = 16,
    LIBDECOR_WINDOW_STATE_TILED_TOP = 32,
    LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 64
};

enum libdecor_capabilities
{
    LIBDECOR_ACTION_MOVE = 1,
    LIBDECOR_ACTION_RESIZE = 2,
    LIBDECOR_ACTION_MINIMIZE = 4,
    LIBDECOR_ACTION_FULLSCREEN = 8,
    LIBDECOR_ACTION_CLOSE = 16
};

struct libdecor_interface
{
    void (*error)(struct libdecor*, enum libdecor_error, const char*);
    void (*reserved0)();
    void (*reserved1)();
    void (*reserved2)();
    void (*reserved3)();
    void (*reserved4)();
    void (*reserved5)();
    void (*reserved6)();
    void (*reserved7)();
    void (*reserved8)();
    void (*reserved9)();
};

struct libdecor_frame_interface
{
    void (*configure)(struct libdecor_frame*, struct libdecor_configuration*, void*);
    void (*close)(struct libdecor_frame*, void*);
    void (*commit)(struct libdecor_frame*, void*);
    void (*dismiss_popup)(struct libdecor_frame*, const char*, void*);
    void (*reserved0)();
    void (*reserved1)();
    void (*reserved2)();
    void (*reserved3)();
    void (*reserved4)();
    void (*reserved5)();
    void (*reserved6)();
    void (*reserved7)();
    void (*reserved8)();
    void (*reserved9)();
};

typedef struct libdecor* (*PFN_libdecor_new)(struct wl_display*, const struct libdecor_interface*);
typedef void (*PFN_libdecor_unref)(struct libdecor*);
typedef int (*PFN_libdecor_get_fd)(struct libdecor*);
typedef int (*PFN_libdecor_dispatch)(struct libdecor*, int);
typedef struct libdecor_frame* (*PFN_libdecor_decorate)(struct libdecor*, struct wl_surface*,
                                                        const struct libdecor_frame_interface*, void*);
typedef void (*PFN_libdecor_frame_unref)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_set_app_id)(struct libdecor_frame*, const char*);
typedef void (*PFN_libdecor_frame_set_title)(struct libdecor_frame*, const char*);
typedef void (*PFN_libdecor_frame_set_minimized)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_set_fullscreen)(struct libdecor_frame*, struct wl_output*);
typedef void (*PFN_libdecor_frame_unset_fullscreen)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_map)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_commit)(struct libdecor_frame*, struct libdecor_state*,
                                          struct libdecor_configuration*);
typedef void (*PFN_libdecor_frame_set_min_content_size)(struct libdecor_frame*, int, int);
typedef void (*PFN_libdecor_frame_set_max_content_size)(struct libdecor_frame*, int, int);
typedef void (*PFN_libdecor_frame_set_maximized)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_unset_maximized)(struct libdecor_frame*);
typedef void (*PFN_libdecor_frame_set_capabilities)(struct libdecor_frame*, enum libdecor_capabilities);
typedef void (*PFN_libdecor_frame_unset_capabilities)(struct libdecor_frame*, enum libdecor_capabilities);
typedef void (*PFN_libdecor_frame_set_visibility)(struct libdecor_frame*, bool visible);
typedef struct xdg_toplevel* (*PFN_libdecor_frame_get_xdg_toplevel)(struct libdecor_frame*);
typedef bool (*PFN_libdecor_configuration_get_content_size)(struct libdecor_configuration*, struct libdecor_frame*,
                                                            int*, int*);
typedef bool (*PFN_libdecor_configuration_get_window_state)(struct libdecor_configuration*,
                                                            enum libdecor_window_state*);
typedef struct libdecor_state* (*PFN_libdecor_state_new)(int, int);
typedef void (*PFN_libdecor_state_free)(struct libdecor_state*);
    #define libdecor_new _grwl.wl.libdecor.libdecor_new_
    #define libdecor_unref _grwl.wl.libdecor.libdecor_unref_
    #define libdecor_get_fd _grwl.wl.libdecor.libdecor_get_fd_
    #define libdecor_dispatch _grwl.wl.libdecor.libdecor_dispatch_
    #define libdecor_decorate _grwl.wl.libdecor.libdecor_decorate_
    #define libdecor_frame_unref _grwl.wl.libdecor.libdecor_frame_unref_
    #define libdecor_frame_set_app_id _grwl.wl.libdecor.libdecor_frame_set_app_id_
    #define libdecor_frame_set_title _grwl.wl.libdecor.libdecor_frame_set_title_
    #define libdecor_frame_set_minimized _grwl.wl.libdecor.libdecor_frame_set_minimized_
    #define libdecor_frame_set_fullscreen _grwl.wl.libdecor.libdecor_frame_set_fullscreen_
    #define libdecor_frame_unset_fullscreen _grwl.wl.libdecor.libdecor_frame_unset_fullscreen_
    #define libdecor_frame_map _grwl.wl.libdecor.libdecor_frame_map_
    #define libdecor_frame_commit _grwl.wl.libdecor.libdecor_frame_commit_
    #define libdecor_frame_set_min_content_size _grwl.wl.libdecor.libdecor_frame_set_min_content_size_
    #define libdecor_frame_set_max_content_size _grwl.wl.libdecor.libdecor_frame_set_max_content_size_
    #define libdecor_frame_set_maximized _grwl.wl.libdecor.libdecor_frame_set_maximized_
    #define libdecor_frame_unset_maximized _grwl.wl.libdecor.libdecor_frame_unset_maximized_
    #define libdecor_frame_set_capabilities _grwl.wl.libdecor.libdecor_frame_set_capabilities_
    #define libdecor_frame_unset_capabilities _grwl.wl.libdecor.libdecor_frame_unset_capabilities_
    #define libdecor_frame_set_visibility _grwl.wl.libdecor.libdecor_frame_set_visibility_
    #define libdecor_frame_get_xdg_toplevel _grwl.wl.libdecor.libdecor_frame_get_xdg_toplevel_
    #define libdecor_configuration_get_content_size _grwl.wl.libdecor.libdecor_configuration_get_content_size_
    #define libdecor_configuration_get_window_state _grwl.wl.libdecor.libdecor_configuration_get_window_state_
    #define libdecor_state_new _grwl.wl.libdecor.libdecor_state_new_
    #define libdecor_state_free _grwl.wl.libdecor.libdecor_state_free_

typedef enum _GRWLdecorationSideWayland
{
    GRWL_MAIN_WINDOW,
    GRWL_TOP_DECORATION,
    GRWL_LEFT_DECORATION,
    GRWL_RIGHT_DECORATION,
    GRWL_BOTTOM_DECORATION
} _GRWLdecorationSideWayland;

typedef struct _GRWLdecorationWayland
{
    struct wl_surface* surface;
    struct wl_subsurface* subsurface;
    struct wp_viewport* viewport;
} _GRWLdecorationWayland;

typedef struct _GRWLofferWayland
{
    struct wl_data_offer* offer;
    bool text_plain_utf8;
    bool text_uri_list;
} _GRWLofferWayland;

typedef struct _GRWLscaleWayland
{
    struct wl_output* output;
    int factor;
} _GRWLscaleWayland;

// Wayland-specific per-window data
//
typedef struct _GRWLwindowWayland
{
    int width, height;
    bool visible;
    bool maximized;
    bool activated;
    bool fullscreen;
    bool hovered;
    bool transparent;
    struct wl_surface* surface;
    struct wl_callback* callback;

    struct
    {
        struct wl_egl_window* window;
    } egl;

    struct
    {
        int width, height;
        bool maximized;
        bool iconified;
        bool activated;
        bool fullscreen;
    } pending;

    struct
    {
        struct xdg_surface* surface;
        struct xdg_toplevel* toplevel;
        struct zxdg_toplevel_decoration_v1* decoration;
        uint32_t decorationMode;
    } xdg;

    struct
    {
        struct libdecor_frame* frame;
        int mode;
    } libdecor;

    _GRWLcursor* currentCursor;
    double cursorPosX, cursorPosY;

    char* title;
    char* appId;

    // We need to track the monitors the window spans on to calculate the
    // optimal scaling factor.
    int contentScale;
    _GRWLscaleWayland* scales;
    int scaleCount;
    int scaleSize;

    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer;
    struct zwp_confined_pointer_v1* confinedPointer;

    struct zwp_idle_inhibitor_v1* idleInhibitor;
    struct xdg_activation_token_v1* activationToken;

    struct
    {
        struct wl_buffer* buffer;
        _GRWLdecorationWayland top, left, right, bottom;
        _GRWLdecorationSideWayland focus;
    } decorations;

    struct zwp_text_input_v1* textInputV1;
    struct zwp_text_input_v3* textInputV3;

    struct
    {
        char* preeditText;
        char* commitTextOnReset;
    } textInputV1Context;
} _GRWLwindowWayland;

// Wayland-specific global data
//
typedef struct _GRWLlibraryWayland
{
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_subcompositor* subcompositor;
    struct wl_shm* shm;
    struct wl_seat* seat;
    struct wl_pointer* pointer;
    struct wl_keyboard* keyboard;
    struct wl_data_device_manager* dataDeviceManager;
    struct wl_data_device* dataDevice;
    struct xdg_wm_base* wmBase;
    struct zxdg_decoration_manager_v1* decorationManager;
    struct wp_viewporter* viewporter;
    struct zwp_relative_pointer_manager_v1* relativePointerManager;
    struct zwp_pointer_constraints_v1* pointerConstraints;
    struct zwp_idle_inhibit_manager_v1* idleInhibitManager;
    struct zwp_text_input_manager_v1* textInputManagerV1;
    struct zwp_text_input_manager_v3* textInputManagerV3;
    struct xdg_activation_v1* activationManager;

    _GRWLofferWayland* offers;
    unsigned int offerCount;

    struct wl_data_offer* selectionOffer;
    struct wl_data_source* selectionSource;

    struct wl_data_offer* dragOffer;
    _GRWLwindow* dragFocus;
    uint32_t dragSerial;

    const char* tag;

    struct wl_cursor_theme* cursorTheme;
    struct wl_cursor_theme* cursorThemeHiDPI;
    struct wl_surface* cursorSurface;
    const char* cursorPreviousName;
    int cursorTimerfd;
    uint32_t serial;
    uint32_t pointerEnterSerial;

    int keyRepeatTimerfd;
    int32_t keyRepeatRate;
    int32_t keyRepeatDelay;
    int keyRepeatScancode;

    char* clipboardString;
    char* keyboardLayoutName;
    short int keycodes[256];
    short int scancodes[GRWL_KEY_LAST + 1];
    char keynames[GRWL_KEY_LAST + 1][5];

    struct
    {
        void* handle;
        struct xkb_context* context;
        struct xkb_keymap* keymap;
        struct xkb_state* state;

        struct xkb_compose_state* composeState;

        xkb_mod_index_t controlIndex;
        xkb_mod_index_t altIndex;
        xkb_mod_index_t shiftIndex;
        xkb_mod_index_t superIndex;
        xkb_mod_index_t capsLockIndex;
        xkb_mod_index_t numLockIndex;
        unsigned int modifiers;
        xkb_layout_index_t group;

        PFN_xkb_context_new context_new;
        PFN_xkb_context_unref context_unref;
        PFN_xkb_keymap_new_from_string keymap_new_from_string;
        PFN_xkb_keymap_unref keymap_unref;
        PFN_xkb_keymap_mod_get_index keymap_mod_get_index;
        PFN_xkb_keymap_key_repeats keymap_key_repeats;
        PFN_xkb_keymap_key_get_syms_by_level keymap_key_get_syms_by_level;
        PFN_xkb_state_new state_new;
        PFN_xkb_state_unref state_unref;
        PFN_xkb_state_key_get_syms state_key_get_syms;
        PFN_xkb_state_update_mask state_update_mask;
        PFN_xkb_state_key_get_layout state_key_get_layout;
        PFN_xkb_state_mod_index_is_active state_mod_index_is_active;
        PFN_xkb_state_serialize_mods state_serialize_mods;
        PFN_xkb_keymap_layout_get_name keymap_layout_get_name;
        PFN_xkb_compose_table_new_from_locale compose_table_new_from_locale;
        PFN_xkb_compose_table_unref compose_table_unref;
        PFN_xkb_compose_state_new compose_state_new;
        PFN_xkb_compose_state_unref compose_state_unref;
        PFN_xkb_compose_state_feed compose_state_feed;
        PFN_xkb_compose_state_get_status compose_state_get_status;
        PFN_xkb_compose_state_get_one_sym compose_state_get_one_sym;
    } xkb;

    _GRWLwindow* pointerFocus;
    _GRWLwindow* keyboardFocus;

    struct
    {
        void* handle;
        PFN_wl_display_flush display_flush;
        PFN_wl_display_cancel_read display_cancel_read;
        PFN_wl_display_dispatch_pending display_dispatch_pending;
        PFN_wl_display_read_events display_read_events;
        PFN_wl_display_disconnect display_disconnect;
        PFN_wl_display_roundtrip display_roundtrip;
        PFN_wl_display_get_fd display_get_fd;
        PFN_wl_display_prepare_read display_prepare_read;
        PFN_wl_proxy_marshal proxy_marshal;
        PFN_wl_proxy_add_listener proxy_add_listener;
        PFN_wl_proxy_destroy proxy_destroy;
        PFN_wl_proxy_marshal_constructor proxy_marshal_constructor;
        PFN_wl_proxy_marshal_constructor_versioned proxy_marshal_constructor_versioned;
        PFN_wl_proxy_get_user_data proxy_get_user_data;
        PFN_wl_proxy_set_user_data proxy_set_user_data;
        PFN_wl_proxy_get_tag proxy_get_tag;
        PFN_wl_proxy_set_tag proxy_set_tag;
        PFN_wl_proxy_get_version proxy_get_version;
        PFN_wl_proxy_marshal_flags proxy_marshal_flags;
    } client;

    struct
    {
        void* handle;

        PFN_wl_cursor_theme_load theme_load;
        PFN_wl_cursor_theme_destroy theme_destroy;
        PFN_wl_cursor_theme_get_cursor theme_get_cursor;
        PFN_wl_cursor_image_get_buffer image_get_buffer;
    } cursor;

    struct
    {
        void* handle;

        PFN_wl_egl_window_create window_create;
        PFN_wl_egl_window_destroy window_destroy;
        PFN_wl_egl_window_resize window_resize;
    } egl;

    struct
    {
        void* handle;
        struct libdecor* context;
        PFN_libdecor_new libdecor_new_;
        PFN_libdecor_unref libdecor_unref_;
        PFN_libdecor_get_fd libdecor_get_fd_;
        PFN_libdecor_dispatch libdecor_dispatch_;
        PFN_libdecor_decorate libdecor_decorate_;
        PFN_libdecor_frame_unref libdecor_frame_unref_;
        PFN_libdecor_frame_set_app_id libdecor_frame_set_app_id_;
        PFN_libdecor_frame_set_title libdecor_frame_set_title_;
        PFN_libdecor_frame_set_minimized libdecor_frame_set_minimized_;
        PFN_libdecor_frame_set_fullscreen libdecor_frame_set_fullscreen_;
        PFN_libdecor_frame_unset_fullscreen libdecor_frame_unset_fullscreen_;
        PFN_libdecor_frame_map libdecor_frame_map_;
        PFN_libdecor_frame_commit libdecor_frame_commit_;
        PFN_libdecor_frame_set_min_content_size libdecor_frame_set_min_content_size_;
        PFN_libdecor_frame_set_max_content_size libdecor_frame_set_max_content_size_;
        PFN_libdecor_frame_set_maximized libdecor_frame_set_maximized_;
        PFN_libdecor_frame_unset_maximized libdecor_frame_unset_maximized_;
        PFN_libdecor_frame_set_capabilities libdecor_frame_set_capabilities_;
        PFN_libdecor_frame_unset_capabilities libdecor_frame_unset_capabilities_;
        PFN_libdecor_frame_set_visibility libdecor_frame_set_visibility_;
        PFN_libdecor_frame_get_xdg_toplevel libdecor_frame_get_xdg_toplevel_;
        PFN_libdecor_configuration_get_content_size libdecor_configuration_get_content_size_;
        PFN_libdecor_configuration_get_window_state libdecor_configuration_get_window_state_;
        PFN_libdecor_state_new libdecor_state_new_;
        PFN_libdecor_state_free libdecor_state_free_;
    } libdecor;
} _GRWLlibraryWayland;

// Wayland-specific per-monitor data
//
typedef struct _GRWLmonitorWayland
{
    struct wl_output* output;
    uint32_t name;
    int currentMode;

    int x;
    int y;
    int contentScale;
} _GRWLmonitorWayland;

// Wayland-specific per-cursor data
//
typedef struct _GRWLcursorWayland
{
    struct wl_cursor* cursor;
    struct wl_cursor* cursorHiDPI;
    struct wl_buffer* buffer;
    int width, height;
    int xhot, yhot;
    int currentImage;
} _GRWLcursorWayland;

bool _grwlConnectWayland(int platformID, _GRWLplatform* platform);
int _grwlInitWayland();
void _grwlTerminateWayland();

bool _grwlCreateWindowWayland(_GRWLwindow* window, const _GRWLwndconfig* wndconfig, const _GRWLctxconfig* ctxconfig,
                              const _GRWLfbconfig* fbconfig);
void _grwlDestroyWindowWayland(_GRWLwindow* window);
void _grwlSetWindowTitleWayland(_GRWLwindow* window, const char* title);
void _grwlSetWindowIconWayland(_GRWLwindow* window, int count, const GRWLimage* images);
void _grwlSetWindowProgressIndicatorWayland(_GRWLwindow* window, int progressState, double value);
void _grwlSetWindowBadgeWayland(_GRWLwindow* window, int count);
void _grwlSetWindowBadgeStringWayland(_GRWLwindow* window, const char* string);
void _grwlGetWindowPosWayland(_GRWLwindow* window, int* xpos, int* ypos);
void _grwlSetWindowPosWayland(_GRWLwindow* window, int xpos, int ypos);
void _grwlGetWindowSizeWayland(_GRWLwindow* window, int* width, int* height);
void _grwlSetWindowSizeWayland(_GRWLwindow* window, int width, int height);
void _grwlSetWindowSizeLimitsWayland(_GRWLwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _grwlSetWindowAspectRatioWayland(_GRWLwindow* window, int numer, int denom);
void _grwlGetFramebufferSizeWayland(_GRWLwindow* window, int* width, int* height);
void _grwlGetWindowFrameSizeWayland(_GRWLwindow* window, int* left, int* top, int* right, int* bottom);
void _grwlGetWindowContentScaleWayland(_GRWLwindow* window, float* xscale, float* yscale);
void _grwlIconifyWindowWayland(_GRWLwindow* window);
void _grwlRestoreWindowWayland(_GRWLwindow* window);
void _grwlMaximizeWindowWayland(_GRWLwindow* window);
void _grwlShowWindowWayland(_GRWLwindow* window);
void _grwlHideWindowWayland(_GRWLwindow* window);
void _grwlRequestWindowAttentionWayland(_GRWLwindow* window);
void _grwlFocusWindowWayland(_GRWLwindow* window);
void _grwlSetWindowMonitorWayland(_GRWLwindow* window, _GRWLmonitor* monitor, int xpos, int ypos, int width, int height,
                                  int refreshRate);
bool _grwlWindowFocusedWayland(_GRWLwindow* window);
bool _grwlWindowIconifiedWayland(_GRWLwindow* window);
bool _grwlWindowVisibleWayland(_GRWLwindow* window);
bool _grwlWindowMaximizedWayland(_GRWLwindow* window);
bool _grwlWindowHoveredWayland(_GRWLwindow* window);
bool _grwlFramebufferTransparentWayland(_GRWLwindow* window);
void _grwlSetWindowResizableWayland(_GRWLwindow* window, bool enabled);
void _grwlSetWindowDecoratedWayland(_GRWLwindow* window, bool enabled);
void _grwlSetWindowFloatingWayland(_GRWLwindow* window, bool enabled);
float _grwlGetWindowOpacityWayland(_GRWLwindow* window);
void _grwlSetWindowOpacityWayland(_GRWLwindow* window, float opacity);
void _grwlSetWindowMousePassthroughWayland(_GRWLwindow* window, bool enabled);

void _grwlSetRawMouseMotionWayland(_GRWLwindow* window, bool enabled);
bool _grwlRawMouseMotionSupportedWayland();

void _grwlPollEventsWayland();
void _grwlWaitEventsWayland();
void _grwlWaitEventsTimeoutWayland(double timeout);
void _grwlPostEmptyEventWayland();

void _grwlGetCursorPosWayland(_GRWLwindow* window, double* xpos, double* ypos);
void _grwlSetCursorPosWayland(_GRWLwindow* window, double xpos, double ypos);
void _grwlSetCursorModeWayland(_GRWLwindow* window, int mode);
const char* _grwlGetScancodeNameWayland(int scancode);
int _grwlGetKeyScancodeWayland(int key);
const char* _grwlGetKeyboardLayoutNameWayland();
bool _grwlCreateCursorWayland(_GRWLcursor* cursor, const GRWLimage* image, int xhot, int yhot);
bool _grwlCreateStandardCursorWayland(_GRWLcursor* cursor, int shape);
void _grwlDestroyCursorWayland(_GRWLcursor* cursor);
void _grwlSetCursorWayland(_GRWLwindow* window, _GRWLcursor* cursor);
void _grwlSetClipboardStringWayland(const char* string);
const char* _grwlGetClipboardStringWayland();

void _grwlUpdatePreeditCursorRectangleWayland(_GRWLwindow* window);
void _grwlResetPreeditTextWayland(_GRWLwindow* window);
void _grwlSetIMEStatusWayland(_GRWLwindow* window, int active);
int _grwlGetIMEStatusWayland(_GRWLwindow* window);

EGLenum _grwlGetEGLPlatformWayland(EGLint** attribs);
EGLNativeDisplayType _grwlGetEGLNativeDisplayWayland();
EGLNativeWindowType _grwlGetEGLNativeWindowWayland(_GRWLwindow* window);

void _grwlGetRequiredInstanceExtensionsWayland(char** extensions);
bool _grwlGetPhysicalDevicePresentationSupportWayland(VkInstance instance, VkPhysicalDevice device,
                                                      uint32_t queuefamily);
VkResult _grwlCreateWindowSurfaceWayland(VkInstance instance, _GRWLwindow* window,
                                         const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _grwlFreeMonitorWayland(_GRWLmonitor* monitor);
void _grwlGetMonitorPosWayland(_GRWLmonitor* monitor, int* xpos, int* ypos);
void _grwlGetMonitorContentScaleWayland(_GRWLmonitor* monitor, float* xscale, float* yscale);
void _grwlGetMonitorWorkareaWayland(_GRWLmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GRWLvidmode* _grwlGetVideoModesWayland(_GRWLmonitor* monitor, int* count);
void _grwlGetVideoModeWayland(_GRWLmonitor* monitor, GRWLvidmode* mode);
bool _grwlGetGammaRampWayland(_GRWLmonitor* monitor, GRWLgammaramp* ramp);
void _grwlSetGammaRampWayland(_GRWLmonitor* monitor, const GRWLgammaramp* ramp);

void _grwlAddOutputWayland(uint32_t name, uint32_t version);
void _grwlUpdateContentScaleWayland(_GRWLwindow* window);

_GRWLusercontext* _grwlCreateUserContextWayland(_GRWLwindow* window);

void _grwlAddSeatListenerWayland(struct wl_seat* seat);
void _grwlAddDataDeviceListenerWayland(struct wl_data_device* device);
#endif
