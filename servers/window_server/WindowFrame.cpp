/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowFrame.h"
#include "Button.h"
#include "Window.h"
#include "WindowManager.h"
#include <libg/Font.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libg/Rect.h>
#include <std/Utility.h>
#include <syscalls.h>

#define CONTROL_PANEL_CLOSE 0x0
#define CONTROL_PANEL_MAXIMIZE 0x1
#define CONTROL_PANEL_MINIMIZE 0x2

static const uint32_t s_close_button_glyph_data[10] = {
    0b1100000011,
    0b1110000111,
    0b0111001110,
    0b0011101100,
    0b0001110000,
    0b0000111000,
    0b0011011100,
    0b0111001110,
    0b1110000111,
    0b1100000011,
};

static const uint32_t s_maximise_button_glyph_data[10] = {
    0b1111111111,
    0b1111111111,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1111111111,
    0b1111111111
};

static const uint32_t s_minimise_button_glyph_data[10] = {
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b1111111111,
    0b1111111111
};

WindowFrame::WindowFrame(Window& window)
    : m_window(window)
    , m_window_control_buttons()
    , m_control_panel_buttons()
{
    auto* close = new Button();
    close->set_icon(LG::GlyphBitmap(s_close_button_glyph_data, 10, 10));
    auto* maximize = new Button();
    maximize->set_icon(LG::GlyphBitmap(s_maximise_button_glyph_data, 10, 10));
    auto* minimize = new Button();
    minimize->set_icon(LG::GlyphBitmap(s_minimise_button_glyph_data, 10, 10));

    m_window_control_buttons.push_back(close);
    m_window_control_buttons.push_back(maximize);
    m_window_control_buttons.push_back(minimize);
}

WindowFrame::WindowFrame(Window& window, Vector<Button*>&& control_panel_buttons, Vector<Button*>&& window_control_buttons)
    : m_window(window)
    , m_window_control_buttons(move(window_control_buttons))
    , m_control_panel_buttons(move(control_panel_buttons))
{
}

void WindowFrame::set_app_name(const String& title)
{
    if (m_control_panel_buttons.size() > 0) {
        m_control_panel_buttons[0]->set_title(title);
    } else {
        auto* new_control = new Button();
        new_control->set_title(title);
        m_control_panel_buttons.push_back(new_control);
    }
}

void WindowFrame::set_app_name(String&& title)
{
    if (m_control_panel_buttons.size() > 0) {
        m_control_panel_buttons[0]->set_title(title);
    } else {
        auto* new_control = new Button();
        new_control->set_title(title);
        m_control_panel_buttons.push_back(new_control);
    }
    Compositor::the().invalidate(bounds());
}

void WindowFrame::add_control(const String& title)
{
    auto* new_control = new Button();
    new_control->set_title(title);
    m_control_panel_buttons.push_back(new_control);
    Compositor::the().invalidate(bounds());
}

void WindowFrame::draw(LG::Context& ctx)
{
    if (!visible()) {
        return;
    }
    int x = m_window.bounds().min_x();
    int y = m_window.bounds().min_y();
    size_t width = m_window.bounds().width();
    size_t height = m_window.bounds().height();

    int right_x = x + width - right_border_size();
    int bottom_y = y + height - bottom_border_size();

    // Drawing frame
    ctx.set_fill_color(color());
    ctx.fill(LG::Rect(x, y, width, top_border_size()));
    ctx.fill(LG::Rect(x, y, left_border_size(), height));
    ctx.fill(LG::Rect(right_x, y, right_border_size(), height));
    ctx.fill(LG::Rect(x, bottom_y, width, bottom_border_size()));

    // Drawing labels, icons.
    // Drawing positions are calculated using a start of the frame.
    ctx.set_fill_color(LG::Color::White);
    ctx.draw({ x + spacing(), y + icon_y_offset() }, icon());

    constexpr int start_controls_offset = icon_width() + 2 * spacing();
    int start_controls = x + start_controls_offset;
    for (int i = 0; i < m_control_panel_buttons.size(); i++) {
        m_control_panel_buttons[i]->display(ctx, { start_controls, y + text_y_offset() });
        start_controls += spacing() + m_control_panel_buttons[i]->bounds().width();
    }

    int start_buttons = right_x - spacing() - m_window_control_buttons[0]->bounds().width();
    for (int i = 0; i < m_window_control_buttons.size(); i++) {
        m_window_control_buttons[i]->display(ctx, { start_buttons, y + button_y_offset() });
        start_buttons += -spacing() - m_window_control_buttons[i]->bounds().width();
    }
}

void WindowFrame::receive_tap_event(const LG::Point<int>& tap)
{
    // Calculating buttons' positions
    size_t width = m_window.bounds().width();
    int right_x = width - right_border_size();
    int start_buttons = right_x - spacing() - m_window_control_buttons[0]->bounds().width();
    for (int button_id = 0; button_id < m_window_control_buttons.size(); button_id++) {
        auto button_frame = m_window_control_buttons[button_id]->bounds();
        button_frame.offset_by(start_buttons, button_y_offset());
        if (button_frame.contains(tap)) {
            handle_control_panel_tap(button_id);
            return;
        }
        start_buttons += -spacing() - button_frame.width();
    }
}

const LG::Rect WindowFrame::bounds() const
{
    const auto& bounds = m_window.bounds();
    return LG::Rect(bounds.min_x(), bounds.min_y(), bounds.width(), top_border_size());
}

void WindowFrame::handle_control_panel_tap(int button_id)
{
    auto& wm = WindowManager::the();
    switch (button_id) {
    case CONTROL_PANEL_CLOSE:
        wm.close_window(m_window);
        break;
    default:
        break;
    }
}

void WindowFrame::reload_icon()
{
    LG::PNG::PNGLoader loader;
    m_icon = loader.load_from_file(m_window.icon_path() + "/12x12.png");
}