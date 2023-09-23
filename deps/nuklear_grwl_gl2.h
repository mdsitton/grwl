/*
 * Nuklear - v1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_GRWL_GL2_H_
#define NK_GRWL_GL2_H_

#include <GRWL/grwl.h>

enum nk_grwl_init_state
{
    NK_GRWL_DEFAULT = 0,
    NK_GRWL_INSTALL_CALLBACKS
};
NK_API struct nk_context* nk_grwl_init(GRWLwindow* win, enum nk_grwl_init_state);
NK_API void nk_grwl_font_stash_begin(struct nk_font_atlas** atlas);
NK_API void nk_grwl_font_stash_end(void);

NK_API void nk_grwl_new_frame(void);
NK_API void nk_grwl_render(enum nk_anti_aliasing);
NK_API void nk_grwl_shutdown(void);

NK_API void nk_grwl_char_callback(GRWLwindow* win, unsigned int codepoint);
NK_API void nk_gflw3_scroll_callback(GRWLwindow* win, double xoff, double yoff);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_GRWL_GL2_IMPLEMENTATION

#ifndef NK_GRWL_TEXT_MAX
    #define NK_GRWL_TEXT_MAX 256
#endif
#ifndef NK_GRWL_DOUBLE_CLICK_LO
    #define NK_GRWL_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GRWL_DOUBLE_CLICK_HI
    #define NK_GRWL_DOUBLE_CLICK_HI 0.2
#endif

struct nk_grwl_device
{
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint font_tex;
};

struct nk_grwl_vertex
{
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_grwl
{
    GRWLwindow* win;
    int width, height;
    int display_width, display_height;
    struct nk_grwl_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GRWL_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} grwl;

NK_INTERN void nk_grwl_device_upload_atlas(const void* image, int width, int height)
{
    struct nk_grwl_device* dev = &grwl.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void nk_grwl_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_grwl_device* dev = &grwl.ogl;
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0, 0, (GLsizei)grwl.display_width, (GLsizei)grwl.display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, grwl.width, grwl.height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_grwl_vertex);
        size_t vp = offsetof(struct nk_grwl_vertex, position);
        size_t vt = offsetof(struct nk_grwl_vertex, uv);
        size_t vc = offsetof(struct nk_grwl_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command* cmd;
        const nk_draw_index* offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_grwl_vertex, position) },
            { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_grwl_vertex, uv) },
            { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_grwl_vertex, col) },
            { NK_VERTEX_LAYOUT_END }
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_grwl_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_grwl_vertex);
        config.null = dev->null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&grwl.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {
            const void* vertices = nk_buffer_memory_const(&vbuf);
            glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
            glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
            glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));
        }

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &grwl.ctx, &dev->cmds)
        {
            if (!cmd->elem_count)
            {
                continue;
            }
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor((GLint)(cmd->clip_rect.x * grwl.fb_scale.x),
                      (GLint)((grwl.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * grwl.fb_scale.y),
                      (GLint)(cmd->clip_rect.w * grwl.fb_scale.x), (GLint)(cmd->clip_rect.h * grwl.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&grwl.ctx);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

NK_API void nk_grwl_char_callback(GRWLwindow* win, unsigned int codepoint)
{
    (void)win;
    if (grwl.text_len < NK_GRWL_TEXT_MAX)
    {
        grwl.text[grwl.text_len++] = codepoint;
    }
}

NK_API void nk_gflw3_scroll_callback(GRWLwindow* win, double xoff, double yoff)
{
    (void)win;
    (void)xoff;
    grwl.scroll.x += (float)xoff;
    grwl.scroll.y += (float)yoff;
}

NK_API void nk_grwl_mouse_button_callback(GRWLwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != GRWL_MOUSE_BUTTON_LEFT)
    {
        return;
    }
    grwlGetCursorPos(window, &x, &y);
    if (action == GRWL_PRESS)
    {
        double dt = grwlGetTime() - grwl.last_button_click;
        if (dt > NK_GRWL_DOUBLE_CLICK_LO && dt < NK_GRWL_DOUBLE_CLICK_HI)
        {
            grwl.is_double_click_down = nk_true;
            grwl.double_click_pos = nk_vec2((float)x, (float)y);
        }
        grwl.last_button_click = grwlGetTime();
    }
    else
    {
        grwl.is_double_click_down = nk_false;
    }
}

NK_INTERN void nk_grwl_clipboard_paste(nk_handle usr, struct nk_text_edit* edit)
{
    const char* text = grwlGetClipboardString(grwl.win);
    if (text)
    {
        nk_textedit_paste(edit, text, nk_strlen(text));
    }
    (void)usr;
}

NK_INTERN void nk_grwl_clipboard_copy(nk_handle usr, const char* text, int len)
{
    char* str = 0;
    (void)usr;
    if (!len)
    {
        return;
    }
    str = (char*)malloc((size_t)len + 1);
    if (!str)
    {
        return;
    }
    NK_MEMCPY(str, text, (size_t)len);
    str[len] = '\0';
    grwlSetClipboardString(grwl.win, str);
    free(str);
}

NK_API struct nk_context* nk_grwl_init(GRWLwindow* win, enum nk_grwl_init_state init_state)
{
    grwl.win = win;
    if (init_state == NK_GRWL_INSTALL_CALLBACKS)
    {
        grwlSetScrollCallback(win, nk_gflw3_scroll_callback);
        grwlSetCharCallback(win, nk_grwl_char_callback);
        grwlSetMouseButtonCallback(win, nk_grwl_mouse_button_callback);
    }
    nk_init_default(&grwl.ctx, 0);
    grwl.ctx.clip.copy = nk_grwl_clipboard_copy;
    grwl.ctx.clip.paste = nk_grwl_clipboard_paste;
    grwl.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&grwl.ogl.cmds);

    grwl.is_double_click_down = nk_false;
    grwl.double_click_pos = nk_vec2(0, 0);

    return &grwl.ctx;
}

NK_API void nk_grwl_font_stash_begin(struct nk_font_atlas** atlas)
{
    nk_font_atlas_init_default(&grwl.atlas);
    nk_font_atlas_begin(&grwl.atlas);
    *atlas = &grwl.atlas;
}

NK_API void nk_grwl_font_stash_end(void)
{
    const void* image;
    int w, h;
    image = nk_font_atlas_bake(&grwl.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_grwl_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&grwl.atlas, nk_handle_id((int)grwl.ogl.font_tex), &grwl.ogl.null);
    if (grwl.atlas.default_font)
    {
        nk_style_set_font(&grwl.ctx, &grwl.atlas.default_font->handle);
    }
}

NK_API void nk_grwl_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context* ctx = &grwl.ctx;
    struct GRWLwindow* win = grwl.win;

    grwlGetWindowSize(win, &grwl.width, &grwl.height);
    grwlGetFramebufferSize(win, &grwl.display_width, &grwl.display_height);
    grwl.fb_scale.x = (float)grwl.display_width / (float)grwl.width;
    grwl.fb_scale.y = (float)grwl.display_height / (float)grwl.height;

    nk_input_begin(ctx);
    for (i = 0; i < grwl.text_len; ++i)
    {
        nk_input_unicode(ctx, grwl.text[i]);
    }

    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
    {
        grwlSetInputMode(grwl.win, GRWL_CURSOR, GRWL_CURSOR_HIDDEN);
    }
    else if (ctx->input.mouse.ungrab)
    {
        grwlSetInputMode(grwl.win, GRWL_CURSOR, GRWL_CURSOR_NORMAL);
    }

    nk_input_key(ctx, NK_KEY_DEL, grwlGetKey(win, GRWL_KEY_DELETE) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, grwlGetKey(win, GRWL_KEY_ENTER) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, grwlGetKey(win, GRWL_KEY_TAB) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, grwlGetKey(win, GRWL_KEY_BACKSPACE) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_UP, grwlGetKey(win, GRWL_KEY_UP) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, grwlGetKey(win, GRWL_KEY_DOWN) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, grwlGetKey(win, GRWL_KEY_HOME) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, grwlGetKey(win, GRWL_KEY_END) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, grwlGetKey(win, GRWL_KEY_HOME) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, grwlGetKey(win, GRWL_KEY_END) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, grwlGetKey(win, GRWL_KEY_PAGE_DOWN) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, grwlGetKey(win, GRWL_KEY_PAGE_UP) == GRWL_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT,
                 grwlGetKey(win, GRWL_KEY_LEFT_SHIFT) == GRWL_PRESS ||
                     grwlGetKey(win, GRWL_KEY_RIGHT_SHIFT) == GRWL_PRESS);

    if (grwlGetKey(win, GRWL_KEY_LEFT_CONTROL) == GRWL_PRESS || grwlGetKey(win, GRWL_KEY_RIGHT_CONTROL) == GRWL_PRESS)
    {
        nk_input_key(ctx, NK_KEY_COPY, grwlGetKey(win, GRWL_KEY_C) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, grwlGetKey(win, GRWL_KEY_V) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, grwlGetKey(win, GRWL_KEY_X) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, grwlGetKey(win, GRWL_KEY_Z) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, grwlGetKey(win, GRWL_KEY_R) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, grwlGetKey(win, GRWL_KEY_LEFT) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, grwlGetKey(win, GRWL_KEY_RIGHT) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, grwlGetKey(win, GRWL_KEY_B) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, grwlGetKey(win, GRWL_KEY_E) == GRWL_PRESS);
    }
    else
    {
        nk_input_key(ctx, NK_KEY_LEFT, grwlGetKey(win, GRWL_KEY_LEFT) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, grwlGetKey(win, GRWL_KEY_RIGHT) == GRWL_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    grwlGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    if (ctx->input.mouse.grabbed)
    {
        grwlSetCursorPos(grwl.win, (double)ctx->input.mouse.prev.x, (double)ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, grwlGetMouseButton(win, GRWL_MOUSE_BUTTON_LEFT) == GRWL_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y,
                    grwlGetMouseButton(win, GRWL_MOUSE_BUTTON_MIDDLE) == GRWL_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y,
                    grwlGetMouseButton(win, GRWL_MOUSE_BUTTON_RIGHT) == GRWL_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)grwl.double_click_pos.x, (int)grwl.double_click_pos.y,
                    grwl.is_double_click_down);
    nk_input_scroll(ctx, grwl.scroll);
    nk_input_end(&grwl.ctx);
    grwl.text_len = 0;
    grwl.scroll = nk_vec2(0, 0);
}

NK_API
void nk_grwl_shutdown(void)
{
    struct nk_grwl_device* dev = &grwl.ogl;
    nk_font_atlas_clear(&grwl.atlas);
    nk_free(&grwl.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    NK_MEMSET(&grwl, 0, sizeof(grwl));
}

#endif
