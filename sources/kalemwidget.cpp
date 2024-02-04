#include "kalemwidget.h"
#include<QDebug>
#include<QMouseEvent>
#include<QToolButton>
#include<QAction>
#include<QMenu>
#include<QGridLayout>
namespace qpdfview
{
bool KalemWidget::eventFilter(QObject * obj, QEvent *event)
{
   /* if (event->type() == QEvent::Show && obj == sekilButton->menu())
    {
         int button_x = mapToParent(sekilButton->pos()).x();
        int button_y = mapToParent(sekilButton->pos()).y();

        int menu_width = sekilButton->menu()->size().width();
        int menu_height = sekilButton->menu()->size().height();

        QPoint pos = QPoint(button_x - menu_width-1 ,
                            button_y-menu_height/2);
        sekilButton->menu()->move(pos);
        return true;
    }
    if (event->type() == QEvent::Show && obj == penButton->menu())
    {
         int button_x = mapToParent(penButton->pos()).x();
        int button_y = mapToParent(penButton->pos()).y();

        int menu_width = penButton->menu()->size().width();
        int menu_height = penButton->menu()->size().height();

        QPoint pos = QPoint(button_x - menu_width-1 ,
                            button_y);
        penButton->menu()->move(pos);
        return true;
    }
    if (event->type() == QEvent::Show && obj == zeminButton->menu())
    {
         int button_x = mapToParent(zeminButton->pos()).x();
        int button_y = mapToParent(zeminButton->pos()).y();

        int menu_width = zeminButton->menu()->size().width();
        int menu_height = zeminButton->menu()->size().height();

        QPoint pos = QPoint(button_x - menu_width-1 ,
                            button_y-menu_height/2);
        zeminButton->menu()->move(pos);
        return true;
    }
  /*  if (event->type() == QEvent::Show && obj == colorButton->menu())
    {
         int button_x = mapToParent(colorButton->pos()).x();
        int button_y = mapToParent(colorButton->pos()).y();

        int menu_width = colorButton->menu()->size().width();
        int menu_height = colorButton->menu()->size().height();

        QPoint pos = QPoint(button_x - menu_width-1 ,
                            button_y-menu_height/2);
        colorButton->menu()->move(pos);
        return true;
    }*/
    return false;
}
void KalemWidget::buttonStateClear()
{
    handButton->setChecked(false);
    copyButton->setChecked(false);
    penButton->setChecked(false);
    eraseButton->setChecked(false);
    sekilButton->setChecked(false);

}
void KalemWidget::handButtonSlot()
{
    buttonStateClear();handButton->setChecked(true);
    emit kalemModeSignal(Scene::Mode::SelectObject,DiagramItem::NoType);

}
void KalemWidget::penButtonSlot()
{
    buttonStateClear();penButton->setChecked(true);
    emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::NormalPen);

}
void KalemWidget::clearButtonSlot()
{
    emit kalemModeSignal(Scene::Mode::ClearMode,DiagramItem::DiagramType::NoType);
}
void KalemWidget::modeKontrolSlot()
{

//qDebug()<<"oldMode:"<<oldMode<<"currentMode"<<currentMode;

    if(oldMode==Scene::Mode::DrawPen||
            oldMode==Scene::Mode::SelectObject||
            oldMode==Scene::Mode::EraseMode||
            oldMode==Scene::Mode::SekilMode)
    {

    if(currentMode==Scene::Mode::ClearMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::NextPageMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::BackPageMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::ZoomPozitifMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::ZoomNegatifMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::ZoomSelectionMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::FitWindowMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::FitPageMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::ZeminMode)
        emit kalemModeSignal(oldMode,oldType);

    //if(currentMode==Scene::Mode::SekilMode)
     //   emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::IleriAlMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::GeriAlMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::JumpPageMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::SaveMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::PrintMode)
        emit kalemModeSignal(oldMode,oldType);


    if(currentMode==Scene::Mode::ListMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::SearchMode)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::PanelSideLeftRight)
        emit kalemModeSignal(oldMode,oldType);

    if(currentMode==Scene::Mode::PanelSideBottom)
        emit kalemModeSignal(oldMode,oldType);


}
}

KalemWidget::KalemWidget(int _en,int _boy,QWidget *parent) : QToolBar(parent)
{
    en=_en;
    boy=_boy;
mouseClick=false;
currentMode=Scene::Mode::SelectObject;
//int boy=40;
//int en=this->width()/2-24;

this->setObjectName("kalem");
/*this->setStyleSheet("QWidget#kalem{"
                      "border: 1px solid rgb(62, 140, 183);"
                      "border-radius: 5px;"
                      "background-color:rgb(240,240,240,250);"
                      "}");
                      */

/*moveLabel=new QLabel(this);
moveLabel->setStyleSheet("background-color:#3e8db8;");
moveLabel->setFixedSize(en*1.5,boy*0.6);
//QPixmap movepixmap(":icons/icons/move.png");
//moveLabel->setPixmap(movepixmap);
moveLabel->setScaledContents(true);
moveLabel->setText("Kalem");
moveLabel->setStyleSheet("border: 0px solid #3e8db8;"
"border-radius: 3px;"
"background-color: #3e8db8;"
"qproperty-alignment: AlignCenter;"
"qproperty-text: 'Kalem';"
"qproperty-wordWrap: true;");

QFont ff( "Arial", 7, QFont::Normal);
moveLabel->setFont(ff);
*/
/*******************************************************************/

handButton = new QToolButton(this);
/*handButton->setFixedSize(en*1.5,boy);
handButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
handButton->setIcon(QIcon(":icons/icons/hand.png"));
handButton->setIconSize(QSize(en,boy-10));
handButton->setAutoRaise(true);
handButton->setAutoFillBackground(true);
//handButton->setStyleSheet("background-color: #acacac");*/

handButton=butonToolSlot(handButton,"",":icons/icons/hand.png",QColor(255,0,0,0),en*1.5,boy);
handButton->setCheckable(true);
handButton->setChecked(true);
connect(handButton, &QToolButton::clicked, [=]() {
    handButtonSlot();
     });

copyButton = new QToolButton(this);
copyButton=butonToolSlot(copyButton,"",":icons/icons/copy.png",QColor(255,0,0,0),en*1.5,boy);
copyButton->setCheckable(true);
connect(copyButton, &QToolButton::clicked, [=]() {
    buttonStateClear();copyButton->setChecked(true);
    emit kalemModeSignal(Scene::Mode::CopyMode,DiagramItem::DiagramType::NoType);
   });

penButton = new QToolButton(this);
penButton=butonToolSlot(penButton,"Kalem",":icons/icons/pen.png",QColor(255,0,0,0),en*1.5,boy);
penButton->setIconSize(QSize(en*0.8,boy*1.3));
penButton->setMenu(penMenu());
penButton->setPopupMode(QToolButton::MenuButtonPopup);
penButton->setCheckable(true);
connect(penButton, &QToolButton::clicked, [=]() {
penButtonSlot();
});


eraseButton = new QToolButton(this);
eraseButton=butonToolSlot(eraseButton,"Silgi",":icons/icons/erase.png",QColor(255,0,0,0),en*1.5,boy);
eraseButton->setCheckable(true);
eraseButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(eraseButton, &QToolButton::clicked, [=]() {
       buttonStateClear();eraseButton->setChecked(true);
       emit kalemModeSignal(Scene::Mode::EraseMode,DiagramItem::DiagramType::NoType);
   });

QToolButton *clearButton = new QToolButton(this);
clearButton=butonToolSlot(clearButton,"Hepsini Sil",":icons/icons/clear.png",QColor(255,0,0,0),en*1.5,boy);
clearButton->setIconSize(QSize(en*1,boy*1));
connect(clearButton, &QToolButton::clicked, [=]() {
  clearButtonSlot();
});

QToolButton *blackButton = new QToolButton(this);
blackButton=butonToolSlot(blackButton,"","",QColor(0,0,0,255),en*1.5,boy/2);
connect(blackButton, &QToolButton::clicked, [=]() {
    emit kalemColorSignal("penColor",QColor(0,0,0,255));
   });


QToolButton *redButton = new QToolButton(this);
redButton=butonToolSlot(redButton,"","",QColor(255,0,0,255),en*1.5,boy/2);
connect(redButton, &QToolButton::clicked, [=]() {
    emit kalemColorSignal("penColor",QColor(255,0,0,255));
   });

QToolButton *blueButton = new QToolButton(this);
blueButton=butonToolSlot(blueButton,"","",QColor(0,0,255,255),en*1.5,boy/2);
connect(blueButton, &QToolButton::clicked, [=]() {
emit kalemColorSignal("penColor",QColor(0,0,255,255));

});

penColorButton = new QToolButton(this);
penColorButton=butonToolSlot(penColorButton,"",":icons/icons/pencolor.png",QColor(0,0,0,255),en*1.5,boy*0.8);
connect(penColorButton, &QToolButton::clicked, [=]() {
 QMenu *menu= colorMenu("penColor","yatay",en,boy,true);
 menu->show();menu->hide();
 colorMenu("penColor","yatay",en,boy,true)->exec(mapToGlobal(penColorButton->pos())-QPoint(menu->width(),0));

});
QWidget *urw=new QWidget(this);
urw->setFixedSize(en*1.5,boy);
undoButton = new QToolButton(urw);
undoButton=butonToolSlot(undoButton,"",":icons/icons/undo.png",QColor(255,0,0,0),en*0.75,boy);
undoButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(undoButton, &QToolButton::clicked, [=]() {
    emit kalemModeSignal(Scene::Mode::GeriAlMode,DiagramItem::DiagramType::NoType);

});

redoButton = new QToolButton(urw);
redoButton=butonToolSlot(redoButton,"",":icons/icons/redo.png",QColor(255,0,0,0),en*0.75,boy);
redoButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(redoButton, &QToolButton::clicked, [=]() {
    emit kalemModeSignal(Scene::Mode::IleriAlMode,DiagramItem::DiagramType::NoType);

});

QHBoxLayout *line00 = new QHBoxLayout;
line00->addWidget(undoButton,Qt::AlignHCenter);
line00->addWidget(redoButton,Qt::AlignHCenter);

line00->setContentsMargins(0,0, 0,0);
line00->setSpacing(1);
urw->setLayout(line00);

sekilButton = new QToolButton(this);
DiagramItem *ditem=new DiagramItem();int ken=300;
QPixmap pixmap(image(ditem->sekilStore(DiagramItem::DiagramType::Cizgi,QRectF(QPointF(20,50),QPointF(ken-70,ken-70))),ken,ken));
sekilButton=butonToolSlot(sekilButton,"Şekiller",":icons/icons/transparanboard.png",QColor(255,0,0,0),en*1.5,boy);
sekilButton->setIcon(pixmap);
sekilButton->setMenu(sekilMenu());
sekilButton->setPopupMode(QToolButton::MenuButtonPopup);
sekilButton->setCheckable(true);
connect(sekilButton, &QToolButton::clicked, [=]() {
   buttonStateClear(); sekilButton->setChecked(true);
   emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Cizgi);
   });

