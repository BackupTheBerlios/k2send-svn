/***************************************************************************
*   Copyright (C) 2004 by david                                           *
*   david@pimp.centrium.all                                               *
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

#include "pref.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <getopt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <klocale.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qapplication.h>


#include <kdebug.h>
#include <klistbox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include "k2sendwidget.h"

k2sendPreferences::k2sendPreferences(KConfig *c)
    : KDialogBase(TreeList, i18n("k2send Preferences"),
                Help|Default|Ok|Apply|Cancel, Ok) ,m_config(c)
{


    QFrame *frame;
    frame = addPage(i18n("Bluetooth"), i18n("Bluetooth settings"));
    QGridLayout *  layout_grid = new QGridLayout( frame, 1, 1, 11, 6, "baselayout");

    m_scan_label  = new QLabel( i18n(""),frame, "textLabel1" );
    layout_grid->addWidget( m_scan_label, 0, 1 );

    m_hci_label  = new QLabel( i18n(""),frame, "textLabel2" );
    layout_grid->addWidget( m_hci_label, 0, 0);

    m_list_device = new KListBox( frame, "list_devices" );
    layout_grid->addMultiCellWidget( m_list_device, 1, 2, 0, 0 );

    m_button_scan = new KPushButton( i18n("Scan"),frame, "button_scan");
    layout_grid->addWidget( m_button_scan, 2, 1 );

    QSpacerItem * spacer = new QSpacerItem( 20, 260, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout_grid->addItem( spacer, 1, 1 );

    frame = addPage(i18n("Player"), i18n("Player settings"));
    m_pagePlayer = new k2sendPrefPagePlayer(frame);

    frame = addPage(i18n("Console"), i18n("Debug console"));


    QVBoxLayout * layout_box = new QVBoxLayout( frame );
    layout_box->setSpacing(KDialog::spacingHint());

    QButtonGroup *  group = new QButtonGroup( 1, Qt::Horizontal, i18n("TTY settings"),frame ); ;
    layout_box->addWidget( group);

    QCheckBox * check = new QCheckBox( i18n("Enable debug console"),group, "checkBox1" );
    QLabel * label = new QLabel(i18n("Device"),group, "textLabel1" );
    m_edit_tty = new KLineEdit( group, "kLineEdit1" );
    m_config->setGroup("console");
    m_edit_tty->setText(m_config->readEntry ("tty","/dev/ttySX"));

    connect( m_button_scan, SIGNAL( released() ), this, SLOT( slotScan() ) );
    connect( this, SIGNAL(okClicked ()), this, SLOT( slotOk() ) );
    connect( this, SIGNAL(applyClicked ()), this, SLOT( slotApply() ) );

}

void k2sendPreferences::customEvent( QCustomEvent * e )
{

    ScanThreadEvent::Type  t = (ScanThreadEvent::Type)e->type();
    ScanThreadEvent * se = (ScanThreadEvent*)e;
    QListBoxText  * item;
    switch (t){
        case ScanThreadEvent::Error:
            KMessageBox::error (this, se->string1(),se->string2());
            break;
        case ScanThreadEvent::BAddr:
            item = new AddressItem(se->string1(),se->string2());
            m_list_device->insertItem(item);
           break;
        case ScanThreadEvent::Message:
            m_scan_label->setText(se->string1());
            break;
        case ScanThreadEvent::HCIAddr:
            m_hci_label->setText(se->string1());
            break;
        case ScanThreadEvent::Clear:
            m_list_device->clear();
            break;
    }
}

void k2sendPreferences::slotApply()
{

    if (!m_edit_tty->text().isEmpty()){
        kdDebug(200010) << "k2sendPreferences::slotApply tty=" << m_edit_tty->text() << endl;
        m_config->setGroup("console");
        m_config->writeEntry ("tty", m_edit_tty->text());

    }
    if (m_list_device->selectedItem()){
        AddressItem * item = (AddressItem *)m_list_device->selectedItem();
        kdDebug(200010) << "k2sendPreferences::slotApply baddr=" << item->addr() << endl;
        m_config->setGroup("bluetooth");
        m_config->writeEntry ("baddr",item->addr());

    }
    m_config->sync();
}

void k2sendPreferences::slotOk()
{
    kdDebug(200010) << "k2sendPreferences::slotOk()" << endl;
    slotApply();
    done(TRUE);
}

void k2sendPreferences::slotScan()
{
    m_scan_thread = new ScanThread(this);
    m_scan_thread->start();
}

ScanThread::~ScanThread()
{
    if (running()){
        terminate();
        wait();
    }
}

void ScanThread::run()
{
        ScanThreadEvent *se;
        inquiry_info *info = NULL;
        int num_rsp, length, flags;
        char addr[18];
        char name[248];
        int i, dd,dev_id;
        bdaddr_t ba;
        hci_dev_info di;
        QString str;

        length  = 5;  /* ~10 seconds */
        num_rsp = 5;
        flags = 0;
        flags |= IREQ_CACHE_FLUSH;

        se = new ScanThreadEvent(ScanThreadEvent::Message,"scanning...");
        QApplication::postEvent( m_parent, se );

        dev_id = hci_get_route(NULL);
        if (dev_id < 0) {
            se = new ScanThreadEvent(ScanThreadEvent::Error,"Device is not available", "Bluetooth");
            QApplication::postEvent( m_parent, se );
        }
        if (hci_devba(dev_id, &ba) < 0) {
            se = new ScanThreadEvent(ScanThreadEvent::Error,"Device is not available", "Bluetooth");
            QApplication::postEvent( m_parent, se );
            return;
        }
        hci_devinfo(dev_id,&di);
        ba2str(&di.bdaddr, addr);
        str.sprintf("HCI0 %s",addr);
        se = new ScanThreadEvent(ScanThreadEvent::HCIAddr,str);
        QApplication::postEvent( m_parent, se );
        num_rsp = hci_inquiry(dev_id, length, num_rsp, NULL, &info, flags);
        if (num_rsp < 0) {
            se = new ScanThreadEvent(ScanThreadEvent::Error,"Inquiry failed", "Bluetooth");
            QApplication::postEvent( m_parent, se );
            return;
        }

        dd = hci_open_dev(dev_id);
        if (dd < 0) {
            se = new ScanThreadEvent(ScanThreadEvent::Error,"HCI device open failed", "Bluetooth");
            QApplication::postEvent( m_parent, se );
            free(info);
            return;
        }
        if (num_rsp){
            se = new ScanThreadEvent(ScanThreadEvent::Clear);
            QApplication::postEvent( m_parent, se );
        }
        str.sprintf("Found %i Devices",num_rsp);
        se = new ScanThreadEvent(ScanThreadEvent::Message,str);
        QApplication::postEvent( m_parent, se );
        for (i = 0; i < num_rsp; i++) {
            memset(name, 0, sizeof(name));
            if (hci_read_remote_name(dd, &(info+i)->bdaddr, sizeof(name), name, 100000) < 0)
                            strcpy(name, "n/a");
            ba2str(&(info+i)->bdaddr, addr);
            str.sprintf("%s (%s)",name,addr);
            se = new ScanThreadEvent(ScanThreadEvent::BAddr,str,addr);
            QApplication::postEvent( m_parent, se );
            str.sprintf("Fetched info %i ",num_rsp);
            se = new ScanThreadEvent(ScanThreadEvent::Message,str);
            QApplication::postEvent( m_parent, se );
        }
        se = new ScanThreadEvent(ScanThreadEvent::Message,"Done");
        QApplication::postEvent( m_parent, se );
        free(info);
        sleep(3);
        se = new ScanThreadEvent(ScanThreadEvent::Message,"");
        QApplication::postEvent( m_parent, se );

}

k2sendPrefPagePlayer::k2sendPrefPagePlayer(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);
    new QLabel(i18n("NYI"), this);
}



#include "pref.moc"
