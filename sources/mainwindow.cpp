/*

Copyright 2014-2015, 2018 S. Razi Alavizadeh
Copyright 2018 Marshall Banana
Copyright 2012-2018 Adam Reichold
Copyright 2018 Pavel Sanda
Copyright 2014 Dorian Scholz
Copyright 2018 Martin Spacek
Copyright 2012 Michał Trybus
Copyright 2012 Alexander Volkov

This file is part of qpdfview.

qpdfview is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

qpdfview is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with qpdfview.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mainwindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDockWidget>
#include <QDrag>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QScrollBar>
#include <QShortcut>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

#include <QStandardPaths>

#endif // QT_VERSION

#ifdef WITH_DBUS

#include <QDBusInterface>
#include <QDBusReply>

#endif // WITH_DBUS

#include "model.h"
#include "settings.h"
#include "shortcuthandler.h"
#include "thumbnailitem.h"
#include "searchmodel.h"
#include "searchitemdelegate.h"
#include "documentview.h"
#include "miscellaneous.h"
#include "printdialog.h"
#include "settingsdialog.h"
#include "fontsdialog.h"
#include "helpdialog.h"
#include "recentlyusedmenu.h"
#include "recentlyclosedmenu.h"
#include "bookmarkmodel.h"
#include "bookmarkmenu.h"
#include "bookmarkdialog.h"
#include "database.h"


namespace qpdfview
{

namespace
{

QModelIndex synchronizeOutlineView(int currentPage, const QAbstractItemModel* model, const QModelIndex& parent)
{
    for(int row = 0, rowCount = model->rowCount(parent); row < rowCount; ++row)
    {
        const QModelIndex index = model->index(row, 0, parent);

        bool ok = false;
        const int page = model->data(index, Model::Document::PageRole).toInt(&ok);

        if(ok && page == currentPage)
        {
            return index;
        }
    }

    for(int row = 0, rowCount = model->rowCount(parent); row < rowCount; ++row)
    {
        const QModelIndex index = model->index(row, 0, parent);

        const QModelIndex match = synchronizeOutlineView(currentPage, model, index);

        if(match.isValid())
        {
            return match;
        }
    }

    return QModelIndex();
}

inline void setToolButtonMenu(QToolBar* toolBar, QAction* action, QMenu* menu)
{
    if(QToolButton* toolButton = qobject_cast< QToolButton* >(toolBar->widgetForAction(action)))
    {
        toolButton->setMenu(menu);
    }
}

inline void setSectionResizeMode(QHeaderView* header, QHeaderView::ResizeMode mode)
{
    if(header->count() > 0)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

        header->setSectionResizeMode(mode);

#else

        header->setResizeMode(mode);

#endif // QT_VERSION
    }
}

inline void setSectionResizeMode(QHeaderView* header, int index, QHeaderView::ResizeMode mode)
{
    if(header->count() > index)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)

        header->setSectionResizeMode(index, mode);

#else

        header->setResizeMode(index, mode);

#endif // QT_VERSION
    }
}

inline QAction* createTemporaryAction(QObject* parent, const QString& text, const QString& objectName)
{
    QAction* action = new QAction(text, parent);

    action->setObjectName(objectName);

    return action;
}

void addWidgetActions(QWidget* widget, const QStringList& actionNames, const QList< QAction* >& actions)
{
    foreach(const QString& actionName, actionNames)
    {
        if(actionName == QLatin1String("separator"))
        {
            QAction* separator = new QAction(widget);
            separator->setSeparator(true);

            widget->addAction(separator);

            continue;
        }

        foreach(QAction* action, actions)
        {
            if(actionName == action->objectName())
            {
                widget->addAction(action);

                break;
            }
        }
    }
}

class SignalBlocker
{
public:
    SignalBlocker(QObject* object) : m_object(object)
    {
        m_object->blockSignals(true);
    }

    ~SignalBlocker()
    {
        m_object->blockSignals(false);
    }

private:
    Q_DISABLE_COPY(SignalBlocker)

    QObject* m_object;

};

DocumentView* findCurrentTab(QObject* const object)
{
    if(DocumentView* const tab = qobject_cast< DocumentView* >(object))
    {
        return tab;
    }

    if(Splitter* const splitter = qobject_cast< Splitter* >(object))
    {
        return findCurrentTab(splitter->currentWidget());
    }

    return 0;
}

QVector< DocumentView* > findAllTabs(QObject* const object)
{
    QVector< DocumentView* > tabs;

    if(DocumentView* const tab = qobject_cast< DocumentView* >(object))
    {
        tabs.append(tab);
    }

    if(Splitter* const splitter = qobject_cast< Splitter* >(object))
    {
        for(int index = 0, count = splitter->count(); index < count; ++index)
        {
            tabs += findAllTabs(splitter->widget(index));
        }
    }

    return tabs;
}

} // anonymous

class MainWindow::CurrentTabChangeBlocker
{
    Q_DISABLE_COPY(CurrentTabChangeBlocker)

private:
    MainWindow* const that;

public:
    CurrentTabChangeBlocker(MainWindow* const that) : that(that)
    {
        that->m_currentTabChangedBlocked = true;
    }

    ~CurrentTabChangeBlocker()
    {
        that->m_currentTabChangedBlocked = false;

        that->on_tabWidget_currentChanged();
    }

};

class MainWindow::RestoreTab : public Database::RestoreTab
{
private:
    MainWindow* const that;

public:
    RestoreTab(MainWindow* that) : that(that) {}

    DocumentView* operator()(const QString& absoluteFilePath) const
    {
        if(that->openInNewTab(absoluteFilePath, -1, QRectF(), true))
        {
            return that->currentTab();
        }
        else
        {
            return 0;
        }
    }

};

class MainWindow::TextValueMapper : public MappingSpinBox::TextValueMapper
{
private:
    MainWindow* const that;

public:
    TextValueMapper(MainWindow* that) : that(that) {}

    QString textFromValue(int val, bool& ok) const
    {
        const DocumentView* currentTab = that->currentTab();

        if(currentTab == 0 || !(currentTab->hasFrontMatter() || that->s_settings->mainWindow().usePageLabel()))
        {
            ok = false;
            return QString();
        }

        ok = true;
        return currentTab->pageLabelFromNumber(val);
    }

    int valueFromText(const QString& text, bool& ok) const
    {
        const DocumentView* currentTab = that->currentTab();

        if(currentTab == 0 || !(currentTab->hasFrontMatter() || that->s_settings->mainWindow().usePageLabel()))
        {
            ok = false;
            return 0;
        }

        const QString& prefix = that->m_currentPageSpinBox->prefix();
        const QString& suffix = that->m_currentPageSpinBox->suffix();

        int from = 0;
        int size = text.size();

        if(!prefix.isEmpty() && text.startsWith(prefix))
        {
            from += prefix.size();
            size -= from;
        }

        if(!suffix.isEmpty() && text.endsWith(suffix))
        {
            size -= suffix.size();
        }

        const QString& trimmedText = text.mid(from, size).trimmed();

        ok = true;
        return currentTab->pageNumberFromLabel(trimmedText);
    }

};

Settings* MainWindow::s_settings = 0;
Database* MainWindow::s_database = 0;
ShortcutHandler* MainWindow::s_shortcutHandler = 0;
SearchModel* MainWindow::s_searchModel = 0;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
    m_outlineView(0),
    m_thumbnailsView(0)
{

    if(s_settings == 0)
    {
        s_settings = Settings::instance();
    }

    if(s_shortcutHandler == 0)
    {
        s_shortcutHandler = ShortcutHandler::instance();
    }

    if(s_searchModel == 0)
    {
        s_searchModel = SearchModel::instance();
    }

    prepareStyle();

    setAcceptDrops(true);

    createWidgets();
    createActions();
    createToolBars();
    createDocks();
    createMenus();
    restoreGeometry(s_settings->mainWindow().geometry());
    restoreState(s_settings->mainWindow().state());

   prepareDatabase();


   /***********************************************/
  /*   QPixmap newpix(":icons/icons/move.png");
     QPixmap openpix(":icons/icons/zoomselect.png");
     QPixmap quitpix(":icons/icons/pen.png");

     QToolBar *toolbar = addToolBar("main toolbar");
     toolbar->addAction(QIcon(newpix), "New File");
     toolbar->addAction(QIcon(openpix), "Open File");
     toolbar->addSeparator();

     QAction *quit = toolbar->addAction(QIcon(quitpix),
         "Quit Application");

     connect(quit, &QAction::triggered, qApp, &QApplication::quit);

     toolbar->setOrientation(Qt::Horizontal);//Qt::Vertical
     addToolBar(Qt::BottomToolBarArea, toolbar);
    /* LeftToolBarArea = 0x1,
         RightToolBarArea = 0x2,
         TopToolBarArea = 0x4,
         BottomToolBarArea = 0x8,
         ToolBarArea_Mask = 0xf,
         AllToolBarAreas = ToolBarArea_Mask,
         NoToolBarArea = 0
   /******************************************/
  // qDebug()<<"screen1"<<QApplication::desktop()->screenGeometry();
  // qDebug()<<"screen2"<<QApplication::desktop()->availableGeometry();

    QRect r1=QApplication::desktop()->screenGeometry();
   QRect r2=QApplication::desktop()->availableGeometry();
   int x=r1.x()-r2.x();
   int y=r1.y()-r2.y();
   int w=r1.width()-r2.width();
   int h=r1.height()-r2.height();

  // qDebug()<<"screenx"<<x<<y<<w<<h;

   screenSize = qApp->screens()[0]->size();
   boy=(screenSize.height()/100)*5.70;
   en=boy;


    kw=new KalemWidget(en*1/3*2-2,boy*0.8,this);
  //  kw->setFixedSize(en*1-1,boy*0.8*19.7);

   // kw->move(screenSize.width()-en*1-10-en+x,screenSize.height()/2- kw->height()/2+y);
    kw->setAttribute(Qt::WA_StyledBackground, true);
 ///   kw->setStyleSheet("background-color:argb(65, 193, 244, 255);");

    kw->setOrientation(Qt::Vertical);//Qt::Vertical Horizontal
    addToolBar(Qt::RightToolBarArea, kw);
    //addToolBar(Qt::TopToolBarArea, kw);

    connect(kw, SIGNAL(kalemModeSignal(Scene::Mode,DiagramItem::DiagramType)),
            this, SLOT(kalemModeSignalSlot(Scene::Mode,DiagramItem::DiagramType)));

    connect(kw, SIGNAL(kalemColorSignal(QString,QColor)),
            this, SLOT(kalemColorSignalSlot(QString,QColor)));

    connect(kw, SIGNAL(kalemPenModeSignal(DiagramItem::DiagramType)),
            this, SLOT(kalemPenModeSignalSlot(DiagramItem::DiagramType)));

    connect(kw, SIGNAL(kalemZeminModeSignal(DiagramItem::DiagramType)),
            this, SLOT(kalemZeminModeSignalSlot(DiagramItem::DiagramType)));

    connect(kw, SIGNAL(kalemSekilModeSignal(DiagramItem::DiagramType)),
            this, SLOT(kalemSekilModeSignalSlot(DiagramItem::DiagramType)));

   /// zKitapLoad();//dosya yükleme

    SettingsDialog *setting= new SettingsDialog(this);
    setting->reset();
    setting->accept();
    s_settings->pageItem().setCacheSize(524288);
    prepareDatabase();

    m_fileToolBar->setVisible(true);   //dosya toolbar gizleniyor
    m_editToolBar->setVisible(false);   //edit toolbar gizleniyor
    m_viewToolBar->setVisible(false);   //view toolbar gizleniyor
    menuBar()->setVisible(false);       //Menu gizleniyor

