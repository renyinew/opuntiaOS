/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../shared/Connections/WSConnection.h"
#include "ServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <vector>

namespace WinServer {

class CursorManager;
class ResourceManager;
class MenuBar;

class Compositor {
public:
    static Compositor& the();
    Compositor();

    void refresh();

    void optimized_invalidate_insert(std::vector<LG::Rect>& data, const LG::Rect& area)
    {
        LG::Rect area_union;
        size_t area_square = area.square();
        size_t inv_area_square = 0;

        for (int i = 0; i < data.size(); i++) {
            inv_area_square = data[i].square();
            area_union = area.union_of(data[i]);
            if (area_square + inv_area_square > area_union.square()) {
                data[i].unite(area);
                return;
            }
        }

        data.push_back(area);
    }

    inline void invalidate(const LG::Rect& area) { optimized_invalidate_insert(m_invalidated_areas, area); }
    inline CursorManager& cursor_manager() { return m_cursor_manager; }
    inline const CursorManager& cursor_manager() const { return m_cursor_manager; }
    inline ResourceManager& resource_manager() { return m_resource_manager; }
    inline const ResourceManager& resource_manager() const { return m_resource_manager; }
    inline MenuBar& menu_bar() { return m_menu_bar; }
    inline const MenuBar& menu_bar() const { return m_menu_bar; }

private:
    void copy_changes_to_second_buffer(const std::vector<LG::Rect>& areas);

    MenuBar& m_menu_bar;
    CursorManager& m_cursor_manager;
    ResourceManager& m_resource_manager;
    std::vector<LG::Rect> m_invalidated_areas;
};

} // namespace WinServer