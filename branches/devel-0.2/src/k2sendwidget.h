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

// $Id: k2sendwidget.h 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $


#ifndef _K2SENDWIDGET_H_
#define _K2SENDWIDGET_H_

#include <kstatusbar.h>
#include <qslider.h>

#include "k2sendwidgetbase.h"

class K2sendPlayListItem;
class K2sendPlayer;
class K2sendConsole;
class KURL;
class KPrinter;

class k2sendWidget : public k2sendWidgetBase
{
    Q_OBJECT

public:
    k2sendWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0, KConfig * c = 0);
    ~k2sendWidget();
    void consoleConfigRefresh(QString& tty);
    void configRefresh(QString& addr);
    void customEvent( QCustomEvent * e );
    QString currentURL();
    virtual void openURL(QString url);
    virtual void openURL(const KURL& url);
    void print(QPainter *, KPrinter * kp,int height, int width);


signals:
    void signalChangeStatusbar(const QString& text);
    void signalChangeCaption(const QString& text);

public slots:
    virtual void slotPlay();
    virtual void slotStop();
    virtual void slotSkip();
    virtual void slotLoudness();
    virtual void slotVolume();
    virtual void slotAddFiles();
    virtual void slotRemoveFiles();
    virtual void slotConfig();
    virtual void slotConsoleConfig();
    virtual void slotLength();
    virtual void slotSelectItem( QListViewItem * item,const QPoint&, int);
    virtual void slotPlaylistClear();
    virtual void slotConsolePlay();
    virtual void slotConsoleStop();
    virtual void slotConsoleClear();
    virtual void slotLengthPressed();

private slots:
    void slotOnURL(const QString& url);
    void slotSetTitle(const QString& title);

private:
    bool isDoubleEntry(const QString& file);
    void readPlaylist();
    void writePlaylist();
    void setIndex();
    void nextIndex();
    void addDir(const QString & path);
    void setSelected();

    K2sendPlayer       * m_player;
    K2sendConsole      * m_console_cont;
    KConfig            * m_config;
    K2sendPlayListItem * m_head;
    K2sendPlayListItem * m_last;
    bool               length_pressed;
    QString             m_url;
};





#endif

