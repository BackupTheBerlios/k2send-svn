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
#include <kfilemetainfo.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

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
#include <qtimer.h>

#include "k2sendplaylistitem.h"

int
    K2sendPlayListItem::_id =
    0;


K2sendPlayListItem::K2sendPlayListItem (KListView * p, QString fn):
KListViewItem (p), m_filename (fn), _playing (FALSE), _tag ("k2sendtag")
{

    QString rate;
    _id++;
    _my_id = _id;
    _valid = TRUE;
    TagLib::MPEG::File mp3file (m_filename.latin1 ());
    if (!mp3file.tag () || !mp3file.isValid ()) {
        kdDebug (200010) << "K2sendPlayListItem " << m_filename << " not valid " << endl;
        _valid = FALSE;
        return;
    }

    str_id.sprintf ("%03i", p->childCount ());
    setText (0, str_id);

    if (!mp3file.tag ()->title ().isEmpty ()) {
        m_title = mp3file.tag ()->title ().toCString ();
        setText (1, m_title);
        m_album = mp3file.tag ()->album ().toCString ();
        setText (2, m_album);
        m_artist = mp3file.tag ()->artist ().toCString ();
        setText (3, m_artist);

    }
    else {
        QFileInfo info (m_filename);
        m_title = info.baseName ();
        setText (1, m_title);
    }
    int len = mp3file.audioProperties ()->length ();
    if (len)
        m_length.sprintf ("%02i:%02i", len / 60, len % 60);
    else {
        kdDebug (200010) << "K2sendPlayListItem " << m_filename << " is zero lenght " << endl;
        _valid = FALSE;
        return;
    }
    rate.sprintf ("%i", mp3file.audioProperties ()->bitrate ());
    setText (4, m_length);
    setText (5, rate);
    _normal_height = height ();
    _double_height = (int) (height () * 1.5);

}

void
K2sendPlayListItem::setPlaying (bool p)
{
    _playing = p;
    if (p) {
        setSelected (FALSE);
        setHeight (_double_height);
        setText (0, 0);
        setPixmap (0, DesktopIcon ("player_play", 16));
    }
    else {
        setHeight (_normal_height);
        setPixmap (0, 0);
        setText (0, str_id);
    }
}


void
K2sendPlayListItem::paintCell (QPainter * p, const QColorGroup & cg, int column, int width, int alignment)
{
    QColorGroup _cg (cg);
    QColor c = _cg.text ();
    if (_playing) {
        _cg.setColor (QColorGroup::Text, _color);
        //_cg.setColor( QColorGroup::Base, Qt::white );
        QListViewItem::paintCell (p, _cg, column, width, alignment);
    }
    if (isAlternate ())
        _cg.setColor (QColorGroup::Base, KGlobalSettings::alternateBackgroundColor ());
    QListViewItem::paintCell (p, _cg, column, width, alignment);


}

void
K2sendPlayListItem::setColor (int r, int g, int b)
{
    _color.setRgb (r, g, b);
    repaint ();
}
QString & K2sendPlayListItem::title()
{
    return m_title;
}
QString & K2sendPlayListItem::artist()
{
    return m_artist;
}
QString & K2sendPlayListItem::album()
{
    return m_album;
}
QString & K2sendPlayListItem::length()
{
    return m_length;
}

K2sendPlayListItem::~K2sendPlayListItem ()
{

}