setWindowTitle("PDF OKUYUCU");
   /* setWindowTitle("ABC Yayıncılık");
    QPixmap logopixmap(":icons/icons/logo.png");
    QLabel *lg=new QLabel(this);
    lg->setFixedSize(100,75);
    lg->setPixmap(logopixmap);
    lg->setScaledContents(true);
    lg->move(screenSize.width()-lg->width()-w-en*1.3,en*1.3);
    //lg->show();
    lg->setStyleSheet("background-color: rgba(255,255,255,0);");
*/
}
void MainWindow::kalemModeSignalSlot(Scene::Mode mode,DiagramItem::DiagramType type)
{
     /// qDebug()<<"kalem mode:"<<mode<<type;
    kw->oldMode=kw->currentMode;
    kw->currentMode=mode;

    kw->oldType=kw->currentType;
    kw->currentType=type;

    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;
     currentTab()->setDragMode(QGraphicsView::NoDrag);
     foreach(QGraphicsItem* item, currentTab()->scene->items()){
         VERectangle * selection = dynamic_cast<VERectangle *>(item);
         if(selection)
         {
             if(selection->sekilTr==DiagramItem::DiagramType::PatternPage){

                 if(currentTab()->currentPage()-1==selection->pageOfNumber)
                 {
                     // qDebug()<<"arka plan";
                     selection->setVisible(true);
                     currentTab()->scene->update();
                 }
             }
         }
     }
     /****************************************************************/
     page->copyMode=false;
     on_copyToClipboardMode_triggered(false);

    if(Scene::Mode::DrawPen==mode) kalemPenModeSignalSlot(type);
    if(Scene::Mode::SekilMode==mode) kalemSekilModeSignalSlot(type);
    if(Scene::Mode::ZeminMode==mode) kalemZeminModeSignalSlot(type);
    if(Scene::Mode::SelectObject==mode) slotHand();
    if(Scene::Mode::EraseMode==mode) slotErase();
    if(Scene::Mode::ClearMode==mode) slotClear();
    if(Scene::Mode::CopyMode==mode) slotCopy();
    if(Scene::Mode::NextPageMode==mode) {
        on_nextPage_triggered();
        }
    if(Scene::Mode::BackPageMode==mode){
        on_previousPage_triggered();
     }
    if(Scene::Mode::ZoomPozitifMode==mode){
        PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
            const QRectF pageRect = page->boundingRect().translated(page->pos());
        //qDebug()<<"1. ölçek:"<<pageRect.height();

        on_zoomIn_triggered();
        const QRectF pageRect1 = page->boundingRect().translated(page->pos());
        //qDebug()<<"2. ölçek:"<<pageRect1.height();
        // qDebug()<<"ölçek:"<<pageRect1.height()/pageRect.height();
        slotScaleSceneItem(pageRect1.width()/pageRect.width(),pageRect1.height()/pageRect.height());
      // kw->handButtonSlot();
    }
    if(Scene::Mode::ZoomNegatifMode==mode) {


        PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
            const QRectF pageRect = page->boundingRect().translated(page->pos());
        //qDebug()<<"1. ölçek:"<<pageRect.height();

         on_zoomOut_triggered();
        const QRectF pageRect1 = page->boundingRect().translated(page->pos());
        //qDebug()<<"2. ölçek:"<<pageRect1.height();
         //qDebug()<<"ölçek:"<<pageRect1.height()/pageRect.height();
        slotScaleSceneItem(pageRect1.width()/pageRect.width(),pageRect1.height()/pageRect.height());


    }
    if(Scene::Mode::ZoomSelectionMode==mode) {
        currentTab()->scene->sceneMode=Scene::Mode::ZoomSelectionMode;
        PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
        const QRectF pageRect = page->boundingRect().translated(page->pos());

        //currentTab()->setContinuousMode(false);     //sayfalar birleşik gözükmeyecek
       // currentTab()->setLayoutMode(SinglePageMode);//Tek sayfa gösterme ayarlandı
         on_fitToPageSizeMode_triggered(true);

        const QRectF pageRect1 = page->boundingRect().translated(page->pos());
        slotScaleSceneItem(pageRect1.width()/pageRect.width(),pageRect1.height()/pageRect.height());

      //  PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
        page->zoomSelectMode=true;
        on_copyToClipboardMode_triggered(true);
        foreach(QGraphicsItem* item, currentTab()->scene->items()){
            VERectangle * selection = dynamic_cast<VERectangle *>(item);
            if(selection)
            {
                if(selection->sekilTr==DiagramItem::DiagramType::PatternPage){
                    // qDebug()<<"arka plan";
                    if(currentTab()->currentPage()-1==selection->pageOfNumber)
                        selection->setVisible(false);
                    update();
                }
            }
        }
    }
    if(Scene::Mode::FitWindowMode==mode){


         PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
         const QRectF pageRect = page->boundingRect().translated(page->pos());

         //currentTab()->setContinuousMode(false);     //sayfalar birleşik gözükmeyecek
        // currentTab()->setLayoutMode(SinglePageMode);//Tek sayfa gösterme ayarlandı
          on_fitToPageSizeMode_triggered(true);

         const QRectF pageRect1 = page->boundingRect().translated(page->pos());
         slotScaleSceneItem(pageRect1.width()/pageRect.width(),pageRect1.height()/pageRect.height());
//currentTab()->scene->update();

    }
    if(Scene::Mode::FitPageMode==mode){

       PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
       const QRectF pageRect = page->boundingRect().translated(page->pos());

      // currentTab()->setContinuousMode(false);     //sayfalar birleşik gözükmeyecek
      // currentTab()->setLayoutMode(SinglePageMode);//Tek sayfa gösterme ayarlandı
      on_fitToPageWidthMode_triggered(true);

       const QRectF pageRect1 = page->boundingRect().translated(page->pos());
       slotScaleSceneItem(pageRect1.width()/pageRect.width(),pageRect1.height()/pageRect.height());
        ///currentTab()->scene->update();
       // qDebug()<<"aaaaa";
      // kalemModeSignalSlot(Scene::Mode::NoMode,DiagramItem::DiagramType::NoType);
      // kw->buttonStateClear();
      // kw->handButton->setChecked(true);
       currentTab()->scene->update();
       page->update();

     }
    if(Scene::Mode::IleriAlMode==mode){
      // qDebug()<<"ileri al";
      Scene::Mode tempmode=currentTab()->scene->sceneMode;
      currentTab()->scene->setMode(Scene::Mode::IleriAlMode, DiagramItem::DiagramType::NoType);
      sceneItemAddedSignalSlot();
     currentTab()->scene->sceneMode=tempmode;

     }
    if(Scene::Mode::GeriAlMode==mode){
         // qDebug()<<"geri al";
        Scene::Mode tempmode=currentTab()->scene->sceneMode;
        currentTab()->scene->setMode(Scene::Mode::GeriAlMode, DiagramItem::DiagramType::NoType);
        sceneItemAddedSignalSlot();
        currentTab()->scene->sceneMode=tempmode;
      }
    if(Scene::Mode::JumpPageMode==mode) slotJumpPage();
    if(Scene::Mode::SaveMode==mode) on_saveAs_triggered();
    if(Scene::Mode::PrintMode==mode) on_print_triggered();
    if(Scene::Mode::ListMode==mode) {
        if(m_thumbnailsDock->isVisible())
             m_thumbnailsDock->hide();
        else
                m_thumbnailsDock->show();

    }
    if(Scene::Mode::SearchMode==mode) {
   /* if(m_searchDock->isVisible())
         m_searchDock->hide();
    else
            m_searchDock->show();
*/
        slotSearch();
    }

    if(Scene::Mode::PanelSideLeftRight==mode) {
        if (kw->panelSide=="Right"){
           addToolBar(Qt::LeftToolBarArea, kw);
            kw->panelSide="Left";
            return;
        }
        if (kw->panelSide=="Left"){
           addToolBar(Qt::RightToolBarArea, kw);
            kw->panelSide="Right";
            return;
        }

        if (kw->panelSide=="Bottom"||kw->panelSide=="Top"){
            addToolBar(Qt::RightToolBarArea, kw);
            kw->panelSide="Right";
            return;
        }
       // qDebug()<<"sağsol hizala";
       // kw->setOrientation(Qt::Vertical);//Qt::Vertical Horizontal
       // addToolBar(Qt::RightToolBarArea, kw);
        //kw->setAllowedAreas(Qt::RightToolBarArea);

    }
    if(Scene::Mode::PanelSideBottom==mode) {
        if (kw->panelSide=="Left"||kw->panelSide=="Right"){
           addToolBar(Qt::BottomToolBarArea, kw);
           kw->panelSide="Bottom";
           return;
        }
        if (kw->panelSide=="Bottom"){
            addToolBar(Qt::TopToolBarArea, kw);
            kw->panelSide="Top";
            return;
        }
        if (kw->panelSide=="Top"){
            addToolBar(Qt::BottomToolBarArea, kw);
            kw->panelSide="Bottom";
            return;
        }
          // qDebug()<<"aşağı hizala";
          // addToolBar(Qt::LeftToolBarArea, kw);
         //kw->setAllowedAreas(Qt::LeftToolBarArea);
    }
    if(Scene::Mode::ExitMode==mode){
        QMainWindow::close();
       }
    slotScaleSceneItem(1,1);
    kw->pageOfNumber->setText(QString::number(currentTab()->currentPage())+"/"+
                             QString::number(currentTab()->numberOfPages()));

 if(Scene::Mode::ZoomSelectionMode!=mode||
         Scene::Mode::SekilMode!=mode)
     kw->modeKontrolSlot();
}
void MainWindow::slotErase()
{

      currentTab()->scene->setEraseSize(kw->penSize*2);
    currentTab()->scene->sceneMode=Scene::Mode::EraseMode;
    currentTab()->scene->mySekilType=DiagramItem::DiagramType::NoType;

}
void MainWindow::slotClear()
{
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;

    currentTab()->scene->sceneMode=Scene::Mode::ClearMode;
    currentTab()->scene->mySekilType=DiagramItem::DiagramType::NoType;
    currentTab()->scene->removeAllItem();
///qDebug()<<"sil*****";

}
void MainWindow::slotCopy()
{
    currentTab()->scene->sceneMode=Scene::Mode::CopyMode;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    page->copyMode=true;
    on_copyToClipboardMode_triggered(true);
    foreach(QGraphicsItem* item, currentTab()->scene->items()){
        VERectangle * selection = dynamic_cast<VERectangle *>(item);
        if(selection)
        {
            if(selection->sekilTr==DiagramItem::DiagramType::PatternPage){
                // qDebug()<<"arka plan";
                if(currentTab()->currentPage()-1==selection->pageOfNumber)
                    selection->setVisible(false);
                update();
            }
        }
    }
    qDebug()<<"copy çalıştı";
}
void MainWindow::slotScaleSceneItem(double sizex,double sizey)
{
    foreach(QGraphicsItem* item, currentTab()->scene->items()){
        VERectangle * selection = dynamic_cast<VERectangle *>(item);
        if(selection)
        {
            if(selection->sekilTr!=0){
              //  PageItem* page =currentTab()->m_pageItems.at((selection->pageOfNumber));
                //const QRectF pageRect = page->boundingRect().translated(page->pos());

                //qDebug()<<"currentpage:"<<currentTab()->currentPage()-1<<selection->pageOfNumber;
                ///if(currentTab()->currentPage()-1==selection->pageOfNumber)
              ///      selection->setVisible(true);
               /// else selection->setVisible(false);
                // qDebug()<<"render param:"<<page->m_renderParam.resolutionX() * page->m_renderParam.scaleFactor() / 72.0;
                selection->setRect(0,0,selection->rect().width()*sizex,selection->rect().height()*sizey);
                selection->setPos(selection->pos().x()*sizex,selection->pos().y()*sizey);
            }

            if(selection->sekilTr!=0&&

             selection->pageOfNumber==currentTab()->scene->pageOfNumberScene&&
             selection->sekilTr==DiagramItem::DiagramType::PatternPage){
             PageItem* page =currentTab()->m_pageItems.at((selection->pageOfNumber));
             const QRectF pageItemRect = page->boundingRect().translated(page->pos());

            /// selection->setImage(currentTab()->scene->myImage);
             selection->setRect(0,0,pageItemRect.width(),pageItemRect.height());
             selection->setPos(pageItemRect.left(),pageItemRect.top());

             selection->update();
             }
        }
     }

currentTab()->update();
}
void MainWindow::slotHand()
{
   // qDebug()<<"hand";
   currentTab()->scene->sceneMode=Scene::Mode::SelectObject;
   currentTab()->scene->mySekilType=DiagramItem::DiagramType::NoType;
   currentTab()->setDragMode(QGraphicsView::ScrollHandDrag);
   currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
   PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
   const QRectF pageItemRect = page->boundingRect().translated(page->pos());
   currentTab()->scene->pageItemRect=pageItemRect;

   foreach(QGraphicsItem* item, currentTab()->scene->items()){
       VERectangle * selection = dynamic_cast<VERectangle *>(item);
       if(selection)
       {
           if(selection->sekilTr==DiagramItem::DiagramType::PatternPage){
              // qDebug()<<"arka plan";
               if(currentTab()->currentPage()-1==selection->pageOfNumber)
                selection->setVisible(false);
                update();
              }
       }
    }
//    tvl->hide();

}
void MainWindow::slotPenInit()
{  ///currentTab()->scene->sceneMode=Scene::Mode::DrawPen;

    currentTab()->scene->setPenSize(kw->penSize);
    currentTab()->scene->setPenAlpha(kw->penAlpha);
    currentTab()->scene->setPenColor(kw->penColor);
    currentTab()->scene->setPenStyle(kw->penStyle);

    currentTab()->scene->mySekilType=DiagramItem::DiagramType::NoType;
    currentTab()->scene->setSekilKalemColor(kw->penColor);
    currentTab()->scene->mySekilPenStyle=kw->penStyle;
    kw->sekilZeminColor=QColor(255,255,255,0);
    currentTab()->scene->setSekilZeminColor(kw->sekilZeminColor);
    currentTab()->scene->setSekilPenSize(kw->penSize);

    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    currentTab()->scene->setSekilTanimlamaStatus(false);
    currentTab()->scene->pageOfNumberScene=0;
    /*******************önemli ayarlar**************************************/
    currentTab()->setContinuousMode(true);     //sayfalar birleşik gözükmeyecek
    currentTab()->setLayoutMode(SinglePageMode);//Tek sayfa gösterme ayarlandı

    //currentTab()->setScaleMode(FitToPageWidthMode);//sayfayı orijinal boyutta göster
    currentTab()->setScaleMode(FitToPageSizeMode);//sayfayı sığdır fit to window
    kw->pageOfNumber->setText(QString::number(currentTab()->currentPage())+" / "+
                             QString::number(currentTab()->numberOfPages()));
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;
    kalemZeminModeSignalSlot(DiagramItem::DiagramType::TransparanPage);
    currentTab()->scene->makeItemsControllable(false);

    connect(currentTab()->scene, SIGNAL(sceneItemAddedSignal()),
    this,SLOT(sceneItemAddedSignalSlot()));
    sceneItemAddedSignalSlot();
    kw->handButtonSlot();
    //kalemPenModeSignalSlot(DiagramItem::DiagramType::NormalPen);


    //slotHand();
   //  currentTab()->scene->update();
}
void  MainWindow::kalemPenModeSignalSlot(DiagramItem::DiagramType type)
{
   // qDebug()<<"pen mode:"<<type;
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;
    //if(DiagramItem::DiagramType::SolidLine==type)  currentTab()->scene->setSekilPenStyle(Qt::SolidLine);
    //else if(DiagramItem::DiagramType::DashLine==type)  currentTab()->scene->setSekilPenStyle(Qt::DashLine);
   // else if(DiagramItem::DiagramType::DotLine==type)  currentTab()->scene->setSekilPenStyle(Qt::DotLine);

    if(DiagramItem::DiagramType::SolidLine==type){
        currentTab()->scene->setSekilPenStyle(Qt::SolidLine);
        currentTab()->scene->setPenStyle(Qt::SolidLine);
    }
    if(DiagramItem::DiagramType::DashLine==type){
        currentTab()->scene->setPenStyle(Qt::DashLine);
        currentTab()->scene->setSekilPenStyle(Qt::DashLine);
    }
    if(DiagramItem::DiagramType::DotLine==type){
        currentTab()->scene->setPenStyle(Qt::DotLine);
        currentTab()->scene->setSekilPenStyle(Qt::DotLine);
    }

    if(DiagramItem::DiagramType::PenSize2==type){kw->penSize=2; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize4==type){kw->penSize=4; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize6==type){kw->penSize=6; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize8==type){kw->penSize=8; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize10==type){kw->penSize=10; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize12==type){kw->penSize=12; currentTab()->scene->setPenSize(kw->penSize);}
    if(DiagramItem::DiagramType::PenSize14==type){kw->penSize=14; currentTab()->scene->setPenSize(kw->penSize);}


    if(DiagramItem::DiagramType::NormalPen==type){
        currentTab()->scene->sceneMode=Scene::Mode::DrawPen;
        currentTab()->scene->setSekilTanimlamaStatus(false);
    }
    if(DiagramItem::DiagramType::SmartPen==type){
        currentTab()->scene->sceneMode=Scene::Mode::DrawPen;
        currentTab()->scene->setSekilTanimlamaStatus(true);
    }
    if(DiagramItem::DiagramType::FosforluPen==type){
        currentTab()->scene->sceneMode=Scene::Mode::DrawPenFosfor;
        currentTab()->scene->setSekilTanimlamaStatus(false);
         currentTab()->scene->setPenAlpha(50);

    }

}
void MainWindow::kalemZeminModeSignalSlot(DiagramItem::DiagramType type)
{

//qDebug()<<"zemin mode:"<<type<<kw->penSize;
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;
    currentTab()->scene->setSekilZeminColor(kw->sekilZeminColor);
    currentTab()->scene->setSekilPenSize(kw->penSize);
    currentTab()->scene->setSekilKalemColor(kw->penColor);

if(DiagramItem::DiagramType::NDogru==type){ currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::NKoordinat==type){ currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::NNKoordinat==type){ currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::HNKoordinat==type){ currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::NoktaliKagit==type) {currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::IzometrikKagit==type){ currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::KareliKagit==type) {currentTab()->scene->donSlot(type);return;}
if(DiagramItem::DiagramType::CizgiliKagit==type) {currentTab()->scene->donSlot(type);return;}


if(DiagramItem::DiagramType::TransparanPage==type){
    kw->pagePattern=type;
    kw->sekilZeminColor=QColor(0,0,0,0);
    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*10,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage= bkgnd;
   // currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);

  }
if(DiagramItem::DiagramType::BlackPage==type){
    kw->pagePattern=type;
    kw->sekilZeminColor=QColor(0,0,0,255);

    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*10,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi, kw->sekilZeminColor, kw->penColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
  //  currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);
  }
if(DiagramItem::DiagramType::WhitePage==type){
    kw->pagePattern=type;
    kw->sekilZeminColor=QColor(255,255,255,255);
    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
   // currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

 }
if(DiagramItem::DiagramType::CustomColorPage==type){
    kw->pagePattern=type;
    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*10,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
   // currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

}
if(DiagramItem::DiagramType::CizgiliPage==type){
    kw->pagePattern=type;
    currentTab()->scene->sceneGridYatay=true;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->zeminGridColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
   // currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

  }
if(DiagramItem::DiagramType::KareliPage==type){
    kw->pagePattern=type;
    currentTab()->scene->sceneGridYatay=true;
    currentTab()->scene->sceneGridDikey=true;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->zeminGridColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
   // currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

  }
if(DiagramItem::DiagramType::MuzikPage==type){
    kw->pagePattern=type;
    currentTab()->scene->sceneGridYatay=true;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=false;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    DiagramItem *ditem=new DiagramItem();
    QPixmap pim(kw->zeminImage(ditem->sekilStore(DiagramItem::DiagramType::Muzik,QRectF(QPointF(0,0),QPointF(screenSize.width(),screenSize.height()))),screenSize.width(),screenSize.height(),kw->zeminGridColor,2));
    currentTab()->scene->myImage=pim;
    //currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

  }
if(DiagramItem::DiagramType::GuzelYaziPage==type){
    kw->pagePattern=type;
    currentTab()->scene->sceneGridYatay=false;
    currentTab()->scene->sceneGridDikey=false;
    currentTab()->scene->sceneGuzelYazi=true;
    ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
    QSize screenSize = qApp->screens()[0]->size();
    GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
    QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
    currentTab()->scene->myImage=bkgnd;
    //currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);
    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

  }

if(DiagramItem::DiagramType::CustomImagePage==type)
{ kw->pagePattern=type;

   /* Qt::WindowFlags flags = 0;
    flags |= Qt::Window;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::CustomizeWindowHint;
    this->setWindowFlags(flags);

    QFileDialog abc;
   /// abc.setSidebarUrls(urls);
    abc.setFileMode(QFileDialog::AnyFile);

    flags |= Qt::SplashScreen;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::WindowStaysOnTopHint;
    // */
QFileDialog abc;
   // abc.setWindowFlags(flags);
    QString os="";
#ifdef WIN32
    // Windows code here
    os="windows";
#else
    // UNIX code here
    os="linux";
#endif

    if(os=="linux"){
        //qDebug()<<"linux fileopen";
        abc.setWindowTitle("Resim Aç jpg png bmp");
        abc.setDirectory(QDir::homePath()+"/Masaüstü");
        abc.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
        if(abc.exec()) {
            if(abc.selectedFiles()[0]!="")
            {
                QPixmap image = QPixmap(abc.selectedFiles()[0]);

               /* currentTab()->scene->sceneGridYatay=false;
                currentTab()->scene->sceneGridDikey=false;
                currentTab()->scene->sceneGuzelYazi=true;
                ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
                //QSize screenSize = qApp->screens()[0]->size();
                //GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
               */// QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
                currentTab()->scene->myImage=image;
                //currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);

               // currentTab()->scene->setImage(image);
                 //currentTab()->scene->donSlot(type);


            }
        }

    }
    else
    {
        //qDebug()<<"windows fileopen";
        abc.setWindowTitle("Resim Aç jpg png bmp");
        abc.setDirectory(QDir::homePath()+"/desktop");
        abc.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
        if(abc.exec()) {
            if(abc.selectedFiles()[0]!="")
            {
                QPixmap image = QPixmap(abc.selectedFiles()[0]);

                /*currentTab()->scene->sceneGridYatay=false;
                currentTab()->scene->sceneGridDikey=false;
                currentTab()->scene->sceneGuzelYazi=true;
                ///qDebug()<<myGridSize<<gridYatay<<gridDikey<<guzelYazi<<myZeminColor<< myGridColor;
                QSize screenSize = qApp->screens()[0]->size();
                //GridLines *gridLines = new GridLines (screenSize.width(), screenSize.height(),kw->gridSize*3.4,currentTab()->scene->sceneGridYatay,currentTab()->scene->sceneGridDikey,currentTab()->scene->sceneGuzelYazi,kw->sekilZeminColor, kw->penColor);
               */// QPixmap bkgnd=gridLines->PixItem(gridLines,screenSize.width(),screenSize.height());
                currentTab()->scene->myImage=image;
                //currentTab()->scene->donSlot(DiagramItem::DiagramType::PatternPage);

               // currentTab()->scene->setImage(image);
                // currentTab()->scene->donSlot(type);

            }
        }

    }

   /* flags |= Qt::Window;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::WindowStaysOnTopHint;
    this->setWindowFlags(flags);
    show();*/
   // currentTab()->scene->mySekilType=type;

    kalemModeSignalSlot(Scene::Mode::DrawPen,DiagramItem::DiagramType::NoType);
    kw->buttonStateClear();
    kw->penButton->setChecked(true);

}

bool pageadd=false;
foreach(QGraphicsItem* item, currentTab()->scene->items()){
         VERectangle * selection = dynamic_cast<VERectangle *>(item);
          if(selection)
         {
            if(selection->sekilTr!=0&&
             selection->pageOfNumber==currentTab()->scene->pageOfNumberScene&&
             selection->sekilTr==DiagramItem::DiagramType::PatternPage){

             pageadd=true;
             selection->setImage(currentTab()->scene->myImage);
           //  qDebug()<<"zemin..x";
             selection->setRect(0,0,pageItemRect.width(),pageItemRect.height());
             selection->setPos(pageItemRect.left(),pageItemRect.top());

             selection->update();
             }
         }
     }

if(!pageadd)

{
    // qDebug()<<"zemin..0";
   VERectangle *zeminItem = new VERectangle(currentTab()->scene);
   zeminItem->setPen(QPen(currentTab()->scene->mySekilKalemColor, kw->penSize, currentTab()->scene->mySekilPenStyle));

   zeminItem->sekilTur(DiagramItem::DiagramType::PatternPage);
   zeminItem->setImage(currentTab()->scene->myImage);
   currentTab()->scene->addItem(zeminItem);
   zeminItem->pageOfNumber=currentTab()->scene->pageOfNumberScene;
   currentTab()->scene->graphicsList.append(zeminItem);
   currentTab()->scene->graphicsListTemp.append(zeminItem);
   currentTab()->scene->historyBack.append(zeminItem);
   currentTab()->scene->historyBackAction.append("added");
   zeminItem->setRect(0,0,pageItemRect.width(),pageItemRect.height());
   zeminItem->setPos(pageItemRect.left(),pageItemRect.top());
  // zeminItem->setSelected(false);
}



}
void MainWindow::kalemSekilModeSignalSlot(DiagramItem::DiagramType type){
  //  qDebug()<<"sekil butonclick"<<type;
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;

    kw->sekilButtonIconSlot(type);//şekil buton iconu ayarlanıyor
    currentTab()->scene->setSekilPenSize(kw->penSize);
    currentTab()->scene->setSekilKalemColor(kw->penColor);
    currentTab()->scene->setSekilZeminColor(kw->sekilZeminColor);
    currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;


    //if(DiagramItem::DiagramType::SolidLine==type)  currentTab()->scene->setSekilPenStyle(Qt::SolidLine);
    //else if(DiagramItem::DiagramType::DashLine==type)  currentTab()->scene->setSekilPenStyle(Qt::DashLine);
   // else if(DiagramItem::DiagramType::DotLine==type)  currentTab()->scene->setSekilPenStyle(Qt::DotLine);
    if(DiagramItem::DiagramType::Pergel==type) currentTab()->scene->donSlot(type);
    else if(DiagramItem::DiagramType::Cetvel==type) currentTab()->scene->donSlot(type);
    else if(DiagramItem::DiagramType::Gonye==type) currentTab()->scene->donSlot(type);
    else if(DiagramItem::DiagramType::Iletki==type) currentTab()->scene->donSlot(type);
    else if(DiagramItem::DiagramType::Kure==type)
    {
        currentTab()->scene->mySekilType=type;
        currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;
        currentTab()->scene->myImage=QPixmap(":icons/icons/kure.png");
      }
    else if(DiagramItem::DiagramType::Silindir==type)
    {
        currentTab()->scene->mySekilType=type;
        currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;
        currentTab()->scene->myImage=QPixmap(":icons/icons/silindir.png");
      }
    else if(DiagramItem::DiagramType::Kup==type)
    {
        currentTab()->scene->mySekilType=type;
        currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;
        currentTab()->scene->myImage=QPixmap(":icons/icons/kup.png");
      }
    else if(DiagramItem::DiagramType::Pramit==type)
    {
        currentTab()->scene->mySekilType=type;
        currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;
        currentTab()->scene->myImage=QPixmap(":icons/icons/pramit.png");
      }

    else if(DiagramItem::DiagramType::Resim==type)
    {
        Qt::WindowFlags flags = 0;
        flags |= Qt::Window;
        flags |= Qt::X11BypassWindowManagerHint;
        flags |= Qt::CustomizeWindowHint;
        this->setWindowFlags(flags);

        QFileDialog abc;
       /// abc.setSidebarUrls(urls);
        abc.setFileMode(QFileDialog::AnyFile);

        flags |= Qt::SplashScreen;
        flags |= Qt::X11BypassWindowManagerHint;
        flags |= Qt::WindowStaysOnTopHint;
        // QFileDialog abc;
        abc.setWindowFlags(flags);
        QString os="";
    #ifdef WIN32
        // Windows code here
        os="windows";
    #else
        // UNIX code here
        os="linux";
    #endif

        if(os=="linux"){
            //qDebug()<<"linux fileopen";
            abc.setWindowTitle("Resim Aç jpg png bmp");
            abc.setDirectory(QDir::homePath()+"/Masaüstü");
            abc.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
            if(abc.exec()) {
                if(abc.selectedFiles()[0]!="")
                {
                    QPixmap image = QPixmap(abc.selectedFiles()[0]);
                   currentTab()->scene->setImage(image);
                    currentTab()->scene->donSlot(type);


                }
            }

        }
        else
        {
            //qDebug()<<"windows fileopen";
            abc.setWindowTitle("Resim Aç jpg png bmp");
            abc.setDirectory(QDir::homePath()+"/desktop");
            abc.setNameFilter(tr("Image Files (*.png *.jpg *.bmp)"));
            if(abc.exec()) {
                if(abc.selectedFiles()[0]!="")
                {
                    QPixmap image = QPixmap(abc.selectedFiles()[0]);
                    currentTab()->scene->setImage(image);
                    currentTab()->scene->donSlot(type);

                }
            }

        }

        flags |= Qt::Window;
        flags |= Qt::X11BypassWindowManagerHint;
        flags |= Qt::WindowStaysOnTopHint;
        this->setWindowFlags(flags);
        show();
        currentTab()->scene->mySekilType=type;
    }

    else currentTab()->scene->mySekilType=type;

//qDebug()<<"buraya geldinn..";
//kw->modeKontrolSlot();

}
void MainWindow::kalemColorSignalSlot(QString colorType, QColor color)
{
 // qDebug()<<"colorsignal slot"<<colorType<<color;
    currentTab()->scene->pageOfNumberScene=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
    const QRectF pageItemRect = page->boundingRect().translated(page->pos());
    currentTab()->scene->pageItemRect=pageItemRect;

  if (colorType=="penColor"){
     // qDebug()<<"penColor";
     kw->penColor=color;
     currentTab()->scene->setSekilKalemColor( kw->penColor);
     currentTab()->scene->setPenColor( kw->penColor);
     kw->penColorButton->setStyleSheet(QString("QToolButton{ background: %1; }").arg(kw->penColor.name()));
  }
  if (colorType=="sekilZeminColor"){
     //qDebug()<<"sekilZeminColor"<<color;
     kw->sekilZeminColor=color;
     currentTab()->scene->setSekilZeminColor(kw->sekilZeminColor);
   }


  if (colorType=="zeminGridColor"){
     //qDebug()<<"sekilZeminColor"<<color;
     kw->zeminGridColor=color;
     //currentTab()->scene->setSekilZeminColor(kw->sekilZeminColor);
   }

  /* if(colorType=="pencolor") setPenColor(renk);
   if(colorType=="mySekilKalemColor")  mySekilKalemColor=renk;
   if(colorType=="mySekilZeminColor")  mySekilZeminColor=renk;
   if(colorType=="myZeminColor")  myZeminColor=renk;
   if(colorType=="myGridColor")  myGridColor=renk;*/

}
void MainWindow::mainClipBoard(QPoint pos,bool copy,bool zoom)
{

    if(copy) selectCopySlot(pos);///Kopyalama
    if(zoom) selectZoomSlot(pos);

  //  qDebug()<<"mainClipBoard gerçekleşti";
   // copyWidget();


}
void MainWindow::selectCopySlot(QPoint pos)
{
    const QClipboard *cb = QApplication::clipboard();
    const QMimeData *md = cb->mimeData();
    QImage img;
    if (md->hasImage())
    {
        img=QImage(cb->image());
       //  logo->setPixmap(QPixmap::fromImage(img).scaled(400,300,Qt::IgnoreAspectRatio));
    }
    else if(md->formats().contains(QStringLiteral("text/uri-list")))
    {
       img=QImage(QUrl(cb->text()).toLocalFile());

    }

    // logo->setPixmap(QPixmap::fromImage(img).scaled(400,300,Qt::IgnoreAspectRatio));
    currentTab()->scene->sceneMode=Scene::Mode::CopyMode;
    tempCopyModeItemToRectDraw=new VERectangle(currentTab()->scene);
    tempCopyModeItemToRectDraw->sekilTur(DiagramItem::DiagramType::Copy);
    QPixmap temp=QPixmap::fromImage(img).scaled(img.width(),img.height(),Qt::IgnoreAspectRatio);

    tempCopyModeItemToRectDraw->setImage(temp);
    currentTab()->scene->addItem(tempCopyModeItemToRectDraw);
    tempCopyModeItemToRectDraw->setRect(0,0,temp.width(),temp.height());

    tempCopyModeItemToRectDraw->pageOfNumber=currentTab()->currentPage()-1;
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
        const QRectF pageRect = page->boundingRect().translated(page->pos());
    //tempCopyModeItemToRectDraw->setPos(pageRect.left(),pageRect.top()+pageRect.height()/3);
    tempCopyModeItemToRectDraw->setPos(pageRect.left()+pageRect.width()/2-temp.width()/2,pageRect.top()+pageRect.height()/2-temp.height()/2);

    tempCopyModeItemToRectDraw->fareState(false);
    currentTab()->scene->makeItemsControllable(false);
    tempCopyModeItemToRectDraw->fareState(true);
    tempCopyModeItemToRectDraw->sekilTr=DiagramItem::DiagramType::Copy;

   // currentTab()->scene->sceneMode=Scene::Mode::SelectObject;
    currentTab()->scene->graphicsList.append(tempCopyModeItemToRectDraw);
    currentTab()->scene->graphicsListTemp.append(tempCopyModeItemToRectDraw);
    currentTab()->scene->historyBack.append(tempCopyModeItemToRectDraw);
    currentTab()->scene->historyBackAction.append("added");
    depo::historyBackCount=currentTab()->scene->historyBack.count();
    depo::historyNextCount=currentTab()->scene->historyNext.count();

    tempCopyModeItemToRectDraw=0;
   // kalemModeSignalSlot(Scene::Mode::NoMode,DiagramItem::DiagramType::NoType);
   // kw->buttonStateClear();
    //kw->handButton->setChecked(true);
    currentTab()->scene->update();
    page->update();
    kw->handButtonSlot();
}
void MainWindow::selectZoomSlot(QPoint pos)
{
    const QClipboard *cb = QApplication::clipboard();
    const QMimeData *md = cb->mimeData();
    QImage img;
    if (md->hasImage())
    {
        img=QImage(cb->image());
       //  logo->setPixmap(QPixmap::fromImage(img).scaled(400,300,Qt::IgnoreAspectRatio));
    }
    else if(md->formats().contains(QStringLiteral("text/uri-list")))
    {
       img=QImage(QUrl(cb->text()).toLocalFile());

    }

    // logo->setPixmap(QPixmap::fromImage(img).scaled(400,300,Qt::IgnoreAspectRatio));
    ///currentTab()->scene->sceneMode=Scene::Mode::CopyMode;
    ///tempCopyModeItemToRectDraw=new VERectangle(currentTab()->scene);
   /// tempCopyModeItemToRectDraw->sekilTur(DiagramItem::DiagramType::Copy);
    QPixmap temp=QPixmap::fromImage(img).scaled(img.width(),img.height(),Qt::IgnoreAspectRatio);
   // QLabel *logo=new QLabel();
   // logo->setPixmap(temp);
    //logo->show();


   /// tempCopyModeItemToRectDraw->setImage(temp);
   /// currentTab()->scene->addItem(tempCopyModeItemToRectDraw);
   /// tempCopyModeItemToRectDraw->setRect(0,0,temp.width(),temp.height());

   /// tempCopyModeItemToRectDraw->pageOfNumber=currentTab()->currentPage()-1;
  ////  PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
  ////  const QRectF pageRect = page->boundingRect().translated(page->pos());
 ////   qDebug()<<pageRect<<pageRect.left()<<pageRect.top();

    //tempCopyModeItemToRectDraw->setPos(pageRect.left(),pageRect.top()+pageRect.height()/3);
   /// tempCopyModeItemToRectDraw->setPos(pageRect.left()+pageRect.width()/2-temp.width()/2,pageRect.top()+pageRect.height()/2-temp.height()/2);
   // on_zoomIn_triggered();
   /// page->setPos(pageRect.left()-50,pageRect.top());
 /// on_fitToPageSizeMode_triggered(true);

     // currentTab()->on_pages_zoomToSelection(currentTab()->currentPage(),rt);
    PageItem* page =currentTab()->m_pageItems.at((currentTab()->currentPage()-1));
        const QRectF pageRect = page->boundingRect().translated(page->pos());
    //qDebug()<<"1. ölçek:"<<pageRect.height();
  // currentTab()->scene->sceneMode=Scene::Mode::DrawRectangle;
        VERectangle *gri= new VERectangle(currentTab()->scene);
        gri->sekilTur(DiagramItem::DiagramType::Dortgen);
        gri->setRect(0,0,temp.width(),temp.height());
        gri->setPos(pos.x()+pageRect.left(),pos.y());
       // gri->setPos(pos.x()+pageRect.left(),pos.y(),temp.width(),temp.height());
        currentTab()->scene->addItem(gri);
        gri->fareState(false);
        currentTab()->scene->makeItemsControllable(false);
        gri->fareState(true);
        currentTab()->scene->sceneMode=Scene::Mode::SelectObject;

        on_fitToPageWidthMode_triggered(true);
       // on_zoomIn_triggered();
        //on_zoomIn_triggered();
      /*  if(m_tabWidget->hasCurrent())
        {
            bool ok = false;
            qreal scaleFactor =100 / 100.0;

            scaleFactor = qMax(scaleFactor, s_settings->documentView().minimumScaleFactor());
            scaleFactor = qMin(scaleFactor, s_settings->documentView().maximumScaleFactor());


                currentTab()->setScaleFactor(scaleFactor);
                currentTab()->setScaleMode(ScaleFactorMode);


            on_currentTab_scaleFactorChanged(currentTab()->scaleFactor());
            on_currentTab_scaleModeChanged(currentTab()->scaleMode());
        }*/
       // on_zoomIn_triggered();



    const QRectF pageRect1 = page->boundingRect().translated(page->pos());
    //qDebug()<<"2. ölçek:"<<pageRect1.height();
    // qDebug()<<"ölçek:"<<pageRect1.height()/pageRect.height();
    qreal sx=pageRect1.width()/pageRect.width();
    qreal sy=pageRect1.height()/pageRect.height();
    slotScaleSceneItem(sx,sy);

    gri->fareState(true);

//
    QRectF rt=QRectF(gri->boundingRect());
  //  page->zoomToSelection(currentTab()->currentPage(),rt);
   qDebug()<<"point"<<gri->pos()<<gri->boundingRect().translated(gri->pos());
   qDebug()<<"page"<<page->pos()<<pageRect1<<pageRect1.height()/2;

   QPointF gripos=gri->boundingRect().translated(gri->pos()).center();
   QPointF pagepos=page->boundingRect().translated(page->pos()).center();
   qDebug()<<"fark"<<gripos.y()<<pagepos.y()<<pagepos.y()-gripos.y();

  // page->setPos(mapToGlobal(currentTab()->mapFromScene(gri->boundingRect().center())));
   if(gripos.y()>pageRect1.height()/4*3&&gripos.y()<pageRect1.height())
   {
       page->setPos(pageRect1.left(),pageRect1.top()-gripos.y());
       qDebug()<<"alt-alt";
   }

   if(gripos.y()>pageRect1.height()/2&&gripos.y()<pageRect1.height()/4*3)
   {
       page->setPos(pageRect1.left(),pageRect1.top()-gripos.y()/2);
       qDebug()<<"alt-ust";
   }


   if(gripos.y()>pageRect1.height()/4&&gripos.y()<pageRect1.height()/2)
   {
       page->setPos(pageRect1.left(),pageRect1.top()+gripos.y()/2);
       qDebug()<<"ust-alt";
   }
   if(gripos.y()<pageRect1.height()/4&&gripos.y()>pageRect1.top())
   {
       page->setPos(pageRect1.left(),page->boundingRect().top()+gripos.y()/2);
       qDebug()<<"ust-ust";
   }
    currentTab()->scene->removeItem(gri);

   // slotHand();
   // kalemModeSignalSlot(Scene::Mode::NoMode,DiagramItem::DiagramType::NoType);
   // kw->buttonStateClear();
   // kw->handButton->setChecked(true);
    currentTab()->scene->update();
    page->update();

    kw->modeKontrolSlot();
}
void MainWindow::sceneItemAddedSignalSlot()
{
   //qDebug()<<"item eklendi";
   if(depo::historyBackCount>0)kw->undoButton->setEnabled(true);else kw->undoButton->setEnabled(false);
   if(depo::historyNextCount>0)kw->redoButton->setEnabled(true);else kw->redoButton->setEnabled(false);
}
void MainWindow::zKitapLoad()
{
    const QString path = s_settings->mainWindow().openPath();
   // const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open in new tab"), path, DocumentView::openFilter().join(";;"));
    QStringList filePaths;
    if(QFile::exists("/tmp/zkitap.pdf"))
        QFile::remove("/tmp/zkitap.pdf");

     QFile::copy(QString(":kitap/file/zkitap.pdf"),QString("/tmp/zkitap.pdf"));
     filePaths.append("/tmp/zkitap.pdf");

    if(!filePaths.isEmpty())
    {
        CurrentTabChangeBlocker currentTabChangeBlocker(this);

        foreach(const QString& filePath, filePaths)
        {
          // qDebug()<<"open file"<<filePath;
           openInNewTab(filePath);
        }
    }

}
QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}