zeminButton = new QToolButton(this);
zeminButton=butonToolSlot(zeminButton,"Arkaplan",":icons/icons/transparanboard.png",QColor(255,0,0,0),en*1.5,boy);
zeminButton->setMenu(zeminMenu());
zeminButton->setPopupMode(QToolButton::MenuButtonPopup);

connect(zeminButton, &QToolButton::clicked, [=]() {
    emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::TransparanPage);
   });
//QIcon(QLatin1String(":icons/fit-to-page-width")), SLOT(on_fitToPageWidthMode_triggered(bool)), true);
//QIcon(QLatin1String(":icons/fit-to-page-size")),  SLOT(on_fitToPageSizeMode_triggered(bool)), true);
QWidget *wpw=new QWidget(this);
wpw->setFixedSize(en*1.5,boy);

QToolButton *fitWindowButton = new QToolButton(wpw);
fitWindowButton=butonToolSlot(fitWindowButton,"",":icons/icons/fitwindowsize.png",QColor(255,0,0,0),en*0.75,boy*0.8);
fitWindowButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(fitWindowButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::FitWindowMode,DiagramItem::DiagramType::NoType);

});

QToolButton *fitPageButton = new QToolButton(wpw);
fitPageButton=butonToolSlot(fitPageButton,"",":icons/icons/fitpagesize.png",QColor(255,0,0,0),en*0.75,boy*0.8);
fitPageButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(fitPageButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::FitPageMode,DiagramItem::DiagramType::NoType);
});


QHBoxLayout *line01 = new QHBoxLayout;
line01->addWidget(fitWindowButton,Qt::AlignHCenter);
line01->addWidget(fitPageButton,Qt::AlignHCenter);

line01->setContentsMargins(0,0, 0,0);
line01->setSpacing(1);
wpw->setLayout(line01);

QWidget *znpw=new QWidget(this);
znpw->setFixedSize(en*1.5,boy);

QToolButton *zoomPozitifButton = new QToolButton(znpw);
zoomPozitifButton=butonToolSlot(zoomPozitifButton,"",":icons/icons/zoompozitif.png",QColor(255,0,0,0),en*0.75,boy);
zoomPozitifButton->setIconSize(QSize(en*0.75,boy*0.75));
connect(zoomPozitifButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::ZoomPozitifMode,DiagramItem::DiagramType::NoType);
   });

QToolButton *zoomNegatifButton = new QToolButton(znpw);
zoomNegatifButton=butonToolSlot(zoomNegatifButton, "",":icons/icons/zoomnegatif.png",QColor(255,0,0,0),en*0.75,boy);
zoomNegatifButton->setIconSize(QSize(en*0.75,boy*0.75));
connect(zoomNegatifButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::ZoomNegatifMode,DiagramItem::DiagramType::NoType);
   });
QHBoxLayout *line03 = new QHBoxLayout;
line03->addWidget(zoomPozitifButton,Qt::AlignHCenter);
line03->addWidget(zoomNegatifButton,Qt::AlignHCenter);

line03->setContentsMargins(0,0, 0,0);
line03->setSpacing(1);
znpw->setLayout(line03);

QToolButton *zoomSelectionButton = new QToolButton(this);
zoomSelectionButton=butonToolSlot(zoomSelectionButton, "",":icons/icons/zoomselect.png",QColor(255,0,0,0),en*1.5,boy*0.8);
zoomSelectionButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(zoomSelectionButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::ZoomSelectionMode,DiagramItem::DiagramType::NoType);
   });


pageOfNumber=new QLineEdit(this);
pageOfNumber->setFixedSize(en*1.5,boy*0.7);
//pageOfNumber->setStyleSheet("background-color: #acacac");
pageOfNumber->setText(" / ");
pageOfNumber->setAlignment(Qt::AlignCenter);
pageOfNumber->setEnabled(false);
QFont ff( "Arial", 8, 0);
pageOfNumber->setFont(ff);

QWidget *nbw=new QWidget(this);
nbw->setFixedSize(en*1.5,boy);
//nbw->setStyleSheet("background-color: #ac0000");
QToolButton *nextButton = new QToolButton(nbw);
nextButton=butonToolSlot(nextButton,"",":icons/icons/nextpage.png",QColor(255,0,0,0),en*0.75,boy);
nextButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(nextButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::NextPageMode,DiagramItem::DiagramType::NoType);
   });

QToolButton *backButton = new QToolButton(nbw);
backButton=butonToolSlot(backButton,"",":icons/icons/backpage.png",QColor(255,0,0,0),en*0.75,boy);
backButton->setIconSize(QSize(en*0.8,boy*0.8));
connect(backButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::BackPageMode,DiagramItem::DiagramType::NoType);
   });

QHBoxLayout *line02 = new QHBoxLayout;
line02->addWidget(backButton,Qt::AlignHCenter);
line02->addWidget(nextButton,Qt::AlignHCenter);

line02->setContentsMargins(0,0, 0,0);
line02->setSpacing(1);
nbw->setLayout(line02);

QToolButton *jumpPageButton = new QToolButton(this);
jumpPageButton=butonToolSlot(jumpPageButton,"Sayfaya Git",":icons/icons/jumppage.png",QColor(255,0,0,0),en*1.5,boy);
jumpPageButton->setIconSize(QSize(en*1.3,boy*1.3));
connect(jumpPageButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::JumpPageMode,DiagramItem::DiagramType::NoType);
   });
QToolButton *listButton = new QToolButton(this);
listButton=butonToolSlot(listButton,"",":icons/icons/list.png",QColor(255,0,0,0),en*1.5,boy*0.7);
connect(listButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::ListMode,DiagramItem::DiagramType::NoType);
   });
QToolButton *searchButton = new QToolButton(this);
searchButton=butonToolSlot(searchButton,"",":icons/icons/search.png",QColor(255,0,0,0),en*1.5,boy*0.7);
connect(searchButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::SearchMode,DiagramItem::DiagramType::NoType);
   });
/*
QToolButton *saveButton = new QToolButton(this);
saveButton=butonToolSlot(saveButton,"",":icons/icons/save.png",QColor(255,0,0,0),en*1.5,boy);
connect(saveButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::SaveMode,DiagramItem::DiagramType::NoType);
   });
*/
QToolButton *printButton = new QToolButton(this);
printButton=butonToolSlot(printButton,"",":icons/icons/print.png",QColor(255,0,0,0),en*1.5,boy);
connect(printButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::PrintMode,DiagramItem::DiagramType::NoType);
   });
QWidget *prldw=new QWidget(this);
prldw->setFixedSize(en*1.5,boy);

QToolButton *panelLeftRightButton = new QToolButton(prldw);
panelLeftRightButton=butonToolSlot(panelLeftRightButton,"Sağa/Sola Hizala",":icons/icons/panelside.png",QColor(255,0,0,0),en*0.75,boy);
panelLeftRightButton->setIconSize(QSize(en*0.75,boy*0.75));
connect(panelLeftRightButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::PanelSideLeftRight,DiagramItem::DiagramType::NoType);
   });

QToolButton *panelBottomButton = new QToolButton(prldw);
panelBottomButton=butonToolSlot(panelBottomButton,"Alta Hizala",":icons/icons/panelsidedown.png",QColor(255,0,0,0),en*0.75,boy);
panelBottomButton->setIconSize(QSize(en*0.75,boy*0.75));
connect(panelBottomButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::PanelSideBottom,DiagramItem::DiagramType::NoType);
   });
QHBoxLayout *line06 = new QHBoxLayout;
line06->addWidget(panelLeftRightButton,Qt::AlignHCenter);
line06->addWidget(panelBottomButton,Qt::AlignHCenter);

line06->setContentsMargins(0,0, 0,0);
line06->setSpacing(1);
prldw->setLayout(line06);



QToolButton *exitButton = new QToolButton(this);
exitButton=butonToolSlot(exitButton,"Kapat",":icons/icons/close.png",QColor(255,0,0,0),en*1.5,boy);
connect(exitButton, &QToolButton::clicked, [=]() {
emit kalemModeSignal(Scene::Mode::ExitMode,DiagramItem::DiagramType::NoType);
   });
///QAction *quit = toolbar->addAction(QIcon(quitpix),"Quit Application");

///connect(quit, &QAction::triggered, qApp, &QApplication::quit);

/*QAction *pQAction = this->addAction("Click Me", [](bool) {
    qDebug() << "Clicked.";
});*/
 //QToolButton *pQToolBtn= dynamic_cast<QToolButton*>(this->widgetForAction(pQAction));

this->addWidget(handButton);
this->addWidget(copyButton);
this->addWidget(penButton);
this->addWidget(eraseButton);
this->addWidget(clearButton);
this->addWidget(blackButton);
this->addWidget(redButton);
this->addWidget(blueButton);
this->addWidget(penColorButton);
//this->addWidget(undoButton);
//this->addWidget(redoButton);
this->addWidget(urw);
this->addWidget(sekilButton);
this->addWidget(zeminButton);
//this->addWidget(zoomPozitifButton);
//this->addWidget(zoomNegatifButton);

this->addWidget(znpw);
this->addWidget(zoomSelectionButton);
//this->addWidget(fitWindowButton);
//this->addWidget(fitPageButton);
this->addWidget(wpw);
this->addWidget(pageOfNumber);
this->addWidget(nbw);
//this->addWidget(backButton);
//this->addWidget(nextButton);
this->addWidget(jumpPageButton);
QWidget *spacer0 = new QWidget();
spacer0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
this->addWidget(spacer0);

/*QWidget *spacer = new QWidget(this);
spacer->setMinimumHeight(50);
spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
this->addWidget(spacer);*/
//this->addWidget(saveButton);
this->addWidget(printButton);
// Spacer
    QWidget *spacer1 = new QWidget();
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->addWidget(spacer1);
//this->addWidget(panelLeftRightButton);
//this->addWidget(panelDownButton);
    this->addWidget(listButton);

        this->addWidget(searchButton);

this->addWidget(prldw);
this->addWidget(exitButton);
/*
 *
auto layout = new QGridLayout();
layout->setContentsMargins(1,0, 0,0);
layout->setSpacing(1);

layout->addWidget(moveLabel, 0, 0,1,2);
layout->addWidget(handButton, 1, 0,1,2);
layout->addWidget(copyButton, 2, 0,1,2);

layout->addWidget(penButton, 10, 0,1,2);
//layout->addWidget(penPopupButton, 10, 1,1,1);

layout->addWidget(eraseButton, 20, 0,1,2);
layout->addWidget(clearButton, 30, 0,1,2);

layout->addWidget(blackButton, 40, 0,1,2);
layout->addWidget(redButton, 41, 0,1,2);
layout->addWidget(blueButton, 42, 0,1,2);
layout->addWidget(penColorButton, 43, 0,1,2);

layout->addWidget(undoButton, 45, 0,1,1);
layout->addWidget(redoButton, 45, 1,1,1);

layout->addWidget(sekilButton, 50, 0,1,2);
layout->addWidget(zeminButton, 60, 0,1,2);


layout->addWidget(zoomPozitifButton, 70, 0,1,1);
layout->addWidget(zoomNegatifButton, 70, 1,1,1);
layout->addWidget(zoomSelectionButton, 71, 0,1,2);

layout->addWidget(fitWindowButton, 73, 0,1,1);
layout->addWidget(fitPageButton, 73, 1,1,1);

layout->addWidget(pageOfNumber, 79, 0,1,2);
layout->addWidget(backButton, 80, 0,1,1);
layout->addWidget(nextButton, 80, 1,1,1);
layout->addWidget(jumpPageButton, 89, 0,1,2);
layout->addWidget(saveButton, 90, 0,1,2);
layout->addWidget(printButton, 91, 0,1,2);
layout->addWidget(exitButton, 92, 0,1,2);

this->setLayout(layout);

*/
}

