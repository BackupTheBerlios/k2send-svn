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

// $Id: k2sendwidget.cpp 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $


#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qprogressbar.h>
#include <qfileinfo.h>
#include <qvalidator.h>
#include <qvalidator.h>
#include <qdir.h>
#include <qstrlist.h>
#include <qpainter.h>
#include <qfont.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kuser.h>
#include <kurl.h>
#include <kfiletreebranch.h>
#include <kfiletreeview.h>
#include <kinputdialog.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <ktextbrowser.h>
#include <kprinter.h>
#include <kfiletreebranch.h>

#include "k2sendwidget.h"
#include "k2sendplaylist.h"
#include "k2sendplaylistitem.h"
#include "k2sendplayer.h"
#include "k2sendconsole.h"


k2sendWidget::k2sendWidget(QWidget* parent, const char* name, WFlags fl,KConfig * c)
        : k2sendWidgetBase(parent,name,fl) ,m_config(c), m_head(NULL) , m_last(NULL),length_pressed(FALSE)
{
    KUser user;
    m_config->setGroup("bluetooth");
    QString addr = m_config->readEntry ("baddr","00:00:00:00:00:00");

    m_console->setTextFormat(QTextEdit::PlainText);
    m_source->setShowFolderOpenPixmap(TRUE);
    m_source->addColumn("File");
    //m_source->addBranch(KURL(user.homeDir()),"Home");
    m_source->setDragEnabled(TRUE);

    m_config->setGroup("player");
    int volume    = m_config->readNumEntry ("volume",50);
    int loud_filt = m_config->readNumEntry ("loud_filt",0);

    m_player = new K2sendPlayer(this,volume,loud_filt);

    m_config->setGroup("console");
    QString tty = m_config->readEntry ("tty","/dev/ttySX");
    m_console_cont = new K2sendConsole(this);

    m_bar->setPercentageVisible(TRUE);
    m_player->setAddr(addr);
    m_player->start();
    m_console->clear();
    m_console_cont->setTty(tty);
    m_volume->setValue(volume);

    readPlaylist();
    readSourcelist();

    connect( m_playlist, SIGNAL(doubleClicked ( QListViewItem * ,const QPoint&, int) ),
        this, SLOT( slotSelectItem( QListViewItem * ,const QPoint&, int) ) );
}

k2sendWidget::~k2sendWidget()
{
    m_config->setGroup("player");
    m_config->writeEntry ("volume",m_player->currentVolume());
    m_config->writeEntry ("loud_filt",m_player->currentLoundness());
    m_config->sync();
    writePlaylist();
    writeSourcelist();
    kdDebug(200010) << "k2sendWidget::~k2sendWidget config written " <<   endl;
    delete m_player;
}


void k2sendWidget::writePlaylist()
{
    QListViewItemIterator it( m_playlist );
    QStrList list;
    int cnt = 0;
    int total = list.count();
    while ( it.current() ) {
        K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();
        list.append(item->file().latin1());
        ++it;
        cnt++;
        m_bar->setProgress( int((100.0 / total) * cnt));
    }
    m_config->setGroup("playlist");
    m_config->writeEntry ("files",list);
    m_config->sync();
}

void k2sendWidget::readPlaylist()
{
    QStrList list;
    m_config->setGroup("playlist");
    m_config->readListEntry ("files",list);
    if (list.count()){
        QStrListIterator it( list );
        while (it.current()) {
            QString file = QString::fromUtf8(it.current());
            K2sendPlayListItem * new_item = new K2sendPlayListItem((KListView*)m_playlist,file);
            if (new_item->valid())
                m_playlist->insertItem (new_item);
            else {
                delete new_item;
            }++it;
        }
    }
    QString msg = QString("%1 Files").arg(m_playlist->childCount());
    ((KMainWindow*)parent())->statusBar()->changeItem(msg, 0);
}

