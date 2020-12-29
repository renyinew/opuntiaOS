#include "DockView.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/Context.h>
#include <std/Dbg.h>

static DockView* this_view;

DockView::DockView(const LG::Rect& frame)
    : View(frame)
{
}

void DockView::display(const LG::Rect& rect)
{
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill(bounds());

    int offsetx = 2;
    for (auto& entity : m_dock_entites) {
        ctx.draw({ offsetx, 2 }, entity.icon());
        offsetx += entity.icon().bounds().width() + 8;
    }
}

DockEntity* DockView::find_entity(int window_id)
{
    for (auto& ent : m_dock_entites) {
        if (ent.window_id() == window_id) {
            return &ent;
        }
    }
    return nullptr;
}

void DockView::new_entity(int window_id)
{
    // Don't add an icon of dock (self).
    if (window()->id() != window_id) {
        auto* entity = new DockEntity(window_id);
        m_dock_entites.push_back(entity);
    }
}

void DockView::remove_entity(int window_id)
{
    auto* ent = find_entity(window_id);
    if (!ent) {
        return;
    }
    m_dock_entites.remove(ent);
    delete ent;
    set_needs_display();
}

void DockView::set_icon(int window_id, const LG::String& path)
{
    auto* ent = find_entity(window_id);
    if (!ent) {
        return;
    }
    LG::PNG::PNGLoader loader;
    ent->set_icon(loader.load_from_file(path + "/32x32.png"));
    set_needs_display();
}