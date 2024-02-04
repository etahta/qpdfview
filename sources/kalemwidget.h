#ifndef KALEMWIDGET_H
#define KALEMWIDGET_H

#include <QWidget>
#include<QLabel>
#include<QLineEdit>
#include<sources/scene.h>
#include<sources/diagramitem.h>
#include<QMenu>
#include<QPushButton>
#include<QPalette>
#include<QWidgetAction>
#include<QFileDialog>
#include<QToolButton>
#include<sources/popupmenu.h>
#include<QToolBar>
#include<sources/scene.h>

class QMouseEvent;

namespace qpdfview
{

class KalemWidget : public QToolBar
{
    Q_OBJECT
public:
    Scene::Mode oldMode;
    Scene::Mode currentMode;
    DiagramItem::DiagramType oldType;
    DiagramItem::DiagramType currentType;
    QString panelSide="Right";

    QPoint offset;
    bool mouseClick;
    explicit KalemWidget(int _en, int _boy, QWidget *parent = nullptr);
    int en;
    int boy;
    int penSize=4;
    int gridSize=15;
    int penAlpha=50;
    QColor penColor=QColor(0,0,0,255);
    Qt::PenStyle penStyle=Qt::SolidLine;
    QLabel *moveLabel;
    QColor sekilZeminColor=QColor(0,0,0,0);
    QColor zeminColor=QColor(0,0,0,0);
    QColor zeminGridColor=QColor(0,0,0,255);
    DiagramItem::DiagramType pagePattern=DiagramItem::DiagramType::TransparanPage;
    QToolButton *penColorButton;
    QLineEdit *pageOfNumber;
    QPixmap zeminImage(const QPolygonF &myPolygon, int w, int h, QColor color, int pensize) const;
    QToolButton *handButton;
    QToolButton *copyButton;
    QToolButton *penButton;
    QToolButton *eraseButton;
    QToolButton *sekilButton;
    QToolButton *zeminButton;
    QToolButton *undoButton;
    QToolButton *redoButton;


signals:
    void kalemColorSignal(QString colorType,QColor color);
    void kalemModeSignal(Scene::Mode mode,DiagramItem::DiagramType type);

    void kalemZeminModeSignal(DiagramItem::DiagramType type);
    void kalemPenModeSignal(DiagramItem::DiagramType type);
    void kalemSekilModeSignal(DiagramItem::DiagramType type);

public slots:
    void handButtonSlot();
    void penButtonSlot();
    void clearButtonSlot();
    void modeKontrolSlot();

   void buttonStateClear();
   void setGridSize(int s);
    QMenu *eraseMenu();
    QWidget *cizgiBoyutMenu();
    void sekilButtonIconSlot(DiagramItem::DiagramType mySekilType);
    QPixmap lineImage(const QPolygonF &myPolygon,const Qt::PenStyle &stl,int w,int h) const;
    QPixmap imageEllipse(const QPolygonF &myPolygon, int w, int h) const;
    QPixmap image(const QPolygonF &myPolygon, int w, int h) const;
    QMenu *sekilMenu();
    QMenu *colorMenu(QString colorType, QString yon, int w, int h, bool close);
    QWidget *colorWidget(QString colorType, QString yon, int w, int h, bool close);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject * obj, QEvent *event);
    QPushButton *butonSlot(QPushButton *btn, QString text, QString icon, QColor color, int w, int h, int iw, int ih);

    QToolButton *butonToolSlot(QToolButton *btn, QString text, QString icon, QColor color, int w, int h);

private:
    QMenu *penMenu();

    QMenu *zeminMenu();
    QMenu *pageMenu();

    QPalette *palette;
    QLabel *eraseSizePopLabel;
    QLabel *sekilKalemSizePopLabel;
    QLabel *kalemSizePopLabel;
    QLabel *gridSizePopLabel;



};
}
#endif // KALEMWIDGET_H
