#include <qdragobject.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstrlist.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <klistview.h>
#include <kdebug.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kconfig.h>
#include <kmainwindow.h>

#include "k2sendplaylistitem.h"
#include "k2sendplaylist.h"
#include "k2sendwidget.h"

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
    setSorting(-1);
    setSelectionMode(QListView::Extended);
    setShowSortIndicator(TRUE);
    setDragEnabled(TRUE);
    setAcceptDrops(TRUE);
    setDropVisualizer(TRUE);
    setItemsMovable (TRUE);

     connect( this, SIGNAL(dropped (QDropEvent *, QListViewItem *, QListViewItem *)),
         this, SLOT(insertDroppedEvent(QDropEvent *, QListViewItem *, QListViewItem *)));
}

K2sendPlayList::~K2sendPlayList()
{

}

bool K2sendPlayList::acceptDrag(QDropEvent * event) const
{
    kdDebug(200010) << "K2sendPlayList::acceptDrag " << event->format() << endl;
    if (KURLDrag::canDecode(event)){
        event->accept();
        return TRUE;
    } else if (event->provides("application/x-qlistviewitem")){
        event->accept();
        return TRUE;
    }
    return FALSE;
}


void K2sendPlayList::insertDroppedEvent(QDropEvent *event, QListViewItem *p, QListViewItem *after)
{
    KURL::List urls;
    if (KURLDrag::decode(event, urls) && !urls.isEmpty()){
        KURL::List::iterator it;
        for ( it = urls.begin(); it != urls.end(); ++it ){
            if ((*it).isLocalFile()){
                add((*it).path(),(K2sendPlayListItem*)after);
            }
        }
    }
    QString msg = QString("%1 Files").arg(this->childCount());
    emit signalChangeStatusbar(msg);
}

bool K2sendPlayList::isDoubleEntry(const QString& file)
{
    QListViewItemIterator it( this );
    while ( it.current() ) {
        K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();
        if ( item->file() == file ){
            return TRUE;
        }
        ++it;
    }
    return FALSE;
}

void K2sendPlayList::add(const QString & path,K2sendPlayListItem * after )
{
    QFileInfo file(path);
    if (file.isDir()){
        addDir(path,after);
    } else if(file.isFile()){
        addFile(path,after);
    }
}


void K2sendPlayList::addFile(const QString & path,K2sendPlayListItem * after )
{
     if (isDoubleEntry(path))
         return;

    K2sendPlayListItem * new_item;
    new_item = new K2sendPlayListItem((KListView*)this, path);
    if (new_item->valid()){
        if (after){
            new_item->moveItem(after);
        }
    } else {
        this->removeItem(new_item);
        delete new_item;
    }
}


void K2sendPlayList::addDir(const QString & path,K2sendPlayListItem * after )
{
    K2sendPlayListItem * new_item;
    K2sendPlayListItem * last_item;

    QDir dir(path);
    dir.setMatchAllDirs (TRUE);
    dir.setFilter( QDir::All );
    dir.setSorting( QDir::Name | QDir::DirsFirst);
    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    int cnt = 0;
    last_item = after;
    while ( (fi = it.current()) != 0 ) {
        cnt++;
        if (fi->isDir()){
            if (fi->fileName() != "." &&  fi->fileName() != "..")
                addDir(fi->filePath());
        } else {
            if (!this->isDoubleEntry(fi->filePath())){

                new_item = new K2sendPlayListItem((KListView*)this, fi->filePath());
                if (new_item->valid()){
                    this->insertItem (new_item);
                    new_item->moveItem(last_item);
                    last_item = new_item;
                } else {
                    this->removeItem (new_item);
                    delete new_item;
                }
            }
        }
        ++it;
    }
}

void K2sendPlayList::write(KConfig * config,k2sendWidget * w)
{
    QListViewItemIterator it( this );
    QStrList list;
    int cnt = 0;
    int total = list.count();
    while ( it.current() ) {
        K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();
        list.append(item->file().latin1());
        ++it;
        cnt++;
        w->setProgress( int((100.0 / total) * cnt));
    }
    config->setGroup("playlist");
    config->writeEntry ("files",list);
    config->sync();
}

void K2sendPlayList::read(KConfig * config)
{
    QStrList list;
    config->setGroup("playlist");
    config->readListEntry ("files",list);
    if (list.count()){
        QStrListIterator it( list );
        while (it.current()) {
            QString file = QString::fromUtf8(it.current());
            K2sendPlayListItem * new_item = new K2sendPlayListItem((KListView*)this,file);
            if (new_item->valid())
                this->insertItem (new_item);
            else {
                delete new_item;
            }++it;
        }
    }
    QString msg = QString("%1 Files").arg(this->childCount());
    emit signalChangeStatusbar(msg);
}



void K2sendPlayList::setIndex()
{
    QListViewItemIterator it( this );
    while ( it.current() ) {
        K2sendPlayListItem *item = (K2sendPlayListItem*)it.current();
        if ( item->isSelected() ){
            m_last = m_head;
            m_head = item;

            break;
        }
        ++it;
    }
    if (!m_head){
        kdDebug(200010) << " K2sendPlayList::setIndex take firstChild" << endl;
        m_head = (K2sendPlayListItem*) this->firstChild();
    }
    if (m_head)
        kdDebug(200010) << " K2sendPlayList::setIndex m_head=" << m_head->file() << endl;
}
void K2sendPlayList::nextIndex()
{

    QListViewItemIterator it( this );
    if (m_head){
        while ( it.current() ) {
            K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();
            if ( item->id() == m_head->id() ){
                ++it;
                m_last = m_head;
                m_head = (K2sendPlayListItem*)it.current();
                break;
            }
            ++it;
        }
    }
    if (!m_head){
        kdDebug(200010) << " K2sendPlayList::nextIndex take firstChild" << endl;
        m_head = (K2sendPlayListItem*)  this->firstChild();
    }
    if (m_head)
        kdDebug(200010) << " K2sendPlayList::nextIndex m_head=" << m_head->file() << endl;
}