void k2sendWidget::writeSourcelist()
{
    QStrList list;
    KFileTreeBranchList branches;
    KFileTreeBranch * branch;
    branches = m_source->branches();
    for ( branch = branches.first(); branch; branch = branches.next() )
        list.append(branch->rootUrl().path().latin1());
    m_config->setGroup("source");
    m_config->writeEntry ("files",list);
    m_config->sync();
}

void k2sendWidget::readSourcelist()
{
    QStrList list;
    m_config->setGroup("source");
    m_config->readListEntry ("files",list);
    if (list.count()){
        QStrListIterator it( list );
        while (it.current()) {
            QString file = QString::fromUtf8(it.current());
            openURL(file);
            ++it;
        }
    }
}


void k2sendWidget::customEvent( QCustomEvent * e )
{

    K2sendStatusEvent::Type  t = (K2sendStatusEvent::Type)e->type();
    K2sendStatusEvent * se = (K2sendStatusEvent*)e;
    QString * fn;
    switch (t){
        case K2sendStatusEvent::EventMessage:
            ((KMainWindow*)parent())->statusBar()->message (se->string(),se->value());
            break;
        case K2sendStatusEvent::EventError:
            KMessageBox::error(this, se->string());
            break;
        case K2sendStatusEvent::EventTime:
            ((KMainWindow*)parent())->statusBar()->changeItem (se->string(), 1);
            break;
        case K2sendStatusEvent::EventRate:
            ((KMainWindow*)parent())->statusBar()->changeItem (se->string(), 2);
            break;
        case K2sendStatusEvent::EventLoudness:
            ((KMainWindow*)parent())->statusBar()->changeItem (se->string(), 3);
            break;
        case K2sendStatusEvent::EventAddr:
            ((KMainWindow*)parent())->statusBar()->changeItem (se->string(), 4);
            break;
        case K2sendStatusEvent::EventProgress:
            m_bar->setProgress(se->value());
            break;
        case K2sendStatusEvent::EventLength:
            if (!length_pressed)
                m_length->setValue(se->value());
            break;
        case K2sendStatusEvent::EventTitle:
            topLevelWidget()->setCaption(se->string());
            break;
        case K2sendStatusEvent::EventConsole:
            m_console->insert(se->string());
            m_console->scrollToBottom ();
            break;
        case K2sendStatusEvent::EventVolume:
            m_volume->setTracking(FALSE);
            m_volume->setValue(se->value());
            m_volume->setTracking(TRUE);
            break;
        case K2sendStatusEvent::EventSkip:
            nextIndex();
            break;
        case K2sendStatusEvent::EventEnqueue:
            kdDebug(200010) << " K2sendStatusEvent::EventEnqueue: fetch file" << endl;
            if (m_head){
                if (m_last){
                    m_last->setPlaying(FALSE);
                }
                m_head->setPlaying(TRUE);
                kdDebug(200010) << " K2sendStatusEvent::EventEnqueue: m_head=" << m_head->file() << endl;
                fn = new QString(m_head->file());
                m_player->addFile(fn);
            }
            break;
        default:
            break;

    }
}


void k2sendWidget::setIndex()
{
    QListViewItemIterator it( m_playlist );
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
        kdDebug(200010) << " K2sendWidget::setIndex take firstChild" << endl;
        m_head = (K2sendPlayListItem*) m_playlist->firstChild();
    }
    if (m_head)
        kdDebug(200010) << " K2sendWidget::setIndex m_head=" << m_head->file() << endl;
}
void k2sendWidget::nextIndex()
{

    QListViewItemIterator it( m_playlist );
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
        kdDebug(200010) << " K2sendWidget::nextIndex take firstChild" << endl;
        m_head = (K2sendPlayListItem*)  m_playlist->firstChild();
    }
    if (m_head)
        kdDebug(200010) << " K2sendWidget::nextIndex m_head=" << m_head->file() << endl;
}


void k2sendWidget::slotSelectItem(QListViewItem * item,const QPoint& p, int i){
    if (m_playlist->childCount()){
        if (m_head)
            m_head->setPlaying(FALSE);
        m_head = (K2sendPlayListItem*) item;
        K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Play,0);
        m_player->addCommand(c);
   }
}


