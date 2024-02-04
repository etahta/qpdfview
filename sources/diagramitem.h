/*****************************************************************************
 *   Copyright (C) 2020 by Bayram KARAHAN                                    *
 *   <bayramk@gmail.com>                                                     *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .          *
 *****************************************************************************/
#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include<QtMath>
QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
QT_END_NAMESPACE

class Arrow;

//! [0]
class DiagramItem
{
public:
     enum DiagramType {
         NoType,NDogru,NNKoordinat,NKoordinat,HNKoordinat,
         Kup,Pramit,Kure,Silindir,Pergel,Cetvel,Gonye,Iletki,
         Cizgi,Ok,CiftOk,Ucgen,Dortgen,Cember,
         Baklava,ParalelKenar,Yamuk,Besgen,
         Altigen,Sekizgen,DikUcgen,
         GuzelYazi,Muzik,CizgiliSayfa,
         KareliKagit,CizgiliKagit,NoktaliKagit,IzometrikKagit,
         Kalem,Resim,Copy,DrawPath,DrawPoligon,Pdf,
         SolidLine,DashLine,DotLine,
         NormalPen,SmartPen,FosforluPen,
         PenSize2,PenSize4,PenSize6,PenSize8,PenSize10,PenSize12,PenSize14,
         PatternPage,
         TransparanPage,BlackPage,WhitePage,CustomColorPage,
         KareliPage,CizgiliPage,MuzikPage,GuzelYaziPage,CustomImagePage,
         GridYatayPage,GridDikeyPage,
         NoColor,ColorGreen,ColorRed,ColorBlue,ColorYellow,ColorCyan,ColorPink,ColorWhite,ColorBlack,ColorGray
         };

   /* DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent = 0);

    void removeArrow(Arrow *arrow);
    void removeArrows();
    DiagramType diagramType() const { return myDiagramType; }
    QPolygonF polygon() const { return myPolygon; }
    void addArrow(Arrow *arrow);
    QPixmap image() const;
    int type() const override { return Type;}
*/