void MainWindow::slotSearch()
{
  QFont ff( "Arial", en/4, 0);
    QLabel *textvalue=new QLabel();
    QDialog * d = new QDialog();
    d->setWindowTitle("Arama Penceresi");
    d->setFixedSize(en*7,boy*2);
        auto layout = new QGridLayout();
         layout->setContentsMargins(5, 5, 5,5);
        // layout->setVerticalSpacing(0);
          // layout->setColumnMinimumWidth(0, 10);
        layout->addWidget(m_searchLineEdit,0,1,1,3);
        layout->addWidget(m_matchCaseCheckBox,1,1,1,1);
        layout->addWidget(m_wholeWordsCheckBox, 1,2,1,1);
        layout->addWidget(m_highlightAllCheckBox, 1,3,1,1);
        layout->addWidget(m_findPreviousButton,4,1,1,1,Qt::AlignCenter);
        layout->addWidget(m_findNextButton, 4,2,1,1,Qt::AlignCenter);
        layout->addWidget(m_cancelSearchButton, 4,3,1,1,Qt::AlignCenter);
        m_findPreviousButton->setEnabled(true);
        m_findNextButton->setEnabled(true);
        m_cancelSearchButton->setEnabled(true);


    d->setLayout(layout);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
       // qDebug()<<"tamam";

    }

    if(result == QDialog::Rejected)
    {
        qDebug()<<"iptal";

    }


}


void MainWindow::slotJumpPage()
{

    //int e=en;
    //int b=boy;
   // qDebug()<<"ayar click"<<en;
    QFont ff( "Arial", en/4, 0);
    QLabel *textvalue=new QLabel();
    QDialog * d = new QDialog();
    d->setWindowTitle("Sayfaya Git");
    QSpinBox *sb = new QSpinBox();
    sb->setValue(currentTab()->currentPage());
    sb->setRange(1, currentTab()->numberOfPages());
   // sb->setRange(1, 1000);
    sb->setDisabled(true);
    sb->setFont(ff);
  //  sb->setFixedSize(130,30);

    sb->setStyleSheet(
                 "QSpinBox { border: 1px solid gray; border-radius: 5px; min-height: 60px; min-width: 30px; }"

                 "QSpinBox::up-arrow { border-left: 10px solid none;"
                 "border-right: 10px solid none; border-bottom: 10px solid black; width: 0px; height: 0px; }"
                 "QSpinBox::up-arrow:hover { border-left: 10px solid none; "
                 "border-right: 10px solid none; border-bottom: 10px solid black; width: 0px; height: 0px; }"
                 "QSpinBox::up-button { min-width: 20px; min-height: 27px; background-color: gray; }"
                 "QSpinBox::up-button:hover { min-width: 20px; min-height: 27px; background-color: gray; }"

                 "QSpinBox::down-arrow { border-left: 10px solid none;"
                 "border-right: 10px solid none; border-top: 10px solid black; width: 0px; height: 0px; }"
                 "QSpinBox::down-arrow:hover { border-left: 10px solid none;"
                 "border-right: 10px solid none; border-top: 10px solid black; width: 0px; height: 0px; }"
                 "QSpinBox::down-button { min-width: 20px; min-height: 27px; background-color: gray; }"
                 "QSpinBox::down-button:hover { min-width: 20px; min-height: 27px; background-color: gray; }"

                 );


    connect(sb, &QSpinBox::editingFinished,
                this, [&]()
        {
        textvalue->setText(QString::number(sb->value()));

    });

    QLabel *pageNumberLabel=new QLabel();
    pageNumberLabel->setFixedSize(en*3,en/4*3);
    pageNumberLabel->setText(QString::number(currentTab()->currentPage())+" / "+
                             QString::number(currentTab()->numberOfPages()));
    //pageNumberLabel->setStyleSheet("background-color: #aa0000");
    pageNumberLabel->setFont(ff);
    pageNumberLabel->setAlignment(Qt::AlignCenter);


durum=false;
/******************************************/
    //  int en=40;
     //int boy=30;
         QPushButton *birButton= new QPushButton;
         birButton->setFixedSize(en, boy);
         birButton->setIconSize(QSize(en,boy));
         birButton->setText("1");
         birButton->setStyleSheet("Text-align:center");
         //birButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(birButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("1");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                 return;
             }
             if(durum)
             {
                 textvalue->setText(textvalue->text()+"1");
         sb->setValue(textvalue->text().toInt());
          textvalue->setText(QString::number(sb->value()));
          if(sb->value()==currentTab()->numberOfPages()) durum=false;
             }

         });

         QPushButton *ikiButton= new QPushButton;
         ikiButton->setFixedSize(en, boy);
         ikiButton->setIconSize(QSize(en,boy));
         ikiButton->setText("2");
         ikiButton->setStyleSheet("Text-align:center");
        // ikiButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(ikiButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("2");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                 return;
             }
             if(durum)
             {
                textvalue->setText(textvalue->text()+"2");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;

             }


         });


         QPushButton *ucButton= new QPushButton;
         ucButton->setFixedSize(en, boy);
         ucButton->setIconSize(QSize(en,boy));
         ucButton->setText("3");
         ucButton->setStyleSheet("Text-align:center");
        // ucButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(ucButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("3");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                 return;
             }
             if(durum)
             {

             textvalue->setText(textvalue->text()+"3");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;

             }

         });
         /***************************4***************/
         QPushButton *dortButton= new QPushButton;
         dortButton->setFixedSize(en, boy);
         dortButton->setIconSize(QSize(en,boy));
         dortButton->setText("4");
         dortButton->setStyleSheet("Text-align:center");
        // dortButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(dortButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("4");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                   return;
             }
             if(durum)
             {
            textvalue->setText(textvalue->text()+"4");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;
             }

         });

         /***************************5***************/
         QPushButton *besButton= new QPushButton;
         besButton->setFixedSize(en, boy);
         besButton->setIconSize(QSize(en,boy));
         besButton->setText("5");
         besButton->setStyleSheet("Text-align:center");
         //besButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(besButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("5");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                  return;
             }
             if(durum)
             {
             textvalue->setText(textvalue->text()+"5");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;
             }

         });

         /***************************6***************/
         QPushButton *altiButton= new QPushButton;
         altiButton->setFixedSize(en, boy);
         altiButton->setIconSize(QSize(en,boy));
         altiButton->setText("6");
         altiButton->setStyleSheet("Text-align:center");
        // altiButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(altiButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("6");
                 sb->setValue(textvalue->text().toInt());
                 textvalue->setText(QString::number(sb->value()));

                 durum=true;
                  return;
             }
             if(durum)
             {   textvalue->setText(textvalue->text()+"6");
                 sb->setValue(textvalue->text().toInt());
                 textvalue->setText(QString::number(sb->value()));
                 if(sb->value()==currentTab()->numberOfPages()) durum=false;
             }

         });

         /***************************7***************/
         QPushButton *yediButton= new QPushButton;
         yediButton->setFixedSize(en, boy);
         yediButton->setIconSize(QSize(en,boy));
         yediButton->setText("7");
         yediButton->setStyleSheet("Text-align:center");
         //yediButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(yediButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("7");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                 return;
             }
             if(durum)
             {
             textvalue->setText(textvalue->text()+"7");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;
}
         });

         /***************************8***************/
         QPushButton *sekizButton= new QPushButton;
         sekizButton->setFixedSize(en, boy);
         sekizButton->setIconSize(QSize(en,boy));
         sekizButton->setText("8");
         sekizButton->setStyleSheet("Text-align:center");
         //sekizButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(sekizButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("8");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                  return;
             }
             if(durum)
             {
                 textvalue->setText(textvalue->text()+"8");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;

}
         });

         /***************************9***************/
         QPushButton *dokuzButton= new QPushButton;
         dokuzButton->setFixedSize(en, boy);
         dokuzButton->setIconSize(QSize(en,boy));
         dokuzButton->setText("9");
         dokuzButton->setStyleSheet("Text-align:center");
        // dokuzButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(dokuzButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("9");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                  return;
             }
             if(durum)
             {
                 textvalue->setText(textvalue->text()+"9");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;

}
         });

         /***************************0***************/
         QPushButton *sifirButton= new QPushButton;
         sifirButton->setFixedSize(en, boy);
         sifirButton->setIconSize(QSize(en,boy));
         sifirButton->setText("0");
         sifirButton->setStyleSheet("Text-align:center");
         //sifirButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(sifirButton, &QPushButton::clicked, [=]() {
             if(!durum){
                 textvalue->setText("1");
                 sb->setValue(textvalue->text().toInt());
                  textvalue->setText(QString::number(sb->value()));

                 durum=true;
                  return;
             }
             if(durum)
             {
                 textvalue->setText(textvalue->text()+"0");
             sb->setValue(textvalue->text().toInt());
             textvalue->setText(QString::number(sb->value()));
             if(sb->value()==currentTab()->numberOfPages()) durum=false;

}
         });

         /***************************backspace***************/
         QPushButton *geriSilButton= new QPushButton;
         geriSilButton->setFixedSize(en*2+10, boy);
         geriSilButton->setIconSize(QSize(en,boy));
         geriSilButton->setText("<-");
         geriSilButton->setStyleSheet("Text-align:center");
         //geriSilButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(geriSilButton, &QPushButton::clicked, [=]() {
             QString v=textvalue->text();
             v.chop(1);

             textvalue->setText(v);
             sb->setValue(textvalue->text().toInt());



         });

         /***************************Enter***************/
         QPushButton *enterButton= new QPushButton;
         enterButton->setFixedSize(en, boy);
         enterButton->setIconSize(QSize(en,boy));
         enterButton->setText("Git");
         enterButton->setStyleSheet("Text-align:center");
        // enterButton->setFlat(true);
         // kilitButton->setIcon(QIcon(":icons/saveprofile.png"));

         connect(enterButton, &QPushButton::clicked, [=]() {
              durum=false;
        currentTab()->jumpToPage(sb->value());

         });

         QPushButton *nextPageButton= new QPushButton;
         nextPageButton->setFixedSize(en, boy);
         nextPageButton->setIconSize(QSize(en,boy));
//         nextPageButton->setText("Git");
         nextPageButton->setStyleSheet("Text-align:center");
         nextPageButton->setIcon(QIcon(":icons/icons/nextpage.png"));

         connect(nextPageButton, &QPushButton::clicked, [=]() {
       // currentTab()->nextPage();
              durum=false;
        on_nextPage_triggered();
        sb->setValue(currentTab()->currentPage());

         });

         QPushButton *backPageButton= new QPushButton;
         backPageButton->setFixedSize(en, boy);
         backPageButton->setIconSize(QSize(en,boy));
         //backPageButton->setText("Git");
         backPageButton->setStyleSheet("Text-align:center");
         backPageButton->setIcon(QIcon(":icons/icons/backpage.png"));

         connect(backPageButton, &QPushButton::clicked, [=]() {
     //  currentTab()->previusPage();
              durum=false;
         on_previousPage_triggered();
         sb->setValue(currentTab()->currentPage());

         });




        auto layout = new QGridLayout();
         layout->setContentsMargins(5, 5, 5,5);
        // layout->setVerticalSpacing(0);
          // layout->setColumnMinimumWidth(0, 10);
         layout->addWidget(pageNumberLabel,0,1,1,3);
         layout->addWidget(sb,1,1,1,2);
         layout->addWidget(enterButton, 1,3,1,1);

         layout->addWidget(birButton, 5,1,1,1);
         layout->addWidget(ikiButton, 5,2,1,1);
         layout->addWidget(ucButton, 5,3,1,1);
         layout->addWidget(dortButton, 10,1,1,1);
         layout->addWidget(besButton, 10,2,1,1);
         layout->addWidget(altiButton, 10,3,1,1);
         layout->addWidget(yediButton, 15,1,1,1);
         layout->addWidget(sekizButton, 15,2,1,1);
         layout->addWidget(dokuzButton, 15,3,1,1);
         layout->addWidget(sifirButton,20,1,1,1);
         layout->addWidget(geriSilButton, 20,2,1,2);

         layout->addWidget(backPageButton, 25,1,1,1);
         //layout->addWidget(geriSilButton, 5,2,1,1);
         layout->addWidget(nextPageButton, 25,3,1,1);

     /**********************************************/







  /* QVBoxLayout * vbox = new QVBoxLayout();
    QHBoxLayout * hbox1= new QHBoxLayout();
    QHBoxLayout * hbox2= new QHBoxLayout();
    QHBoxLayout * hbox3= new QHBoxLayout();
    QHBoxLayout * hbox4= new QHBoxLayout();
    QHBoxLayout * hbox5= new QHBoxLayout();
    QHBoxLayout * hbox6= new QHBoxLayout();

    hbox1->addWidget(pageNumberLabel);
    hbox2->addWidget(sb);
  //  hbox3->addWidget(gotoPageButton);
    hbox4->addLayout(layout);


    //vbox->addWidget(pcnameLabel);
    vbox->addLayout(hbox1,Qt::AlignVCenter);
    vbox->addLayout(hbox2,Qt::AlignVCenter);
   // vbox->addLayout(hbox3,Qt::AlignVCenter);
    vbox->addLayout(hbox4,Qt::AlignVCenter);
*/
    d->setLayout(layout);

    int result = d->exec();
    if(result == QDialog::Accepted)
    {
       // qDebug()<<"tamam";

    }

    if(result == QDialog::Rejected)
    {
        qDebug()<<"iptal";

    }


}


QMenu* MainWindow::createPopupMenu()
{
    QMenu* menu = new QMenu();

    //menu->addAction(m_fileToolBar->toggleViewAction());
    //menu->addAction(m_editToolBar->toggleViewAction());
   // menu->addAction(m_viewToolBar->toggleViewAction());
   // menu->addSeparator();
   // menu->addAction(m_outlineDock->toggleViewAction());
   // menu->addAction(m_propertiesDock->toggleViewAction());
    menu->addAction(m_thumbnailsDock->toggleViewAction());
   // menu->addAction(m_bookmarksDock->toggleViewAction());

    if(s_settings->mainWindow().extendedSearchDock())
    {
        menu->addAction(m_searchDock->toggleViewAction());
    }

    return menu;
}
void MainWindow::show()
{
    QMainWindow::show();

    if(s_settings->mainWindow().restoreTabs())
    {
        s_database->restoreTabs(RestoreTab(this));

        const int currentTabIndex = s_settings->mainWindow().currentTabIndex();

        if(currentTabIndex != -1)
        {
            m_tabWidget->setCurrentIndex(currentTabIndex);
        }
    }

    if(s_settings->mainWindow().restoreBookmarks())
    {
        s_database->restoreBookmarks();
    }

    on_tabWidget_currentChanged();
}
bool MainWindow::open(const QString& filePath, int page, const QRectF& highlight, bool quiet)
{
    if(DocumentView* const tab = currentTab())
    {
        if(!saveModifications(tab))
        {
            return false;
        }

        if(tab->open(filePath))
        {
            s_settings->mainWindow().setOpenPath(tab->fileInfo().absolutePath());
            m_recentlyUsedMenu->addOpenAction(tab->fileInfo());

            m_tabWidget->setCurrentTabText(tab->title());
            m_tabWidget->setCurrentTabToolTip(tab->fileInfo().absoluteFilePath());

            restorePerFileSettings(tab);
            scheduleSaveTabs();

            tab->jumpToPage(page, false);
            tab->setFocus();

            if(!highlight.isNull())
            {
                tab->temporaryHighlight(page, highlight);
            }

            return true;
        }
        else
        {
            if(!quiet)
            {
                QMessageBox::warning(this, tr("Warning"), tr("Could not open '%1'.").arg(filePath));
            }
        }
    }

    return false;
}

bool MainWindow::openInNewTab(const QString& filePath, int page, const QRectF& highlight, bool quiet)
{
    DocumentView* const newTab = new DocumentView(this);

    if(newTab->open(filePath))
    {
        s_settings->mainWindow().setOpenPath(newTab->fileInfo().absolutePath());
        m_recentlyUsedMenu->addOpenAction(newTab->fileInfo());

        addTab(newTab);
        addTabAction(newTab);
        connectTab(newTab);

        newTab->show();

        restorePerFileSettings(newTab);
        scheduleSaveTabs();

        newTab->jumpToPage(page, false);
        newTab->setFocus();

        if(!highlight.isNull())
        {
            newTab->temporaryHighlight(page, highlight);
        }

        return true;
    }
    else
    {
        delete newTab;

        if(!quiet)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Could not open '%1'.").arg(filePath));
        }
    }

    return false;
}

bool MainWindow::jumpToPageOrOpenInNewTab(const QString& filePath, int page, bool refreshBeforeJump, const QRectF& highlight, bool quiet)
{
    const QFileInfo fileInfo(filePath);

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        foreach(DocumentView* tab, allTabs(index))
        {
            if(tab->fileInfo() == fileInfo)
            {
                m_tabWidget->setCurrentIndex(index);

                if(refreshBeforeJump)
                {
                    if(!tab->refresh())
                    {
                        return false;
                    }
                }

                tab->jumpToPage(page);
                tab->setFocus();

                if(!highlight.isNull())
                {
                    tab->temporaryHighlight(page, highlight);
                }

                return true;
            }
        }
    }

    return openInNewTab(filePath, page, highlight, quiet);
}

void MainWindow::startSearch(const QString& text)
{
    if(DocumentView* const tab = currentTab())
    {
        m_searchDock->setVisible(true);

        m_searchLineEdit->setText(text);
        m_searchLineEdit->startSearch();

        tab->setFocus();
    }
}

void MainWindow::saveDatabase()
{
    QTimer::singleShot(0, this, SLOT(on_saveDatabase_timeout()));
}