QPushButton *KalemWidget::butonSlot(QPushButton *btn, QString text, QString icon, QColor color, int w, int h, int iw, int ih)
{
    int e=w;
    int b=h;
    btn->setFixedSize(e, b);
    btn->setIconSize(QSize(iw,ih));
    if(icon!="") btn->setIcon(QIcon(icon));
    btn->setText(text);
    btn->setFlat(true);
    //btn->setStyleSheet(QString("QPushButton{ background-color: rgba(%1, %2, %3, %4); }").arg(QString::number(color.red()),
    // btn->setPalette(*plt);
    QPalette *palet=new QPalette();
    palet->setColor(QPalette::Button, color);
    btn->setPalette(*palet);
    btn->setAutoFillBackground(true);
    QFont ff( "Arial", 8, QFont::Normal);
    btn->setFont(ff);

    btn->update();
    return btn;
}
QToolButton *KalemWidget::butonToolSlot(QToolButton *btn,QString text,QString icon,QColor color,int w, int h)
{
    int e=w;
    int b=h;
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setFixedSize(e, b);
    btn->setIconSize(QSize(e*0.7,b*0.7));
    if(icon!="") btn->setIcon(QIcon(icon));
    //if(text!="") btn->setText(text);
     btn->setAutoRaise(true);
    btn->setAutoFillBackground(true);
    btn->setStyleSheet(QString("QToolButton{ background-color: rgba(%1, %2, %3, %4); }").arg(QString::number(color.red()),
                                                                                             QString::number(color.green()),
                                                                                             QString::number(color.blue()),
                                                                                      QString::number(color.alpha())));
    //QFont ff( "Arial", 7, QFont::Normal);
    //btn->setFont(ff);

    return btn;
}

 void KalemWidget::mouseMoveEvent(QMouseEvent *event)
{ //qDebug()<<"main mouse  move"<<event->pos();
       if ((event->buttons() & Qt::LeftButton) && mouseClick==true) {

          //this->move(mapToParent(event->pos()));
         /*  QPoint newpos_min = mapToParent(event->pos()-offset);
                 QPoint newpos_max = QPoint(newpos_min.x() + this->width(), newpos_min.y() + this->height());

                 if(newpos_min.x() > 0 &&
                    newpos_min.y() > 0 &&
                    newpos_max.x() < this->parentWidget()->width() &&
                    newpos_max.y() < this->parentWidget()->height()){

                     this->move(mapToParent(event->pos() - offset));
                 }
                 */
           //Excute movement follow mouse position
                /*  move(mapToParent(event->pos() - offset));
                  //Make sure control do not move out parent size
                  if (x() < 0)
                      move(1, y());
                  if (y() < 0)
                      move(x(), 1);
                  if (x() + width() > parentWidget()->width())
                      move(parentWidget()->width() - 1 - width(), y());
                  if (y() + height() > parentWidget()->height())
                      move(x(), parentWidget()->height() - 1 - height());
*/

   }

}

void KalemWidget::mousePressEvent(QMouseEvent *event)
{
   // qDebug()<<"main press";

            if ((event->buttons() & Qt::LeftButton)) {
 offset = event->pos();

            mouseClick = true;


            }

}

void KalemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // qDebug()<<"main mouse release";
    if (event->button() == Qt::LeftButton && mouseClick) {

          mouseClick = false;

      }
}

QMenu* KalemWidget::colorMenu(QString colorType,QString yon,int w,int h,bool close)
{   QMenu *menu = new QMenu(this);
   // qDebug()<<"renk invoke"<<colorType;
    int e=w;
      int b=h;

   palette=new QPalette();
   palette->setColor(QPalette::Button,QColor(255,0,0,255));

     QPushButton *color1Button= new QPushButton;
     color1Button=butonSlot(color1Button,"","",QColor(255,0,0,255),e,b,e,b);
   connect(color1Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,0,0,255);
       emit kalemColorSignal(colorType,QColor(255,0,0,255));
      if(close) menu->close();
   });


   QPushButton *color2Button= new QPushButton;
   color2Button=butonSlot(color2Button,"","",QColor(0,255,0,255),e,b,e,b);

   connect(color2Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,255,0,255);
       emit kalemColorSignal(colorType,QColor(0,255,0,255));
      if(close)  menu->close();
});

   QPushButton *color3Button= new QPushButton;
   color3Button=butonSlot(color3Button,"","",QColor(0,0,255,255),e,b,e,b);

   connect(color3Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,255,255);
       emit kalemColorSignal(colorType,QColor(0,0,255,255));
       if(close) menu->close();
});

   QPushButton *color4Button= new QPushButton;
   color4Button=butonSlot(color4Button,"","",QColor(255,255,0,255),e,b,e,b);

   connect(color4Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,255,0,255);
       emit kalemColorSignal(colorType,QColor(255,255,0,255));
       if(close) menu->close();
});

   QPushButton *color5Button= new QPushButton;
   color5Button=butonSlot(color5Button,"","",QColor(0,255,255,255),e,b,e,b);

   connect(color5Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,255,255,255);
       emit kalemColorSignal(colorType,QColor(0,255,255,255));
       if(close) menu->close();
});

   QPushButton *color6Button= new QPushButton;
   color6Button=butonSlot(color6Button,"","",QColor(255,0,255,255),e,b,e,b);
   connect(color6Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,0,255,255);
       emit kalemColorSignal(colorType,QColor(255,0,255,255));
        if(close)   menu->close();
});

   QPushButton *color7Button= new QPushButton;
   color7Button=butonSlot(color7Button,"","",QColor(255,255,255,255),e,b,e,b);

   connect(color7Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,255,255,255);
       emit kalemColorSignal(colorType,QColor(255,255,255,255));
       if(close) menu->close();
});

   QPushButton *color8Button= new QPushButton;
   color8Button=butonSlot(color8Button,"","",QColor(0,0,0,255),e,b,e,b);

   connect(color8Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,0,255);
       emit kalemColorSignal(colorType,QColor(0,0,0,255));
       if(close) menu->close();
});

   QPushButton *color9Button= new QPushButton;
   color9Button=butonSlot(color9Button,"","",QColor(127,127,127,255),e,b,e,b);

   connect(color9Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(127,127,127,255);
       emit kalemColorSignal(colorType,QColor(127,127,127,255));
       if(close) menu->close();
});

   QPushButton *color11Button= new QPushButton;
   color11Button=butonSlot(color11Button,"RY","",QColor(0,0,0,0),e,b,e,b);

   connect(color11Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,0,0);
       emit kalemColorSignal(colorType,QColor(0,0,0,0));
         if(close) menu->close();
});
    auto widget = new QWidget;
    auto layout = new QGridLayout(widget);
    layout->setContentsMargins(0, 5, 0,5);
    layout->setVerticalSpacing(2);
  //  layout->setColumnMinimumWidth(0, 37);
    if(yon=="dikey"){
    layout->addWidget(color1Button, 1,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color2Button, 2,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color3Button, 3,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color4Button, 4,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color5Button, 5,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color6Button, 6,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color7Button, 7,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color8Button, 8,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color9Button, 9,1,1,1,Qt::AlignHCenter);
  // layout->addWidget(color10Button, 10,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color11Button, 11,1,1,1,Qt::AlignHCenter);
    }
    if(yon=="yatay"){
    layout->addWidget(color1Button, 1,0,1,1,Qt::AlignHCenter);
    layout->addWidget(color2Button, 1,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color3Button, 1,2,1,1,Qt::AlignHCenter);
    layout->addWidget(color4Button, 1,3,1,1,Qt::AlignHCenter);
    layout->addWidget(color5Button, 1,4,1,1,Qt::AlignHCenter);
    layout->addWidget(color6Button, 1,5,1,1,Qt::AlignHCenter);
    layout->addWidget(color7Button, 1,6,1,1,Qt::AlignHCenter);
    layout->addWidget(color8Button, 1,7,1,1,Qt::AlignHCenter);
    layout->addWidget(color9Button, 1,8,1,1,Qt::AlignHCenter);
  // layout->addWidget(color10Button, 9,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color11Button, 1,10,1,1,Qt::AlignHCenter);
    }

    auto wa = new QWidgetAction(this);
  //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(widget);
    menu->addAction(wa);

     menu->setStyleSheet("QMenu {background-color:#acacac; }");
   return menu;
}