void k2sendWidget::slotPlay()
{
    if (m_playlist->childCount()){
        setIndex();
        K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Play,0);
        m_player->addCommand(c);
    }
}

void k2sendWidget::slotStop()
{
    K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Stop,0);
    if (m_head)
        m_head->setPlaying(FALSE);
    m_player->addCommand(c);

}

void k2sendWidget::slotSkip()
{
    if (m_playlist->childCount()){
        nextIndex();
        K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Skip,0);
        m_player->addCommand(c);
    }
}

void k2sendWidget::slotLoudness()
{
    K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Loudness,0);
    m_player->addCommand(c);
}

void k2sendWidget::slotVolume()
{
    K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Volume,m_volume->value());
    m_player->addCommand(c);
}

void k2sendWidget::slotLength()
{
    K2sendPlayerCommand * c = new K2sendPlayerCommand(K2sendPlayerCommand::Length,m_length->value());
    m_player->addCommand(c);
    length_pressed = FALSE;

}

void k2sendWidget::slotLengthPressed()
{
        length_pressed = TRUE;
}



void k2sendWidget::addDir(const QString & path)
{
    K2sendPlayListItem * new_item;

    QDir dir(path);
    dir.setMatchAllDirs (TRUE);
    dir.setFilter( QDir::All );
    dir.setSorting( QDir::Name | QDir::DirsFirst);
    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    int cnt = 0;
    int total = list->count();


    while ( (fi = it.current()) != 0 ) {
        cnt++;
        m_bar->setProgress( int((100.0 / total) * cnt));
        if (fi->isDir()){
            if (fi->fileName() != "." &&  fi->fileName() != "..")
                addDir(fi->filePath());
        } else {
            if (!isDoubleEntry(fi->filePath())){
                kdDebug(200010) << "k2sendWidget::addDir file=" << fi->filePath() << endl;

                new_item = new K2sendPlayListItem((KListView*)m_playlist, fi->filePath());
                if (new_item->valid())
                    m_playlist->insertItem (new_item);
                else
                    delete new_item;
            }
        }
        ++it;
    }
    m_bar->setProgress(0);
}
bool k2sendWidget::isDoubleEntry(const QString& file) {
    QListViewItemIterator it( m_playlist );
    while ( it.current() ) {
        K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();
        if ( item->file() == file ){
            return TRUE;
        }
        ++it;
    }
    return FALSE;
}


void k2sendWidget::slotAddFiles()
{
    KFileTreeViewItem * item = m_source->currentKFileTreeViewItem();
    if (item){
        if (!item->isDir()){
            if (!isDoubleEntry(item->path())){
                kdDebug(200010) << "k2sendWidget::slotAddFiles file=" << item->path() << endl;
                K2sendPlayListItem * new_item = new K2sendPlayListItem((KListView*)m_playlist,item->path());
                if (new_item->valid())
                    m_playlist->insertItem (new_item);
                else
                    delete new_item;
            }
        } else {
            addDir(item->path());
        }
        QString msg = QString("%1 Files").arg(m_playlist->childCount());
        ((KMainWindow*)parent())->statusBar()->changeItem(msg, 0);
    }
}

void k2sendWidget::slotRemoveFiles()
{
    QPtrList< QListViewItem > list = m_playlist->selectedItems();
    if (list.count()){
        K2sendPlayListItem * item;
        for (item = (K2sendPlayListItem*)list.first(); item; item = (K2sendPlayListItem*)list.next() ){
            m_playlist->removeItem(item);
        }
        QString msg = QString("%1 Files").arg(m_playlist->childCount());
        ((KMainWindow*)parent())->statusBar()->changeItem(msg, 0);
    }
}

