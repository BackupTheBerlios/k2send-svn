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

// $Id: k2send.h 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $





#ifndef _K2SEND_H_
#define _K2SEND_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>

#include "k2sendwidget.h"

class KPrinter;
class KToggleAction;
class KURL;
class KConfig;
class KSystemTray;

class k2send:public KMainWindow
{
  Q_OBJECT public:
    k2send ();
    virtual ~ k2send ();
    void load (const KURL & url);
  protected:
      virtual void dragEnterEvent (QDragEnterEvent * event);
    virtual void dropEvent (QDropEvent * event);


    private slots:void fileNew ();
    void fileOpen ();
    void filePrint ();
    void importPlaylist ();
    void optionsShowToolbar ();
    void optionsShowStatusbar ();
    void optionsConfigureKeys ();
    void optionsConfigureToolbars ();
    void optionsPreferences ();
    void newToolbarConfig ();
    void changeStatusbar (const QString & text);
    void changeCaption (const QString & text);

  private:
    void setupAccel ();
    void setupActions ();

  private:
      k2sendWidget * m_view;
    KPrinter *m_printer;
    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
    KConfig *m_config;
  protected:
      KSystemTray * trayicon;
};

#endif // _K2SEND_H_