void MainWindow::on_tabWidget_currentChanged()
{
    if(m_currentTabChangedBlocked)
    {
        return;
    }

    DocumentView* const tab = currentTab();
    const bool hasCurrent = tab != 0;
    kw->setEnabled(hasCurrent);
    if(hasCurrent)
    {
           slotPenInit();
    }

    m_refreshAction->setEnabled(hasCurrent);
    m_printAction->setEnabled(hasCurrent);

    m_previousPageAction->setEnabled(hasCurrent);
    m_nextPageAction->setEnabled(hasCurrent);
    m_firstPageAction->setEnabled(hasCurrent);
    m_lastPageAction->setEnabled(hasCurrent);

    m_setFirstPageAction->setEnabled(hasCurrent);

    m_jumpToPageAction->setEnabled(hasCurrent);

    m_searchAction->setEnabled(hasCurrent);

    m_copyToClipboardModeAction->setEnabled(hasCurrent);
    m_addAnnotationModeAction->setEnabled(hasCurrent);

    m_continuousModeAction->setEnabled(hasCurrent);
    m_twoPagesModeAction->setEnabled(hasCurrent);
    m_twoPagesWithCoverPageModeAction->setEnabled(hasCurrent);
    m_multiplePagesModeAction->setEnabled(hasCurrent);
    m_rightToLeftModeAction->setEnabled(hasCurrent);

    m_zoomInAction->setEnabled(hasCurrent);
    m_zoomOutAction->setEnabled(hasCurrent);
    m_originalSizeAction->setEnabled(hasCurrent);
    m_fitToPageWidthModeAction->setEnabled(hasCurrent);
    m_fitToPageSizeModeAction->setEnabled(hasCurrent);

    m_rotateLeftAction->setEnabled(hasCurrent);
    m_rotateRightAction->setEnabled(hasCurrent);

    m_invertColorsAction->setEnabled(hasCurrent);
    m_convertToGrayscaleAction->setEnabled(hasCurrent);
    m_trimMarginsAction->setEnabled(hasCurrent);

    m_compositionModeMenu->setEnabled(hasCurrent);
    m_darkenWithPaperColorAction->setEnabled(hasCurrent);
    m_lightenWithPaperColorAction->setEnabled(hasCurrent);

    m_fontsAction->setEnabled(hasCurrent);

    m_presentationAction->setEnabled(hasCurrent);

    m_previousTabAction->setEnabled(hasCurrent);
    m_nextTabAction->setEnabled(hasCurrent);
    m_closeTabAction->setEnabled(hasCurrent);
    m_closeAllTabsAction->setEnabled(hasCurrent);
    m_closeAllTabsButCurrentTabAction->setEnabled(hasCurrent);

    m_previousBookmarkAction->setEnabled(hasCurrent);
    m_nextBookmarkAction->setEnabled(hasCurrent);
    m_addBookmarkAction->setEnabled(hasCurrent);
    m_removeBookmarkAction->setEnabled(hasCurrent);

    m_currentPageSpinBox->setEnabled(hasCurrent);
    m_scaleFactorComboBox->setEnabled(hasCurrent);
    m_searchLineEdit->setEnabled(hasCurrent);
    m_matchCaseCheckBox->setEnabled(hasCurrent);
    m_wholeWordsCheckBox->setEnabled(hasCurrent);
    m_highlightAllCheckBox->setEnabled(hasCurrent);

    m_openCopyInNewTabAction->setEnabled(hasCurrent);
    m_openCopyInNewWindowAction->setEnabled(hasCurrent);
    m_openContainingFolderAction->setEnabled(hasCurrent);
    m_moveToInstanceAction->setEnabled(hasCurrent);
    m_splitViewHorizontallyAction->setEnabled(hasCurrent);
    m_splitViewVerticallyAction->setEnabled(hasCurrent);
    m_closeCurrentViewAction->setEnabled(hasCurrent);

    m_searchDock->toggleViewAction()->setEnabled(hasCurrent);

    if(hasCurrent)
    {
        const bool canSave = tab->canSave();
        m_saveAction->setEnabled(canSave);
        m_saveAsAction->setEnabled(canSave);
        m_saveCopyAction->setEnabled(canSave);

        if(m_searchDock->isVisible())
        {
            m_searchLineEdit->stopTimer();
            m_searchLineEdit->setProgress(tab->searchProgress());

            if(tab->hasSearchResults())
            {
                m_searchLineEdit->setText(tab->searchText());
                m_matchCaseCheckBox->setChecked(tab->searchMatchCase());
                m_wholeWordsCheckBox->setChecked(tab->searchWholeWords());
            }
        }

        m_bookmarksView->setModel(bookmarkModelForCurrentTab());

        on_thumbnails_dockLocationChanged(dockWidgetArea(m_thumbnailsDock));

        m_thumbnailsView->setScene(tab->thumbnailsScene());
        tab->setThumbnailsViewportSize(m_thumbnailsView->viewport()->size());

        on_currentTab_documentChanged();

        on_currentTab_numberOfPagesChaned(tab->numberOfPages());
        on_currentTab_currentPageChanged(tab->currentPage());

        on_currentTab_canJumpChanged(tab->canJumpBackward(), tab->canJumpForward());

        on_currentTab_continuousModeChanged(tab->continuousMode());
        on_currentTab_layoutModeChanged(tab->layoutMode());
        on_currentTab_rightToLeftModeChanged(tab->rightToLeftMode());
        on_currentTab_scaleModeChanged(tab->scaleMode());
        on_currentTab_scaleFactorChanged(tab->scaleFactor());

        on_currentTab_invertColorsChanged(tab->invertColors());
        on_currentTab_convertToGrayscaleChanged(tab->convertToGrayscale());
        on_currentTab_trimMarginsChanged(tab->trimMargins());

        on_currentTab_compositionModeChanged(tab->compositionMode());

        on_currentTab_highlightAllChanged(tab->highlightAll());
        on_currentTab_rubberBandModeChanged(tab->rubberBandMode());
    }
    else
    {
        m_saveAction->setEnabled(false);
        m_saveAsAction->setEnabled(false);
        m_saveCopyAction->setEnabled(false);

        if(m_searchDock->isVisible())
        {
            m_searchLineEdit->stopTimer();
            m_searchLineEdit->setProgress(0);

            m_searchDock->setVisible(false);
        }

        m_outlineView->setModel(0);
        m_propertiesView->setModel(0);
        m_bookmarksView->setModel(0);

        m_thumbnailsView->setScene(0);

        setWindowTitleForCurrentTab();
        setCurrentPageSuffixForCurrentTab();

        m_currentPageSpinBox->setValue(1);
        m_scaleFactorComboBox->setCurrentIndex(4);

        m_jumpBackwardAction->setEnabled(false);
        m_jumpForwardAction->setEnabled(false);

        m_copyToClipboardModeAction->setChecked(false);
        m_addAnnotationModeAction->setChecked(false);

        m_continuousModeAction->setChecked(false);
        m_twoPagesModeAction->setChecked(false);
        m_twoPagesWithCoverPageModeAction->setChecked(false);
        m_multiplePagesModeAction->setChecked(false);

        m_fitToPageSizeModeAction->setChecked(false);
        m_fitToPageWidthModeAction->setChecked(false);

        m_invertColorsAction->setChecked(false);
        m_convertToGrayscaleAction->setChecked(false);
        m_trimMarginsAction->setChecked(false);

        m_darkenWithPaperColorAction->setChecked(false);
        m_lightenWithPaperColorAction->setChecked(false);
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    on_closeTabs_triggered(allTabs(index));
}

void MainWindow::on_tabWidget_tabDragRequested(int index)
{
    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls(QList< QUrl >() << QUrl::fromLocalFile(currentTab(index)->fileInfo().absoluteFilePath()));

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

void MainWindow::on_tabWidget_tabContextMenuRequested(QPoint globalPos, int index)
{
    QMenu menu;

    // We block their signals since we need to handle them using the selected instead of the current tab.
    SignalBlocker openCopyInNewTabSignalBlocker(m_openCopyInNewTabAction);
    SignalBlocker openCopyInNewWindowSignalBlocker(m_openCopyInNewWindowAction);
    SignalBlocker openContainingFolderSignalBlocker(m_openContainingFolderAction);
    SignalBlocker moveToInstanceSignalBlocker(m_moveToInstanceAction);
    SignalBlocker splitViewHorizontallySignalBlocker(m_splitViewHorizontallyAction);
    SignalBlocker splitViewVerticallySignalBlocker(m_splitViewVerticallyAction);
    SignalBlocker closeCurrentViewSignalBlocker(m_closeCurrentViewAction);

    QAction* copyFilePathAction = createTemporaryAction(&menu, tr("Copy file path"), QLatin1String("copyFilePath"));
    QAction* selectFilePathAction = createTemporaryAction(&menu, tr("Select file path"), QLatin1String("selectFilePath"));

    QAction* closeAllTabsAction = createTemporaryAction(&menu, tr("Close all tabs"), QLatin1String("closeAllTabs"));
    QAction* closeAllTabsButThisOneAction = createTemporaryAction(&menu, tr("Close all tabs but this one"), QLatin1String("closeAllTabsButThisOne"));
    QAction* closeAllTabsToTheLeftAction = createTemporaryAction(&menu, tr("Close all tabs to the left"), QLatin1String("closeAllTabsToTheLeft"));
    QAction* closeAllTabsToTheRightAction = createTemporaryAction(&menu, tr("Close all tabs to the right"), QLatin1String("closeAllTabsToTheRight"));

    selectFilePathAction->setVisible(QApplication::clipboard()->supportsSelection());

    QList< QAction* > actions;

    actions << m_openCopyInNewTabAction << m_openCopyInNewWindowAction << m_openContainingFolderAction << m_moveToInstanceAction
            << m_splitViewHorizontallyAction << m_splitViewVerticallyAction << m_closeCurrentViewAction
            << copyFilePathAction << selectFilePathAction
            << closeAllTabsAction << closeAllTabsButThisOneAction
            << closeAllTabsToTheLeftAction << closeAllTabsToTheRightAction;

    addWidgetActions(&menu, s_settings->mainWindow().tabContextMenu(), actions);

    const QAction* action = menu.exec(globalPos);

    DocumentView* const tab = currentTab(index);

    if(action == m_openCopyInNewTabAction)
    {
        on_openCopyInNewTab_triggered(tab);
    }
    else if(action == m_openCopyInNewWindowAction)
    {
        on_openCopyInNewWindow_triggered(tab);
    }
    else if(action == m_openContainingFolderAction)
    {
        on_openContainingFolder_triggered(tab);
    }
    else if(action == m_moveToInstanceAction)
    {
        on_moveToInstance_triggered(tab);
    }
    else if(action == m_splitViewHorizontallyAction)
    {
        on_splitView_split_triggered(Qt::Horizontal, index);
    }
    else if(action == m_splitViewVerticallyAction)
    {
        on_splitView_split_triggered(Qt::Vertical, index);
    }
    else if(action == m_closeCurrentViewAction)
    {
        on_splitView_closeCurrent_triggered(index);
    }
    else if(action == copyFilePathAction)
    {
        QApplication::clipboard()->setText(tab->fileInfo().absoluteFilePath());
    }
    else if(action == selectFilePathAction)
    {
        QApplication::clipboard()->setText(tab->fileInfo().absoluteFilePath(), QClipboard::Selection);
    }
    else if(action == closeAllTabsAction)
    {
        on_closeAllTabs_triggered();
    }
    else if(action == closeAllTabsButThisOneAction)
    {
        on_closeAllTabsButThisOne_triggered(index);
    }
    else if(action == closeAllTabsToTheLeftAction)
    {
        on_closeAllTabsToTheLeft_triggered(index);
    }
    else if(action == closeAllTabsToTheRightAction)
    {
        on_closeAllTabsToTheRight_triggered(index);
    }
}

#define ONLY_IF_SENDER_IS_CURRENT_TAB if(!senderIsCurrentTab()) { return; }

void MainWindow::on_currentTab_documentChanged()
{
    DocumentView* const senderTab = findCurrentTab(sender());

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        if(senderTab == currentTab(index))
        {
            m_tabWidget->setTabText(index, senderTab->title());
            m_tabWidget->setTabToolTip(index, senderTab->fileInfo().absoluteFilePath());

            break;
        }
    }

    foreach(QAction* tabAction, m_tabsMenu->actions())
    {
        if(senderTab == tabAction->parent())
        {
            tabAction->setText(senderTab->title());

            break;
        }
    }

    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_outlineView->setModel(currentTab()->outlineModel());
    m_propertiesView->setModel(currentTab()->propertiesModel());

    m_outlineView->restoreExpansion();

    setWindowTitleForCurrentTab();
    setWindowModified(currentTab()->wasModified());
}

void MainWindow::on_currentTab_documentModified()
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    setWindowModified(true);
}

void MainWindow::on_currentTab_numberOfPagesChaned(int numberOfPages)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_currentPageSpinBox->setRange(1, numberOfPages);

    setWindowTitleForCurrentTab();
    setCurrentPageSuffixForCurrentTab();
}

void MainWindow::on_currentTab_currentPageChanged(int currentPage)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_currentPageSpinBox->setValue(currentPage);

    if(s_settings->mainWindow().synchronizeOutlineView() && m_outlineView->model() != 0)
    {
        const QModelIndex match = synchronizeOutlineView(currentPage, m_outlineView->model(), QModelIndex());

        if(match.isValid())
        {
            m_outlineView->collapseAll();

            m_outlineView->expandAbove(match);
            m_outlineView->setCurrentIndex(match);
        }
    }

    m_thumbnailsView->ensureVisible(currentTab()->thumbnailItems().at(currentPage - 1));

    setWindowTitleForCurrentTab();
    setCurrentPageSuffixForCurrentTab();
}

void MainWindow::on_currentTab_canJumpChanged(bool backward, bool forward)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_jumpBackwardAction->setEnabled(backward);
    m_jumpForwardAction->setEnabled(forward);
}

void MainWindow::on_currentTab_continuousModeChanged(bool continuousMode)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_continuousModeAction->setChecked(continuousMode);
}

void MainWindow::on_currentTab_layoutModeChanged(LayoutMode layoutMode)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_twoPagesModeAction->setChecked(layoutMode == TwoPagesMode);
    m_twoPagesWithCoverPageModeAction->setChecked(layoutMode == TwoPagesWithCoverPageMode);
    m_multiplePagesModeAction->setChecked(layoutMode == MultiplePagesMode);
}

void MainWindow::on_currentTab_rightToLeftModeChanged(bool rightToLeftMode)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_rightToLeftModeAction->setChecked(rightToLeftMode);
}

void MainWindow::on_currentTab_scaleModeChanged(ScaleMode scaleMode)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    switch(scaleMode)
    {
    default:
    case ScaleFactorMode:
        m_fitToPageWidthModeAction->setChecked(false);
        m_fitToPageSizeModeAction->setChecked(false);

        on_currentTab_scaleFactorChanged(currentTab()->scaleFactor());
        break;
    case FitToPageWidthMode:
        m_fitToPageWidthModeAction->setChecked(true);
        m_fitToPageSizeModeAction->setChecked(false);

        m_scaleFactorComboBox->setCurrentIndex(0);

        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
        break;
    case FitToPageSizeMode:
        m_fitToPageWidthModeAction->setChecked(false);
        m_fitToPageSizeModeAction->setChecked(true);

        m_scaleFactorComboBox->setCurrentIndex(1);

        m_zoomInAction->setEnabled(true);
        m_zoomOutAction->setEnabled(true);
        break;
    }
}

void MainWindow::on_currentTab_scaleFactorChanged(qreal scaleFactor)
{
    scheduleSaveTabs();
    scheduleSavePerFileSettings();

    ONLY_IF_SENDER_IS_CURRENT_TAB

    if(currentTab()->scaleMode() == ScaleFactorMode)
    {
        m_scaleFactorComboBox->setCurrentIndex(m_scaleFactorComboBox->findData(scaleFactor));
        m_scaleFactorComboBox->lineEdit()->setText(QString("%1 %").arg(qRound(scaleFactor * 100.0)));

        m_zoomInAction->setDisabled(qFuzzyCompare(scaleFactor, s_settings->documentView().maximumScaleFactor()));
        m_zoomOutAction->setDisabled(qFuzzyCompare(scaleFactor, s_settings->documentView().minimumScaleFactor()));
    }
}

void MainWindow::on_currentTab_rotationChanged(Rotation rotation)
{
    Q_UNUSED(rotation);

    scheduleSaveTabs();
    scheduleSavePerFileSettings();
}

void MainWindow::on_currentTab_linkClicked(int page)
{
    openInNewTab(currentTab()->fileInfo().filePath(), page);
}

void MainWindow::on_currentTab_linkClicked(bool newTab, const QString& filePath, int page)
{
    if(newTab)
    {
        openInNewTab(filePath, page);
    }
    else
    {
        jumpToPageOrOpenInNewTab(filePath, page, true);
    }
}

void MainWindow::on_currentTab_renderFlagsChanged(qpdfview::RenderFlags renderFlags)
{
    Q_UNUSED(renderFlags);

    scheduleSaveTabs();
    scheduleSavePerFileSettings();
}

void MainWindow::on_currentTab_invertColorsChanged(bool invertColors)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_invertColorsAction->setChecked(invertColors);
}

void MainWindow::on_currentTab_convertToGrayscaleChanged(bool convertToGrayscale)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_convertToGrayscaleAction->setChecked(convertToGrayscale);
}

void MainWindow::on_currentTab_trimMarginsChanged(bool trimMargins)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_trimMarginsAction->setChecked(trimMargins);
}

void MainWindow::on_currentTab_compositionModeChanged(CompositionMode compositionMode)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    switch(compositionMode)
    {
    default:
    case DefaultCompositionMode:
        m_darkenWithPaperColorAction->setChecked(false);
        m_lightenWithPaperColorAction->setChecked(false);
        break;
    case DarkenWithPaperColorMode:
        m_darkenWithPaperColorAction->setChecked(true);
        m_lightenWithPaperColorAction->setChecked(false);
        break;
    case LightenWithPaperColorMode:
        m_darkenWithPaperColorAction->setChecked(false);
        m_lightenWithPaperColorAction->setChecked(true);
        break;
    }
}

void MainWindow::on_currentTab_highlightAllChanged(bool highlightAll)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_highlightAllCheckBox->setChecked(highlightAll);
}

void MainWindow::on_currentTab_rubberBandModeChanged(RubberBandMode rubberBandMode)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_copyToClipboardModeAction->setChecked(rubberBandMode == CopyToClipboardMode);
    m_addAnnotationModeAction->setChecked(rubberBandMode == AddAnnotationMode);
}

void MainWindow::on_currentTab_searchFinished()
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_searchLineEdit->setProgress(0);
}

void MainWindow::on_currentTab_searchProgressChanged(int progress)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    m_searchLineEdit->setProgress(progress);
}

void MainWindow::on_currentTab_customContextMenuRequested(QPoint pos)
{
    ONLY_IF_SENDER_IS_CURRENT_TAB

    QMenu menu;

    QAction* sourceLinkAction = sourceLinkActionForCurrentTab(&menu, pos);

    QList< QAction* > actions;

    actions << m_openCopyInNewTabAction << m_openCopyInNewWindowAction << m_openContainingFolderAction << m_moveToInstanceAction
            << m_splitViewHorizontallyAction << m_splitViewVerticallyAction << m_closeCurrentViewAction
            << m_previousPageAction << m_nextPageAction
            << m_firstPageAction << m_lastPageAction
            << m_jumpToPageAction << m_jumpBackwardAction << m_jumpForwardAction
            << m_setFirstPageAction;

    if(m_searchDock->isVisible())
    {
        actions << m_findPreviousAction << m_findNextAction << m_cancelSearchAction;
    }

    menu.addAction(sourceLinkAction);
    menu.addSeparator();

    addWidgetActions(&menu, s_settings->mainWindow().documentContextMenu(), actions);

    const QAction* action = menu.exec(currentTab()->mapToGlobal(pos));

    if(action == sourceLinkAction)
    {
        currentTab()->openInSourceEditor(sourceLinkAction->data().value< DocumentView::SourceLink >());
    }
}

void MainWindow::on_splitView_splitHorizontally_triggered()
{
    on_splitView_split_triggered(Qt::Horizontal, m_tabWidget->currentIndex());
}

void MainWindow::on_splitView_splitVertically_triggered()
{
    on_splitView_split_triggered(Qt::Vertical, m_tabWidget->currentIndex());
}

void MainWindow::on_splitView_split_triggered(Qt::Orientation orientation, int index)
{
    const QString path = s_settings->mainWindow().openPath();
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open"), path, DocumentView::openFilter().join(";;"));

    if(filePath.isEmpty())
    {
        return;
    }

    DocumentView* const newTab = new DocumentView(this);

    if(!newTab->open(filePath))
    {
        delete newTab;
        return;
    }

    Splitter* splitter = new Splitter(orientation, this);
    connect(splitter, SIGNAL(currentWidgetChanged(QWidget*)), this, SLOT(on_splitView_currentWidgetChanged(QWidget*)));

    QWidget* const tab = m_tabWidget->widget(index);
    const QString tabText = m_tabWidget->tabText(index);
    const QString tabToolTip = m_tabWidget->tabToolTip(index);

    m_tabWidget->removeTab(index);

    splitter->addWidget(tab);
    splitter->addWidget(newTab);

    m_tabWidget->insertTab(index, splitter, tabText);
    m_tabWidget->setTabToolTip(index, tabToolTip);

    addTabAction(newTab);
    connectTab(newTab);

    m_tabWidget->setCurrentIndex(index);
    tab->setFocus();

    splitter->setUniformSizes();
    tab->show();
    newTab->show();

    if(s_settings->mainWindow().synchronizeSplitViews())
    {
        DocumentView* const oldTab = findCurrentTab(tab);

        connect(oldTab, SIGNAL(currentPageChanged(int,bool)), newTab, SLOT(jumpToPage(int,bool)));
        connect(oldTab->horizontalScrollBar(), SIGNAL(valueChanged(int)), newTab->horizontalScrollBar(), SLOT(setValue(int)));
        connect(oldTab->verticalScrollBar(), SIGNAL(valueChanged(int)), newTab->verticalScrollBar(), SLOT(setValue(int)));
    }
}

void MainWindow::on_splitView_closeCurrent_triggered()
{
    on_splitView_closeCurrent_triggered(m_tabWidget->currentIndex());
}

void MainWindow::on_splitView_closeCurrent_triggered(int index)
{
    DocumentView* const tab = currentTab(index);

    if(saveModifications(tab))
    {
        closeTab(tab);
    }
}

void MainWindow::on_splitView_currentWidgetChanged(QWidget* currentWidget)
{
    for(QWidget* parentWidget = currentWidget->parentWidget(); parentWidget != 0; parentWidget = parentWidget->parentWidget())
    {
        if(parentWidget == m_tabWidget->currentWidget())
        {
            on_tabWidget_currentChanged();

            return;
        }
    }
}

#undef ONLY_IF_SENDER_IS_CURRENT_TAB

void MainWindow::on_currentPage_editingFinished()
{
    if(m_tabWidget->hasCurrent())
    {
        currentTab()->jumpToPage(m_currentPageSpinBox->value());
    }
}

void MainWindow::on_currentPage_returnPressed()
{
    currentTab()->setFocus();
}

void MainWindow::on_scaleFactor_activated(int index)
{
    if(index == 0)
    {
        currentTab()->setScaleMode(FitToPageWidthMode);
    }
    else if(index == 1)
    {
        currentTab()->setScaleMode(FitToPageSizeMode);
    }
    else
    {
        bool ok = false;
        const qreal scaleFactor = m_scaleFactorComboBox->itemData(index).toReal(&ok);

        if(ok)
        {
            currentTab()->setScaleFactor(scaleFactor);
            currentTab()->setScaleMode(ScaleFactorMode);
        }
    }

    currentTab()->setFocus();
}

