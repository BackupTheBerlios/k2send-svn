
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

// $Id: k2sendconsole.cpp 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $


#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>

#include "k2sendconsole.h"
#include "k2sendwidget.h"
#include "k2sendplayer.h"



K2sendConsole::K2sendConsole(k2sendWidget *p)
{
    m_parent = p;
}

K2sendConsole::~K2sendConsole()
{
    if (running()){
        terminate();
        wait();
    }
}

void K2sendConsole::run()
{
    K2sendStatusEvent * se;
    fd_set            readfds;
    struct timeval    tv;
    int rc;
    while(running()){
        QFileInfo  info(tty_dev);
        if (!info.isReadable()){
            QString msg = QString("Cannot read from " + tty_dev) ;
            se = new K2sendStatusEvent(K2sendStatusEvent::EventMessage,msg,2000);
            QApplication::postEvent( m_parent, se );
            kdDebug(200010) << "K2sendConsole::run terminate" << endl;
/*            terminate();
            wait();*/
            return;

        } else {
            char buffer[128];
            int fd = open( tty_dev.latin1(), O_RDWR | O_NOCTTY | O_NDELAY );
            if (!fd) {
                QString msg = QString("Cannot open " + tty_dev) ;
                se = new K2sendStatusEvent(K2sendStatusEvent::EventMessage,msg,2000);
                QApplication::postEvent(m_parent, se );
                kdDebug(200010) << "K2sendConsole::run terminate" << endl;
/*                terminate();
                wait();*/
                return;
            }
            fcntl( fd, F_SETFL, 0 );
            struct termios options;
            /*Get the current options for the port*/
            tcgetattr(fd, &options);
            /*Set the Baud rates to 115200*/
            cfsetispeed(&options, B115200);
            cfsetospeed(&options, B115200);
            /*Enable received and set local mode*/
            options.c_cflag |= (CLOCAL | CREAD);
            /*Set new options for port*/
            tcsetattr(fd, TCSANOW, &options);
            /*Set data bits*/
            options.c_cflag &= ~CSIZE;     /*Mask the character size bits*/
            options.c_cflag |= CS8;          /*Select 8 data bits*/
            /*Set RAW input*/
            options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
            /*Set Raw output*/
            options.c_oflag &= ~OPOST;
            /*Set timeout to 1 sec*/
            options.c_cc[VMIN] = 0;
            options.c_cc[VTIME] = 10;
            int len;
            while (running()){
                FD_ZERO(&readfds);
                FD_SET(fd, &readfds );
                tv.tv_sec=0;
                tv.tv_usec=0;
                rc = select(fd+1, &readfds, NULL, NULL, &tv);
                if (rc>0){
                    if (FD_ISSET(fd, &readfds)) {
                        len = read(fd,buffer,128);
                        buffer[len] = 0;
                        if (len && buffer[0] != '\n'){
                            se = new K2sendStatusEvent(K2sendStatusEvent::EventConsole,buffer);
                            QApplication::postEvent( m_parent, se );
                         }
                    }
                } else if(!rc){
                    this->msleep(100);
                } else {
                    kdDebug(200010) << "K2sendConsole::run select error " << tty_dev << endl;
                    this->sleep(1);
                }

             }
             close(fd);
        }
    }
}

void K2sendConsole::setTty(QString & str){
    tty_dev = str ;
    kdDebug(200010) << "K2sendConsole::setTty " << tty_dev << endl;
}

void K2sendConsole::restart(){

    if (running()){
        kdDebug(200010) << "K2sendConsole::restart stop thread " << tty_dev << endl;
        terminate();
        wait();
    }
    kdDebug(200010) << "K2sendConsole::restart start thread " << tty_dev << endl;
    start();

}

void K2sendConsole::stop(){

    if (running()){
        kdDebug(200010) << "K2sendConsole::stop " << tty_dev << endl;
        terminate();
        wait();
    }
}

