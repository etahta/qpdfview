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

#ifndef FILECRUD_H
#define FILECRUD_H
#include<QFile>
#include<QDir>

class FileCrud
{
  public:
     QString dosya;//FileCrud *fc=new FileCrud();
     //fc->dosya="test.txt";
     /*fc->fileWrite("bayram=1");
     fc->fileWrite("ahmet=6");
     fc->fileWrite("mehmet=3");
     fc->fileWrite("sabri=2");
     fc->fileWrite("fatih=5");
 */
   //qDebug()<<fc->fileSearch("mehmet");
     // fc->fileEdit(fc->fileSearch("mehmet"),"mehmet=88\n");
      //fc->fileDelete("sabri\n");
     //fc->fileRead();
     /*fc->fileWrite("bayram=1");
     fc->fileWrite("ahmet=6");
     fc->fileWrite("mehmet=3");
     fc->fileWrite("sabri=2");
     fc->fileWrite("fatih=5");
 */
     //qDebug()<<fc->fileexists();

    void fileremove()
{
        QFile file(QDir::homePath()+"/"+dosya);
//const QString yol=QDir::homePath()+"/"+dosya;
    //bool snc=QDir::remove(yol);
        file.remove();
       }
    bool fileexists()
    {
        QFile file(QDir::homePath()+"/"+dosya);

//        QFile file(dosya);
        if(file.exists()) return true; else return false;

    }
    void fileWrite(QString veri)
    {
        QFile data(QDir::homePath()+"/"+dosya);
        QFile newData(QDir::homePath()+"/"+dosya);

        /*for writing line by line to text file */
       /* if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << "1_XYZ="<<"abc"<<endl;
            stream << "2_XYZ="<<"xyx"<<endl;
        }
*/

      //  QFile data("/home/etapadmin/.config/eta/virtualkeyboard/config.ini");
        data.open(QIODevice::Text | QIODevice::ReadOnly);
        QString dataText = data.readAll();
      //  QRegularExpression re("AutoShow=true");
       // QString replacementText("AutoShow=false");
       // dataText.replace(re, replacementText);
         dataText.append(veri+"\n");
        //QFile newData("/home/etapadmin/.config/eta/virtualkeyboard/config.ini");
        if(newData.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&newData);
        out << dataText;
            }
            newData.close();
    }
    void fileRead()
    {
        QFile file(QDir::homePath()+"/"+dosya);
        /*
         * If file not exit it will create
         * */
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::ReadWrite))
        {
            qDebug() << "FAIL TO CREATE FILE / FILE NOT EXIT***";
        }

        /*for Reading line by line from text file*/
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            qDebug() << "read output - " << line;
        }

    }
    QString fileSearch(QString data)
    {
        QString rdata;
        QFile file(QDir::homePath()+"/"+dosya);
        /*
         * If file not exit it will create
         * */
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::ReadWrite))
        {
            qDebug() << "FAIL TO CREATE FILE / FILE NOT EXIT***";
        }
        QRegularExpression re(data);
        /*for Reading line by line from text file*/
        while (!file.atEnd()) {
            QString line = file.readLine();
           // qDebug() << "read output - " << line;
            if(line.contains(re)) rdata=line; //qDebug()<<"bulundu......";
        }
return rdata;
    }
    void fileDelete(QString delveri)
    {
        //   qDebug()<<"selamf";
         QFile data(QDir::homePath()+"/"+dosya);
           QFile newData(QDir::homePath()+"/"+dosya);
           data.open(QIODevice::Text | QIODevice::ReadOnly);
           QString dataText = data.readAll();
           QRegularExpression re(delveri);
          QString replacementText("");
           dataText.replace(re, replacementText);
            //dataText.remove(delveri);
           if(newData.open(QFile::WriteOnly | QFile::Truncate)) {
           QTextStream out(&newData);
           out << dataText;
               }
               newData.close();

    }
    void fileEdit(QString oldveri,QString newveri)
    {
        //   qDebug()<<"selamf";
           QFile data(QDir::homePath()+"/"+dosya);
           QFile newData(QDir::homePath()+"/"+dosya);
           data.open(QIODevice::Text | QIODevice::ReadOnly);
           QString dataText = data.readAll();
           QRegularExpression re(oldveri);
           QString replacementText(newveri);
           dataText.replace(re, replacementText);

           if(newData.open(QFile::WriteOnly | QFile::Truncate)) {
           QTextStream out(&newData);
           out << dataText;
               }
               newData.close();
    }
};





#endif // FILECRUD_H
