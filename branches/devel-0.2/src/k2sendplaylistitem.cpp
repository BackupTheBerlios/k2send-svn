/***************************************************************************
 *   Copyright (C) 2004 by David Voswinkel                                 *
 *   d.voswinkel@netcologne.de                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// $Id: k2sendplaylistitem.cpp 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $


#include <klistview.h>
#include <kdebug.h>

#include <taglib.h>
#include <tag.h>
#include <tstring.h>
#include <mpegproperties.h>
#include <mpegfile.h>

#include <qlistview.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qobject.h>

#include "k2sendplaylistitem.h"

int K2sendPlayListItem::_id=0;


K2sendPlayListItem::K2sendPlayListItem(KListView * p , QString  fn) :
                    QListViewItem(p) , filename(fn) ,_playing(FALSE) , _tag("k2sendtag")
{

    QString length;
    QString rate;
    _id++;
    _my_id = _id;
    _valid = TRUE;
    c = 0;
    dir = 0;
    TagLib::MPEG::File mp3file(filename.latin1());

    if(!mp3file.tag() || !mp3file.isValid()){
          kdDebug(200010) << "K2sendPlayListItem " << filename << " not valid " << endl;
         _valid = FALSE;
         return;
    }
    QString str_id;
    str_id.sprintf("%03i",p->childCount());
    setText(0,str_id);

    if (!mp3file.tag()->title().isEmpty()){
        setText(1,mp3file.tag()->title().toCString());
        setText(2,mp3file.tag()->album().toCString());
        setText(3,mp3file.tag()->artist().toCString());

    } else {
        QFileInfo info(filename);
        setText(1,info.baseName());

    }
    int len = mp3file.audioProperties()->length();
    if (len)
        length.sprintf("%02i:%02i",len/60,len%60);
    else {
        kdDebug(200010) << "K2sendPlayListItem " << filename << " is zero lenght " << endl;
        _valid = FALSE;
        return;
    }
    rate.sprintf("%i", mp3file.audioProperties()->bitrate());
    setText(4,length);
    setText(5,rate);
    //startTimer(100);
}


// void K2sendPlayListItem::timerEvent( QTimerEvent *e ){
//     if (dir){
//         c = c + 15;
//         if (c >=255)
//             dir = !dir;
//     } else {
//         c = c - 15;
//         if (c <= 0)
//             dir = !dir;
//     }
// }
//

void K2sendPlayListItem::paintCell( QPainter *p, const QColorGroup &cg,
                                 int column, int width, int alignment )
{
    QColorGroup _cg( cg );
    QColor c = _cg.text();

    if (_playing)
        _cg.setColor( QColorGroup::Text, Qt::red );

    QListViewItem::paintCell( p, _cg, column, width, alignment );

    _cg.setColor( QColorGroup::Text, c );
}

K2sendPlayListItem::~K2sendPlayListItem(){

}

