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

// $Id$
//
// $HeadURL$
//
// $LastChangedBy$

#include <qdragobject.h>
#include <qdir.h>

#include <qstrlist.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kfiletreebranch.h>
#include <kfiletreeview.h>

#include "k2sendsource.h"
#include "k2sendwidget.h"

K2sendSource::K2sendSource( QWidget* parent, const char* name )
    : KFileTreeView( parent, name )
{
    setShowFolderOpenPixmap(TRUE);
    addColumn("File");
    setDragEnabled(TRUE);
    setSelectionModeExt(KListView::Extended);
}

QDragObject *  K2sendSource::dragObject ()
{
    KURL::List urls;
    const QPtrList<QListViewItem> fileList = selectedItems();
    QPtrListIterator<QListViewItem> it( fileList );
    for ( ; it.current(); ++it ){
        urls.append( static_cast<KFileTreeViewItem*>(it.current())->url() );
    }
    QPoint hotspot;
    QPixmap pixmap;
    if( urls.count() > 1 ){
        pixmap = DesktopIcon( "kmultiple", 16 );
    }
    if( pixmap.isNull() )
        pixmap = DesktopIcon( "document", 16 );

    QDragObject* dragObject = new KURLDrag( urls, this );
    if (!pixmap.isNull()){
        hotspot.setX( pixmap.width() / 2 );
        hotspot.setY( pixmap.height() / 2 );
        if( dragObject )
            dragObject->setPixmap( pixmap, hotspot );
    }
    return dragObject;
}


void K2sendSource::write(KConfig * config)
{
    QStrList list;
    KFileTreeBranchList branches;
    KFileTreeBranch * branch;
    branches = ((KFileTreeView*)this)->branches();
    for ( branch = branches.first(); branch; branch = branches.next() )
        list.append(branch->rootUrl().path().latin1());

    config->setGroup("source");
    config->writeEntry ("files",list);
    config->sync();
}

void K2sendSource::read(KConfig * config,k2sendWidget * w)
{
    QStrList list;
    config->setGroup("source");
    config->readListEntry ("files",list);
    if (list.count()){
        QStrListIterator it( list );
        while (it.current()) {
            QString file = QString::fromUtf8(it.current());
            w->openURL(file);
            ++it;
        }
    }
}
