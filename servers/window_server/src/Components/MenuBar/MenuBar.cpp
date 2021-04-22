/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MenuBar.h"
#include "../../Compositor.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static MenuBar* s_the;

MenuBar& MenuBar::the()
{
    return *s_the;
}

MenuBar::MenuBar()
    : m_background_color(LG::Color(222, 222, 222, 180))
    , m_bounds(0, 0, width(), height())
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_logo = loader.load_from_file("/res/system/logo_dark_12.png");
}

MenuItemAnswer MenuBar::widget_recieve_mouse_status_change(const CursorManager& cursor_manager, size_t wind)
{
    if (wind >= m_widgets.size()) {
        return MenuItemAnswer::Bad;
    }

    MenuItemAnswer answer = MenuItemAnswer::Empty;
    size_t widget_min_x = start_of_widget(wind);
    if (cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
        answer = m_widgets[wind]->click_began(cursor_manager.x() - widget_min_x, cursor_manager.y());
    } else {
        answer = m_widgets[wind]->click_ended();
    }

    if (answer & MenuItemAnswer::Bad) {
        return answer;
    }
    if (answer & MenuItemAnswer::InvalidateMe) {
        Compositor::the().invalidate(LG::Rect(widget_min_x, 0, m_widgets[wind]->width(), height()));
    }

    return answer;
}

MenuItemAnswer MenuBar::panel_item_recieve_mouse_status_change(const CursorManager& cursor_manager, size_t ind)
{
    if (!m_menubar_content) {
        return MenuItemAnswer::Bad;
    }

    auto& content = *m_menubar_content;
    if (ind >= content.size()) {
        return MenuItemAnswer::Bad;
    }

    MenuItemAnswer answer = MenuItemAnswer::Empty;
    size_t widget_min_x = start_of_menubar_panel_item(ind);
    if (cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
        answer = content[ind].click_began(cursor_manager.x() - widget_min_x, cursor_manager.y());
    } else {
        answer = content[ind].click_ended();
    }

    if (answer & MenuItemAnswer::Bad) {
        return answer;
    }
    if (answer & MenuItemAnswer::InvalidateMe) {
        Compositor::the().invalidate(LG::Rect(widget_min_x, 0, m_widgets[ind]->width(), height()));
    }
    if (answer & MenuItemAnswer::PopupShow) {
        content[ind].popup_rect(m_popup_bounds);
        popup_will_be_shown();
    }
    if (answer & MenuItemAnswer::PopupClose) {
        popup_will_be_closed();
    }

    return answer;
}

void MenuBar::invalidate_menubar_panel(Compositor& compositor)
{
    if (menubar_content()) {
        size_t inv_len = menubar_panel_width(*m_menubar_content);
        compositor.invalidate(LG::Rect(menubar_content_offset(), 0, inv_len, height()));
    }
}

void MenuBar::invalidate_menubar_panel()
{
    invalidate_menubar_panel(Compositor::the());
}

} // namespace WinServer