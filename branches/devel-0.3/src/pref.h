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

#ifndef _K2SENDPREF_H_
#define _K2SENDPREF_H_

#include <kdialogbase.h>
#include <qframe.h>
#include <qgrid.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qthread.h>

class k2sendPrefPagePlayer;
class KListBox;
class KPushButton;
class KLineEdit;
class ScanThread;
class KConfig;
class K2sendWidget;

class k2sendPreferences:public KDialogBase
{
  Q_OBJECT public:
    k2sendPreferences (KConfig * c);
    void customEvent (QCustomEvent * e);

  private:
      k2sendPrefPagePlayer * m_pagePlayer;
    KListBox *m_list_device;
    KPushButton *m_button_scan;
    KLineEdit *m_edit_tty;
    QLabel *m_scan_label;
    QLabel *m_hci_label;
    ScanThread *m_scan_thread;
    KConfig *m_config;

    private slots:virtual void slotScan ();
    virtual void slotApply ();
    virtual void slotOk ();


};


class k2sendPrefPagePlayer:public QFrame
{
  Q_OBJECT public:
    k2sendPrefPagePlayer (QWidget * parent = 0);
};

class AddressItem:public QListBoxText
{
  public:
    AddressItem (const QString & text = QString::null, const QString & a = QString::null)
    : QListBoxText (text), s (a)
    {
    }
    QString addr () const
    {
        return s;
    }
  private:
      QString s;
};


class ScanThread:public QThread
{
  public:
    ScanThread (QWidget * p):m_parent (p)
    {
    }
     ~ScanThread ();
    virtual void run ();
  private:
    QWidget * m_parent;
};


class ScanThreadEvent:public QCustomEvent
{
  public:
    ScanThreadEvent (int type, QString string1 = NULL, QString string2 = NULL)
    : QCustomEvent (type), s1 (string1), s2 (string2)
    {
    }

    QString string1 () const
    {
        return s1;
    }
    QString string2 () const
    {
        return s2;
    }
    typedef enum Type
    { Error = 66000,
        BAddr,
        Message,
        Clear,
        HCIAddr,
    }
    Type;
  private:
    QString s1;
    QString s2;
};

#endif // _K2SENDPREF_H_