QWidget* KalemWidget::colorWidget(QString colorType,QString yon,int w,int h,bool close)
{   QWidget *colorwidget = new QWidget(this);
   // qDebug()<<"renk invoke"<<colorType;
    int e=w;
      int b=h;

   palette=new QPalette();
   palette->setColor(QPalette::Button,QColor(255,0,0,255));

     QPushButton *color1Button= new QPushButton;
     color1Button=butonSlot(color1Button,"","",QColor(255,0,0,255),e,b,e,b);
   connect(color1Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,0,0,255);
       emit kalemColorSignal(colorType,QColor(255,0,0,255));
     // if(close) menu->close();
   });


   QPushButton *color2Button= new QPushButton;
   color2Button=butonSlot(color2Button,"","",QColor(0,255,0,255),e,b,e,b);

   connect(color2Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,255,0,255);
       emit kalemColorSignal(colorType,QColor(0,255,0,255));
     // if(close)  menu->close();
});

   QPushButton *color3Button= new QPushButton;
   color3Button=butonSlot(color3Button,"","",QColor(0,0,255,255),e,b,e,b);

   connect(color3Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,255,255);
       emit kalemColorSignal(colorType,QColor(0,0,255,255));
      // if(close) menu->close();
});

   QPushButton *color4Button= new QPushButton;
   color4Button=butonSlot(color4Button,"","",QColor(255,255,0,255),e,b,e,b);

   connect(color4Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,255,0,255);
       emit kalemColorSignal(colorType,QColor(255,255,0,255));
      // if(close) menu->close();
});

   QPushButton *color5Button= new QPushButton;
   color5Button=butonSlot(color5Button,"","",QColor(0,255,255,255),e,b,e,b);

   connect(color5Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,255,255,255);
       emit kalemColorSignal(colorType,QColor(0,255,255,255));
      // if(close) menu->close();
});

   QPushButton *color6Button= new QPushButton;
   color6Button=butonSlot(color6Button,"","",QColor(255,0,255,255),e,b,e,b);
   connect(color6Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,0,255,255);
       emit kalemColorSignal(colorType,QColor(255,0,255,255));
       // if(close)   menu->close();
});

   QPushButton *color7Button= new QPushButton;
   color7Button=butonSlot(color7Button,"","",QColor(255,255,255,255),e,b,e,b);

   connect(color7Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(255,255,255,255);
       emit kalemColorSignal(colorType,QColor(255,255,255,255));
       //if(close) menu->close();
});

   QPushButton *color8Button= new QPushButton;
   color8Button=butonSlot(color8Button,"","",QColor(0,0,0,255),e,b,e,b);

   connect(color8Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,0,255);
       emit kalemColorSignal(colorType,QColor(0,0,0,255));
      // if(close) menu->close();
});

   QPushButton *color9Button= new QPushButton;
   color9Button=butonSlot(color9Button,"","",QColor(127,127,127,255),e,b,e,b);

   connect(color9Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(127,127,127,255);
       emit kalemColorSignal(colorType,QColor(127,127,127,255));
       //if(close) menu->close();
});

   QPushButton *color11Button= new QPushButton;
   color11Button=butonSlot(color11Button,"RY","",QColor(0,0,0,0),e,b,e,b);

   connect(color11Button, &QPushButton::clicked, [=]() {
       QColor renk=QColor(0,0,0,0);
       emit kalemColorSignal(colorType,QColor(0,0,0,0));
       //  if(close) menu->close();
});
   // auto widget = new QWidget;
    auto layout = new QGridLayout(colorwidget);
    layout->setContentsMargins(0, 5, 0,5);
    layout->setVerticalSpacing(2);
  //  layout->setColumnMinimumWidth(0, 37);
    if(yon=="dikey"){
    layout->addWidget(color1Button, 1,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color2Button, 2,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color3Button, 3,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color4Button, 4,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color5Button, 5,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color6Button, 6,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color7Button, 7,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color8Button, 8,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color9Button, 9,1,1,1,Qt::AlignHCenter);
  // layout->addWidget(color10Button, 10,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color11Button, 11,1,1,1,Qt::AlignHCenter);
    }
    if(yon=="yatay"){
    layout->addWidget(color1Button, 1,0,1,1,Qt::AlignHCenter);
    layout->addWidget(color2Button, 1,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color3Button, 1,2,1,1,Qt::AlignHCenter);
    layout->addWidget(color4Button, 1,3,1,1,Qt::AlignHCenter);
    layout->addWidget(color5Button, 1,4,1,1,Qt::AlignHCenter);
    layout->addWidget(color6Button, 1,5,1,1,Qt::AlignHCenter);
    layout->addWidget(color7Button, 1,6,1,1,Qt::AlignHCenter);
    layout->addWidget(color8Button, 1,7,1,1,Qt::AlignHCenter);
    layout->addWidget(color9Button, 1,8,1,1,Qt::AlignHCenter);
  // layout->addWidget(color10Button, 9,1,1,1,Qt::AlignHCenter);
    layout->addWidget(color11Button, 1,10,1,1,Qt::AlignHCenter);
    }

    auto wa = new QWidgetAction(this);
  //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(colorwidget);
    colorwidget->addAction(wa);

   //  colorwidget->setStyleSheet("background-color:#ffffff; ");
   return colorwidget;
}
void KalemWidget::setGridSize(int s)
{
    gridSize=s;
    gridSizePopLabel->setText("Satır ve Sütun Genişliği: "+QString::number(gridSize));
    emit kalemModeSignal(Scene::Mode::ZeminMode,pagePattern);
    emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    buttonStateClear();
    penButton->setChecked(true);
}
QWidget *KalemWidget::cizgiBoyutMenu()
{     int e=(en*0.8)/4*5.5;
      int b=(boy*0.6)/4*5;
      QWidget *menu = new QWidget(this);
      gridSizePopLabel= new QLabel();
      gridSizePopLabel->setText("Satır ve Sütun Genişliği: "+QString::number(gridSize));
      QFont ff( "Arial", 8, QFont::Normal);
      gridSizePopLabel->setFont(ff);

      QSlider *gSize= new QSlider(Qt::Horizontal,menu);
       gSize->setRange(5,60);
      gSize->setSliderPosition(15);
      //gSize->setSingleStep(10);
      gSize->setTickInterval(10);
      connect(gSize,SIGNAL(valueChanged(int)),this,SLOT(setGridSize(int)));

     // auto widget = new QWidget;
      auto layout = new QGridLayout(menu);
     // layout->setContentsMargins(0, 0, 0, 3);
     // layout->setColumnMinimumWidth(0, 37);


      layout->addWidget(gridSizePopLabel, 1, 1,1,2);
      layout->addWidget(gSize,4,1,1,2);
    //  layout->setColumnStretch(6, 255);
    // add a widget action to the context menu
      auto wa = new QWidgetAction(this);
    //  wa->setIcon(QIcon(":/icon1"));
      wa->setDefaultWidget(menu);
      menu->addAction(wa);
      gSize->setStyleSheet(".QSlider::groove:Horizontal {"
                                     "background: rgba(0, 0, 50, 200);"
                                     "width:"+QString::number(e*5)+"px;"
                                      "height:"+QString::number(b/2)+"px;"
                                     "}"

                                     ".QSlider::handle:Horizontal {"
                                     "background: rgba(242, 242, 242, 95);"
                                     "border: 2px solid rgb(0,0,0);"
                                     "background: rgba(0, 0, 50, 255);"
                                     "width: "+QString::number(e/2)+"px;"
                                     "height: "+QString::number(b)+"px;"
                                      "margin:-"+QString::number(e/8*3)+"px   0   -"+QString::number(b/8*3)+"px   0;"
                                     "}");

      gSize->setFixedSize(QSize(e*5,b));
     // menu->setStyleSheet("QMenu { width: 290 px; height: 180 px; }");
     return menu;
}

QMenu *KalemWidget::eraseMenu()
{   int e=(en*0.8)/4*9;
    int b=(boy*0.6)/4*5;
    QMenu *menu = new QMenu(this);
    eraseSizePopLabel= new QLabel();

    eraseSizePopLabel->setText("Silgi Boyutu: "+QString::number(penSize));
  //  eraseSizePopLabel->resize(e,b);
    QFont ff( "Arial", 8, QFont::Normal);
    eraseSizePopLabel->setFont(ff);

    QSlider *eraseSize= new QSlider(Qt::Horizontal,menu);
    //eraseSize->setMinimum(2);
    //eraseSize->setMaximum(1);
    eraseSize->setRange(1,15);
    eraseSize->setSliderPosition(4);
    ///connect(eraseSize,SIGNAL(valueChanged(int)),this,SLOT(setEraseSize(int)));
    QPushButton *silMenuButton=new QPushButton();
    silMenuButton->setFixedSize(e, b);
    silMenuButton->setIconSize(QSize(e,b));
    QPushButton *temizleMenuButton=new QPushButton();
    temizleMenuButton->setFixedSize(e, b);
    temizleMenuButton->setIconSize(QSize(e,b));

    auto widget = new QWidget;
    auto layout = new QGridLayout(widget);
   // layout->setContentsMargins(0, 0, 0, 3);
   // layout->setColumnMinimumWidth(0, 37);


    layout->addWidget(eraseSizePopLabel, 1, 1,1,2);
    layout->addWidget(silMenuButton, 2, 1,1,1,Qt::AlignHCenter);
    layout->addWidget(temizleMenuButton, 2, 2,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Sil</font>"),3,1,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Tahta Temizle</font>"),3,2,1,1,Qt::AlignHCenter);

    layout->addWidget(eraseSize,4,1,1,2);
  //  layout->setColumnStretch(6, 255);
    connect(silMenuButton, &QPushButton::clicked, [=]() {
        //scene->setPopMenuStatus(false);
        menu->close();
        //kalemButtonClick();
    });
    connect(temizleMenuButton, &QPushButton::clicked, [=]() {
       // scene->setPopMenuStatus(false);
        menu->close();
        //kalemButtonClick();
    });
    // add a widget action to the context menu
    auto wa = new QWidgetAction(this);
  //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(widget);
    menu->addAction(wa);
    eraseSize->setStyleSheet(".QSlider::groove:Horizontal {"
                                   "background: rgba(0, 0, 50, 200);"
                                   "width:"+QString::number(e*3)+"px;"
                                    "height:"+QString::number(b/2)+"px;"
                                   "}"

                                   ".QSlider::handle:Horizontal {"
                                   "background: rgba(242, 242, 242, 95);"
                                   "border: 2px solid rgb(0,0,0);"
                                   "background: rgba(0, 0, 50, 255);"
                                   "width: "+QString::number(e/2)+"px;"
                                   "height: "+QString::number(b)+"px;"
                                    "margin:-"+QString::number(e/8*3)+"px   0   -"+QString::number(b/8*3)+"px   0;"
                                   "}");

    eraseSize->setFixedSize(QSize(e*3,b));
   // menu->setStyleSheet("QMenu { width: 290 px; height: 180 px; }");
   return menu;
}

QMenu* KalemWidget::sekilMenu()
{
    int ken=300;
    //ken=300;
    int e=(en*0.8)/4*9;
    int b=(boy*0.6)/4*5;
    e=e/2;
    QMenu *menu= new QMenu();
      menu->installEventFilter(this);
      /*************************************************************************/
    DiagramItem *ditem=new DiagramItem();

    QPushButton *penStyleSolidLine = new QPushButton;
    penStyleSolidLine->setFixedSize(e, b);
    penStyleSolidLine->setIconSize(QSize(e,b));
   // penStyleSolidLine->setFlat(true);
    penStyleSolidLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::SolidLine,ken,ken));
    connect(penStyleSolidLine, &QPushButton::clicked, [=]() {
     emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::SolidLine);

    });

    QPushButton *penStyleDashLine = new QPushButton;
    penStyleDashLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::DashLine,ken,ken));
    penStyleDashLine->setFixedSize(e, b);
    penStyleDashLine->setIconSize(QSize(e,b));
   // penStyleDashLine->setFlat(true);
    connect(penStyleDashLine, &QPushButton::clicked, [=]() {
  emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::DashLine);

    });

    QPushButton *penStyleDotLine = new QPushButton;
    penStyleDotLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::DotLine,ken,ken));
    penStyleDotLine->setFixedSize(e, b);
    penStyleDotLine->setIconSize(QSize(e,b));
   // penStyleDotLine->setFlat(true);
    connect(penStyleDotLine, &QPushButton::clicked, [=]() {
         emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::DotLine);

    });
