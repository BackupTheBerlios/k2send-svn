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

// $Id: k2sendplaylistitem.h 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $

#include <qlistview.h>
#include <klistview.h>

class KlistView;
class QTimer;

class K2sendPlayListItem:public KListViewItem
{

  public:
    K2sendPlayListItem (KListView * p, QString fn);
    ~K2sendPlayListItem ();
    const QString & file () const
    {
        return m_filename;
    }
    int id () const
    {
        return _my_id;
    }
    int valid () const
    {
        return _valid;
    }
    virtual void paintCell (QPainter * p, const QColorGroup & cg, int column, int width, int alignment);
    void setPlaying (bool p);
    bool playing ()
    {
        return _playing;
    }
    QString tag ()
    {
        return _tag;
    }
    void setColor (int r, int g, int b);
    QString & title();
    QString & artist();
    QString & album();
    QString & length();


  private:
    QString m_filename;
    QString m_title;
    QString m_album;
    QString m_artist;
    QString m_length;

    static int _id;
    int _my_id;
    bool _valid;
    bool _playing;
    QString _tag;
    QColor _color;
    int _normal_height;
    int _double_height;
    QString str_id;

};