void MainWindow::on_scaleFactor_editingFinished()
{
    if(m_tabWidget->hasCurrent())
    {
        bool ok = false;
        qreal scaleFactor = m_scaleFactorComboBox->lineEdit()->text().toInt(&ok) / 100.0;

        scaleFactor = qMax(scaleFactor, s_settings->documentView().minimumScaleFactor());
        scaleFactor = qMin(scaleFactor, s_settings->documentView().maximumScaleFactor());

        if(ok)
        {
            currentTab()->setScaleFactor(scaleFactor);
            currentTab()->setScaleMode(ScaleFactorMode);
        }

        on_currentTab_scaleFactorChanged(currentTab()->scaleFactor());
        on_currentTab_scaleModeChanged(currentTab()->scaleMode());
    }
}

void MainWindow::on_scaleFactor_returnPressed()
{
    currentTab()->setFocus();
}

void MainWindow::on_open_triggered()
{
    if(m_tabWidget->hasCurrent())
    {
        const QString path = s_settings->mainWindow().openPath();
        const QString filePath = QFileDialog::getOpenFileName(this, tr("Open"), path, DocumentView::openFilter().join(";;"));

        if(!filePath.isEmpty())
        {
            open(filePath);
        }
    }
    else
    {
        on_openInNewTab_triggered();
    }
}

void MainWindow::on_openInNewTab_triggered()
{
    const QString path = s_settings->mainWindow().openPath();
    const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open in new tab"), path, DocumentView::openFilter().join(";;"));

    if(!filePaths.isEmpty())
    {
        CurrentTabChangeBlocker currentTabChangeBlocker(this);

        foreach(const QString& filePath, filePaths)
        {
            openInNewTab(filePath);
        }
    }
}

void MainWindow::on_openCopyInNewTab_triggered()
{
    on_openCopyInNewTab_triggered(currentTab());
}

void MainWindow::on_openCopyInNewTab_triggered(const DocumentView* tab)
{
    openInNewTab(tab->fileInfo().filePath(), tab->currentPage());
}

void MainWindow::on_openCopyInNewWindow_triggered()
{
    on_openCopyInNewWindow_triggered(currentTab());
}

void MainWindow::on_openCopyInNewWindow_triggered(const DocumentView* tab)
{
    openInNewWindow(tab->fileInfo().absoluteFilePath(), tab->currentPage());
}

void MainWindow::on_openContainingFolder_triggered()
{
    on_openContainingFolder_triggered(currentTab());
}

void MainWindow::on_openContainingFolder_triggered(const DocumentView* tab)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(tab->fileInfo().absolutePath()));
}

void MainWindow::on_moveToInstance_triggered()
{
    on_moveToInstance_triggered(currentTab());
}

void MainWindow::on_moveToInstance_triggered(DocumentView* tab)
{
#ifdef WITH_DBUS

    bool ok = false;
    const QString instanceName = QInputDialog::getItem(this, tr("Choose instance"), tr("Instance:"), s_database->knownInstanceNames(), 0, true, &ok);

    if(!ok || instanceName == qApp->objectName())
    {
        return;
    }

    if(!saveModifications(tab))
    {
        return;
    }

    QScopedPointer< QDBusInterface > interface(MainWindowAdaptor::createInterface(instanceName));

    if(!interface->isValid())
    {
        QMessageBox::warning(this, tr("Move to instance"), tr("Failed to access instance '%1'.").arg(instanceName));
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    interface->call("raiseAndActivate");

    QDBusReply< bool > reply = interface->call("jumpToPageOrOpenInNewTab", tab->fileInfo().absoluteFilePath(), tab->currentPage(), true, QRectF(), false);

    if(!reply.isValid() || !reply.value())
    {
        QMessageBox::warning(this, tr("Move to instance"), tr("Failed to access instance '%1'.").arg(instanceName));
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    interface->call("saveDatabase");

    closeTab(tab);

#else

    Q_UNUSED(tab);

    QMessageBox::information(this, tr("Information"), tr("Instance-to-instance communication requires D-Bus support."));

#endif // WITH_DBUS
}

void MainWindow::on_refresh_triggered()
{
    DocumentView* const tab = currentTab();

    if(saveModifications(tab) && !tab->refresh())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not refresh '%1'.").arg(currentTab()->fileInfo().filePath()));
    }
}

void MainWindow::on_save_triggered()
{
    DocumentView* const tab = currentTab();
    const QString filePath = tab->fileInfo().filePath();

    if(!tab->save(filePath, true))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not save as '%1'.").arg(filePath));
        return;
    }

    if(!tab->refresh())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not refresh '%1'.").arg(filePath));
    }
}

void MainWindow::on_saveAs_triggered()
{
    DocumentView* const tab = currentTab();
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save as"), tab->fileInfo().filePath(), tab->saveFilter().join(";;"));

    if(filePath.isEmpty())
    {
        return;
    }

    if(!tab->save(filePath, true))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not save as '%1'.").arg(filePath));
        return;
    }

    open(filePath, tab->currentPage());
}

void MainWindow::on_saveCopy_triggered()
{
    const QDir dir = QDir(s_settings->mainWindow().savePath());
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save copy"), dir.filePath(currentTab()->fileInfo().fileName()), currentTab()->saveFilter().join(";;"));

    if(!filePath.isEmpty())
    {
        if(currentTab()->save(filePath, false))
        {
            s_settings->mainWindow().setSavePath(QFileInfo(filePath).absolutePath());
        }
        else
        {
            QMessageBox::warning(this, tr("Warning"), tr("Could not save copy at '%1'.").arg(filePath));
        }
    }
}

void MainWindow::on_print_triggered()
{
    QScopedPointer< QPrinter > printer(PrintDialog::createPrinter());
    QScopedPointer< PrintDialog > printDialog(new PrintDialog(printer.data(), this));

    printer->setDocName(currentTab()->fileInfo().completeBaseName());
    printer->setFullPage(true);

    printDialog->setMinMax(1, currentTab()->numberOfPages());
    printDialog->setOption(QPrintDialog::PrintToFile, false);

#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)

    printDialog->setOption(QPrintDialog::PrintCurrentPage, true);

#endif // QT_VERSION

    if(printDialog->exec() != QDialog::Accepted)
    {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(4,7,0)

    if(printDialog->printRange() == QPrintDialog::CurrentPage)
    {
        printer->setFromTo(currentTab()->currentPage(), currentTab()->currentPage());
    }

#endif // QT_VERSION

    if(!currentTab()->print(printer.data(), printDialog->printOptions()))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Could not print '%1'.").arg(currentTab()->fileInfo().filePath()));
    }
}

void MainWindow::on_recentlyUsed_openTriggered(const QString& filePath)
{
    if(!jumpToPageOrOpenInNewTab(filePath, -1, true))
    {
        m_recentlyUsedMenu->removeOpenAction(filePath);
    }
}

void MainWindow::on_previousPage_triggered()
{
    currentTab()->previousPage();
}

void MainWindow::on_nextPage_triggered()
{
    currentTab()->nextPage();
}

void MainWindow::on_firstPage_triggered()
{
    currentTab()->firstPage();
}

void MainWindow::on_lastPage_triggered()
{
    currentTab()->lastPage();
}

void MainWindow::on_setFirstPage_triggered()
{
    bool ok = false;
    const int pageNumber = getMappedNumber(new TextValueMapper(this),
                                           this, tr("Set first page"), tr("Select the first page of the body matter:"),
                                           currentTab()->currentPage(), 1, currentTab()->numberOfPages(), &ok);

    if(ok)
    {
        currentTab()->setFirstPage(pageNumber);
    }
}

void MainWindow::on_jumpToPage_triggered()
{
    bool ok = false;
    const int pageNumber = getMappedNumber(new TextValueMapper(this),
                                           this, tr("Jump to page"), tr("Page:"),
                                           currentTab()->currentPage(), 1, currentTab()->numberOfPages(), &ok);

    if(ok)
    {
        currentTab()->jumpToPage(pageNumber);
    }
}

void MainWindow::on_jumpBackward_triggered()
{
    currentTab()->jumpBackward();
}

void MainWindow::on_jumpForward_triggered()
{
    currentTab()->jumpForward();
}

void MainWindow::on_search_triggered()
{
    m_searchDock->setVisible(true);
    m_searchDock->raise();

    m_searchLineEdit->selectAll();
    m_searchLineEdit->setFocus();
}

void MainWindow::on_findPrevious_triggered()
{
    if(!m_searchLineEdit->text().isEmpty())
    {
        currentTab()->findPrevious();
    }
}

void MainWindow::on_findNext_triggered()
{
    if(!m_searchLineEdit->text().isEmpty())
    {
        currentTab()->findNext();
    }
}

void MainWindow::on_cancelSearch_triggered()
{
    m_searchLineEdit->stopTimer();
    m_searchLineEdit->setProgress(0);

    foreach(DocumentView* tab, allTabs())
    {
        tab->cancelSearch();
    }

    if(!s_settings->mainWindow().extendedSearchDock())
    {
        m_searchDock->setVisible(false);
    }
}

void MainWindow::on_copyToClipboardMode_triggered(bool checked)
{
    currentTab()->setRubberBandMode(checked ? CopyToClipboardMode : ModifiersMode);
}

void MainWindow::on_addAnnotationMode_triggered(bool checked)
{
    currentTab()->setRubberBandMode(checked ? AddAnnotationMode : ModifiersMode);
}

void MainWindow::on_settings_triggered()
{
    QScopedPointer< SettingsDialog > settingsDialog(new SettingsDialog(this));

    if(settingsDialog->exec() != QDialog::Accepted)
    {
        return;
    }

    s_settings->sync();

    m_tabWidget->setTabPosition(static_cast< QTabWidget::TabPosition >(s_settings->mainWindow().tabPosition()));
    m_tabWidget->setTabBarPolicy(static_cast< TabWidget::TabBarPolicy >(s_settings->mainWindow().tabVisibility()));
    m_tabWidget->setSpreadTabs(s_settings->mainWindow().spreadTabs());

    m_tabsMenu->setSearchable(s_settings->mainWindow().searchableMenus());
    m_bookmarksMenu->setSearchable(s_settings->mainWindow().searchableMenus());

    foreach(DocumentView* tab, allTabs())
    {
        if(saveModifications(tab) && !tab->refresh())
        {
            QMessageBox::warning(this, tr("Warning"), tr("Could not refresh '%1'.").arg(currentTab()->fileInfo().filePath()));
        }
    }
}

void MainWindow::on_continuousMode_triggered(bool checked)
{
    currentTab()->setContinuousMode(checked);
}

void MainWindow::on_twoPagesMode_triggered(bool checked)
{
    currentTab()->setLayoutMode(checked ? TwoPagesMode : SinglePageMode);
}

void MainWindow::on_twoPagesWithCoverPageMode_triggered(bool checked)
{
    currentTab()->setLayoutMode(checked ? TwoPagesWithCoverPageMode : SinglePageMode);
}

void MainWindow::on_multiplePagesMode_triggered(bool checked)
{
    currentTab()->setLayoutMode(checked ? MultiplePagesMode : SinglePageMode);
}

void MainWindow::on_rightToLeftMode_triggered(bool checked)
{
    currentTab()->setRightToLeftMode(checked);
}

void MainWindow::on_zoomIn_triggered()
{
    currentTab()->zoomIn();
}

void MainWindow::on_zoomOut_triggered()
{
    currentTab()->zoomOut();
}

void MainWindow::on_originalSize_triggered()
{
    currentTab()->originalSize();
}

void MainWindow::on_fitToPageWidthMode_triggered(bool checked)
{
    currentTab()->setScaleMode(checked ? FitToPageWidthMode : ScaleFactorMode);
}

void MainWindow::on_fitToPageSizeMode_triggered(bool checked)
{
    currentTab()->setScaleMode(checked ? FitToPageSizeMode : ScaleFactorMode);
}

void MainWindow::on_rotateLeft_triggered()
{
    currentTab()->rotateLeft();
}

void MainWindow::on_rotateRight_triggered()
{
    currentTab()->rotateRight();
}

void MainWindow::on_invertColors_triggered(bool checked)
{
    currentTab()->setInvertColors(checked);
}

void MainWindow::on_convertToGrayscale_triggered(bool checked)
{
    currentTab()->setConvertToGrayscale(checked);
}

void MainWindow::on_trimMargins_triggered(bool checked)
{
    currentTab()->setTrimMargins(checked);
}

void MainWindow::on_darkenWithPaperColor_triggered(bool checked)
{
    currentTab()->setCompositionMode(checked ? DarkenWithPaperColorMode : DefaultCompositionMode);
}

void MainWindow::on_lightenWithPaperColor_triggered(bool checked)
{
    currentTab()->setCompositionMode(checked ? LightenWithPaperColorMode : DefaultCompositionMode);
}

void MainWindow::on_fonts_triggered()
{
    QScopedPointer< QAbstractItemModel > fontsModel(currentTab()->fontsModel());
    QScopedPointer< FontsDialog > dialog(new FontsDialog(fontsModel.data(), this));

    dialog->exec();
}

void MainWindow::on_fullscreen_triggered(bool checked)
{
    if(checked)
    {
        m_fullscreenAction->setData(saveGeometry());

        showFullScreen();
    }
    else
    {
        restoreGeometry(m_fullscreenAction->data().toByteArray());

        showNormal();

        restoreGeometry(m_fullscreenAction->data().toByteArray());
    }

    if(s_settings->mainWindow().toggleToolAndMenuBarsWithFullscreen())
    {
        m_toggleToolBarsAction->trigger();
        m_toggleMenuBarAction->trigger();
    }
}

void MainWindow::on_presentation_triggered()
{
    currentTab()->startPresentation();
}

void MainWindow::on_previousTab_triggered()
{
    m_tabWidget->previousTab();
}

void MainWindow::on_nextTab_triggered()
{
    m_tabWidget->nextTab();
}

void MainWindow::on_closeTab_triggered()
{
    on_closeTabs_triggered(allTabs(m_tabWidget->currentIndex()));
}

void MainWindow::on_closeAllTabs_triggered()
{
    on_closeTabs_triggered(allTabs());
}

void MainWindow::on_closeAllTabsButCurrentTab_triggered()
{
    on_closeAllTabsButThisOne_triggered(m_tabWidget->currentIndex());
}

void MainWindow::on_closeAllTabsButThisOne_triggered(int thisIndex)
{
    QVector< DocumentView* > tabs;

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        if(index != thisIndex)
        {
            tabs += allTabs(index);
        }
    }

    on_closeTabs_triggered(tabs);
}

void MainWindow::on_closeAllTabsToTheLeft_triggered(int ofIndex)
{
    QVector< DocumentView* > tabs;

    for(int index = 0; index < ofIndex; ++index)
    {
        tabs += allTabs(index);
    }

    on_closeTabs_triggered(tabs);
}

void MainWindow::on_closeAllTabsToTheRight_triggered(int ofIndex)
{
    QVector< DocumentView* > tabs;

    for(int index = ofIndex + 1, count = m_tabWidget->count(); index < count; ++index)
    {
        tabs += allTabs(index);
    }

    on_closeTabs_triggered(tabs);
}

void MainWindow::on_closeTabs_triggered(const QVector< DocumentView* >& tabs)
{
    CurrentTabChangeBlocker currentTabChangeBlocker(this);

    foreach(DocumentView* tab, tabs)
    {
        if(saveModifications(tab))
        {
            closeTab(tab);
        }
    }
}

void MainWindow::on_restoreMostRecentlyClosedTab_triggered()
{
    m_recentlyClosedMenu->triggerLastTabAction();
}

void MainWindow::on_recentlyClosed_tabActionTriggered(QAction* tabAction)
{
    DocumentView* tab = static_cast< DocumentView* >(tabAction->parent());

    tab->setParent(m_tabWidget);
    tab->setVisible(true);

    addTab(tab);
    m_tabsMenu->addAction(tabAction);
}

void MainWindow::on_tabAction_triggered()
{
    DocumentView* const senderTab = static_cast< DocumentView* >(sender()->parent());

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        if(allTabs(index).contains(senderTab))
        {
            m_tabWidget->setCurrentIndex(index);
            senderTab->setFocus();

            break;
        }
    }
}

void MainWindow::on_tabShortcut_activated()
{
    for(int index = 0; index < 9; ++index)
    {
        if(sender() == m_tabShortcuts[index])
        {
            m_tabWidget->setCurrentIndex(index);

            break;
        }
    }
}

void MainWindow::on_previousBookmark_triggered()
{
    if(const BookmarkModel* model = bookmarkModelForCurrentTab())
    {
        QList< int > pages;

        for(int row = 0, rowCount = model->rowCount(); row < rowCount; ++row)
        {
            pages.append(model->index(row).data(BookmarkModel::PageRole).toInt());
        }

        if(!pages.isEmpty())
        {
            qSort(pages);

            QList< int >::const_iterator lowerBound = --qLowerBound(pages, currentTab()->currentPage());

            if(lowerBound >= pages.constBegin())
            {
                currentTab()->jumpToPage(*lowerBound);
            }
            else
            {
                currentTab()->jumpToPage(pages.last());
            }
        }
    }
}

void MainWindow::on_nextBookmark_triggered()
{
    if(const BookmarkModel* model = bookmarkModelForCurrentTab())
    {
        QList< int > pages;

        for(int row = 0, rowCount = model->rowCount(); row < rowCount; ++row)
        {
            pages.append(model->index(row).data(BookmarkModel::PageRole).toInt());
        }

        if(!pages.isEmpty())
        {
            qSort(pages);

            QList< int >::const_iterator upperBound = qUpperBound(pages, currentTab()->currentPage());

            if(upperBound < pages.constEnd())
            {
                currentTab()->jumpToPage(*upperBound);
            }
            else
            {
                currentTab()->jumpToPage(pages.first());
            }
        }
    }
}

void MainWindow::on_addBookmark_triggered()
{
    const QString& currentPageLabel = s_settings->mainWindow().usePageLabel() || currentTab()->hasFrontMatter()
            ? currentTab()->pageLabelFromNumber(currentTab()->currentPage())
            : currentTab()->defaultPageLabelFromNumber(currentTab()->currentPage());

    BookmarkItem bookmark(currentTab()->currentPage(), tr("Jump to page %1").arg(currentPageLabel));

    BookmarkModel* model = bookmarkModelForCurrentTab(false);

    if(model != 0)
    {
        model->findBookmark(bookmark);
    }

    QScopedPointer< BookmarkDialog > dialog(new BookmarkDialog(bookmark, this));

    if(dialog->exec() == QDialog::Accepted)
    {
        if(model == 0)
        {
            model = bookmarkModelForCurrentTab(true);

            m_bookmarksView->setModel(model);
        }

        model->addBookmark(bookmark);

        m_bookmarksMenuIsDirty = true;
        scheduleSaveBookmarks();
    }
}

void MainWindow::on_removeBookmark_triggered()
{
    BookmarkModel* model = bookmarkModelForCurrentTab();

    if(model != 0)
    {
        model->removeBookmark(BookmarkItem(currentTab()->currentPage()));

        if(model->isEmpty())
        {
            m_bookmarksView->setModel(0);

            BookmarkModel::removePath(currentTab()->fileInfo().absoluteFilePath());
        }

        m_bookmarksMenuIsDirty = true;
        scheduleSaveBookmarks();
    }
}

void MainWindow::on_removeAllBookmarks_triggered()
{
    m_bookmarksView->setModel(0);

    BookmarkModel::removeAllPaths();

    m_bookmarksMenuIsDirty = true;
    scheduleSaveBookmarks();
}

void MainWindow::on_bookmarksMenu_aboutToShow()
{
    if(!m_bookmarksMenuIsDirty)
    {
        return;
    }

    m_bookmarksMenuIsDirty = false;


    m_bookmarksMenu->clear();

    m_bookmarksMenu->addActions(QList< QAction* >() << m_previousBookmarkAction << m_nextBookmarkAction);
    m_bookmarksMenu->addSeparator();
    m_bookmarksMenu->addActions(QList< QAction* >() << m_addBookmarkAction << m_removeBookmarkAction << m_removeAllBookmarksAction);
    m_bookmarksMenu->addSeparator();

    foreach(const QString& absoluteFilePath, BookmarkModel::paths())
    {
        const BookmarkModel* model = BookmarkModel::fromPath(absoluteFilePath);

        BookmarkMenu* menu = new BookmarkMenu(QFileInfo(absoluteFilePath), m_bookmarksMenu);

        for(int row = 0, rowCount = model->rowCount(); row < rowCount; ++row)
        {
            const QModelIndex index = model->index(row);

            menu->addJumpToPageAction(index.data(BookmarkModel::PageRole).toInt(), index.data(BookmarkModel::LabelRole).toString());
        }

        connect(menu, SIGNAL(openTriggered(QString)), SLOT(on_bookmark_openTriggered(QString)));
        connect(menu, SIGNAL(openInNewTabTriggered(QString)), SLOT(on_bookmark_openInNewTabTriggered(QString)));
        connect(menu, SIGNAL(jumpToPageTriggered(QString,int)), SLOT(on_bookmark_jumpToPageTriggered(QString,int)));
        connect(menu, SIGNAL(removeBookmarkTriggered(QString)), SLOT(on_bookmark_removeBookmarkTriggered(QString)));

        m_bookmarksMenu->addMenu(menu);
    }
}

void MainWindow::on_bookmark_openTriggered(const QString& absoluteFilePath)
{
    if(m_tabWidget->hasCurrent())
    {
        open(absoluteFilePath);
    }
    else
    {
        openInNewTab(absoluteFilePath);
    }
}

void MainWindow::on_bookmark_openInNewTabTriggered(const QString& absoluteFilePath)
{
    openInNewTab(absoluteFilePath);
}

void MainWindow::on_bookmark_jumpToPageTriggered(const QString& absoluteFilePath, int page)
{
    jumpToPageOrOpenInNewTab(absoluteFilePath, page);
}

void MainWindow::on_bookmark_removeBookmarkTriggered(const QString& absoluteFilePath)
{
    BookmarkModel* model = BookmarkModel::fromPath(absoluteFilePath);

    if(model == m_bookmarksView->model())
    {
        m_bookmarksView->setModel(0);
    }

    BookmarkModel::removePath(absoluteFilePath);

    m_bookmarksMenuIsDirty = true;
    scheduleSaveBookmarks();
}

