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
// $HeadURL
//
// $LastChangedBy$


#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qprogressbar.h>
#include <qfileinfo.h>
#include <qvalidator.h>
#include <qvalidator.h>
#include <qdir.h>
#include <qstrlist.h>

#include <kmessagebox.h>
#include <kuser.h>
#include <kurl.h>
#include <kfiletreebranch.h>
#include <kfiletreeview.h>
#include <kinputdialog.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <ktextbrowser.h>

#include "k2sendwidget.h"
#include "k2sendplaylistitem.h"
#include "k2sendplayer.h"
#include "k2sendconsole.h"


k2sendWidget::k2sendWidget(QWidget* parent, const char* name, WFlags fl)
        : k2sendWidgetBase(parent,name,fl) , m_head(NULL)
{
    KUser user;
    m_config =  new KSimpleConfig("k2send");
    QString addr = m_config->readEntry ("baddr","00:A0:96:20:89:FC");
    QString tty = m_config->readEntry ("tty","/dev/ttyS0");

    m_console->setTextFormat(QTextEdit::PlainText);

    m_source->setShowFolderOpenPixmap(TRUE);
    m_source->addColumn("File");

    m_playlist->addColumn("Tile",80);
    m_playlist->addColumn("Album",80);
    m_playlist->addColumn("Artist",80);
    m_playlist->addColumn("Length",40);
    m_playlist->addColumn("Bitrate",40);
    m_playlist->setSorting(2);
    m_playlist->setSelectionMode(QListView::Extended);
    m_playlist->setShowSortIndicator(TRUE);
    m_source->addBranch(KURL(user.homeDir()),"Home");

    int volume    = m_config->readNumEntry ("volume",50);
    int loud_filt = m_config->readNumEntry ("loud_filt",0);

    m_player = new K2sendPlayer(this,volume,loud_filt);
    m_console_cont = new K2sendConsole(this);

    m_status = new KStatusBar (this);
    k2sendwidgetbaseLayout->addMultiCellWidget( m_status, 7, 7, 0, 3 );

    m_status->insertFixedItem("9999 Files", 0, true);
    m_status->insertFixedItem("00:00:00", 1, true);
    m_status->insertFixedItem("000 kbit/s", 2, true);
    m_status->insertFixedItem("Loud: 0", 3, true);
    m_status->insertFixedItem("00:00:00:00:00:00 ", 4, true);
    m_status->changeItem ("0 Files", 0);
    m_status->changeItem ("0 kbit/s", 2);
    m_bar->setPercentageVisible(TRUE);
    m_player->setAddr(addr);
    m_player->start();
    m_console->clear();
    m_console_cont->setTty(tty);
    m_console_cont->start();
    m_volume->setValue(volume);
    readPlaylist();
    connect( m_playlist, SIGNAL(doubleClicked ( QListViewItem * ,const QPoint&, int) ),
        this, SLOT( slotSelectItem( QListViewItem * ,const QPoint&, int) ) );
    length_pressed = FALSE;
}

k2sendWidget::~k2sendWidget()
{
    m_config->writeEntry ("volume",m_player->currentVolume());
    m_config->writeEntry ("loud_filt",m_player->currentLoundness());
    m_config->sync();
    writePlaylist();
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
    m_config->writeEntry ("playlist",list);
    m_config->sync();
}

void k2sendWidget::readPlaylist()
{
    QStrList list;
    m_config->readListEntry ("playlist",list);
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
    m_status->changeItem(msg, 0);
}


void k2sendWidget::customEvent( QCustomEvent * e )
{

    K2sendStatusEvent::Type  t = (K2sendStatusEvent::Type)e->type();
    K2sendStatusEvent * se = (K2sendStatusEvent*)e;
    QString * fn;
    switch (t){

        case K2sendStatusEvent::EventMessage:
            m_status->message (se->string(),se->value());
            break;
        case K2sendStatusEvent::EventError:
            KMessageBox::error(this, se->string());
            break;
        case K2sendStatusEvent::EventTime:
            m_status->changeItem (se->string(), 1);
            break;
        case K2sendStatusEvent::EventRate:
            m_status->changeItem (se->string(), 2);
            break;
        case K2sendStatusEvent::EventLoudness:
            m_status->changeItem (se->string(), 3);
            break;
        case K2sendStatusEvent::EventAddr:
            m_status->changeItem (se->string(), 4);
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
                kdDebug(200010) << " K2sendStatusEvent::EventEnqueue: m_head=" << m_head->file() << endl;
                m_playlist->setCurrentItem(m_head);
                fn = new QString(m_head->file());
                m_player->addFile(fn);
            }
            break;

    }
}


void k2sendWidget::setIndex()
{
    QListViewItemIterator it( m_playlist );
    while ( it.current() ) {
        K2sendPlayListItem *item = (K2sendPlayListItem*)it.current();
        if ( item->isSelected() ){
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
    if (!item->isDir()){
         if (!isDoubleEntry(item->path())){
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
    m_status->changeItem(msg, 0);
}

void k2sendWidget::slotRemoveFiles()
{
    QPtrList< QListViewItem > list = m_playlist->selectedItems();
    K2sendPlayListItem * item;
    for (item = (K2sendPlayListItem*)list.first(); item; item = (K2sendPlayListItem*)list.next() ){
        m_playlist->removeItem(item);
    }
    QString msg = QString("%1 Files").arg(m_playlist->childCount());
    m_status->changeItem(msg, 0);
}

void k2sendWidget::slotPlaylistClear(){
    m_playlist->clear();
    QString msg = QString("%1 Files").arg(m_playlist->childCount());
    m_status->changeItem(msg, 0);
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
        m_config->writeEntry ("tty",tty);
        m_config->sync();
    }
}
void k2sendWidget::slotConsolePlay()
{
    m_status->message("Starting console", 2000);
    m_console_cont->restart();
}



#include "k2sendwidget.moc"