/********************************************************************/


    /***********************pergel cetvel gönye iletki***************/


    QPushButton *pergel = new QPushButton;
    pergel->setFixedSize(e, b);
    pergel->setIconSize(QSize(e,b));
    //penStyleSolidLine->setFlat(true);
    pergel->setIcon(QIcon(":icons/icons/pergel.png"));
    connect(pergel, &QPushButton::clicked, [=]() {
          emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Pergel);

        menu->close();
    });

    QPushButton *cetvel = new QPushButton;
    cetvel->setIcon(QIcon(":icons/icons/cetvel.png"));
    cetvel->setFixedSize(e, b);
    cetvel->setIconSize(QSize(e,b));
    //penStyleDashLine->setFlat(true);
    connect(cetvel, &QPushButton::clicked, [=]() {
         emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Cetvel);

        menu->close();

    });

    QPushButton *gonye = new QPushButton;
    gonye->setIcon(QIcon(":icons/icons/gonye.png"));
    gonye->setFixedSize(e, b);
    gonye->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(gonye, &QPushButton::clicked, [=]() {
         emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Gonye);

        menu->close();

    });

    QPushButton *iletki = new QPushButton;
    iletki->setIcon(QIcon(":icons/icons/iletki.png"));
    iletki->setFixedSize(e, b);
    iletki->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(iletki, &QPushButton::clicked, [=]() {
          emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Iletki);

        menu->close();

    });
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->setSpacing(en/3*2);
    layout1->addWidget(pergel);
    layout1->addWidget(cetvel);
    layout1->addWidget(gonye);
    layout1->addWidget(iletki);

     /***********************küp silindir küre pramid***************/


    QPushButton *kup = new QPushButton;
    kup->setFixedSize(e, b);
    kup->setIconSize(QSize(e,b));
    //penStyleSolidLine->setFlat(true);
    kup->setIcon(QIcon(":icons/icons/kup.png"));
    connect(kup, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Kup);
         menu->close();
    });

    QPushButton *silindir = new QPushButton;
    silindir->setIcon(QIcon(":icons/icons/silindir.png"));
    silindir->setFixedSize(e, b);
    silindir->setIconSize(QSize(e,b));
    //penStyleDashLine->setFlat(true);
    connect(silindir, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Silindir);
        menu->close();

    });

    QPushButton *pramit = new QPushButton;
    pramit->setIcon(QIcon(":icons/icons/pramit.png"));
    pramit->setFixedSize(e, b);
    pramit->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(pramit, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Pramit);
        menu->close();

    });

    QPushButton *kure = new QPushButton;
    kure->setIcon(QIcon(":icons/icons/kure.png"));
    kure->setFixedSize(e, b);
    kure->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(kure, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Kure);
     menu->close();

    });
    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(kup);
    layout2->addWidget(silindir);
    layout2->addWidget(pramit);
    layout2->addWidget(kure);
    layout2->setSpacing(en/3*2);
    /***********************************************************/

    QPushButton *cizgi= new QPushButton;
    cizgi->setFixedSize(e, b);
    cizgi->setIconSize(QSize(e,b));
   // cizgi->setFlat(true);
    cizgi->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Cizgi,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(cizgi, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Cizgi);
        menu->close();
    });

    QPushButton *ok= new QPushButton;
    ok->setFixedSize(e, b);
    ok->setIconSize(QSize(e,b));
    //ok->setFlat(true);
    ok->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Ok,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(ok, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Ok);

    menu->close();
    });

    QPushButton *ciftok= new QPushButton;
    ciftok->setFixedSize(e, b);
    ciftok->setIconSize(QSize(e,b));
    //ciftok->setFlat(true);
    ciftok->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::CiftOk,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(ciftok, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::CiftOk);
        menu->close();
    });

    QPushButton *ucgen= new QPushButton;
    ucgen->setFixedSize(e, b);
    ucgen->setIconSize(QSize(e,b));
   // ucgen->setFlat(true);
    ucgen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Ucgen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(ucgen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Ucgen);
         menu->close();
    });

    QPushButton *dortgen= new QPushButton;
    dortgen->setFixedSize(e, b);
    dortgen->setIconSize(QSize(e,b));
    //dortgen->setFlat(true);
    dortgen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Dortgen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(dortgen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Dortgen);
        menu->close();
    });

    QPushButton *cember= new QPushButton;
    cember->setFixedSize(e, b);
    cember->setIconSize(QSize(e,b));
   // cember->setFlat(true);
    cember->setIcon(imageEllipse(ditem->sekilStore(DiagramItem::DiagramType::Cember,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(cember, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Cember);
       menu->close();
    });

    QPushButton *baklava= new QPushButton;
    baklava->setFixedSize(e, b);
    baklava->setIconSize(QSize(e,b));
    //baklava->setFlat(true);
    baklava->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Baklava,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(baklava, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Baklava);
         menu->close();
    });

    QPushButton *yamuk= new QPushButton;
    yamuk->setFixedSize(e, b);
    yamuk->setIconSize(QSize(e,b));
    //yamuk->setFlat(true);
    yamuk->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Yamuk,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(yamuk, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Yamuk);
          menu->close();
    });

    QPushButton *besgen= new QPushButton;
    besgen->setFixedSize(e, b);
    besgen->setIconSize(QSize(e,b));
    //esgen->setFlat(true);
    besgen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Besgen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(besgen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Besgen);
        menu->close();
     });

    QPushButton *altigen= new QPushButton;
    altigen->setFixedSize(e, b);
    altigen->setIconSize(QSize(e,b));
    //altigen->setFlat(true);
    altigen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Altigen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(altigen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Altigen);

      menu->close();
    });

    QPushButton *sekizgen= new QPushButton;
    sekizgen->setFixedSize(e, b);
    sekizgen->setIconSize(QSize(e,b));
    //sekizgen->setFlat(true);
    sekizgen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Sekizgen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(sekizgen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Sekizgen);
          menu->close();
    });

    QPushButton *dikucgen= new QPushButton;
    dikucgen->setFixedSize(e, b);
    dikucgen->setIconSize(QSize(e,b));
    //dikucgen->setFlat(true);
    dikucgen->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::DikUcgen,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(dikucgen, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::DikUcgen);
     menu->close();
    });

    QPushButton *guzelyazi= new QPushButton;
    guzelyazi->setFixedSize(e, b);
    guzelyazi->setIconSize(QSize(e,b));
    //guzelyazi->setFlat(true);
    guzelyazi->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::GuzelYazi,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(guzelyazi, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::GuzelYazi);
     menu->close();
    });

    QPushButton *muzik= new QPushButton;
    muzik->setFixedSize(e, b);
    muzik->setIconSize(QSize(e,b));
    //muzik->setFlat(true);
    muzik->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::Muzik,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(muzik, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Muzik);
    menu->close();
    });

    QPushButton *cizgilisayfa= new QPushButton;
    cizgilisayfa->setFixedSize(e, b);
    cizgilisayfa->setIconSize(QSize(e,b));
   // cizgilisayfa->setFlat(true);
    cizgilisayfa->setIcon(image(ditem->sekilStore(DiagramItem::DiagramType::CizgiliSayfa,QRectF(QPointF(b,b),QPointF(ken-b,ken-b))),ken,ken));
    connect(cizgilisayfa, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::CizgiliSayfa);
         menu->close();
    });
    QPushButton *resimEkle= new QPushButton;
    resimEkle->setFixedSize(e, b);
    resimEkle->setIconSize(QSize(e,b));
    resimEkle->setFlat(true);
    resimEkle->setIcon(QIcon(":/icons/icons/addimage.png"));
    connect(resimEkle, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::SekilMode,DiagramItem::DiagramType::Resim);

         menu->close();
    });

    QPushButton *sekilKalemRenkButton=new QPushButton();
    sekilKalemRenkButton->setFixedSize(e,b);
    sekilKalemRenkButton->setIconSize(QSize(e,b));
    QPushButton *sekilZeminRenkButton=new QPushButton();

    sekilZeminRenkButton->setFixedSize(e,b);
    sekilZeminRenkButton->setIconSize(QSize(e,b));
    connect(sekilZeminRenkButton, &QPushButton::clicked, [=]() {
        colorMenu("sekilZeminColor","dikey",en,boy,true)->exec(mapToGlobal(mapToParent(menu->pos())));

         //menu->close();
    });
    auto widget = new QWidget;
    auto layout = new QGridLayout(widget);
   // layout->setContentsMargins(20, 0, 0, 0);
    //layout->setColumnMinimumWidth(0, 37);
    //layout->addWidget(sekilLabel, 0, 1,1,3);
   // layout->addWidget(sekilKalemSizePopLabel, 1, 1,1,3);
  //  layout->addWidget(sekilPenSize, 2, 1,1,3);

  /*  layout->addWidget(penStyleSolidLine, 3, 1,1,1,Qt::AlignHCenter);
    layout->addWidget(penStyleDashLine, 3, 2,1,1,Qt::AlignHCenter);
    layout->addWidget(penStyleDotLine, 3, 3,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Çizgi</font>"),4,1,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Kesik Çizgi</font>"),4,2,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Noktalı Çizgi</font>"),4,3,1,1,Qt::AlignHCenter);
*/
    layout->addLayout(layout1, 5, 1,1,3,Qt::AlignHCenter);
    QFont ff( "Arial", 5, QFont::Normal);

    QHBoxLayout *arac = new QHBoxLayout;
    QLabel *pergell= new QLabel("Pergel");      pergell->setFont(ff);
    QLabel *gonyel=  new QLabel("Gönye");        gonyel->setFont(ff);
    QLabel *cetvell= new QLabel("Cetvel");    cetvell->setFont(ff);;
    QLabel *iletkil= new QLabel("İletki");      iletkil->setFont(ff);
    arac->addWidget(pergell,Qt::AlignHCenter);
    arac->addWidget(gonyel,Qt::AlignHCenter);
    arac->addWidget(cetvell,Qt::AlignHCenter);
    arac->addWidget(iletkil,Qt::AlignHCenter);
    arac->setSpacing(en/3*2);
    layout->addLayout(arac, 6, 1,1,3,Qt::AlignHCenter);


    layout->addLayout(layout2, 7, 1,1,3,Qt::AlignHCenter);
   //layout->addLayout(layout0, 8, 1,1,3,Qt::AlignHCenter);
  // layout->addLayout(layout00, 9, 1,1,3,Qt::AlignHCenter);