void MainWindow::on_contents_triggered()
{
    if(m_helpDialog.isNull())
    {
        m_helpDialog = new HelpDialog();

        m_helpDialog->show();
        m_helpDialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(this, SIGNAL(destroyed()), m_helpDialog, SLOT(close()));
    }

    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

void MainWindow::on_about_triggered()
{
    QMessageBox::about(this, tr("About qpdfview"), (tr("<p><b>qpdfview %1</b></p><p>qpdfview is a tabbed document viewer using Qt.</p>"
                                                      "<p>This version includes:"
                                                      "<ul>").arg(APPLICATION_VERSION)
#ifdef WITH_PDF
                                                      + tr("<li>PDF support using Poppler %1</li>").arg(POPPLER_VERSION)
#endif // WITH_PDF
#ifdef WITH_PS
                                                      + tr("<li>PS support using libspectre %1</li>").arg(LIBSPECTRE_VERSION)
#endif // WITH_PS
#ifdef WITH_DJVU
                                                      + tr("<li>DjVu support using DjVuLibre %1</li>").arg(DJVULIBRE_VERSION)
#endif // WITH_DJVU
#ifdef WITH_FITZ
                                                      + tr("<li>PDF support using Fitz %1</li>").arg(FITZ_VERSION)
#endif // WITH_FITZ
#ifdef WITH_CUPS
                                                      + tr("<li>Printing support using CUPS %1</li>").arg(CUPS_VERSION)
#endif // WITH_CUPS
                                                      + tr("</ul>"
                                                           "<p>See <a href=\"https://launchpad.net/qpdfview\">launchpad.net/qpdfview</a> for more information.</p>"
							   "<p>&copy; %1 The qpdfview developers</p>").arg("2012-2018")));
}

void MainWindow::on_focusCurrentPage_activated()
{
    m_currentPageSpinBox->setFocus();
    m_currentPageSpinBox->selectAll();
}

void MainWindow::on_focusScaleFactor_activated()
{
    m_scaleFactorComboBox->setFocus();
    m_scaleFactorComboBox->lineEdit()->selectAll();
}

void MainWindow::on_toggleToolBars_triggered(bool checked)
{
    if(checked)
    {
        m_tabWidget->setTabBarPolicy(static_cast< TabWidget::TabBarPolicy >(m_tabBarHadPolicy));

        m_fileToolBar->setVisible(m_fileToolBarWasVisible);
        m_editToolBar->setVisible(m_editToolBarWasVisible);
        m_viewToolBar->setVisible(m_viewToolBarWasVisible);
    }
    else
    {
        m_tabBarHadPolicy = static_cast< int >(m_tabWidget->tabBarPolicy());

        m_fileToolBarWasVisible = m_fileToolBar->isVisible();
        m_editToolBarWasVisible = m_editToolBar->isVisible();
        m_viewToolBarWasVisible = m_viewToolBar->isVisible();

        m_tabWidget->setTabBarPolicy(TabWidget::TabBarAlwaysOff);

        m_fileToolBar->setVisible(false);
        m_editToolBar->setVisible(false);
        m_viewToolBar->setVisible(false);
    }
}

void MainWindow::on_toggleMenuBar_triggered(bool checked)
{
    menuBar()->setVisible(checked);
}

void MainWindow::on_searchInitiated(const QString& text, bool modified)
{
    if(text.isEmpty())
    {
        return;
    }

    const bool forAllTabs = s_settings->mainWindow().extendedSearchDock() ? !modified : modified;
    const bool matchCase = m_matchCaseCheckBox->isChecked();
    const bool wholeWords = m_wholeWordsCheckBox->isChecked();

    if(forAllTabs)
    {
        foreach(DocumentView* tab, allTabs())
        {
            tab->startSearch(text, matchCase, wholeWords);
        }
    }
    else
    {
        DocumentView* const tab = currentTab();

        if(tab->searchText() != text || tab->searchWasCanceled())
        {
            tab->startSearch(text, matchCase, wholeWords);
        }
        else
        {
            tab->findNext();
        }
    }
}

void MainWindow::on_highlightAll_clicked(bool checked)
{
    currentTab()->setHighlightAll(checked);
}

void MainWindow::on_dock_dockLocationChanged(Qt::DockWidgetArea area)
{
    QDockWidget* dock = qobject_cast< QDockWidget* >(sender());

    if(dock == 0)
    {
        return;
    }

    QDockWidget::DockWidgetFeatures features = dock->features();

    if(area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea)
    {
        features |= QDockWidget::DockWidgetVerticalTitleBar;
    }
    else
    {
        features &= ~QDockWidget::DockWidgetVerticalTitleBar;
    }

    dock->setFeatures(features);
}

void MainWindow::on_outline_sectionCountChanged()
{
    setSectionResizeMode(m_outlineView->header(), 0, QHeaderView::Stretch);
    setSectionResizeMode(m_outlineView->header(), 1, QHeaderView::ResizeToContents);

    m_outlineView->header()->setMinimumSectionSize(0);
    m_outlineView->header()->setStretchLastSection(false);
    m_outlineView->header()->setVisible(false);
}

void MainWindow::on_outline_clicked(const QModelIndex& index)
{
    bool ok = false;
    const int page = index.data(Model::Document::PageRole).toInt(&ok);

    if(!ok)
    {
        return;
    }

    const qreal left = index.data(Model::Document::LeftRole).toReal();
    const qreal top = index.data(Model::Document::TopRole).toReal();
    const QString fileName = index.data(Model::Document::FileNameRole).toString();

    if(fileName.isEmpty())
    {
        currentTab()->jumpToPage(page, true, left, top);
    }
    else
    {
        jumpToPageOrOpenInNewTab(currentTab()->resolveFileName(fileName), page, true);
    }
}

void MainWindow::on_properties_sectionCountChanged()
{
    setSectionResizeMode(m_propertiesView->horizontalHeader(), 0, QHeaderView::Stretch);
    setSectionResizeMode(m_propertiesView->horizontalHeader(), 1, QHeaderView::Stretch);

    m_propertiesView->horizontalHeader()->setVisible(false);

    setSectionResizeMode(m_propertiesView->verticalHeader(), QHeaderView::ResizeToContents);

    m_propertiesView->verticalHeader()->setVisible(false);
}

void MainWindow::on_thumbnails_dockLocationChanged(Qt::DockWidgetArea area)
{
    foreach(DocumentView* tab, allTabs())
    {
        tab->setThumbnailsOrientation(area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea ? Qt::Horizontal : Qt::Vertical);
    }
}

void MainWindow::on_thumbnails_verticalScrollBar_valueChanged(int value)
{
    Q_UNUSED(value);

    if(m_thumbnailsView->scene() != 0)
    {
        const QRectF visibleRect = m_thumbnailsView->mapToScene(m_thumbnailsView->viewport()->rect()).boundingRect();

        foreach(ThumbnailItem* page, currentTab()->thumbnailItems())
        {
            if(!page->boundingRect().translated(page->pos()).intersects(visibleRect))
            {
                page->cancelRender();
            }
        }
    }
}

void MainWindow::on_bookmarks_sectionCountChanged()
{
    setSectionResizeMode(m_bookmarksView->horizontalHeader(), 0, QHeaderView::Stretch);
    setSectionResizeMode(m_bookmarksView->horizontalHeader(), 1, QHeaderView::ResizeToContents);

    m_bookmarksView->horizontalHeader()->setMinimumSectionSize(0);
    m_bookmarksView->horizontalHeader()->setStretchLastSection(false);
    m_bookmarksView->horizontalHeader()->setVisible(false);

    setSectionResizeMode(m_bookmarksView->verticalHeader(), QHeaderView::ResizeToContents);

    m_bookmarksView->verticalHeader()->setVisible(false);
}

void MainWindow::on_bookmarks_clicked(const QModelIndex& index)
{
    bool ok = false;
    const int page = index.data(BookmarkModel::PageRole).toInt(&ok);

    if(ok)
    {
        currentTab()->jumpToPage(page);
    }
}

void MainWindow::on_bookmarks_contextMenuRequested(QPoint pos)
{
    QMenu menu;

    menu.addActions(QList< QAction* >() << m_previousBookmarkAction << m_nextBookmarkAction);
    menu.addSeparator();
    menu.addAction(m_addBookmarkAction);

    QAction* removeBookmarkAction = menu.addAction(tr("&Remove bookmark"));
    QAction* editBookmarkAction = menu.addAction(tr("&Edit bookmark"));

    const QModelIndex index = m_bookmarksView->indexAt(pos);

    removeBookmarkAction->setVisible(index.isValid());
    editBookmarkAction->setVisible(index.isValid());

    const QAction* action = menu.exec(m_bookmarksView->mapToGlobal(pos));

    if(action == removeBookmarkAction)
    {
        BookmarkModel* model = qobject_cast< BookmarkModel* >(m_bookmarksView->model());

        if(model != 0)
        {
            model->removeBookmark(BookmarkItem(index.data(BookmarkModel::PageRole).toInt()));

            if(model->isEmpty())
            {
                m_bookmarksView->setModel(0);

                BookmarkModel::removePath(currentTab()->fileInfo().absoluteFilePath());
            }

            m_bookmarksMenuIsDirty = true;
            scheduleSaveBookmarks();
        }

    }
    else if(action == editBookmarkAction)
    {
        BookmarkModel* model = qobject_cast< BookmarkModel* >(m_bookmarksView->model());

        if(model != 0)
        {
            BookmarkItem bookmark(index.data(BookmarkModel::PageRole).toInt());

            model->findBookmark(bookmark);

            QScopedPointer< BookmarkDialog > dialog(new BookmarkDialog(bookmark, this));

            if(dialog->exec() == QDialog::Accepted)
            {
                model->addBookmark(bookmark);

                m_bookmarksMenuIsDirty = true;
                scheduleSaveBookmarks();
            }
        }
    }
}

void MainWindow::on_search_sectionCountChanged()
{
    setSectionResizeMode(m_searchView->header(), 0, QHeaderView::Stretch);
    setSectionResizeMode(m_searchView->header(), 1, QHeaderView::ResizeToContents);

    m_searchView->header()->setMinimumSectionSize(0);
    m_searchView->header()->setStretchLastSection(false);
    m_searchView->header()->setVisible(false);
}

void MainWindow::on_search_dockLocationChanged(Qt::DockWidgetArea area)
{
    delete m_searchWidget->layout();
    QGridLayout* searchLayout = new QGridLayout(m_searchWidget);

    if(area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea)
    {
        searchLayout->setRowStretch(2, 1);
        searchLayout->setColumnStretch(3, 1);

        searchLayout->addWidget(m_searchLineEdit, 0, 0, 1, 7);
        searchLayout->addWidget(m_matchCaseCheckBox, 1, 0);
        searchLayout->addWidget(m_wholeWordsCheckBox, 1, 1);
        searchLayout->addWidget(m_highlightAllCheckBox, 1, 2);
        searchLayout->addWidget(m_findPreviousButton, 1, 4, Qt::AlignRight);
        searchLayout->addWidget(m_findNextButton, 1, 5, Qt::AlignRight);
        searchLayout->addWidget(m_cancelSearchButton, 1, 6, Qt::AlignRight);

        if(s_settings->mainWindow().extendedSearchDock())
        {
            searchLayout->addWidget(m_searchView, 2, 0, 1, 7);
        }
    }
    else
    {
        searchLayout->setRowStretch(4, 1);
        searchLayout->setColumnStretch(1, 1);

        searchLayout->addWidget(m_searchLineEdit, 0, 0, 1, 5);
        searchLayout->addWidget(m_matchCaseCheckBox, 1, 0);
        searchLayout->addWidget(m_wholeWordsCheckBox, 2, 0);
        searchLayout->addWidget(m_highlightAllCheckBox, 3, 0);
        searchLayout->addWidget(m_findPreviousButton, 1, 2, 3, 1, Qt::AlignTop);
        searchLayout->addWidget(m_findNextButton, 1, 3, 3, 1, Qt::AlignTop);
        searchLayout->addWidget(m_cancelSearchButton, 1, 4, 3, 1, Qt::AlignTop);

        if(s_settings->mainWindow().extendedSearchDock())
        {
            searchLayout->addWidget(m_searchView, 4, 0, 1, 5);
        }
    }
}

void MainWindow::on_search_visibilityChanged(bool visible)
{
    if(!visible)
    {
        m_searchLineEdit->stopTimer();
        m_searchLineEdit->setProgress(0);

        foreach(DocumentView* tab, allTabs())
        {
            tab->cancelSearch();
            tab->clearResults();
        }

        if(DocumentView* tab = currentTab())
        {
            tab->setFocus();
        }
    }
}

void MainWindow::on_search_clicked(const QModelIndex& clickedIndex)
{
    DocumentView* const clickedTab = SearchModel::instance()->viewForIndex(clickedIndex);

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        foreach(DocumentView* tab, allTabs(index))
        {
            if(tab == clickedTab)
            {
                m_tabWidget->setCurrentIndex(index);
                tab->setFocus();

                clickedTab->findResult(clickedIndex);

                return;
            }
        }
    }
}

void MainWindow::on_search_rowsInserted(const QModelIndex& parent, int first, int last)
{
    if(parent.isValid())
    {
        return;
    }

    for(int row = first; row <= last; ++row)
    {
        const QModelIndex index = s_searchModel->index(row, 0, parent);

        if(!m_searchView->isExpanded(index) && s_searchModel->viewForIndex(index) == currentTab())
        {
            m_searchView->expand(index);
        }
    }
}

void MainWindow::on_saveDatabase_timeout()
{
    if(s_settings->mainWindow().restoreTabs())
    {
        s_database->saveTabs(allTabs());

        s_settings->mainWindow().setCurrentTabIndex(m_tabWidget->currentIndex());
    }

    if(s_settings->mainWindow().restoreBookmarks())
    {
        s_database->saveBookmarks();
    }

    if(s_settings->mainWindow().restorePerFileSettings())
    {
        foreach(DocumentView* tab, allTabs())
        {
            s_database->savePerFileSettings(tab);
        }
    }
}

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
    // This event filter is used to override any keyboard shortcuts if the outline widget has the focus.
    if(target == m_outlineView && event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* keyEvent = static_cast< QKeyEvent* >(event);

        const bool modifiers = keyEvent->modifiers().testFlag(Qt::ControlModifier) || keyEvent->modifiers().testFlag(Qt::ShiftModifier);
        const bool keys = keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down;

        if(modifiers && keys)
        {
            keyEvent->accept();
            return true;
        }
    }
    // This event filter is used to fit the thumbnails into the thumbnails view if this is enabled in the settings.
    else if(target == m_thumbnailsView && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        if(DocumentView* tab = currentTab())
        {
            tab->setThumbnailsViewportSize(m_thumbnailsView->viewport()->size());
        }
    }

    return QMainWindow::eventFilter(target, event);
}


void MainWindow::closeEvent(QCloseEvent* event)
{
   // qDebug()<<"Kapandı";

    m_searchDock->setVisible(false);

   /* for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        foreach(DocumentView* tab, allTabs(index))
        {
            if(!saveModifications(tab))
            {
                m_tabWidget->setCurrentIndex(index);
                tab->setFocus();

                event->setAccepted(false);
                return;
            }
        }
    }*/

   /* if(s_settings->mainWindow().restoreTabs())
    {
        s_database->saveTabs(allTabs());

        s_settings->mainWindow().setCurrentTabIndex(m_tabWidget->currentIndex());
    }
    else
    {
        s_database->clearTabs();
    }*/

    if(s_settings->mainWindow().restoreBookmarks())
    {
        s_database->saveBookmarks();
    }
    else
    {
        s_database->clearBookmarks();
    }

    s_settings->mainWindow().setRecentlyUsed(s_settings->mainWindow().trackRecentlyUsed() ? m_recentlyUsedMenu->filePaths() : QStringList());

    s_settings->documentView().setMatchCase(m_matchCaseCheckBox->isChecked());
    s_settings->documentView().setWholeWords(m_wholeWordsCheckBox->isChecked());

    s_settings->mainWindow().setGeometry(m_fullscreenAction->isChecked() ? m_fullscreenAction->data().toByteArray() : saveGeometry());
    s_settings->mainWindow().setState(saveState());


    QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();

        CurrentTabChangeBlocker currentTabChangeBlocker(this);

        foreach(const QUrl& url, event->mimeData()->urls())
        {
#if QT_VERSION >= QT_VERSION_CHECK(4,8,0)
            if(url.isLocalFile())
#else
            if(url.scheme() == "file")
#endif // QT_VERSION
            {
                openInNewTab(url.toLocalFile());
            }
        }
    }
}

void MainWindow::prepareStyle()
{
    if(s_settings->mainWindow().hasIconTheme())
    {
        QIcon::setThemeName(s_settings->mainWindow().iconTheme());
    }

    if(s_settings->mainWindow().hasStyleSheet())
    {
        qApp->setStyleSheet(s_settings->mainWindow().styleSheet());
    }

    ProxyStyle* style = new ProxyStyle();

    style->setScrollableMenus(s_settings->mainWindow().scrollableMenus());

    qApp->setStyle(style);
}

inline DocumentView* MainWindow::currentTab() const
{
    return findCurrentTab(m_tabWidget->currentWidget());
}

inline DocumentView* MainWindow::currentTab(int index) const
{
    return findCurrentTab(m_tabWidget->widget(index));
}

inline QVector< DocumentView* > MainWindow::allTabs(int index) const
{
    return findAllTabs(m_tabWidget->widget(index));
}

QVector< DocumentView* > MainWindow::allTabs() const
{
    QVector< DocumentView* > tabs;

    for(int index = 0, count = m_tabWidget->count(); index < count; ++index)
    {
        tabs += allTabs(index);
    }

    return tabs;
}

bool MainWindow::senderIsCurrentTab() const
{
     return sender() == currentTab() || qobject_cast< DocumentView* >(sender()) == 0;
}

void MainWindow::addTab(DocumentView* tab)
{
    m_tabWidget->addTab(tab, s_settings->mainWindow().newTabNextToCurrentTab(),
                        tab->title(), tab->fileInfo().absoluteFilePath());
}

void MainWindow::addTabAction(DocumentView* tab)
{
    QAction* tabAction = new QAction(tab->title(), tab);
    tabAction->setToolTip(tab->fileInfo().absoluteFilePath());
    tabAction->setData(true); // Flag action for search-as-you-type

    connect(tabAction, SIGNAL(triggered()), SLOT(on_tabAction_triggered()));

    m_tabsMenu->addAction(tabAction);
}

void MainWindow::connectTab(DocumentView* tab)
{
    connect(tab, SIGNAL(mainClipBoard(QPoint,bool,bool)), SLOT(mainClipBoard(QPoint,bool,bool)));//CopyMode

    connect(tab, SIGNAL(documentChanged()), SLOT(on_currentTab_documentChanged()));
    connect(tab, SIGNAL(documentModified()), SLOT(on_currentTab_documentModified()));

    connect(tab, SIGNAL(numberOfPagesChanged(int)), SLOT(on_currentTab_numberOfPagesChaned(int)));
    connect(tab, SIGNAL(currentPageChanged(int)), SLOT(on_currentTab_currentPageChanged(int)));

    connect(tab, SIGNAL(canJumpChanged(bool,bool)), SLOT(on_currentTab_canJumpChanged(bool,bool)));

    connect(tab, SIGNAL(continuousModeChanged(bool)), SLOT(on_currentTab_continuousModeChanged(bool)));
    connect(tab, SIGNAL(layoutModeChanged(LayoutMode)), SLOT(on_currentTab_layoutModeChanged(LayoutMode)));
    connect(tab, SIGNAL(rightToLeftModeChanged(bool)), SLOT(on_currentTab_rightToLeftModeChanged(bool)));
    connect(tab, SIGNAL(scaleModeChanged(ScaleMode)), SLOT(on_currentTab_scaleModeChanged(ScaleMode)));
    connect(tab, SIGNAL(scaleFactorChanged(qreal)), SLOT(on_currentTab_scaleFactorChanged(qreal)));
    connect(tab, SIGNAL(rotationChanged(Rotation)), SLOT(on_currentTab_rotationChanged(Rotation)));

    connect(tab, SIGNAL(linkClicked(int)), SLOT(on_currentTab_linkClicked(int)));
    connect(tab, SIGNAL(linkClicked(bool,QString,int)), SLOT(on_currentTab_linkClicked(bool,QString,int)));

    connect(tab, SIGNAL(renderFlagsChanged(qpdfview::RenderFlags)), SLOT(on_currentTab_renderFlagsChanged(qpdfview::RenderFlags)));

    connect(tab, SIGNAL(invertColorsChanged(bool)), SLOT(on_currentTab_invertColorsChanged(bool)));
    connect(tab, SIGNAL(convertToGrayscaleChanged(bool)), SLOT(on_currentTab_convertToGrayscaleChanged(bool)));
    connect(tab, SIGNAL(trimMarginsChanged(bool)), SLOT(on_currentTab_trimMarginsChanged(bool)));

    connect(tab, SIGNAL(compositionModeChanged(CompositionMode)), SLOT(on_currentTab_compositionModeChanged(CompositionMode)));

    connect(tab, SIGNAL(highlightAllChanged(bool)), SLOT(on_currentTab_highlightAllChanged(bool)));
    connect(tab, SIGNAL(rubberBandModeChanged(RubberBandMode)), SLOT(on_currentTab_rubberBandModeChanged(RubberBandMode)));

    connect(tab, SIGNAL(searchFinished()), SLOT(on_currentTab_searchFinished()));
    connect(tab, SIGNAL(searchProgressChanged(int)), SLOT(on_currentTab_searchProgressChanged(int)));

    connect(tab, SIGNAL(customContextMenuRequested(QPoint)), SLOT(on_currentTab_customContextMenuRequested(QPoint)));
}

void MainWindow::restorePerFileSettings(DocumentView* tab)
{
    s_database->restorePerFileSettings(tab);

    if(m_outlineView->model() == tab->outlineModel())
    {
        m_outlineView->restoreExpansion();
    }
}

