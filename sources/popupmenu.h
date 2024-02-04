#ifndef POPUPMENU_H
#define POPUPMENU_H
#include <QMenu>

class QToolButton;
class QWidget;

class PopupMenu : public QMenu
{
    Q_OBJECT
public:
    explicit PopupMenu(QToolButton* button, PopupMenu* parent = 0);
    void showEvent(QShowEvent* event);
private:
    QToolButton* b;
};
#endif // POPUPMENU_H
