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

// $Id: k2send.cpp 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $

#include "k2send.h"
#include "pref.h"

#include <qdragobject.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>

k2send::k2send()
    : KMainWindow( 0, "k2send" ),
      m_printer(0)
{
    m_config = new KConfig("k2send");
    m_view = new k2sendWidget(this,"k2sendwidget",0,m_config);

    setAcceptDrops(FALSE);

    setCentralWidget(m_view);
    setupActions();
    statusBar()->show();
    setAutoSaveSettings();
    statusBar()->insertFixedItem("9999 Files", 0, true);
    statusBar()->insertFixedItem("00:00:00", 1, true);
    statusBar()->insertFixedItem("000 kbit/s", 2, true);
    statusBar()->insertFixedItem("Loud: 0", 3, true);
    statusBar()->insertFixedItem("00:00:00:00:00:00 ", 4, true);
    statusBar()->changeItem ("0 Files", 0);
    statusBar()->changeItem ("0 kbit/s", 2);

    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));

}

k2send::~k2send()
{
    delete m_view;
}


void k2send::setupActions()
{
    KAction *action;
    KShortcut cut;
    setXMLFile( "k2sendui.rc" );
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());

    KStdAction::open(this, SLOT(fileOpen()), actionCollection());

    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection(),"preferences");

    action = new KAction(i18n("Play"), "player_play", cut, m_view, SLOT(slotPlay()), actionCollection(), "player_play");
    action = new KAction(i18n("Stop"), "player_stop", cut,m_view, SLOT(slotStop()), actionCollection(), "player_stop");
    action = new KAction(i18n("Next"), "player_fwd", cut,m_view, SLOT(slotSkip()), actionCollection(), "player_next");
    action = new KAction(i18n("Loudness"), 0, cut,m_view, SLOT(slotLoudness()), actionCollection(), "player_loudness");

    action = new KAction(i18n("Clear"), "reload", cut,m_view, SLOT(slotPlaylistClear()), actionCollection(), "playlist_clear");

    action = new KAction(i18n("Play"), "player_play", cut,m_view, SLOT(slotConsolePlay()), actionCollection(), "console_play");
    action = new KAction(i18n("Stop"), "player_stop", cut,m_view, SLOT(slotConsoleStop()), actionCollection(), "console_stop");
    action = new KAction(i18n("Clear"), "reload", cut,m_view, SLOT(slotConsoleClear()), actionCollection(), "console_clear");
    createGUI();

}
void k2send::load(const KURL& url)
{
    QString target;
    setCaption(url.prettyURL());
    m_view->openURL(url);
}

void k2send::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept(KURLDrag::canDecode(event));
}

void k2send::dropEvent(QDropEvent *event)
{
    KURL::List urls;
    if (KURLDrag::decode(event, urls) && !urls.isEmpty())
    {
        const KURL &url = urls.first();
        load(url);
    }
}

void k2send::fileNew()
{
    (new k2send)->show();
}

void k2send::fileOpen()
{
    KURL url = KFileDialog::getExistingURL(QString::null,this, i18n("Open Mp3 Location"));
    if (!url.isEmpty())
        m_view->openURL(url);
}

void k2send::filePrint()
{
    if (!m_printer) m_printer = new KPrinter;

    m_printer->setFullPage( true );
    m_printer->setPageSelection(KPrinter::ApplicationSide);
    if (m_printer->setup(this))
    {
        QPainter p;
        p.begin(m_printer);
        QPaintDeviceMetrics metrics(m_printer);
        m_view->print(&p,m_printer,metrics.height(), metrics.width());
        p.end();
    }
}

void k2send::optionsShowToolbar()
{
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void k2send::optionsShowStatusbar()
{
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void k2send::optionsConfigureKeys()
{
    KKeyDialog::configure(actionCollection(), "k2sendui.rc");
}

void k2send::optionsConfigureToolbars()
{
    // use the standard toolbar editor
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config());
# endif
#else
    saveMainWindowSettings(KGlobal::config());
#endif
}

void k2send::newToolbarConfig()
{
    createGUI();

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
#endif
}

void k2send::optionsPreferences()
{
    k2sendPreferences dlg(m_config);
    if (dlg.exec())
    {

        kdDebug(200010) << " k2send::optionsPreferences refresh settings" << endl;
        QString str;
        m_config->setGroup("console");
        str = m_config->readEntry("tty");
        m_view->consoleConfigRefresh(str);
        m_config->setGroup("bluetooth");
        str = m_config->readEntry("baddr");
        m_view->configRefresh(str);
    }
}

void k2send::changeStatusbar(const QString& text)
{
    statusBar()->message(text);
}

void k2send::changeCaption(const QString& text)
{
    setCaption(text);
}
#include "k2send.moc"