bool MainWindow::saveModifications(DocumentView* tab)
{
    s_database->savePerFileSettings(tab);
    scheduleSaveTabs();

    if(tab->wasModified())
    {
        const int button = QMessageBox::warning(this, tr("Warning"), tr("The document '%1' has been modified. Do you want to save your changes?").arg(tab->fileInfo().filePath()), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        if(button == QMessageBox::Save)
        {
            if(tab->save(tab->fileInfo().filePath(), true))
            {
                return true;
            }
            else
            {
                QMessageBox::warning(this, tr("Warning"), tr("Could not save as '%1'.").arg(tab->fileInfo().filePath()));
            }
        }
        else if(button == QMessageBox::Discard)
        {
            return true;
        }

        return false;
    }

    return true;
}

void MainWindow::closeTab(DocumentView* tab)
{
    const int tabIndex = m_tabWidget->indexOf(tab);

    if(s_settings->mainWindow().keepRecentlyClosed() && tabIndex != -1)
    {
        foreach(QAction* tabAction, m_tabsMenu->actions())
        {
            if(tabAction->parent() == tab)
            {
                m_tabsMenu->removeAction(tabAction);
                m_tabWidget->removeTab(tabIndex);

                tab->setParent(this);
                tab->setVisible(false);

                tab->clearResults();

                m_recentlyClosedMenu->addTabAction(tabAction);

                break;
            }
        }
    }
    else
    {
        Splitter* const splitter = qobject_cast< Splitter* >(tab->parentWidget());

        delete tab;

        if(splitter != 0)
        {
            if(splitter->count() > 0)
            {
                splitter->widget(0)->setFocus();
            }
            else
            {
                delete splitter;
            }
        }

        if(s_settings->mainWindow().exitAfterLastTab() && m_tabWidget->count() == 0)
        {
            close();
        }
    }
}

void MainWindow::setWindowTitleForCurrentTab()
{
    QString tabText;
    QString instanceText;

    if(DocumentView* tab = currentTab())
    {
        QString currentPage;

        if(s_settings->mainWindow().currentPageInWindowTitle())
        {
            currentPage = QString(" (%1 / %2)").arg(tab->currentPage()).arg(tab->numberOfPages());
        }

        tabText = m_tabWidget->currentTabText() + currentPage + QLatin1String("[*] - ");
    }

    const QString instanceName = qApp->objectName();

    if(s_settings->mainWindow().instanceNameInWindowTitle() && !instanceName.isEmpty())
    {
        instanceText = QLatin1String(" (") + instanceName + QLatin1String(")");
    }

  ///  setWindowTitle(tabText + QLatin1String("qpdfview") + instanceText); //iptal edildi
}

void MainWindow::setCurrentPageSuffixForCurrentTab()
{
    QString suffix;

    if(DocumentView* tab = currentTab())
    {
        const int currentPage = tab->currentPage();
        const int numberOfPages = tab->numberOfPages();

        const QString& defaultPageLabel = tab->defaultPageLabelFromNumber(currentPage);
        const QString& pageLabel = tab->pageLabelFromNumber(currentPage);

        const QString& lastDefaultPageLabel = tab->defaultPageLabelFromNumber(numberOfPages);

        if((s_settings->mainWindow().usePageLabel() || tab->hasFrontMatter()) && defaultPageLabel != pageLabel)
        {
            suffix = QString(" (%1 / %2)").arg(defaultPageLabel, lastDefaultPageLabel);
        }
        else
        {
            suffix = QString(" / %1").arg(lastDefaultPageLabel);
        }
    }
    else
    {
        suffix = QLatin1String(" / 1");
    }

    m_currentPageSpinBox->setSuffix(suffix);
}

BookmarkModel* MainWindow::bookmarkModelForCurrentTab(bool create)
{
    return BookmarkModel::fromPath(currentTab()->fileInfo().absoluteFilePath(), create);
}

QAction* MainWindow::sourceLinkActionForCurrentTab(QObject* parent, QPoint pos)
{
    QAction* action = createTemporaryAction(parent, QString(), QLatin1String("openSourceLink"));

    if(const DocumentView::SourceLink sourceLink = currentTab()->sourceLink(pos))
    {
        const QString fileName = QFileInfo(sourceLink.name).fileName();

        action->setText(tr("Edit '%1' at %2,%3...").arg(fileName).arg(sourceLink.line).arg(sourceLink.column));
        action->setData(QVariant::fromValue(sourceLink));
    }
    else
    {
        action->setVisible(false);
    }

    return action;
}

void MainWindow::prepareDatabase()
{
    if(s_database == 0)
    {
        s_database = Database::instance();
    }

    m_saveDatabaseTimer = new QTimer(this);
    m_saveDatabaseTimer->setSingleShot(true);

    connect(m_saveDatabaseTimer, SIGNAL(timeout()), SLOT(on_saveDatabase_timeout()));
}

void MainWindow::scheduleSaveDatabase()
{
    const int interval = s_settings->mainWindow().saveDatabaseInterval();

    if(!m_saveDatabaseTimer->isActive() && interval >= 0)
    {
        m_saveDatabaseTimer->start(interval);
    }
}

void MainWindow::scheduleSaveTabs()
{
    if(s_settings->mainWindow().restoreTabs())
    {
        scheduleSaveDatabase();
    }
}

void MainWindow::scheduleSaveBookmarks()
{
    if(s_settings->mainWindow().restoreBookmarks())
    {
        scheduleSaveDatabase();
    }
}

void MainWindow::scheduleSavePerFileSettings()
{
    if(s_settings->mainWindow().restorePerFileSettings())
    {
        scheduleSaveDatabase();
    }
}

void MainWindow::createWidgets()
{
    m_tabWidget = new TabWidget(this);

    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setElideMode(Qt::ElideRight);

    m_tabWidget->setTabPosition(static_cast< QTabWidget::TabPosition >(s_settings->mainWindow().tabPosition()));
    m_tabWidget->setTabBarPolicy(static_cast< TabWidget::TabBarPolicy >(s_settings->mainWindow().tabVisibility()));
    m_tabWidget->setSpreadTabs(s_settings->mainWindow().spreadTabs());

    setCentralWidget(m_tabWidget);

    m_currentTabChangedBlocked = false;

    connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(on_tabWidget_currentChanged()));
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(on_tabWidget_tabCloseRequested(int)));
    connect(m_tabWidget, SIGNAL(tabDragRequested(int)), SLOT(on_tabWidget_tabDragRequested(int)));
    connect(m_tabWidget, SIGNAL(tabContextMenuRequested(QPoint,int)), SLOT(on_tabWidget_tabContextMenuRequested(QPoint,int)));

    // current page

    m_currentPageSpinBox = new MappingSpinBox(new TextValueMapper(this), this);

    m_currentPageSpinBox->setAlignment(Qt::AlignCenter);
    m_currentPageSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_currentPageSpinBox->setKeyboardTracking(false);

    connect(m_currentPageSpinBox, SIGNAL(editingFinished()), SLOT(on_currentPage_editingFinished()));
    connect(m_currentPageSpinBox, SIGNAL(returnPressed()), SLOT(on_currentPage_returnPressed()));

    m_currentPageAction = new QWidgetAction(this);

    m_currentPageAction->setObjectName(QLatin1String("currentPage"));
    m_currentPageAction->setDefaultWidget(m_currentPageSpinBox);

    // scale factor

    m_scaleFactorComboBox = new ComboBox(this);

    m_scaleFactorComboBox->setEditable(true);
    m_scaleFactorComboBox->setInsertPolicy(QComboBox::NoInsert);

    m_scaleFactorComboBox->addItem(tr("Page width"));
    m_scaleFactorComboBox->addItem(tr("Page size"));
    m_scaleFactorComboBox->addItem("50 %", 0.5);
    m_scaleFactorComboBox->addItem("75 %", 0.75);
    m_scaleFactorComboBox->addItem("100 %", 1.0);
    m_scaleFactorComboBox->addItem("125 %", 1.25);
    m_scaleFactorComboBox->addItem("150 %", 1.5);
    m_scaleFactorComboBox->addItem("200 %", 2.0);
    m_scaleFactorComboBox->addItem("300 %", 3.0);
    m_scaleFactorComboBox->addItem("400 %", 4.0);
    m_scaleFactorComboBox->addItem("500 %", 5.0);

    connect(m_scaleFactorComboBox, SIGNAL(activated(int)), SLOT(on_scaleFactor_activated(int)));
    connect(m_scaleFactorComboBox->lineEdit(), SIGNAL(editingFinished()), SLOT(on_scaleFactor_editingFinished()));
    connect(m_scaleFactorComboBox->lineEdit(), SIGNAL(returnPressed()), SLOT(on_scaleFactor_returnPressed()));

    m_scaleFactorAction = new QWidgetAction(this);

    m_scaleFactorAction->setObjectName(QLatin1String("scaleFactor"));
    m_scaleFactorAction->setDefaultWidget(m_scaleFactorComboBox);

    // search

    m_searchLineEdit = new SearchLineEdit(this);
    m_matchCaseCheckBox = new QCheckBox(tr("Benzer"), this);
    m_wholeWordsCheckBox = new QCheckBox(tr("Tamamı"), this);
    m_highlightAllCheckBox = new QCheckBox(tr("Vurgulanmış"), this);

    connect(m_searchLineEdit, SIGNAL(searchInitiated(QString,bool)), SLOT(on_searchInitiated(QString,bool)));
    connect(m_matchCaseCheckBox, SIGNAL(clicked()), m_searchLineEdit, SLOT(startTimer()));
    connect(m_wholeWordsCheckBox, SIGNAL(clicked()), m_searchLineEdit, SLOT(startTimer()));
    connect(m_highlightAllCheckBox, SIGNAL(clicked(bool)), SLOT(on_highlightAll_clicked(bool)));

    m_matchCaseCheckBox->setChecked(s_settings->documentView().matchCase());
    m_wholeWordsCheckBox->setChecked(s_settings->documentView().wholeWords());
}

QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QIcon& icon, const QList< QKeySequence >& shortcuts, const char* member, bool checkable, bool checked)
{
    QAction* action = new QAction(text, this);

    action->setObjectName(objectName);
    setVisibleIcon(action, icon, !checkable);
    action->setShortcuts(shortcuts);

    if(!objectName.isEmpty())
    {
        s_shortcutHandler->registerAction(action);
    }

    if(checkable)
    {
        action->setCheckable(true);
        action->setChecked(checked);

        connect(action, SIGNAL(triggered(bool)), member);
    }
    else
    {
        connect(action, SIGNAL(triggered()), member);
    }

    addAction(action);

    return action;
}

inline QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QIcon& icon, const QKeySequence& shortcut, const char* member, bool checkable, bool checked)
{
    return createAction(text, objectName, icon, QList< QKeySequence >() << shortcut, member, checkable, checked);
}

inline QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QString& iconName, const QList< QKeySequence >& shortcuts, const char* member, bool checkable, bool checked)
{
    return createAction(text, objectName, loadIconWithFallback(iconName), shortcuts, member, checkable, checked);
}

inline QAction* MainWindow::createAction(const QString& text, const QString& objectName, const QString& iconName, const QKeySequence& shortcut, const char* member, bool checkable, bool checked)
{
    return createAction(text, objectName, iconName, QList< QKeySequence >() << shortcut, member, checkable, checked);
}
void MainWindow::slotBos()
{

}

void MainWindow::createActions()
{
    // file

    m_openAction = createAction(tr("&Open..."), QLatin1String("open"), QLatin1String("document-open"), QKeySequence::Open, SLOT(slotBos()));
    m_openInNewTabAction = createAction(tr("Open in new &tab..."), QLatin1String("openInNewTab"), QLatin1String("tab-new"), QKeySequence::AddTab, SLOT(slotBos()));
    m_refreshAction = createAction(tr("&Refresh"), QLatin1String("refresh"), QLatin1String("view-refresh"), QKeySequence(QKeySequence::Refresh), SLOT(on_refresh_triggered()));
    m_saveAction = createAction(tr("&Save"), QLatin1String("save"), QLatin1String("document-save"), QKeySequence::Save, SLOT(slotBos()));
    m_saveAsAction = createAction(tr("Save &as..."), QLatin1String("saveAs"), QLatin1String("document-save-as"), QKeySequence::SaveAs, SLOT(slotBos()));
    m_saveCopyAction = createAction(tr("Save &copy..."), QLatin1String("saveCopy"), QIcon(), QKeySequence(), SLOT(slotBos()));
    m_printAction = createAction(tr("&Print..."), QLatin1String("print"), QLatin1String("document-print"), QKeySequence::Print, SLOT(slotBos()));
    m_exitAction = createAction(tr("E&xit"), QLatin1String("exit"), QIcon::fromTheme("application-exit"), QKeySequence::Quit, SLOT(close()));
    /*buradan başla*/
    m_openAction = createAction(tr("&Open..."), QLatin1String("open"), QLatin1String("document-open"), QKeySequence::Open, SLOT(on_open_triggered()));
    m_openInNewTabAction = createAction(tr("Open in new &tab..."), QLatin1String("openInNewTab"), QLatin1String("tab-new"), QKeySequence::AddTab, SLOT(on_openInNewTab_triggered()));
    m_refreshAction = createAction(tr("&Refresh"), QLatin1String("refresh"), QLatin1String("view-refresh"), QKeySequence(QKeySequence::Refresh), SLOT(on_refresh_triggered()));
    m_saveAction = createAction(tr("&Save"), QLatin1String("save"), QLatin1String("document-save"), QKeySequence::Save, SLOT(on_save_triggered()));
    m_saveAsAction = createAction(tr("Save &as..."), QLatin1String("saveAs"), QLatin1String("document-save-as"), QKeySequence::SaveAs, SLOT(on_saveAs_triggered()));
    m_saveCopyAction = createAction(tr("Save &copy..."), QLatin1String("saveCopy"), QIcon(), QKeySequence(), SLOT(on_saveCopy_triggered()));
    m_printAction = createAction(tr("&Print..."), QLatin1String("print"), QLatin1String("document-print"), QKeySequence::Print, SLOT(on_print_triggered()));
    m_exitAction = createAction(tr("E&xit"), QLatin1String("exit"), QIcon::fromTheme("application-exit"), QKeySequence::Quit, SLOT(close()));
    /*burada bitir*/
    m_exitAction->setMenuRole(QAction::QuitRole);

    // edit

    m_previousPageAction = createAction(tr("&Previous page"), QLatin1String("previousPage"), QLatin1String("go-previous"), ShortcutHandler::defaultPreviousPage(), SLOT(on_previousPage_triggered()));
    m_nextPageAction = createAction(tr("&Next page"), QLatin1String("nextPage"), QLatin1String("go-next"), ShortcutHandler::defaultNextPage(), SLOT(on_nextPage_triggered()));

    const QList< QKeySequence > firstPageShortcuts = QList< QKeySequence >()
            << QKeySequence(Qt::Key_Home)
            << QKeySequence(Qt::KeypadModifier + Qt::Key_Home)
            << QKeySequence(Qt::ControlModifier + Qt::Key_Home)
            << QKeySequence(Qt::ControlModifier + Qt::KeypadModifier + Qt::Key_Home);
    m_firstPageAction = createAction(tr("&First page"), QLatin1String("firstPage"), QLatin1String("go-first"), firstPageShortcuts, SLOT(on_firstPage_triggered()));

    const QList< QKeySequence > lastPageShortcuts = QList< QKeySequence >()
            << QKeySequence(Qt::Key_End)
            << QKeySequence(Qt::KeypadModifier + Qt::Key_End)
            << QKeySequence(Qt::ControlModifier + Qt::Key_End)
            << QKeySequence(Qt::ControlModifier + Qt::KeypadModifier + Qt::Key_End);
    m_lastPageAction = createAction(tr("&Last page"), QLatin1String("lastPage"), QLatin1String("go-last"), lastPageShortcuts, SLOT(on_lastPage_triggered()));

    m_setFirstPageAction = createAction(tr("&Set first page..."), QLatin1String("setFirstPage"), QIcon(), QKeySequence(), SLOT(on_setFirstPage_triggered()));

    m_jumpToPageAction = createAction(tr("&Jump to page..."), QLatin1String("jumpToPage"), QLatin1String("go-jump"), ShortcutHandler::defaultJumpToPage(), SLOT(on_jumpToPage_triggered()));

    m_jumpBackwardAction = createAction(tr("Jump &backward"), QLatin1String("jumpBackward"), QLatin1String("media-seek-backward"), QKeySequence(Qt::CTRL + Qt::Key_Return), SLOT(on_jumpBackward_triggered()));
    m_jumpForwardAction = createAction(tr("Jump for&ward"), QLatin1String("jumpForward"), QLatin1String("media-seek-forward"), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Return), SLOT(on_jumpForward_triggered()));

    m_searchAction = createAction(tr("&Arama..."), QLatin1String("search"), QLatin1String("edit-find"), QKeySequence::Find, SLOT(slotSearch()));///on_search_triggered()
    m_findPreviousAction = createAction(tr("Geri Ara"), QLatin1String("findPrevious"), QLatin1String("go-up"), QKeySequence::FindPrevious, SLOT(on_findPrevious_triggered()));
    m_findNextAction = createAction(tr("İleri Ara"), QLatin1String("findNext"), QLatin1String("go-down"), QKeySequence::FindNext, SLOT(on_findNext_triggered()));
    m_cancelSearchAction = createAction(tr("Aramaktan Vazgeç"), QLatin1String("cancelSearch"), QLatin1String("process-stop"), QKeySequence(Qt::Key_Escape), SLOT(on_cancelSearch_triggered()));

    m_copyToClipboardModeAction = createAction(tr("&Copy to clipboard"), QLatin1String("copyToClipboardMode"), QLatin1String("edit-copy"), QKeySequence(Qt::CTRL + Qt::Key_C), SLOT(on_copyToClipboardMode_triggered(bool)), true);
    m_addAnnotationModeAction = createAction(tr("&Add annotation"), QLatin1String("addAnnotationMode"), QLatin1String("mail-attachment"), QKeySequence(Qt::CTRL + Qt::Key_A), SLOT(on_addAnnotationMode_triggered(bool)), true);

    m_settingsAction = createAction(tr("Settings..."), QString(), QIcon(), QKeySequence(Qt::CTRL+Qt::Key_M), SLOT(on_settings_triggered()));
    m_settingsAction->setMenuRole(QAction::PreferencesRole);

    // view

    m_continuousModeAction = createAction(tr("&Continuous"), QLatin1String("continuousMode"), QIcon(QLatin1String(":icons/continuous")), QKeySequence(Qt::CTRL + Qt::Key_7), SLOT(on_continuousMode_triggered(bool)), true);
    m_twoPagesModeAction = createAction(tr("&Two pages"), QLatin1String("twoPagesMode"), QIcon(QLatin1String(":icons/two-pages")), QKeySequence(Qt::CTRL + Qt::Key_6), SLOT(on_twoPagesMode_triggered(bool)), true);
    m_twoPagesWithCoverPageModeAction = createAction(tr("Two pages &with cover page"), QLatin1String("twoPagesWithCoverPageMode"), QIcon(QLatin1String(":icons/two-pages-with-cover-page")), QKeySequence(Qt::CTRL + Qt::Key_5), SLOT(on_twoPagesWithCoverPageMode_triggered(bool)), true);
    m_multiplePagesModeAction = createAction(tr("&Multiple pages"), QLatin1String("multiplePagesMode"), QIcon(QLatin1String(":icons/multiple-pages")), QKeySequence(Qt::CTRL + Qt::Key_4), SLOT(on_multiplePagesMode_triggered(bool)), true);

    m_rightToLeftModeAction = createAction(tr("Right to left"), QLatin1String("rightToLeftMode"), QIcon(QLatin1String(":icons/right-to-left")), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R), SLOT(on_rightToLeftMode_triggered(bool)), true);
    m_zoomInAction = createAction(tr("Zoom &in"), QLatin1String("zoomIn"), QLatin1String("zoom-in"), ShortcutHandler::defaultZoomIn(), SLOT(on_zoomIn_triggered()));
    m_zoomOutAction = createAction(tr("Zoom &out"), QLatin1String("zoomOut"), QLatin1String("zoom-out"), ShortcutHandler::defaultZoomOut(), SLOT(on_zoomOut_triggered()));
    m_originalSizeAction = createAction(tr("Original &size"), QLatin1String("originalSize"), QLatin1String("zoom-original"), QKeySequence(Qt::CTRL + Qt::Key_0), SLOT(on_originalSize_triggered()));

    m_fitToPageWidthModeAction = createAction(tr("Fit to page width"), QLatin1String("fitToPageWidthMode"), QIcon(QLatin1String(":icons/fit-to-page-width")), QKeySequence(Qt::CTRL + Qt::Key_9), SLOT(on_fitToPageWidthMode_triggered(bool)), true);
    m_fitToPageSizeModeAction = createAction(tr("Fit to page size"), QLatin1String("fitToPageSizeMode"), QIcon(QLatin1String(":icons/fit-to-page-size")), QKeySequence(Qt::CTRL + Qt::Key_8), SLOT(on_fitToPageSizeMode_triggered(bool)), true);

    m_rotateLeftAction = createAction(tr("Rotate &left"), QLatin1String("rotateLeft"), QLatin1String("object-rotate-left"), ShortcutHandler::defaultRotateLeft(), SLOT(on_rotateLeft_triggered()));
    m_rotateRightAction = createAction(tr("Rotate &right"), QLatin1String("rotateRight"), QLatin1String("object-rotate-right"), ShortcutHandler::defaultRotateRight(), SLOT(on_rotateRight_triggered()));

    m_invertColorsAction = createAction(tr("Invert colors"), QLatin1String("invertColors"), QIcon(), QKeySequence(Qt::CTRL + Qt::Key_I), SLOT(on_invertColors_triggered(bool)), true);
    m_convertToGrayscaleAction = createAction(tr("Convert to grayscale"), QLatin1String("convertToGrayscale"), QIcon(), QKeySequence(Qt::CTRL + Qt::Key_U), SLOT(on_convertToGrayscale_triggered(bool)), true);
    m_trimMarginsAction = createAction(tr("Trim margins"), QLatin1String("trimMargins"), QIcon(), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_U), SLOT(on_trimMargins_triggered(bool)), true);

    m_darkenWithPaperColorAction = createAction(tr("Darken with paper color"), QLatin1String("darkenWithPaperColor"), QIcon(), QKeySequence(), SLOT(on_darkenWithPaperColor_triggered(bool)), true);
    m_lightenWithPaperColorAction = createAction(tr("Lighten with paper color"), QLatin1String("lightenWithPaperColor"), QIcon(), QKeySequence(), SLOT(on_lightenWithPaperColor_triggered(bool)), true);

    m_fontsAction = createAction(tr("Fonts..."), QString(), QIcon(), QKeySequence(), SLOT(on_fonts_triggered()));

    m_fullscreenAction = createAction(tr("&Fullscreen"), QLatin1String("fullscreen"), QLatin1String("view-fullscreen"), ShortcutHandler::defaultFullscreen(), SLOT(on_fullscreen_triggered(bool)), true);
    m_presentationAction = createAction(tr("&Presentation..."), QLatin1String("presentation"), QLatin1String("x-office-presentation"), ShortcutHandler::defaultPresentation(), SLOT(on_presentation_triggered()));

    // tabs

    m_previousTabAction = createAction(tr("&Previous tab"), QLatin1String("previousTab"), QIcon(), ShortcutHandler::defaultPreviousTab(), SLOT(slotBos()));
    m_nextTabAction = createAction(tr("&Next tab"), QLatin1String("nextTab"), QIcon(), ShortcutHandler::defaultNextTab(), SLOT(slotBos()));
    m_closeTabAction = createAction(tr("&Close tab"), QLatin1String("closeTab"), QIcon::fromTheme("window-close"), QKeySequence(Qt::CTRL + Qt::Key_W), SLOT(slotBos()));
    m_closeAllTabsAction = createAction(tr("Close &all tabs"), QLatin1String("closeAllTabs"), QIcon(), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W), SLOT(slotBos()));
    m_closeAllTabsButCurrentTabAction = createAction(tr("Close all tabs &but current tab"), QLatin1String("closeAllTabsButCurrent"), QIcon(), QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_W), SLOT(slotBos()));
    m_restoreMostRecentlyClosedTabAction = createAction(tr("Restore &most recently closed tab"), QLatin1String("restoreMostRecentlyClosedTab"), QIcon(), QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_W), SLOT(slotBos()));
/*buradan başla*/
    m_previousTabAction = createAction(tr("&Previous tab"), QLatin1String("previousTab"), QIcon(), ShortcutHandler::defaultPreviousTab(), SLOT(on_previousTab_triggered()));
    m_nextTabAction = createAction(tr("&Next tab"), QLatin1String("nextTab"), QIcon(), ShortcutHandler::defaultNextTab(), SLOT(on_nextTab_triggered()));
    m_closeTabAction = createAction(tr("&Close tab"), QLatin1String("closeTab"), QIcon::fromTheme("window-close"), QKeySequence(Qt::CTRL + Qt::Key_W), SLOT(on_closeTab_triggered()));
    m_closeAllTabsAction = createAction(tr("Close &all tabs"), QLatin1String("closeAllTabs"), QIcon(), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W), SLOT(on_closeAllTabs_triggered()));
    m_closeAllTabsButCurrentTabAction = createAction(tr("Close all tabs &but current tab"), QLatin1String("closeAllTabsButCurrent"), QIcon(), QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_W), SLOT(on_closeAllTabsButCurrentTab_triggered()));
    m_restoreMostRecentlyClosedTabAction = createAction(tr("Restore &most recently closed tab"), QLatin1String("restoreMostRecentlyClosedTab"), QIcon(), QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_W), SLOT(on_restoreMostRecentlyClosedTab_triggered()));