//colorWidget
   QWidget *cw=colorWidget("sekilZeminColor","yatay",en/2,boy/2,false);
   cw->setFixedSize(en*6,boy);
   cw->setVisible(true);
    layout->addWidget(cw, 10, 1,1,3,Qt::AlignHCenter);
   QHBoxLayout *line0 = new QHBoxLayout;
   line0->addWidget(cizgi,Qt::AlignHCenter);
   line0->addWidget(ok,Qt::AlignHCenter);
   line0->addWidget(ciftok,Qt::AlignHCenter);
   line0->addWidget(ucgen,Qt::AlignHCenter);
   line0->setSpacing(en/3*2);
   layout->addLayout(line0, 11, 1,1,3,Qt::AlignHCenter);
   QHBoxLayout *line00 = new QHBoxLayout;
   QLabel *lb00= new QLabel("Çizgi");
   QLabel *lb01=  new QLabel("Ok");
   QLabel *lb02= new QLabel("Çift Ok");
   QLabel *lb03= new QLabel("Üçgen");
  // QFont ff( "Arial", 5, QFont::Normal);
   lb00->setFont(ff);lb01->setFont(ff);lb02->setFont(ff);lb03->setFont(ff);
   lb00->setFixedSize(20,15);
   lb01->setFixedSize(20,15);
   lb02->setFixedSize(22,15);
   lb03->setFixedSize(20,15);

   line00->addWidget(lb00,Qt::AlignHCenter);
   line00->addWidget(lb01,Qt::AlignHCenter);
   line00->addWidget(lb02,Qt::AlignHCenter);
   line00->addWidget(lb03,Qt::AlignHCenter);
   line00->setSpacing(en/3*2);
   layout->addLayout(line00, 12, 1,1,3,Qt::AlignHCenter);

   /***************************************************************/
   QHBoxLayout *line01 = new QHBoxLayout;
   line01->addWidget(dortgen,Qt::AlignHCenter);
   line01->addWidget(cember,Qt::AlignHCenter);
   line01->addWidget(baklava,Qt::AlignHCenter);
   line01->addWidget(yamuk,Qt::AlignHCenter);
   line01->setSpacing(en/3*2);
   layout->addLayout(line01, 13, 1,1,3,Qt::AlignHCenter);
   QHBoxLayout *line10 = new QHBoxLayout;
   QLabel *lb10= new QLabel("Dörtgen");
   QLabel *lb11=  new QLabel("Çember");
   QLabel *lb12= new QLabel("Baklava");
   QLabel *lb13= new QLabel("Yamuk");
  // QFont ff( "Arial", 5, QFont::Normal);
   lb10->setFont(ff);lb11->setFont(ff);lb12->setFont(ff);lb13->setFont(ff);
   lb10->setFixedSize(25,15);
   lb11->setFixedSize(25,15);
   lb12->setFixedSize(24,15);
   lb13->setFixedSize(24,15);

   line10->addWidget(lb10,Qt::AlignHCenter);
   line10->addWidget(lb11,Qt::AlignHCenter);
   line10->addWidget(lb12,Qt::AlignHCenter);
   line10->addWidget(lb13,Qt::AlignHCenter);
   line10->setSpacing(en/3*2);
   layout->addLayout(line10, 14, 1,1,3,Qt::AlignHCenter);

   /**************************************************************/
   QHBoxLayout *line02 = new QHBoxLayout;
   line02->addWidget(besgen,Qt::AlignHCenter);
   line02->addWidget(altigen,Qt::AlignHCenter);
   line02->addWidget(sekizgen,Qt::AlignHCenter);
   line02->addWidget(dikucgen,Qt::AlignHCenter);
   line02->setSpacing(en/3*2);
   layout->addLayout(line02, 15, 1,1,3,Qt::AlignHCenter);
    QLabel *lb20= new QLabel("Beşgen");
   QLabel *lb21=  new QLabel("Altıgen");
   QLabel *lb22= new QLabel("Sekizgen");
   QLabel *lb23= new QLabel("D.Üçgen");
  // QFont ff( "Arial", 5, QFont::Normal);
   lb20->setFont(ff);lb21->setFont(ff);lb22->setFont(ff);lb23->setFont(ff);
   lb20->setFixedSize(25,15);
   lb21->setFixedSize(25,15);
   lb22->setFixedSize(24,15);
   lb23->setFixedSize(24,15);
   QHBoxLayout *line20 = new QHBoxLayout;
   line20->addWidget(lb20,Qt::AlignHCenter);
   line20->addWidget(lb21,Qt::AlignHCenter);
   line20->addWidget(lb22,Qt::AlignHCenter);
   line20->addWidget(lb23,Qt::AlignHCenter);
   line20->setSpacing(en/3*2);
   layout->addLayout(line20, 16, 1,1,3,Qt::AlignHCenter);
   layout->addWidget(resimEkle, 17, 1,1,3,Qt::AlignHCenter);


 //   layout->setColumnStretch(10, 255);
    // add a widget action to the context menu
    auto wa = new QWidgetAction(this);
  //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(widget);
    menu->addAction(wa);
      return menu;
}

QMenu *KalemWidget::penMenu()
{
    int ken=300;
    int e=(en*0.8)/4*9;
    int b=(boy*0.6)/4*5;

    QMenu *menu = new QMenu(this);
      menu->installEventFilter(this);
    kalemSizePopLabel= new QLabel();
    kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
    QFont ff( "Arial", 8, QFont::Normal);
    kalemSizePopLabel->setFont(ff);

   /* QSlider *penSize= new QSlider(Qt::Horizontal,menu);
    penSize->setMinimum(2);
    penSize->setMaximum(8);
    penSize->setSliderPosition(4);
    connect(penSize,SIGNAL(valueChanged(int)),this,SLOT(setPenSize(int)));
*/
    DiagramItem *ditem=new DiagramItem();

    QPushButton *penStyleSolidLine = new QPushButton;
    penStyleSolidLine->setFixedSize(e, b);
    penStyleSolidLine->setIconSize(QSize(e,b));
    //penStyleSolidLine->setFlat(true);*/
    penStyleSolidLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::SolidLine,ken,ken));
    connect(penStyleSolidLine, &QPushButton::clicked, [=]() {
       // setPenStyle(Qt::SolidLine);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::SolidLine);
        menu->close();
    });

    QPushButton *penStyleDashLine = new QPushButton;
    penStyleDashLine->setFixedSize(e, b);
    penStyleDashLine->setIconSize(QSize(e,b));
    //penStyleDashLine->setFlat(true);
    penStyleDashLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::DashLine,ken,ken));
    connect(penStyleDashLine, &QPushButton::clicked, [=]() {
       // setPenStyle(Qt::DashLine);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::DashLine);
        menu->close();
    });

    QPushButton *penStyleDotLine = new QPushButton;
    penStyleDotLine->setFixedSize(e, b);
    penStyleDotLine->setIconSize(QSize(e,b));
   // penStyleDotLine->setFlat(true);
    penStyleDotLine->setIcon(lineImage(ditem->sekilStore(DiagramItem::DiagramType::Kalem,QRectF(QPointF(0,0),QPointF(ken,ken))),Qt::DotLine,ken,ken));
    connect(penStyleDotLine, &QPushButton::clicked, [=]() {
        //setPenStyle(Qt::DotLine);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::DotLine);
        menu->close();
    });

    QPushButton *kalemSekilTanimlama= new QPushButton;
     kalemSekilTanimlama=butonSlot(kalemSekilTanimlama,"",":icons/icons/smartpen.png",QColor(255,0,0,0),e,b,e,b);
    connect(kalemSekilTanimlama, &QPushButton::clicked, [=]()
    {
          emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::SmartPen);
            menu->close();
       /* scene->setPopMenuStatus(false);
        menu->close();
        kalemButtonClick();
        scene->setSekilTanimlamaStatus(true);
        buttonColorClear();
        palette->setColor(QPalette::Button, QColor(212,0,0,255));
       // kalemSekilTanimlama->setPalette(*palette);
        //kalemSekilTanimlama->setAutoFillBackground(true);
        kalemButton->setPalette(*palette);
        kalemButton->setAutoFillBackground(true);
        kalemButton->setIcon(QIcon(":icons/smartpen.png"));
//kalemButton->setIconSize(QSize(20,20));
         currentScreenModeSlot();
         */
    });


    QPushButton *kalemMenuButton=new QPushButton();
    /*kalemMenuButton->setFixedSize(e, b);
    kalemMenuButton->setIconSize(QSize(e,b));*/
    kalemMenuButton=butonSlot(kalemMenuButton,"",":icons/icons/pen.png",QColor(255,0,0,0),e,b,e,b);
    connect(kalemMenuButton, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::NormalPen);
        menu->close();

        /*scene->setPopMenuStatus(false);
        menu->close();
        scene->setSekilTanimlamaStatus(false);
*/
    });

    QPushButton *fosforluKalemButton=new QPushButton();
    fosforluKalemButton=butonSlot(fosforluKalemButton,"",":icons/icons/fosforlupen.png",QColor(255,0,0,0),e,b,e,b);
    connect(fosforluKalemButton, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::FosforluPen);
        menu->close();

        /*   scene->setPopMenuStatus(false);
           menu->close();
           //kalemButtonClick();
           scene->setSekilTanimlamaStatus(false);
   */
       });




    QPushButton *nokta2 = new QPushButton;
      nokta2=butonSlot(nokta2,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*0.2,b*0.2);
    connect(nokta2, &QPushButton::clicked, [=]() {
        //setPenSize(1);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize2);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
    });

    QPushButton *nokta4 = new QPushButton;
     nokta4=butonSlot(nokta4,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*0.4,b*0.4);
    connect(nokta4, &QPushButton::clicked, [=]() {
       // setPenSize(2);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize4);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
    });

    QPushButton *nokta6 = new QPushButton;
     nokta6=butonSlot(nokta6,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*0.6,b*0.6);
    connect(nokta6, &QPushButton::clicked, [=]() {
        //setPenSize(3);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize6);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
    });

    QPushButton *nokta8 = new QPushButton;
      nokta8=butonSlot(nokta8,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*0.8,b*0.8);
    connect(nokta8, &QPushButton::clicked, [=]() {
        //setPenSize(4);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize8);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
    });

    QPushButton *nokta10 = new QPushButton;
     nokta10=butonSlot(nokta10,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*1,b);
    connect(nokta10, &QPushButton::clicked, [=]() {
      //  setPenSize(5);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize10);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
   });

    QPushButton *nokta12 = new QPushButton;
      nokta12=butonSlot(nokta12,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*1.2,b*1.2);
    connect(nokta12, &QPushButton::clicked, [=]() {
        //setPenSize(6);
        emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize12);
        kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
        menu->close();
    });

    QPushButton *nokta14 = new QPushButton;
     nokta14=butonSlot(nokta14,"",":icons/icons/nokta.png",QColor(0,0,0,0),e/2,b,e*1.4,b*1.4);
    connect(nokta14, &QPushButton::clicked, [=]() {
       // setPenSize(7);
     emit kalemModeSignal(Scene::Mode::DrawPen,DiagramItem::DiagramType::PenSize14);
     kalemSizePopLabel->setText("Kalem Boyutu: "+QString::number(penSize));
    menu->close();
    });

    auto widget = new QWidget;
    auto layout = new QGridLayout(widget);
   // layout->setContentsMargins(0, 0, 0, 3);
   // layout->setColumnMinimumWidth(0, 37);


    layout->addWidget(kalemSizePopLabel, 0, 1,1,3);
 //   layout->addWidget(geriAlButton,1,2,1,1);
