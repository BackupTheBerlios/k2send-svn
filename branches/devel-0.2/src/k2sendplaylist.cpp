#include <qdragobject.h>
#include <qapplication.h>

#include <klistview.h>
#include <kdebug.h>

#include "k2sendplaylistitem.h"
#include "k2sendplaylist.h"


K2sendPlayList::K2sendPlayList( QWidget* parent, const char* name )
    : KListView( parent, name )
{
    dragging = FALSE;
    addColumn("Id",30);
    addColumn("Tile",80);
    addColumn("Album",80);
    addColumn("Artist",80);
    addColumn("Length",40);
    addColumn("Bitrate",40);
    setSorting(0);
    setSelectionMode(QListView::Extended);
    setShowSortIndicator(TRUE);
    setDragEnabled(TRUE);
    setAcceptDrops(TRUE);
}

K2sendPlayList::~K2sendPlayList()
{

}

void K2sendPlayList::dragEnterEvent( QDragEnterEvent *e )
{
    kdDebug(200010) << "K2sendPlayList::dragEnterEvent " << e << endl;
    if ( e->provides( "text" ) )
        e->accept();
}

void K2sendPlayList::dropEvent( QDropEvent *e )
{

    kdDebug(200010) << "K2sendPlayList::dropEvent " << e << endl;

    if ( !e->provides( "text/k2sendtag" ) ){
         kdDebug(200010) << "K2sendPlayList::dropEvent reject " << e << endl;
         return;
    }
    QString tag;

    if ( QTextDrag::decode( e, tag ) ) {
        //IconItem item = ((DnDDemo*) parentWidget())->findItem( tag );
         kdDebug(200010) << "K2sendPlayList::dropEvent tag=" << tag << endl;
        QListViewItem *after = itemAt( viewport()->mapFromParent( e->pos() ) );
        //K2sendPlayListItem *litem = new K2sendPlayListItem( this, after, item.name(), tag );
        //litem->setPixmap( 0, *item.pixmap() );
    }
}

// void K2sendPlayList::contentsMousePressEvent( QMouseEvent *e )
// {
//     QListView::contentsMousePressEvent( e );
//     dragging = TRUE;
//     pressPos = e->pos();
// }
//
// void K2sendPlayList::contentsMouseMoveEvent( QMouseEvent *e )
// {
//     QListView::contentsMouseMoveEvent( e );
//
//     if ( ! dragging ) return;
//
//     if ( !currentItem() ) return;
//
//     if ( ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
//         QTextDrag *drg = new QTextDrag( ((K2sendPlayListItem*)currentItem())->tag(), this );
//         drg->setSubtype( "k2sendtag" );
//         drg->dragCopy();
//         dragging = FALSE;
//     }
// }
//
// void K2sendPlayList::contentsMouseReleaseEvent( QMouseEvent *e )
// {
//     QListView::contentsMouseReleaseEvent( e );
//     dragging = FALSE;
// }