/*buradan başla*/
    // tab shortcuts

    for(int index = 0; index < 9; ++index)
    {
        m_tabShortcuts[index] = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1 + index), this, SLOT(on_tabShortcut_activated()));
    }

    // bookmarks

    m_previousBookmarkAction = createAction(tr("&Previous bookmark"), QLatin1String("previousBookmarkAction"), QIcon(), QKeySequence(Qt::CTRL + Qt::Key_PageUp), SLOT(on_previousBookmark_triggered()));
    m_nextBookmarkAction = createAction(tr("&Next bookmark"), QLatin1String("nextBookmarkAction"), QIcon(), QKeySequence(Qt::CTRL + Qt::Key_PageDown), SLOT(on_nextBookmark_triggered()));

    m_addBookmarkAction = createAction(tr("&Add bookmark"), QLatin1String("addBookmark"), QIcon(), QKeySequence(Qt::CTRL + Qt::Key_B), SLOT(on_addBookmark_triggered()));
    m_removeBookmarkAction = createAction(tr("&Remove bookmark"), QLatin1String("removeBookmark"), QIcon(), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B), SLOT(on_removeBookmark_triggered()));
    m_removeAllBookmarksAction = createAction(tr("Remove all bookmarks"), QLatin1String("removeAllBookmark"), QIcon(), QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_B), SLOT(on_removeAllBookmarks_triggered()));

    // help

    m_contentsAction = createAction(tr("&Contents"), QLatin1String("contents"), QIcon::fromTheme("help-contents"), QKeySequence::HelpContents, SLOT(on_contents_triggered()));
    m_aboutAction = createAction(tr("&About"), QString(), QIcon::fromTheme("help-about"), QKeySequence(), SLOT(on_about_triggered()));
    m_aboutAction->setMenuRole(QAction::AboutRole);

    // context

    m_openCopyInNewTabAction = createAction(tr("Open &copy in new tab"), QLatin1String("openCopyInNewTab"), QLatin1String("tab-new"), QKeySequence(), SLOT(on_openCopyInNewTab_triggered()));
    m_openCopyInNewWindowAction = createAction(tr("Open copy in new &window"), QLatin1String("openCopyInNewWindow"), QLatin1String("window-new"), QKeySequence(), SLOT(on_openCopyInNewWindow_triggered()));
    m_openContainingFolderAction = createAction(tr("Open containing &folder"), QLatin1String("openContainingFolder"), QLatin1String("folder"), QKeySequence(), SLOT(on_openContainingFolder_triggered()));
    m_moveToInstanceAction = createAction(tr("Move to &instance..."), QLatin1String("moveToInstance"), QIcon(), QKeySequence(), SLOT(on_moveToInstance_triggered()));
    m_splitViewHorizontallyAction = createAction(tr("Split view horizontally..."), QLatin1String("splitViewHorizontally"), QIcon(), QKeySequence(), SLOT(on_splitView_splitHorizontally_triggered()));
    m_splitViewVerticallyAction = createAction(tr("Split view vertically..."), QLatin1String("splitViewVertically"), QIcon(), QKeySequence(), SLOT(on_splitView_splitVertically_triggered()));
    m_closeCurrentViewAction = createAction(tr("Close current view"), QLatin1String("closeCurrentView"), QIcon(), QKeySequence(), SLOT(on_splitView_closeCurrent_triggered()));

    // tool bars and menu bar

    m_toggleToolBarsAction = createAction(tr("Toggle tool bars"), QLatin1String("toggleToolBars"), QIcon(), QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_T), SLOT(on_toggleToolBars_triggered(bool)), true, true);
    m_toggleMenuBarAction = createAction(tr("Toggle menu bar"), QLatin1String("toggleMenuBar"), QIcon(), QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_M), SLOT(on_toggleMenuBar_triggered(bool)), true, true);

    // progress and error icons

    s_settings->pageItem().setProgressIcon(loadIconWithFallback(QLatin1String("image-loading")));
    s_settings->pageItem().setErrorIcon(loadIconWithFallback(QLatin1String("image-missing")));
}

QToolBar* MainWindow::createToolBar(const QString& text, const QString& objectName, const QStringList& actionNames, const QList< QAction* >& actions)
{
    QToolBar* toolBar = addToolBar(text);
    toolBar->setObjectName(objectName);

    addWidgetActions(toolBar, actionNames, actions);

    toolBar->toggleViewAction()->setObjectName(objectName + QLatin1String("ToggleView"));
    s_shortcutHandler->registerAction(toolBar->toggleViewAction());

    return toolBar;
}

void MainWindow::createToolBars()
{
    m_fileToolBar = createToolBar(tr("&File"), QLatin1String("fileToolBar"), s_settings->mainWindow().fileToolBar(),
                                  QList< QAction* >() << m_openAction << m_openInNewTabAction << m_openContainingFolderAction << m_refreshAction << m_saveAction << m_saveAsAction << m_printAction);

    m_editToolBar = createToolBar(tr("&Edit"), QLatin1String("editToolBar"), s_settings->mainWindow().editToolBar(),
                                  QList< QAction* >() << m_currentPageAction << m_previousPageAction << m_nextPageAction << m_firstPageAction << m_lastPageAction << m_jumpToPageAction << m_searchAction << m_jumpBackwardAction << m_jumpForwardAction << m_copyToClipboardModeAction << m_addAnnotationModeAction);

    m_viewToolBar = createToolBar(tr("&View"), QLatin1String("viewToolBar"), s_settings->mainWindow().viewToolBar(),
                                  QList< QAction* >() << m_scaleFactorAction << m_continuousModeAction << m_twoPagesModeAction << m_twoPagesWithCoverPageModeAction << m_multiplePagesModeAction << m_rightToLeftModeAction << m_zoomInAction << m_zoomOutAction << m_originalSizeAction << m_fitToPageWidthModeAction << m_fitToPageSizeModeAction << m_rotateLeftAction << m_rotateRightAction << m_fullscreenAction << m_presentationAction);

    m_focusCurrentPageShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_K), this, SLOT(on_focusCurrentPage_activated()));
    m_focusScaleFactorShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this, SLOT(on_focusScaleFactor_activated()));
}

QDockWidget* MainWindow::createDock(const QString& text, const QString& objectName, const QKeySequence& toggleViewShortcut)
{
    QDockWidget* dock = new QDockWidget(text, this);
    dock->setObjectName(objectName);
    dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

#ifdef Q_OS_WIN

    dock->setWindowTitle(dock->windowTitle().remove(QLatin1Char('&')));

#endif // Q_OS_WIN

    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(on_dock_dockLocationChanged(Qt::DockWidgetArea)));

    dock->toggleViewAction()->setObjectName(objectName + QLatin1String("ToggleView"));
    dock->toggleViewAction()->setShortcut(toggleViewShortcut);

    s_shortcutHandler->registerAction(dock->toggleViewAction());

    dock->hide();

    return dock;
}

void MainWindow::createSearchDock()
{
    m_searchDock = new QDockWidget(tr("&Arama"), this);
    m_searchDock->setObjectName(QLatin1String("searchDock"));
    m_searchDock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetVerticalTitleBar);

#ifdef Q_OS_WIN

    m_searchDock->setWindowTitle(m_searchDock->windowTitle().remove(QLatin1Char('&')));

#endif // Q_OS_WIN

    addDockWidget(Qt::BottomDockWidgetArea, m_searchDock);

    connect(m_searchDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(on_dock_dockLocationChanged(Qt::DockWidgetArea)));
    connect(m_searchDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(on_search_dockLocationChanged(Qt::DockWidgetArea)));
    connect(m_searchDock, SIGNAL(visibilityChanged(bool)), SLOT(on_search_visibilityChanged(bool)));

    m_searchWidget = new QWidget(this);
    m_searchDock->setWidget(m_searchWidget);

    m_findPreviousButton = new QToolButton(m_searchWidget);
    m_findPreviousButton->setAutoRaise(true);
    m_findPreviousButton->setDefaultAction(m_findPreviousAction);

    m_findNextButton = new QToolButton(m_searchWidget);
    m_findNextButton->setAutoRaise(true);
    m_findNextButton->setDefaultAction(m_findNextAction);

    m_cancelSearchButton = new QToolButton(m_searchWidget);
    m_cancelSearchButton->setAutoRaise(true);
    m_cancelSearchButton->setDefaultAction(m_cancelSearchAction);

    connect(m_searchDock, SIGNAL(visibilityChanged(bool)), m_findPreviousAction, SLOT(setEnabled(bool)));
    connect(m_searchDock, SIGNAL(visibilityChanged(bool)), m_findNextAction, SLOT(setEnabled(bool)));
    connect(m_searchDock, SIGNAL(visibilityChanged(bool)), m_cancelSearchAction, SLOT(setEnabled(bool)));

    m_searchDock->setVisible(false);

    m_findPreviousAction->setEnabled(false);
    m_findNextAction->setEnabled(false);
    m_cancelSearchAction->setEnabled(false);

    if(s_settings->mainWindow().extendedSearchDock())
    {
        m_searchDock->setFeatures(m_searchDock->features() | QDockWidget::DockWidgetClosable);

        m_searchDock->toggleViewAction()->setObjectName(QLatin1String("searchDockToggleView"));
        m_searchDock->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F10));

        s_shortcutHandler->registerAction(m_searchDock->toggleViewAction());

        m_searchView = new QTreeView(m_searchWidget);
        m_searchView->setAlternatingRowColors(true);
        m_searchView->setUniformRowHeights(true);
        m_searchView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_searchView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_searchView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_searchView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_searchView->setItemDelegate(new SearchItemDelegate(m_searchView));

        connect(m_searchView->header(), SIGNAL(sectionCountChanged(int,int)), SLOT(on_search_sectionCountChanged()));
        connect(m_searchView, SIGNAL(clicked(QModelIndex)), SLOT(on_search_clicked(QModelIndex)));
        connect(m_searchView, SIGNAL(activated(QModelIndex)), SLOT(on_search_clicked(QModelIndex)));

        m_searchView->setModel(s_searchModel);

        connect(s_searchModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(on_search_rowsInserted(QModelIndex,int,int)));
    }
    else
    {
        m_searchView = 0;
    }

    on_search_dockLocationChanged(Qt::BottomDockWidgetArea);
}

void MainWindow::createDocks()
{
    // outline

    m_outlineDock = createDock(tr("&Outline"), QLatin1String("outlineDock"), QKeySequence(Qt::Key_F6));

    m_outlineView = new TreeView(Model::Document::ExpansionRole, this);
    m_outlineView->setAlternatingRowColors(true);
    m_outlineView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_outlineView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_outlineView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_outlineView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_outlineView->installEventFilter(this);

    connect(m_outlineView->header(), SIGNAL(sectionCountChanged(int,int)), SLOT(on_outline_sectionCountChanged()));
    connect(m_outlineView, SIGNAL(clicked(QModelIndex)), SLOT(on_outline_clicked(QModelIndex)));
    connect(m_outlineView, SIGNAL(activated(QModelIndex)), SLOT(on_outline_clicked(QModelIndex)));

    m_outlineDock->setWidget(m_outlineView);

    // properties

    m_propertiesDock = createDock(tr("&Properties"), QLatin1String("propertiesDock"), QKeySequence(Qt::Key_F7));

    m_propertiesView = new QTableView(this);
    m_propertiesView->setAlternatingRowColors(true);
    m_propertiesView->setTabKeyNavigation(false);
    m_propertiesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_propertiesView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_propertiesView->horizontalHeader(), SIGNAL(sectionCountChanged(int,int)), SLOT(on_properties_sectionCountChanged()));

    m_propertiesDock->setWidget(m_propertiesView);

    // thumbnails

    m_thumbnailsDock = createDock(tr("Sayfalar"), QLatin1String("thumbnailsDock"), QKeySequence(Qt::Key_F8));

    connect(m_thumbnailsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(on_thumbnails_dockLocationChanged(Qt::DockWidgetArea)));

    m_thumbnailsView = new QGraphicsView(this);

    m_thumbnailsView->installEventFilter(this);

    connect(m_thumbnailsView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(on_thumbnails_verticalScrollBar_valueChanged(int)));

    m_thumbnailsDock->setWidget(m_thumbnailsView);

    // bookmarks

    m_bookmarksDock = createDock(tr("Book&marks"), QLatin1String("bookmarksDock"), QKeySequence(Qt::Key_F9));

    m_bookmarksView = new QTableView(this);
    m_bookmarksView->setShowGrid(false);
    m_bookmarksView->setAlternatingRowColors(true);
    m_bookmarksView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_bookmarksView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_bookmarksView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_bookmarksView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bookmarksView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_bookmarksView->horizontalHeader(), SIGNAL(sectionCountChanged(int,int)), SLOT(on_bookmarks_sectionCountChanged()));
    connect(m_bookmarksView, SIGNAL(clicked(QModelIndex)), SLOT(on_bookmarks_clicked(QModelIndex)));
    connect(m_bookmarksView, SIGNAL(activated(QModelIndex)), SLOT(on_bookmarks_clicked(QModelIndex)));
    connect(m_bookmarksView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(on_bookmarks_contextMenuRequested(QPoint)));

    m_bookmarksDock->setWidget(m_bookmarksView);

    // search

    createSearchDock();
}

void MainWindow::createMenus()
{
    // file

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addActions(QList< QAction* >() << m_openAction << m_openInNewTabAction);

    m_recentlyUsedMenu = new RecentlyUsedMenu(s_settings->mainWindow().recentlyUsed(), s_settings->mainWindow().recentlyUsedCount(), this);

    connect(m_recentlyUsedMenu, SIGNAL(openTriggered(QString)), SLOT(on_recentlyUsed_openTriggered(QString)));

    if(s_settings->mainWindow().trackRecentlyUsed())
    {
        m_fileMenu->addMenu(m_recentlyUsedMenu);

        setToolButtonMenu(m_fileToolBar, m_openAction, m_recentlyUsedMenu);
        setToolButtonMenu(m_fileToolBar, m_openInNewTabAction, m_recentlyUsedMenu);
    }

    m_fileMenu->addActions(QList< QAction* >() << m_refreshAction << m_saveAction << m_saveAsAction << m_saveCopyAction << m_printAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // edit

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addActions(QList< QAction* >() << m_previousPageAction << m_nextPageAction << m_firstPageAction << m_lastPageAction << m_jumpToPageAction);
    m_editMenu->addSeparator();
    m_editMenu->addActions(QList< QAction* >() << m_jumpBackwardAction << m_jumpForwardAction);
    m_editMenu->addSeparator();
    m_editMenu->addActions(QList< QAction* >() << m_searchAction << m_findPreviousAction << m_findNextAction << m_cancelSearchAction);
    m_editMenu->addSeparator();
    m_editMenu->addActions(QList< QAction* >() << m_copyToClipboardModeAction << m_addAnnotationModeAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_settingsAction);

    // view

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addActions(QList< QAction* >() << m_continuousModeAction << m_twoPagesModeAction << m_twoPagesWithCoverPageModeAction << m_multiplePagesModeAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_rightToLeftModeAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions(QList< QAction* >() << m_zoomInAction << m_zoomOutAction << m_originalSizeAction << m_fitToPageWidthModeAction << m_fitToPageSizeModeAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions(QList< QAction* >() << m_rotateLeftAction << m_rotateRightAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions(QList< QAction* >() << m_invertColorsAction << m_convertToGrayscaleAction << m_trimMarginsAction);

    m_compositionModeMenu = m_viewMenu->addMenu(tr("Composition"));
    m_compositionModeMenu->addAction(m_darkenWithPaperColorAction);
    m_compositionModeMenu->addAction(m_lightenWithPaperColorAction);

    m_viewMenu->addSeparator();

    QMenu* toolBarsMenu = m_viewMenu->addMenu(tr("&Tool bars"));
    toolBarsMenu->addActions(QList< QAction* >() << m_fileToolBar->toggleViewAction() << m_editToolBar->toggleViewAction() << m_viewToolBar->toggleViewAction());

    QMenu* docksMenu = m_viewMenu->addMenu(tr("&Docks"));
    docksMenu->addActions(QList< QAction* >() << m_outlineDock->toggleViewAction() << m_propertiesDock->toggleViewAction() << m_thumbnailsDock->toggleViewAction() << m_bookmarksDock->toggleViewAction());

    if(s_settings->mainWindow().extendedSearchDock())
    {
        docksMenu->addAction(m_searchDock->toggleViewAction());
    }

    m_viewMenu->addAction(m_fontsAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions(QList< QAction* >() << m_fullscreenAction << m_presentationAction);

    // tabs

    m_tabsMenu = new SearchableMenu(tr("&Tabs"), this);
    menuBar()->addMenu(m_tabsMenu);

    m_tabsMenu->setSearchable(s_settings->mainWindow().searchableMenus());

    m_tabsMenu->addActions(QList< QAction* >() << m_previousTabAction << m_nextTabAction);
    m_tabsMenu->addSeparator();
    m_tabsMenu->addActions(QList< QAction* >() << m_closeTabAction << m_closeAllTabsAction << m_closeAllTabsButCurrentTabAction);

    m_recentlyClosedMenu = new RecentlyClosedMenu(s_settings->mainWindow().recentlyClosedCount(), this);

    connect(m_recentlyClosedMenu, SIGNAL(tabActionTriggered(QAction*)), SLOT(on_recentlyClosed_tabActionTriggered(QAction*)));

    if(s_settings->mainWindow().keepRecentlyClosed())
    {
        m_tabsMenu->addMenu(m_recentlyClosedMenu);
        m_tabsMenu->addAction(m_restoreMostRecentlyClosedTabAction);
    }

    m_tabsMenu->addSeparator();

    // bookmarks

    m_bookmarksMenu = new SearchableMenu(tr("&Bookmarks"), this);
    menuBar()->addMenu(m_bookmarksMenu);

    m_bookmarksMenu->setSearchable(s_settings->mainWindow().searchableMenus());

    connect(m_bookmarksMenu, SIGNAL(aboutToShow()), this, SLOT(on_bookmarksMenu_aboutToShow()));

    m_bookmarksMenuIsDirty = true;

    // help

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addActions(QList< QAction* >() << m_contentsAction << m_aboutAction);
}

#ifdef WITH_DBUS

MainWindowAdaptor::MainWindowAdaptor(MainWindow* mainWindow) : QDBusAbstractAdaptor(mainWindow)
{
}

QDBusInterface* MainWindowAdaptor::createInterface(const QString& instanceName)
{
    return new QDBusInterface(serviceName(instanceName), QLatin1String("/MainWindow"), QLatin1String("local.qpdfview.MainWindow"), QDBusConnection::sessionBus());
}

MainWindowAdaptor* MainWindowAdaptor::createAdaptor(MainWindow* mainWindow)
{
    QScopedPointer< MainWindowAdaptor > adaptor(new MainWindowAdaptor(mainWindow));

    if(!QDBusConnection::sessionBus().registerService(serviceName()))
    {
        return 0;
    }

    if(!QDBusConnection::sessionBus().registerObject(QLatin1String("/MainWindow"), mainWindow))
    {
        return 0;
    }

    return adaptor.take();
}

void MainWindowAdaptor::raiseAndActivate()
{
    mainWindow()->raise();
    mainWindow()->activateWindow();
}

bool MainWindowAdaptor::open(const QString& absoluteFilePath, int page, const QRectF& highlight, bool quiet)
{
    return mainWindow()->open(absoluteFilePath, page, highlight, quiet);
}

bool MainWindowAdaptor::openInNewTab(const QString& absoluteFilePath, int page, const QRectF& highlight, bool quiet)
{
    return mainWindow()->openInNewTab(absoluteFilePath, page, highlight, quiet);
}

bool MainWindowAdaptor::jumpToPageOrOpenInNewTab(const QString& absoluteFilePath, int page, bool refreshBeforeJump, const QRectF& highlight, bool quiet)
{
    return mainWindow()->jumpToPageOrOpenInNewTab(absoluteFilePath, page, refreshBeforeJump, highlight, quiet);
}

void MainWindowAdaptor::startSearch(const QString& text)
{
    mainWindow()->startSearch(text);
}

void MainWindowAdaptor::saveDatabase()
{
    mainWindow()->saveDatabase();
}

int MainWindowAdaptor::currentPage() const
{
    if(DocumentView* tab = mainWindow()->currentTab())
    {
        return tab->currentPage();
    }
    else
    {
        return -1;
    }
}

void MainWindowAdaptor::jumpToPage(int page)
{
    if(DocumentView* tab = mainWindow()->currentTab())
    {
        tab->jumpToPage(page);
    }
}

#define ONLY_IF_CURRENT_TAB if(mainWindow()->m_tabWidget->currentIndex() == -1) { return; }

void MainWindowAdaptor::previousPage()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_previousPage_triggered();
}

void MainWindowAdaptor::nextPage()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_nextPage_triggered();
}

void MainWindowAdaptor::firstPage()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_firstPage_triggered();
}

void MainWindowAdaptor::lastPage()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_lastPage_triggered();
}

void MainWindowAdaptor::previousBookmark()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_previousBookmark_triggered();
}

void MainWindowAdaptor::nextBookmark()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_nextBookmark_triggered();
}

bool MainWindowAdaptor::jumpToBookmark(const QString& label)
{
    if(mainWindow()->m_tabWidget->currentIndex() == -1) { return false; }

    const BookmarkModel* model = mainWindow()->bookmarkModelForCurrentTab();

    if(model != 0)
    {
        for(int row = 0, rowCount = model->rowCount(); row < rowCount; ++row)
        {
            const QModelIndex index = model->index(row);

            if(label == index.data(BookmarkModel::LabelRole).toString())
            {
                mainWindow()->currentTab()->jumpToPage(index.data(BookmarkModel::PageRole).toInt());

                return true;
            }
        }
    }

    return false;
}

void MainWindowAdaptor::continuousMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_continuousMode_triggered(checked);
}

void MainWindowAdaptor::twoPagesMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_twoPagesMode_triggered(checked);
}

void MainWindowAdaptor::twoPagesWithCoverPageMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_twoPagesWithCoverPageMode_triggered(checked);
}

void MainWindowAdaptor::multiplePagesMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_multiplePagesMode_triggered(checked);
}

void MainWindowAdaptor::fitToPageWidthMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_fitToPageWidthMode_triggered(checked);
}

void MainWindowAdaptor::fitToPageSizeMode(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_fitToPageSizeMode_triggered(checked);
}

void MainWindowAdaptor::invertColors(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_invertColors_triggered(checked);
}

void MainWindowAdaptor::convertToGrayscale(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_convertToGrayscale_triggered(checked);
}

void MainWindowAdaptor::trimMargins(bool checked)
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_trimMargins_triggered(checked);
}

void MainWindowAdaptor::fullscreen(bool checked)
{
    if(mainWindow()->m_fullscreenAction->isChecked() != checked)
    {
        mainWindow()->m_fullscreenAction->trigger();
    }
}

void MainWindowAdaptor::presentation()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_presentation_triggered();
}

void MainWindowAdaptor::closeTab()
{
    ONLY_IF_CURRENT_TAB

    mainWindow()->on_closeTab_triggered();
}

void MainWindowAdaptor::closeAllTabs()
{
    mainWindow()->on_closeAllTabs_triggered();
}

void MainWindowAdaptor::closeAllTabsButCurrentTab()
{
    mainWindow()->on_closeAllTabsButCurrentTab_triggered();
}

bool MainWindowAdaptor::closeTab(const QString& absoluteFilePath)
{
    if(mainWindow()->m_tabWidget->currentIndex() == -1) { return false; }

    const QFileInfo fileInfo(absoluteFilePath);

    foreach(DocumentView* tab, mainWindow()->allTabs())
    {
        if(tab->fileInfo() == fileInfo)
        {
            if(mainWindow()->saveModifications(tab))
            {
                mainWindow()->closeTab(tab);
            }

            return true;
        }
    }

    return false;
}

#undef ONLY_IF_CURRENT_TAB

inline MainWindow* MainWindowAdaptor::mainWindow() const
{
    return qobject_cast< MainWindow* >(parent());
}

QString MainWindowAdaptor::serviceName(QString instanceName)
{
    QString serviceName = QApplication::organizationDomain();

    if(instanceName.isNull())
    {
        instanceName = qApp->objectName();
    }

    if(!instanceName.isEmpty())
    {
        serviceName.append('.');
        serviceName.append(instanceName);
    }

    return serviceName;
}



# endif // WITH_DBUS

} // qpdfview
