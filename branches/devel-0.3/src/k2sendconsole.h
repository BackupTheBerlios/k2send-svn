
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

// $Id: k2sendconsole.h 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $

#ifndef _K2SENDCONSOLE_H_
#define _K2SENDCONSOLE_H_

#include <qthread.h>
#include <qevent.h>
#include <qwaitcondition.h>

class k2sendWidget;

class K2sendConsole:public QThread
{
  public:
    K2sendConsole (k2sendWidget * p);
    ~K2sendConsole ();
    void setTty (QString & str);
      QString & tty ()
    {
        return tty_dev;
    }
    virtual void run ();
    void restart ();
    void stop ();
    void clear ();

  private:
    QString tty_dev;
    k2sendWidget *m_parent;
    QWaitCondition cond_dev;
};
#endif
