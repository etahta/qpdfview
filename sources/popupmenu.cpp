#include "sources/popupmenu.h"

#include <QToolButton>

PopupMenu::PopupMenu(QToolButton *button, PopupMenu *parent) : QMenu(parent), b(button)
{
}

void PopupMenu::showEvent(QShowEvent* event)
{
    QPoint p = this->pos();
    QRect geo = b->geometry();
    this->move(p.x()+geo.width()-this->geometry().width(), p.y());
}