     QPolygonF  sekilStore(DiagramItem::DiagramType sek, QRectF rect)
     {
         //int en=40;
         //int boy=40;
         QPolygon result(2);
   //  int kenar=40;
    // result.setPoint(0, rect.center().x(), rect.top());
    // result.setPoint(1, rect.right(), rect.bottom());
   //  result.setPoint(2, rect.left(), rect.bottom());

       //  QRectF rect (QPointF(_rect.left(),_rect.top()),QPointF(_rect.right()-kenar,_rect.bottom()-kenar));

         // rect.moveTo(10, 10);
         /*if(sek==DiagramItem::DiagramType::Cizgi)
           {
         QPolygon result(15);     //Çizgi
         result.setPoint(0, rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.top());
         result.setPoint(2, rect.right(), rect.bottom());
         result.setPoint(3, rect.left(), rect.bottom());
         result.setPoint(4, rect.left(), rect.top());

         result.setPoint(5, rect.left(), rect.top());
         result.setPoint(6, rect.right(), rect.top());
         result.setPoint(7, rect.right(), rect.top()+rect.height()/3);
         result.setPoint(8, rect.left(), rect.top()+rect.height()/3);
         result.setPoint(9, rect.left(), rect.top());

         result.setPoint(10, rect.left(), rect.top());
         result.setPoint(11, rect.right(), rect.top());
         result.setPoint(12, rect.right(), rect.top()+rect.height()/3*2);
         result.setPoint(13, rect.left(), rect.top()+rect.height()/3*2);
         result.setPoint(14, rect.left(), rect.top());

         return result;
         }*/
         /*if(sek==DiagramItem::DiagramType::Cizgi)
           {
         QPolygon result(20);     //Çizgi
         result.setPoint(0, rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.top());
         result.setPoint(2, rect.right(), rect.bottom());
         result.setPoint(3, rect.left(), rect.bottom());
         result.setPoint(4, rect.left(), rect.top());

         result.setPoint(5, rect.left(), rect.top());
         result.setPoint(6, rect.right(), rect.top());
         result.setPoint(7, rect.right(), rect.top()+rect.height()/4);
         result.setPoint(8, rect.left(), rect.top()+rect.height()/4);
         result.setPoint(9, rect.left(), rect.top());

         result.setPoint(10, rect.left(), rect.top());
         result.setPoint(11, rect.right(), rect.top());
         result.setPoint(12, rect.right(), rect.top()+rect.height()/4*2);
         result.setPoint(13, rect.left(), rect.top()+rect.height()/4*2);
         result.setPoint(14, rect.left(), rect.top());

         result.setPoint(15, rect.left(), rect.top());
         result.setPoint(16, rect.right(), rect.top());
         result.setPoint(17, rect.right(), rect.top()+rect.height()/4*3);
         result.setPoint(18, rect.left(), rect.top()+rect.height()/4*3);
         result.setPoint(19, rect.left(), rect.top());

         return result;
         }*/
         if(sek==DiagramItem::DiagramType::CizgiliSayfa)
                    {
                  QPolygon result(65);     //GuzelYazi
                  result.setPoint(0, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(1, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(2, rect.right(), rect.bottom()-rect.height()/15);
                  result.setPoint(3, rect.left(), rect.bottom()-rect.height()/15);
                  result.setPoint(4, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(5, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(6, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(7, rect.right(), rect.top()+rect.height()/15*2);
                  result.setPoint(8, rect.left(), rect.top()+rect.height()/15*2);
                  result.setPoint(9, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(10, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(11, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(12, rect.right(), rect.top()+rect.height()/15*3);
                  result.setPoint(13, rect.left(), rect.top()+rect.height()/15*3);
                  result.setPoint(14, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(15, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(16, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(17, rect.right(), rect.top()+rect.height()/15*4);
                  result.setPoint(18, rect.left(), rect.top()+rect.height()/15*4);
                  result.setPoint(19, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(20, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(21, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(22, rect.right(), rect.top()+rect.height()/15*5);
                  result.setPoint(23, rect.left(), rect.top()+rect.height()/15*5);
                  result.setPoint(24, rect.left(), rect.top()+rect.height()/15);

         /***************************************************************************/
                  result.setPoint(25, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(26, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(27, rect.right(), rect.top()+rect.height()/15*6);
                  result.setPoint(28, rect.left(), rect.top()+rect.height()/15*6);
                  result.setPoint(29, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(30, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(31, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(32, rect.right(), rect.top()+rect.height()/15*7);
                  result.setPoint(33, rect.left(), rect.top()+rect.height()/15*7);
                  result.setPoint(34, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(35, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(36, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(37, rect.right(), rect.top()+rect.height()/15*8);
                  result.setPoint(38, rect.left(), rect.top()+rect.height()/15*8);
                  result.setPoint(39, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(40, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(41, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(42, rect.right(), rect.top()+rect.height()/15*9);
                  result.setPoint(43, rect.left(), rect.top()+rect.height()/15*9);
                  result.setPoint(44, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(45, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(46, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(47, rect.right(), rect.top()+rect.height()/15*10);
                  result.setPoint(48, rect.left(), rect.top()+rect.height()/15*10);
                  result.setPoint(49, rect.left(), rect.top()+rect.height()/15);
                  /***********************************************************************/
                  result.setPoint(50, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(51, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(52, rect.right(), rect.top()+rect.height()/15*11);
                  result.setPoint(53, rect.left(), rect.top()+rect.height()/15*11);
                  result.setPoint(54, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(55, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(56, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(57, rect.right(), rect.top()+rect.height()/15*12);
                  result.setPoint(58, rect.left(), rect.top()+rect.height()/15*12);
                  result.setPoint(59, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(60, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(61, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(62, rect.right(), rect.top()+rect.height()/15*13);
                  result.setPoint(63, rect.left(), rect.top()+rect.height()/15*13);
                  result.setPoint(64, rect.left(), rect.top()+rect.height()/15);
                      return result;
                  }

         if(sek==DiagramItem::DiagramType::GuzelYazi)
                    {
                  QPolygon result(55);     //GuzelYazi
                  result.setPoint(0, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(1, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(2, rect.right(), rect.bottom()-rect.height()/15);
                  result.setPoint(3, rect.left(), rect.bottom()-rect.height()/15);
                  result.setPoint(4, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(5, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(6, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(7, rect.right(), rect.top()+rect.height()/15*2);
                  result.setPoint(8, rect.left(), rect.top()+rect.height()/15*2);
                  result.setPoint(9, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(10, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(11, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(12, rect.right(), rect.top()+rect.height()/15*3);
                  result.setPoint(13, rect.left(), rect.top()+rect.height()/15*3);
                  result.setPoint(14, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(15, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(16, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(17, rect.right(), rect.top()+rect.height()/15*4);
                  result.setPoint(18, rect.left(), rect.top()+rect.height()/15*4);
                  result.setPoint(19, rect.left(), rect.top()+rect.height()/15);
         /***************************************************************************/
                  result.setPoint(20, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(21, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(22, rect.right(), rect.top()+rect.height()/15*6);
                  result.setPoint(23, rect.left(), rect.top()+rect.height()/15*6);
                  result.setPoint(24, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(25, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(26, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(27, rect.right(), rect.top()+rect.height()/15*7);
                  result.setPoint(28, rect.left(), rect.top()+rect.height()/15*7);
                  result.setPoint(29, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(30, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(31, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(32, rect.right(), rect.top()+rect.height()/15*8);
                  result.setPoint(33, rect.left(), rect.top()+rect.height()/15*8);
                  result.setPoint(34, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(35, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(36, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(37, rect.right(), rect.top()+rect.height()/15*9);
                  result.setPoint(38, rect.left(), rect.top()+rect.height()/15*9);
                  result.setPoint(39, rect.left(), rect.top()+rect.height()/15);
                  /***********************************************************************/
                  result.setPoint(40, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(41, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(42, rect.right(), rect.top()+rect.height()/15*11);
                  result.setPoint(43, rect.left(), rect.top()+rect.height()/15*11);
                  result.setPoint(44, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(45, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(46, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(47, rect.right(), rect.top()+rect.height()/15*12);
                  result.setPoint(48, rect.left(), rect.top()+rect.height()/15*12);
                  result.setPoint(49, rect.left(), rect.top()+rect.height()/15);

                  result.setPoint(50, rect.left(), rect.top()+rect.height()/15);
                  result.setPoint(51, rect.right(), rect.top()+rect.height()/15);
                  result.setPoint(52, rect.right(), rect.top()+rect.height()/15*13);
                  result.setPoint(53, rect.left(), rect.top()+rect.height()/15*13);
                  result.setPoint(54, rect.left(), rect.top()+rect.height()/15);
                      return result;
                  }
         if(sek==DiagramItem::DiagramType::Muzik)
                    {
                  QPolygon result(70);     //Muzik Desen
                  result.setPoint(0, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(1, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(2, rect.right(), rect.bottom()-rect.height()/18);
                  result.setPoint(3, rect.left(), rect.bottom()-rect.height()/18);
                  result.setPoint(4, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(5, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(6, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(7, rect.right(), rect.top()+rect.height()/18*2);
                  result.setPoint(8, rect.left(), rect.top()+rect.height()/18*2);
                  result.setPoint(9, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(10, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(11, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(12, rect.right(), rect.top()+rect.height()/18*3);
                  result.setPoint(13, rect.left(), rect.top()+rect.height()/18*3);
                  result.setPoint(14, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(15, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(16, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(17, rect.right(), rect.top()+rect.height()/18*4);
                  result.setPoint(18, rect.left(), rect.top()+rect.height()/18*4);
                  result.setPoint(19, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(20, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(21, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(22, rect.right(), rect.top()+rect.height()/18*5);
                  result.setPoint(23, rect.left(), rect.top()+rect.height()/18*5);
                  result.setPoint(24, rect.left(), rect.top()+rect.height()/18);
         /***************************************************************************/
                  result.setPoint(25, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(26, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(27, rect.right(), rect.top()+rect.height()/18*7);
                  result.setPoint(28, rect.left(), rect.top()+rect.height()/18*7);
                  result.setPoint(29, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(30, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(31, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(32, rect.right(), rect.top()+rect.height()/18*8);
                  result.setPoint(33, rect.left(), rect.top()+rect.height()/18*8);
                  result.setPoint(34, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(35, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(36, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(37, rect.right(), rect.top()+rect.height()/18*9);
                  result.setPoint(38, rect.left(), rect.top()+rect.height()/18*9);
                  result.setPoint(39, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(40, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(41, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(42, rect.right(), rect.top()+rect.height()/18*10);
                  result.setPoint(43, rect.left(), rect.top()+rect.height()/18*10);
                  result.setPoint(44, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(45, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(46, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(47, rect.right(), rect.top()+rect.height()/18*11);
                  result.setPoint(48, rect.left(), rect.top()+rect.height()/18*11);
                  result.setPoint(49, rect.left(), rect.top()+rect.height()/18);
                  /***********************************************************************/
                  result.setPoint(50, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(51, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(52, rect.right(), rect.top()+rect.height()/18*13);
                  result.setPoint(53, rect.left(), rect.top()+rect.height()/18*13);
                  result.setPoint(54, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(55, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(56, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(57, rect.right(), rect.top()+rect.height()/18*14);
                  result.setPoint(58, rect.left(), rect.top()+rect.height()/18*14);
                  result.setPoint(59, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(60, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(61, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(62, rect.right(), rect.top()+rect.height()/18*15);
                  result.setPoint(63, rect.left(), rect.top()+rect.height()/18*15);
                  result.setPoint(64, rect.left(), rect.top()+rect.height()/18);

                  result.setPoint(65, rect.left(), rect.top()+rect.height()/18);
                  result.setPoint(66, rect.right(), rect.top()+rect.height()/18);
                  result.setPoint(67, rect.right(), rect.top()+rect.height()/18*16);
                  result.setPoint(68, rect.left(), rect.top()+rect.height()/18*16);
                  result.setPoint(69, rect.left(), rect.top()+rect.height()/18);

                      return result;
                  }
         if(sek==DiagramItem::DiagramType::Cizgi)
           {
         QPolygon result(2);     //Çizgi
         result.setPoint(0,rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.bottom());
         return result;
         }
         if(sek==DiagramItem::DiagramType::Kalem)
           {
         QPolygon result(2);     //Çizgi
         result.setPoint(0,rect.left(), rect.top()+rect.height()/2);
         result.setPoint(1, rect.right(), rect.bottom()-rect.height()/2);
         return result;
         }

         if(sek==DiagramItem::DiagramType::Ok)
           {
         QPolygon result(7);     //Ok

      /*  result.setPoint(0,rect.left(), rect.bottom()/2);
         result.setPoint(1, rect.right(), rect.bottom()/2);
         result.setPoint(2,rect.right()-rect.width()/8, rect.bottom()/2-rect.height()/16);
         result.setPoint(3, rect.right(), rect.bottom()/2);
         result.setPoint(4,rect.right()-rect.width()/8, rect.bottom()/2+rect.height()/16);
         result.setPoint(5, rect.right(), rect.bottom()/2);
         result.setPoint(6,rect.left(), rect.bottom()/2);
*/
         int r=20;
          QLineF line(QPoint(rect.left(),rect.top()), QPoint(rect.right(),rect.bottom()));

                    //double angle = std::atan2(-line.dy(), line.dx());
                    double angle = ::acos(line.dx() / line.length());
                        if (line.dy() >= 0)
                            angle = (3.14 * 2) - angle;

                                 QPointF arrowP1 =line.p2()-  QPointF(sin(angle + M_PI / 3) * r,
                                                                       cos(angle + M_PI / 3) * r);
                                 QPointF arrowP2 = line.p2()- QPointF(sin(angle + M_PI - M_PI / 3) * r,
                                                                       cos(angle + M_PI - M_PI / 3) * r);

         result.setPoint(0,rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.bottom());
         result.setPoint(2,arrowP1.x(),arrowP1.y());
         result.setPoint(3, rect.right(), rect.bottom());
         result.setPoint(4,arrowP2.x(),arrowP2.y());
         result.setPoint(5, rect.right(), rect.bottom());
         result.setPoint(6,rect.left(), rect.top());

                  //  QPoint((rect.right())-min_x-0.01,max_y-rect.bottom()+0.01*tan(30*3.14/180));
                   // QPoint((rect.right())-min_x-0.01,max_y-rect.bottom()-0.01*tan(30*3.14/180));
                             // head_arrow << QPoint((lines_vector[i].x_end)-min_x,max_y-(lines_vector[i].y_end));
          //                 head_arrow << QPoint((lines_vector[i].x_end)-min_x-0.01,max_y-lines_vector[i].y_end+0.01*tan(30*3.14/180));
            //               head_arrow << QPoint((lines_vector[i].x_end)-min_x-0.01,max_y-lines_vector[i].y_end-0.01*tan(30*3.14/180));
              //             head_arrow << QPoint((lines_vector[i].x_end)-min_x,max_y-(lines_vector[i].y_end));

                //           scene->addPolygon(head_arrow,black);

       /*  result.setPoint(0,rect.center().x(), rect.top());
         result.setPoint(1, rect.center().x(), rect.bottom());
         result.setPoint(2,rect.center().x()+rect.width()/16, rect.bottom()-rect.height()/16);
         result.setPoint(3, rect.center().x(), rect.bottom());
         result.setPoint(4,rect.center().x()-rect.width()/16, rect.bottom()-rect.height()/16);
         result.setPoint(5, rect.center().x(), rect.bottom());
         result.setPoint(6,rect.center().x(), rect.top());
         */
         return result;
         }
         if(sek==DiagramItem::DiagramType::CiftOk)
           {
         QPolygon result(11);     //ÇiftOk
         int r=20;
          QLineF line(QPoint(rect.left(),rect.top()), QPoint(rect.right(),rect.bottom()));

                    //double angle = std::atan2(-line.dy(), line.dx());
                    double angle = ::acos(line.dx() / line.length());
                        if (line.dy() >= 0)
                            angle = (3.14 * 2) - angle;

                                 QPointF arrowP21 =line.p2()-  QPointF(sin(angle + M_PI / 3) * r,
                                                                       cos(angle + M_PI / 3) * r);
                                 QPointF arrowP22 = line.p2()- QPointF(sin(angle + M_PI - M_PI / 3) * r,
                                                                       cos(angle + M_PI - M_PI / 3) * r);
                                 QPointF arrowP11 =line.p1()+  QPointF(sin(angle + M_PI / 3) * r,
                                                                       cos(angle + M_PI / 3) * r);
                                 QPointF arrowP12 = line.p1()+ QPointF(sin(angle + M_PI - M_PI / 3) * r,
                                                                       cos(angle + M_PI - M_PI / 3) * r);

         result.setPoint(0,rect.left(), rect.top());
          result.setPoint(1,arrowP11.x(),arrowP11.y());
          result.setPoint(2,rect.left(), rect.top());
           result.setPoint(3,arrowP12.x(),arrowP12.y());
          result.setPoint(4,rect.left(), rect.top());

         result.setPoint(5, rect.right(), rect.bottom());
         result.setPoint(6,arrowP21.x(),arrowP21.y());
         result.setPoint(7, rect.right(), rect.bottom());
         result.setPoint(8,arrowP22.x(),arrowP22.y());
         result.setPoint(9, rect.right(), rect.bottom());
         result.setPoint(10,rect.left(), rect.top());

         return result;
         }
     if(sek==DiagramItem::DiagramType::Ucgen)
           {
           QPolygon result(4);     //Üçgen
           //rect.moveTo(-150, -150);
           result.setPoint(0, rect.center().x(), rect.top());
           result.setPoint(1, rect.right(), rect.bottom());
           result.setPoint(2, rect.left(), rect.bottom());
           result.setPoint(3, rect.center().x(), rect.top());
          // sekilLayout->addWidget(createCellWidget(tr("Üçgen"),result),1,0);
           return result;
           }
     if(sek==DiagramItem::DiagramType::Dortgen)
     {//Dörtgen
           QPolygon result(5);
          // rect.moveTo(-150, -150);
           result.setPoint(0, rect.left(), rect.top());
           result.setPoint(1, rect.right(), rect.top());
           result.setPoint(2, rect.right(), rect.bottom());
           result.setPoint(3, rect.left(), rect.bottom());
           result.setPoint(4, rect.left(), rect.top());
          // sekilLayout->addWidget(createCellWidget(tr("Dörtgen"),result),2,0);
           return result;
           }
     if(sek==DiagramItem::DiagramType::Yamuk)
     {  //Yamuk
           QPolygon result(5);
           //rect.moveTo(-150, -150);
           result.setPoint(0, rect.left(), rect.top());
           result.setPoint(1, rect.right()/2, rect.top());
           result.setPoint(2, rect.right(), rect.bottom());
           result.setPoint(3, rect.left(), rect.bottom());
           result.setPoint(4, rect.left(), rect.top());
         //  sekilLayout->addWidget(createCellWidget(tr("Yamuk"),result),3,0);
           return result;
           }

     if(sek==DiagramItem::DiagramType::ParalelKenar)
           {  //paralel kenar
           QPolygon result(5);
           //rect.moveTo(-150, -150);
           result.setPoint(0, rect.center().x()/2, rect.top());
           result.setPoint(1, rect.right(), rect.top());
           result.setPoint(2, rect.center().x()+rect.center().x()/2, rect.bottom());
           result.setPoint(3, rect.left(), rect.bottom());
           result.setPoint(4, rect.center().x()/2, rect.top());
        //   sekilLayout->addWidget(createCellWidget(tr("P.Kenar"),result),4,0);
           return result;
           }
     if(sek==DiagramItem::DiagramType::Baklava)
     {  //Dik paralel kenar
               QPolygon result(5);
               //rect.moveTo(-150, -150);
               result.setPoint(0, rect.center().x(), rect.top());
               result.setPoint(1, rect.right(), rect.top()+rect.height()/2);
               result.setPoint(2, rect.center().x(), rect.bottom());
               result.setPoint(3, rect.left(), rect.top()+rect.height()/2);
                result.setPoint(4, rect.center().x(), rect.top());
        //            sekilLayout->addWidget(createCellWidget(tr("D.Kenar"),result),5,0);
                return result;
               }
     if(sek==DiagramItem::DiagramType::Cember)
     {
         QPolygon result(2);     //Çizgi
         result.setPoint(0,rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.bottom());
         return result;
     }
     if(sek==DiagramItem::DiagramType::Besgen)
     {
         QPolygon result(6);     //Çizgi
         result.setPoint(0,rect.left(), rect.top()+rect.height()/2);
         result.setPoint(1, rect.center().x(), rect.top());
         result.setPoint(2,rect.right(), rect.top()+rect.height()/2);
         result.setPoint(3,rect.left()+rect.width()/4*3, rect.bottom());
         result.setPoint(4,rect.left()+rect.width()/4, rect.bottom());
         result.setPoint(5,rect.left(), rect.top()+rect.height()/2);
            return result;
     }
     if(sek==DiagramItem::DiagramType::Altigen)
     {
         QPolygon result(7);     //Çizgi
         result.setPoint(0,rect.left(), rect.top()+rect.height()/2);
         result.setPoint(1, rect.center().x()-rect.width()/4, rect.top());
         result.setPoint(2, rect.center().x()+rect.width()/4, rect.top());
         result.setPoint(3,rect.right(), rect.top()+rect.height()/2);
         result.setPoint(4, rect.center().x()+rect.width()/4, rect.bottom());
         result.setPoint(5, rect.center().x()-rect.width()/4, rect.bottom());
         result.setPoint(6,rect.left(), rect.top()+rect.height()/2);
         return result;
     }
     if(sek==DiagramItem::DiagramType::Sekizgen)
     {
         QPolygon result(9);     //Çizgi
         result.setPoint(0,rect.left(), rect.top()+rect.height()/4);
         result.setPoint(1, rect.center().x()-rect.width()/4, rect.top());
         result.setPoint(2, rect.center().x()+rect.width()/4, rect.top());
         result.setPoint(3,rect.right(), rect.top()+rect.height()/4);
         result.setPoint(4,rect.right(), rect.bottom()-rect.height()/4);
         result.setPoint(5, rect.center().x()+rect.width()/4, rect.bottom());
         result.setPoint(6, rect.center().x()-rect.width()/4, rect.bottom());
         result.setPoint(7,rect.left(), rect.bottom()-rect.height()/4);
         result.setPoint(8,rect.left(), rect.top()+rect.height()/4);
         return result;
     }
     if(sek==DiagramItem::DiagramType::DikUcgen)
     {
         QPolygon result(4);     //Çizgi
         result.setPoint(0, rect.left(), rect.top());
         result.setPoint(1, rect.right(), rect.bottom());
         result.setPoint(2, rect.left(), rect.bottom());
         result.setPoint(3, rect.left(), rect.top());
         return result;
     }


         return result;
     }

protected:
  //  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
   // QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
  //  DiagramType myDiagramType;
   // QPolygonF myPolygon;
   // QMenu *myContextMenu;
   // QList<Arrow *> arrows;
};
//! [0]

#endif // DIAGRAMITEM_H