//    layout->addWidget(new QLabel("<font size=1>Geri Al</font>"),2,2,1,1,Qt::AlignHCenter);

    layout->addWidget(kalemMenuButton,3,1,1,1,Qt::AlignHCenter);
    layout->addWidget(fosforluKalemButton,3,2,1,1,Qt::AlignHCenter);
    layout->addWidget(kalemSekilTanimlama,3,3,1,2,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Kalem</font>"),4,1,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Fosforlu Kalem</font>"),4,2,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Akıllı Kalem</font>"),4,3,1,1,Qt::AlignHCenter);

   // layout->addWidget(penSize,5,1,1,3,Qt::AlignHCenter);

    QHBoxLayout *layout1 = new QHBoxLayout;
         layout1->addWidget(nokta2);
         layout1->addWidget(nokta4);
         layout1->addWidget(nokta6);
         layout1->addWidget(nokta8);
         layout1->addWidget(nokta10);
         layout1->addWidget(nokta12);
         layout1->addWidget(nokta14);


    layout->addLayout(layout1, 6, 1,1,3,Qt::AlignHCenter);
    //layout->addWidget(nokta4, 6, 2,1,1,Qt::AlignHCenter);
    //layout->addWidget(nokta8, 6, 3,1,1,Qt::AlignHCenter);
    //layout->addWidget(nokta12, 6, 4,1,1,Qt::AlignHCenter);

    layout->addWidget(penStyleSolidLine, 7, 1,1,1,Qt::AlignHCenter);
    layout->addWidget(penStyleDashLine, 7, 2,1,1,Qt::AlignHCenter);
    layout->addWidget(penStyleDotLine, 7, 3,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Düz</font>"),8,1,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Kesik</font>"),8,2,1,1,Qt::AlignHCenter);
    layout->addWidget(new QLabel("<font size=1>Noktalı</font>"),8,3,1,1,Qt::AlignHCenter);


  //  layout->setColumnStretch(6, 255);

    // add a widget action to the context menu
    auto wa = new QWidgetAction(this);
  //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(widget);
    menu->addAction(wa);
   /* penSize->setStyleSheet(".QSlider::groove:Horizontal {"
                                   "background: rgba(0, 0, 0, 200);"
                                   "width:"+QString::number(e*4)+"px;"
                                    "height: "+QString::number(b/2)+"px;"
                                   "}"

                                   ".QSlider::handle:Horizontal {"
                                   "background: rgba(242, 242, 242, 95);"
                                   "border: 2px solid rgb(0,0,0);"
                                   "background: rgba(0, 0, 0, 255);"
                                   "width: "+QString::number(e/2)+"px;"
                                   "height: "+QString::number(b)+"px;"
                                    "margin:-"+QString::number(e/8*3)+"px   0   -"+QString::number(b/8*3)+"px   0;"
                                   "}");
*/
   // penSize->setFixedSize(QSize(e*4,b));
    //menu->setStyleSheet("QMenu { width: 290 px; height: 250 px; }");
return menu;
}

QMenu *KalemWidget::zeminMenu()
{  //int e=en;
   // int b=boy;
    int e=(en*0.8)/4*6;
    int b=(boy*0.6)/4*5;

    QMenu *menu = new QMenu(this);
      menu->installEventFilter(this);
    QLabel  *zeminLabel= new QLabel();
    zeminLabel->setText("Arkaplan Seçenekleri");
    QFont ff( "Arial", 8, QFont::Normal);
    zeminLabel->setFont(ff);

/*
    gridSizePopLabel= new QLabel();
    gridSizePopLabel->setText("Grid Boyutu: "+QString::number(myGridSize));
    gridSizePopLabel->setFont(ff);

    QSlider *penSize= new QSlider(Qt::Horizontal,menu);
    penSize->setMinimum(2);
    penSize->setMaximum(8);
    penSize->setSliderPosition(4);
    ///connect(penSize,SIGNAL(valueChanged(int)),this,SLOT(setGridSize(int)));

*/

    auto widget = new QWidget;
    auto layout = new QGridLayout(widget);
    layout->setContentsMargins(0, 0, 0, 3);
    //layout->setColumnMinimumWidth(0, 37);


    /***********************noktalı izometrik kareli çizgili***************/


    QPushButton *noktaliKagit = new QPushButton;
    noktaliKagit->setFixedSize(e, b);
    noktaliKagit->setIconSize(QSize(e,b));
    //penStyleSolidLine->setFlat(true);
    noktaliKagit->setIcon(QIcon(":icons/icons/noktalikagit.png"));
    connect(noktaliKagit, &QPushButton::clicked, [=]() {
   emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::NoktaliKagit);

        menu->close();
    });

    QPushButton *izometrikKagit = new QPushButton;
    izometrikKagit->setIcon(QIcon(":icons/icons/izometrikkagit.png"));
    izometrikKagit->setFixedSize(e, b);
    izometrikKagit->setIconSize(QSize(e,b));
    //penStyleDashLine->setFlat(true);
    connect(izometrikKagit, &QPushButton::clicked, [=]() {
    emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::IzometrikKagit);
      menu->close();

    });

    QPushButton *kareliKagit = new QPushButton;
    kareliKagit->setIcon(QIcon(":icons/icons/karelikagit.png"));
    kareliKagit->setFixedSize(e, b);
    kareliKagit->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(kareliKagit, &QPushButton::clicked, [=]() {
    emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::KareliKagit);

        menu->close();

    });

    QPushButton *cizgiliKagit = new QPushButton;
    cizgiliKagit->setIcon(QIcon(":icons/icons/gridyatay.png"));
    cizgiliKagit->setFixedSize(e, b);
    cizgiliKagit->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(cizgiliKagit, &QPushButton::clicked, [=]() {
      emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::CizgiliKagit);
           menu->close();

    });
   /* QHBoxLayout *layout0 = new QHBoxLayout;
    layout0->addWidget(noktaliKagit);
    layout0->addWidget(izometrikKagit);
    layout0->addWidget(kareliKagit);
    layout0->addWidget(cizgiliKagit);
    layout0->setSpacing(en/3*2);
    QHBoxLayout *layout00 = new QHBoxLayout;
    layout00->addWidget(new QLabel("<font size=1>Noktalı  </font>"));
    layout00->addWidget(new QLabel("<font size=1>İzometrik</font>"));
    layout00->addWidget(new QLabel("<font size=1>Kareli   </font>"));
    layout00->addWidget(new QLabel("<font size=1>Çizgili  </font>"));
    layout00->setSpacing(en*1.5);


    /***********************ndogru nnkoordinat  hnkoordinat nkoordinat***************/


    QPushButton *ndogru = new QPushButton;
    ndogru->setFixedSize(e, b);
    ndogru->setIconSize(QSize(e,b));
    //penStyleSolidLine->setFlat(true);
    ndogru->setIcon(QIcon(":icons/icons/ndogru.png"));
    connect(ndogru, &QPushButton::clicked, [=]() {
      emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::NDogru);
      menu->close();
    });

    QPushButton *nnkoordinat = new QPushButton;
    nnkoordinat->setIcon(QIcon(":icons/icons/nnkoordinat.png"));
    nnkoordinat->setFixedSize(e, b);
    nnkoordinat->setIconSize(QSize(e,b));
    //penStyleDashLine->setFlat(true);
    connect(nnkoordinat, &QPushButton::clicked, [=]() {
      emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::NNKoordinat);
      menu->close();

    });

    QPushButton *hnkoordinat = new QPushButton;
    hnkoordinat->setIcon(QIcon(":icons/icons/hnkoordinat.png"));
    hnkoordinat->setFixedSize(e, b);
    hnkoordinat->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(hnkoordinat, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::HNKoordinat);
    menu->close();

    });

    QPushButton *nkoordinat = new QPushButton;
    nkoordinat->setIcon(QIcon(":icons/icons/nkoordinat.png"));
    nkoordinat->setFixedSize(e, b);
    nkoordinat->setIconSize(QSize(e,b));
    // penStyleDotLine->setFlat(true);
    connect(nkoordinat, &QPushButton::clicked, [=]() {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::NKoordinat);
     menu->close();

    });
   /* QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addWidget(nkoordinat);
    layout3->addWidget(hnkoordinat);
    layout3->addWidget(ndogru);
    layout3->addWidget(nnkoordinat);
    layout3->setSpacing(en/3*2);
    QHBoxLayout *layout33 = new QHBoxLayout;
    layout33->addWidget(new QLabel("<font size=1>S.Koordinat</font>"));
    layout33->addWidget(new QLabel("<font size=1>Y. Koordinat</font>"));
    layout33->addWidget(new QLabel("<font size=1>Sayı Doğrusu</font>"));
    layout33->addWidget(new QLabel("<font size=1>Koordinat</font>"));
    layout33->setSpacing(en*1);


    /*****************************************************/
    QPushButton *zeminSeffafButton=new QPushButton();
    zeminSeffafButton=butonSlot(zeminSeffafButton,"",":icons/icons/transparanboard.png",QColor(255,0,0,0),e,b,e,b);
    zeminSeffafButton->setFlat(true);
    connect(zeminSeffafButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::TransparanPage);
    });
    QPushButton *zeminSiyahButton=new QPushButton();
    zeminSiyahButton=butonSlot(zeminSiyahButton,"",":icons/icons/blackboard.png",QColor(255,0,0,0),e,b,e,b);
    zeminSiyahButton->setFlat(true);
    connect(zeminSiyahButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::BlackPage);

    });
    QPushButton *zeminBeyazButton=new QPushButton();
    zeminBeyazButton=butonSlot(zeminBeyazButton,"",":icons/icons/whiteboard.png",QColor(255,0,0,0),e,b,e,b);
    zeminBeyazButton->setFlat(true);
    connect(zeminBeyazButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::WhitePage);

    });
    QPushButton *zeminCustomColorButton=new QPushButton();
     zeminCustomColorButton=butonSlot(zeminCustomColorButton,"",":icons/icons/zeminCustomColor.png",QColor(255,0,0,0),e,b,e,b);
     zeminCustomColorButton->setFlat(true);
     connect(zeminCustomColorButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::CustomColorPage);

    });
    QPushButton *zeminCizgiliSayfaButton = new QPushButton;
    zeminCizgiliSayfaButton->setIcon(QIcon(":icons/icons/cizgilisayfa.png"));
    zeminCizgiliSayfaButton=butonSlot(zeminCizgiliSayfaButton,"",":icons/icons/cizgilisayfa.png",QColor(255,0,0,0),e,b,e,b);
    zeminCizgiliSayfaButton->setFlat(true);
    connect(zeminCizgiliSayfaButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::CizgiliPage);
    });

    QPushButton *zeminKareliSayfaButton = new QPushButton;
    zeminKareliSayfaButton=butonSlot(zeminKareliSayfaButton,"",":icons/icons/karelisayfa.png",QColor(255,0,0,0),e,b,e,b);
    zeminKareliSayfaButton->setFlat(true);
    connect(zeminKareliSayfaButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::KareliPage);
    });

    QPushButton *zeminMuzikSayfaButton=new QPushButton();
    zeminMuzikSayfaButton=butonSlot(zeminMuzikSayfaButton,"",":icons/icons/muziksayfa.png",QColor(255,0,0,0),e,b,e,b);
    zeminMuzikSayfaButton->setFlat(true);
    connect(zeminMuzikSayfaButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::MuzikPage);
    });


    QPushButton *zeminGuzelYaziSayfaButton=new QPushButton();
    zeminGuzelYaziSayfaButton=butonSlot(zeminGuzelYaziSayfaButton,"",":icons/icons/guzelyazisayfa.png",QColor(255,0,0,0),e,b,e,b);
    zeminGuzelYaziSayfaButton->setFlat(true);
    connect(zeminGuzelYaziSayfaButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::GuzelYaziPage);
    });


    QPushButton *zeminDesenEkleSayfaButton=new QPushButton();
    zeminDesenEkleSayfaButton=butonSlot(zeminDesenEkleSayfaButton,"",":icons/icons/resimekle.png",QColor(255,0,0,0),e,b,e,b);
    zeminDesenEkleSayfaButton->setFlat(true);
    connect(zeminDesenEkleSayfaButton, &QPushButton::clicked, [=]()
    {
        emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::CustomImagePage);

    });

   QPushButton *zeminTemizleSayfaButton = new QPushButton;
   zeminTemizleSayfaButton=butonSlot(zeminTemizleSayfaButton,"",":icons/icons/zemintemizle.png",QColor(255,0,0,0),e,b,e,b);
   zeminTemizleSayfaButton->setFlat(true);
   connect(zeminTemizleSayfaButton, &QPushButton::clicked, [=]()
    {
       emit kalemModeSignal(Scene::Mode::ZeminMode,DiagramItem::DiagramType::TransparanPage);

    });

    QPushButton *gridRenkButton = new QPushButton;
    gridRenkButton=butonSlot(gridRenkButton,"",":icons/icons/gridrenk.png",QColor(255,0,0,0),e,b,e,b);
    gridRenkButton->setFlat(true);
    QMenu *gcm=colorMenu("zeminGridColor","dikey",en,boy,true);
    gridRenkButton->setMenu(gcm);
   connect(gridRenkButton, &QPushButton::clicked, [=]()
    {
    });

    QPushButton *zeminRenkButton = new QPushButton;

     zeminRenkButton=butonSlot(zeminRenkButton,"",":icons/icons/zeminrenk.png",QColor(255,0,0,0),e,b,e,b);
     zeminRenkButton->setFlat(true);
     QMenu *zcm=colorMenu("sekilZeminColor","dikey",en,boy,true);
      zeminRenkButton->setMenu(zcm);
     connect(zeminRenkButton, &QPushButton::clicked, [=]()
     {
     });

   layout->addWidget(zeminLabel, 0, 1,1,4);
 /*  QWidget *cw=colorWidget("sekilZeminColor","yatay",en/2,boy/2,false);
   cw->setFixedSize(en*8,boy);
   cw->setVisible(true);
    //layout->addWidget(cw, 5, 1,1,4,Qt::AlignHCenter);
*/
   QFont ffl( "Arial", 6, QFont::Normal);

   layout->addWidget(noktaliKagit,10,1,1,1,Qt::AlignHCenter);
   layout->addWidget(izometrikKagit,10,2,1,1,Qt::AlignHCenter);
   layout->addWidget(kareliKagit,10,3,1,1,Qt::AlignHCenter);
   layout->addWidget(cizgiliKagit,10,4,1,1,Qt::AlignHCenter);

   QLabel *noktali=new QLabel("Noktalı");   noktali->setFont(ffl);
   QLabel *izometrik=new QLabel("İzometrik"); izometrik->setFont(ffl);
   QLabel *kareli=new QLabel("Kareli");    kareli->setFont(ffl);
   QLabel *cizgili=new QLabel("Çizgili");   cizgili->setFont(ffl);
   layout->addWidget(noktali,11,1,1,1,Qt::AlignHCenter);
   layout->addWidget(izometrik,11,2,1,1,Qt::AlignHCenter);
   layout->addWidget(kareli,11,3,1,1,Qt::AlignHCenter);
   layout->addWidget(cizgili,11,4,1,1,Qt::AlignHCenter);

   layout->addWidget(nkoordinat,12,1,1,1,Qt::AlignHCenter);
   layout->addWidget(hnkoordinat,12,2,1,1,Qt::AlignHCenter);
   layout->addWidget(ndogru,12,3,1,1,Qt::AlignHCenter);
   layout->addWidget(nnkoordinat,12,4,1,1,Qt::AlignHCenter);

   QLabel *skoordinatl=new QLabel("S.Koordinat");   skoordinatl->setFont(ffl);
   QLabel *hnkoordinatl=new QLabel("Y. Koordinat"); hnkoordinatl->setFont(ffl);
   QLabel *ndogrul=new QLabel("Sayı Doğrusu");      ndogrul->setFont(ffl);
   QLabel *nnkoordinatl=new QLabel("Koordinat");    nnkoordinatl->setFont(ffl);

   layout->addWidget(skoordinatl,13,1,1,1,Qt::AlignHCenter);
   layout->addWidget(hnkoordinatl,13,2,1,1,Qt::AlignHCenter);
   layout->addWidget(ndogrul,13,3,1,1,Qt::AlignHCenter);
   layout->addWidget(nnkoordinatl,13,4,1,1,Qt::AlignHCenter);

    layout->addWidget(zeminSeffafButton,20,1,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminSiyahButton,20,2,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminBeyazButton,20,3,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminCustomColorButton,20,4,1,1,Qt::AlignHCenter);

    QLabel *seffaftahta=new QLabel("Şeffaf Tahta");   seffaftahta->setFont(ffl);
    QLabel *siyahtahta=new QLabel("Siyah Tahta"); siyahtahta->setFont(ffl);
    QLabel *beyaztahta=new QLabel("Beyaz Tahta");      beyaztahta->setFont(ffl);
    QLabel *customtahta=new QLabel("S. Renk Tahta");    customtahta->setFont(ffl);

    layout->addWidget(seffaftahta,30,1,1,1,Qt::AlignHCenter);
    layout->addWidget(siyahtahta,30,2,1,1,Qt::AlignHCenter);
    layout->addWidget(beyaztahta,30,3,1,1,Qt::AlignHCenter);
    layout->addWidget(customtahta,30,4,1,1,Qt::AlignHCenter);


    layout->addWidget(zeminCizgiliSayfaButton,50,1,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminKareliSayfaButton,50,2,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminMuzikSayfaButton,50,3,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminGuzelYaziSayfaButton,50,4,1,1,Qt::AlignHCenter);

    QLabel *cizgilitahta=new QLabel("Çizgili Tahta");   cizgilitahta->setFont(ffl);
    QLabel *karelitahta=new QLabel("Kareli Tahta"); karelitahta->setFont(ffl);
    QLabel *muziktahta=new QLabel("Müzik Tahta");      muziktahta->setFont(ffl);
    QLabel *gyazitahta=new QLabel("G.Yazı Tahta");    gyazitahta->setFont(ffl);

    layout->addWidget(cizgilitahta,60,1,1,1,Qt::AlignHCenter);
    layout->addWidget(karelitahta,60,2,1,1,Qt::AlignHCenter);
    layout->addWidget(muziktahta,60,3,1,1,Qt::AlignHCenter);
    layout->addWidget(gyazitahta,60,4,1,1,Qt::AlignHCenter);


    layout->addWidget(zeminTemizleSayfaButton,90,1,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminDesenEkleSayfaButton,90,2,1,1,Qt::AlignHCenter);
    layout->addWidget(gridRenkButton,90,3,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminRenkButton,90,4,1,1,Qt::AlignHCenter);
    QLabel *temizletahta=new QLabel("Tahta Temizle");   temizletahta->setFont(ffl);
    QLabel *desenekletahta=new QLabel("Desen Ekle");    desenekletahta->setFont(ffl);
    QLabel *cizgirengitahta=new QLabel("Çizgi Rengi");  cizgirengitahta->setFont(ffl);
    QLabel *zeminrengitahta=new QLabel("Zemin Rengi");  zeminrengitahta->setFont(ffl);

    layout->addWidget(temizletahta,91,1,1,1,Qt::AlignHCenter);
    layout->addWidget(desenekletahta,91,2,1,1,Qt::AlignHCenter);
    layout->addWidget(cizgirengitahta,91,3,1,1,Qt::AlignHCenter);
    layout->addWidget(zeminrengitahta,91,4,1,1,Qt::AlignHCenter);

    QWidget *cm=cizgiBoyutMenu();
    cm->setVisible(true);
    //cm->setFixedSize(220,cm->height());
    layout->addWidget(cm,95,1,1,4,Qt::AlignHCenter);
   // add a widget action to the context menu
    auto wa = new QWidgetAction(this);
    //  wa->setIcon(QIcon(":/icon1"));
    wa->setDefaultWidget(widget);
    menu->addAction(wa);

   return menu;
}

