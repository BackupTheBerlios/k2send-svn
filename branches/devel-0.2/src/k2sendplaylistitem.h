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

class  K2sendPlayListItem : public  QListViewItem  {

    public:
        K2sendPlayListItem(KListView * p , QString  fn);
        ~K2sendPlayListItem();
        const QString & file() const { return filename; }
        int id() const { return _my_id; }
        int valid() const { return _valid; }
        virtual void paintCell( QPainter *p, const QColorGroup &cg,
                                 int column, int width, int alignment );
        void setPlaying(bool p);
        QString tag() { return _tag; }


    private:
         QString filename;
         static int _id;
         int _my_id;
         bool _valid;
         bool _playing;
         QString _tag;
         int c;
         int dir;
};



