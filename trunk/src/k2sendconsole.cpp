
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
    while(running()){
        QFileInfo  info(tty_dev);
        if (!info.isReadable()){
            QString msg = QString("Cannot read from " + tty_dev) ;
            se = new K2sendStatusEvent(K2sendStatusEvent::EventMessage,msg,2000);
            QApplication::postEvent( m_parent, se );
            terminate();
            wait();

        } else {
            char buffer[128];
            int fd = open( tty_dev.latin1(), O_RDWR | O_NOCTTY | O_NDELAY );
            if (!fd) {
                QString msg = QString("Cannot open " + tty_dev) ;
                se = new K2sendStatusEvent(K2sendStatusEvent::EventMessage,msg,2000);
                 QApplication::postEvent(m_parent, se );
                terminate();
                wait();
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
                    len = read(fd,buffer,128);
                    buffer[len] = 0;
                    if (len && buffer[0] != '\n'){
                        se = new K2sendStatusEvent(K2sendStatusEvent::EventConsole,buffer);
                        QApplication::postEvent( m_parent, se );
                    } else {
                        this->msleep(100);
                    }
             }
             close(fd);
        }
    }
}

void K2sendConsole::setTty(QString & str){
    tty_dev = str ;
    restart();
}

void K2sendConsole::restart(){

    if (running()){
        kdDebug(200010) << "K2sendConsole::restart restarting thread " << tty_dev << endl;
        terminate();
        wait();
        start();
    }
}

