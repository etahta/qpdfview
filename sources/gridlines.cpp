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

#include "gridlines.h"
#include<QPalette>
#include <QStyleOptionGraphicsItem>
#include<QtDebug>
GridLines::GridLines(int w, int h ,int space,bool gridYatay,bool gridDikey,bool guzelYazi, QColor backColor,QColor lineColor) : QGraphicsItem(),
            _width(w), _height(h), _space(space),_backColor(backColor),_lineColor(lineColor),_gridYatay(gridYatay),_gridDikey(gridDikey),_guzelYazi(guzelYazi)
{

}

QPixmap GridLines::PixItem(QGraphicsItem *item, int w, int h){
    QPixmap pixmap(w,h);
//QPixmap pixmap(item->boundingRect().size().toSize());
pixmap.fill(Qt::transparent);
QPainter painter(&pixmap);
painter.setRenderHint(QPainter::Antialiasing);
QStyleOptionGraphicsItem opt;
item->paint(&painter,&opt);
return pixmap;
}
QPixmap GridLines::PixItemscene(QGraphicsItem *item){
  //  QPixmap pixmap(w,h);
QPixmap pixmap(item->boundingRect().size().toSize());
pixmap.fill(Qt::transparent);
QPainter painter(&pixmap);
painter.setRenderHint(QPainter::Antialiasing);
QStyleOptionGraphicsItem opt;
item->paint(&painter,&opt);
return pixmap;
}


/*
QPixmap GridLines::image() const
{
    QPixmap pixmap(1024, 768);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));
    painter.translate(125, 125);
    //painter.drawPixmap(this);

    return pixmap;
}
*/

void GridLines::paint (QPainter *_painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // re-draw the grid lines from 0,0 to _width, _height

    // do horizontal first

    QColor c (200,200,255,0);
   _painter->setPen(c);

    _painter->setBrush(QBrush(_backColor));
    _painter->drawRect(0,0,_width, _height);
   if(_guzelYazi)
     {
         int a=1;
       for (int y= _space; y < _height; y+=_space)
       {
          if(a==1){ _painter->setPen(QColor(0,0,255,255));
              _painter->drawLine(0,y, _width, y);
               y=y-_space/3;
               }
           if(a==2)
           { _painter->setPen(QColor(255,0,0,255));
               _painter->drawLine(0,y, _width, y);
            //   a=0;//return;
           }
           else if(a==3)
           { _painter->setPen(QColor(255,0,0,255));
           _painter->drawLine(0,y, _width, y);
         //  a=0;//return;
            y=y-_space/3;
           }
           else if(a==4)
           { _painter->setPen(QColor(0,0,255,255));
           _painter->drawLine(0,y, _width, y);
           y=y+_space/4;
           a=0;//return;
           }
         /*  else if(a==5)
           { _painter->setPen(QColor(0,255,0,255));
          // _painter->drawLine(0,y, _width, y);
           a=0;//return;
           }
*/
           a++;
       }
    }

   if(_gridYatay)
     {

      _painter->setPen(_lineColor);


       for (int y= _space; y < _height; y+=_space)
       {//qDebug()<<"gridyatay";
              _painter->drawLine(0,y, _width, y);
       }
    }

   if(_gridDikey)
     {
       _painter->setPen(_lineColor);

    for (int x= 0; x < _width; x+=_space)
    {//qDebug()<<"griddikey";
        _painter->drawLine(x,0, x, _height);
    }
    }
}

QRectF GridLines::boundingRect() const
{
    return QRectF ( static_cast<qreal>(0), static_cast<qreal>(0), static_cast<qreal>(_width), static_cast<qreal>( _height));
}

void GridLines::handleWindowSizeChanged(int w, int h)
{
    _width = w;
    _height = h;
}