QPixmap KalemWidget::lineImage(const QPolygonF &myPolygon,const Qt::PenStyle &stl,int w,int h) const
{
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8,stl));
  // painter.translate(en, boy);
    painter.drawPolyline(myPolygon);
    return pixmap;
}

QPixmap KalemWidget::imageEllipse(const QPolygonF &myPolygon,int w,int h) const
{
    QRectF rectangle(myPolygon[0],myPolygon[1]);
   // QPainter painter(this);
   // painter.drawEllipse(rectangle);
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
  // painter.translate(en, boy);
    painter.drawEllipse(rectangle);
    return pixmap;
}

QPixmap KalemWidget::image(const QPolygonF &myPolygon,int w,int h) const
{
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
  // painter.translate(en, boy);
    painter.drawPolyline(myPolygon);
    return pixmap;
}

QPixmap KalemWidget::zeminImage(const QPolygonF &myPolygon, int w, int h, QColor color,int pensize) const
{
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(color, pensize));
  // painter.translate(en, boy);
    painter.drawPolyline(myPolygon);
    return pixmap;
}
void KalemWidget::sekilButtonIconSlot(DiagramItem::DiagramType mySekilType){
    DiagramItem *ditem=new DiagramItem();
    int ken=300;
    if(DiagramItem::DiagramType::Cember==mySekilType)
    {
        QPixmap pixmap(imageEllipse(ditem->sekilStore(mySekilType,QRectF(QPointF(20,50),QPointF(ken-50,ken-50))),ken,ken));
        sekilButton->setIcon(pixmap);
    }
    else  if(DiagramItem::DiagramType::Pergel==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/pergel.png")); }
    else  if(DiagramItem::DiagramType::Cetvel==mySekilType)   {sekilButton->setIcon(QIcon(":icons/icons/cetvel.png")); }
    else  if(DiagramItem::DiagramType::Gonye==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/gonye.png")); }
    else  if(DiagramItem::DiagramType::Iletki==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/iletki.png")); }
    else  if(DiagramItem::DiagramType::Kup==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/kup.png")); }
    else  if(DiagramItem::DiagramType::Silindir==mySekilType)   {sekilButton->setIcon(QIcon(":icons/icons/silindir.png")); }
    else  if(DiagramItem::DiagramType::Pramit==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/pramit.png")); }
    else  if(DiagramItem::DiagramType::Kure==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/kure.png")); }
    else  if(DiagramItem::DiagramType::NDogru==mySekilType)    {
        sekilButton->setIconSize(QSize(50,50));
        sekilButton->setIcon(QIcon(":icons/icons/ndogru.png"));}
    else  if(DiagramItem::DiagramType::NKoordinat==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/nkoordinat.png")); }
    else  if(DiagramItem::DiagramType::NNKoordinat==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/nnkoordinat.png")); }
    else  if(DiagramItem::DiagramType::HNKoordinat==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/hnkoordinat.png")); }
    else  if(DiagramItem::DiagramType::NoktaliKagit==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/noktalikagit.png")); }
    else  if(DiagramItem::DiagramType::IzometrikKagit==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/izometrikkagit.png")); }
    else  if(DiagramItem::DiagramType::KareliKagit==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/karelikagit.png")); }
    else  if(DiagramItem::DiagramType::CizgiliKagit==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/cizgilikagit.png")); }
    else  if(DiagramItem::DiagramType::Resim==mySekilType)    {sekilButton->setIcon(QIcon(":icons/icons/addimage.png")); }

    else
    {
        QPixmap pixmap(image(ditem->sekilStore(mySekilType,QRectF(QPointF(20,50),QPointF(ken-50,ken-50))),ken,ken));
        sekilButton->setIcon(pixmap);
    }
}
}
