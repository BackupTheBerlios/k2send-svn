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
// $HeadURL
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


#include "k2sendplaylistitem.h"

int K2sendPlayListItem::_id=0;

K2sendPlayListItem::K2sendPlayListItem(KListView * p , QString  fn) : QListViewItem(p) , filename(fn)
{

    QString length;
    QString rate;
    _id++;
    _my_id = _id;
    _valid = TRUE;
    TagLib::MPEG::File mp3file(filename.latin1());

    if(!mp3file.tag() || !mp3file.isValid()){
          kdDebug(200010) << "K2sendPlayListItem " << filename << " not valid " << endl;
         _valid = FALSE;
         return;
    }

    if (!mp3file.tag()->title().isEmpty()){
        this->setText(0,mp3file.tag()->title().toCString());
        this->setText(1,mp3file.tag()->album().toCString());
        this->setText(2,mp3file.tag()->artist().toCString());

    } else {
        QFileInfo info(filename);
        this->setText(0,info.baseName());

    }
    int len = mp3file.audioProperties()->length();
    if (len)
        length.sprintf("%02i:%02i",len/60,len%60);
    else {
        kdDebug(200010) << "K2sendPlayListItem " << filename << " not is zero lenght " << endl;
        _valid = FALSE;
        return;
    }
    rate.sprintf("%i", mp3file.audioProperties()->bitrate());
    this->setText(3,length);
    this->setText(4,rate);
}

K2sendPlayListItem::~K2sendPlayListItem(){


}