void k2sendWidget::slotPlaylistClear(){
    m_head = 0;
    m_playlist->clear();
    QString msg = QString("%1 Files").arg(m_playlist->childCount());
    ((KMainWindow*)parent())->statusBar()->changeItem(msg, 0);
}

void k2sendWidget::slotConfig()
{
    bool    ok;
    QString addr;
    QRegExp rx( "[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}" );
    QValidator* validator = new QRegExpValidator( rx, this );
    addr = KInputDialog::getText("Enter Bluetooth Address","Address",m_player->addr(),&ok,this,0,validator);
    if (ok){
        m_player->setAddr(addr);
        m_config->setGroup("bluetooth");
        m_config->writeEntry ("baddr",addr);
        m_config->sync();
    }
}

void k2sendWidget::slotConsoleConfig()
{
    bool    ok;
    QString tty;
    tty = KInputDialog::getText("Enter Debug TTY ","Device",m_console_cont->tty(),&ok);
    if (ok){
        m_console_cont->setTty(tty);
        m_console_cont->restart();
        m_config->setGroup("console");
        m_config->writeEntry ("tty",tty);
        m_config->sync();
    }
}
void k2sendWidget::configRefresh(QString & addr)
{
    m_player->setAddr(addr);
}

void k2sendWidget::consoleConfigRefresh(QString & tty)
{
    m_console_cont->setTty(tty);
}

void k2sendWidget::slotConsolePlay()
{
    ((KMainWindow*)parent())->statusBar()->message("Starting console", 2000);
    m_console_cont->restart();
}

void k2sendWidget::slotConsoleStop()
{

    if (m_console_cont->running()){
        ((KMainWindow*)parent())->statusBar()->message("Stopping console", 2000);
        m_console_cont->stop();
    }
}

void k2sendWidget::slotConsoleClear()
{
    m_console->clear();
}


void k2sendWidget::print(QPainter *p, KPrinter *kp,int height, int width)
{
    QListViewItemIterator it( m_playlist );
    QStringList list;
    QStringList::Iterator st;
    QFontMetrics fm( m_playlist->font());
    int h = fm.height();
    int pos,len;
    int mul = width / 100;
    int width_per[] = {8,25,25,25,8,8} ;
    int y = 0,x = 0 , i=0;
    y+=h;

    p->setPen( Qt::black );
    p->setFont(m_playlist->font());
    list.append("Id");
    list.append("Tile");
    list.append("Album");
    list.append("Artist");
    list.append("Length");
    list.append("Bitrate");
    for (st = list.begin(),i=0; st != list.end(); ++st,i++) {
        p->drawText(x,y,(*st));
        x += (width_per[i] * mul);
    }
    y+=2;
    p->drawLine(0,y,width,y);
    y+=h;
    while ( it.current() ) {
        K2sendPlayListItem *item =(K2sendPlayListItem*) it.current();

        for (i=0,x=0; i<6; i++){
            pos = item->text(i).length();
            while ((len = fm.width(item->text(i).left(pos)))> (width_per[i] * mul)) {
                pos--;
            }
            p->drawText(x,y,item->text(i).left(pos),len);
            x += (width_per[i] * mul);
        }
        ++it;
        y+=h;
        if (y>=height){
            kp->newPage();
            y=h;
            x=0;
            for (st = list.begin(),i=0; st != list.end(); ++st,i++) {
                p->drawText(x,y,(*st));
                x += (width_per[i] * mul);
            }
            y+=2;
            p->drawLine(0,y,width,y);
            y+=h;
        }

    }
}

QString k2sendWidget::currentURL()
{
    return m_url;
}
void k2sendWidget::openURL(const KURL& url){
    m_url = url.path();
    m_source->addBranch(url,url.fileName());
}

void k2sendWidget::openURL(QString url)
{
    openURL(KURL(url));
}

void k2sendWidget::slotOnURL(const QString& url)
{
    emit signalChangeStatusbar(url);
}

void k2sendWidget::slotSetTitle(const QString& title)
{
    emit signalChangeCaption(title);
}

#include "k2sendwidget.moc"

