
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

#ifndef _K2SENDPLAYER_H_
#define _K2SENDPLAYER_H_

#include <qthread.h>
#include <qevent.h>
#include <qmutex.h>
#include <qptrqueue.h>
#include <qwaitcondition.h>

#define MP3_PSM        11


class k2sendWidget;

class K2sendStatusEvent : public QCustomEvent
{
    public:
        K2sendStatusEvent( int type,  QString string = NULL, int val=0 )
            : QCustomEvent( type ), s ( string ), v(val) {}

        QString string() const { return s; }
        int     value() const { return v; }
        typedef enum Type { EventAddr=65000,
                            EventTitle,
                            EventMessage,
                            EventError,
                            EventTime,
                            EventRate,
                            EventProgress ,
                            EventVolume,
                            EventLoudness,
                            EventLength,
                            EventEnqueue,
                            EventConsole,
                            EventSkip,
                            } Type;
    private:
        QString s;
        int v;
};

class K2sendPlayerCommand
{
    public:
        K2sendPlayerCommand ( int c, int v ) : cmd(c) , val(v) {}
        int value() const { return val; }
        int command() const { return cmd; }

        typedef enum Cmd {  Data=0,
                            Reset,
                            Volume,
                            Fwrev,
                            Buttons,
                            Loudness,
                            Play,
                            Skip,
                            Stop,
                            Length,
                            } Cmd;

    private:
        int cmd;
        int val;
};


class K2sendPlayer : public QThread {

    public:
        K2sendPlayer(k2sendWidget *p, int vol = 50, int filt = 0);
        ~K2sendPlayer();
        bool connected() { return is_connected; }
        void addCommand(K2sendPlayerCommand * cmd);
        void addFile(QString * file);
        void setAddr(QString & str);
        QString & addr()  { return baddr; }
        int currentLoundness() const { return loud_filt; }
        int currentVolume() const { return volume; }

        virtual void run();

    private:
        bool setReset(int hard);
        bool setLoudness(int filt);
        bool setVolume(int vol, bool  do_update = TRUE);
        K2sendPlayerCommand * command();
        QString *  file();
        bool blueOpen();
        bool bluePlay(QString * file);
        void blueClose();
        void updateStatus(int sent,int current, int total,int play_lenght);
        void clearStatus();
        unsigned long long getTime();
        unsigned long long last_ticks;
        unsigned long long start_ticks;

        bool      is_connected;
        int       blue_sock;
        int       buffer_size;
        int       chunk_size;
        int       loud_filt;
        int       volume;
        QString   baddr;
        k2sendWidget  * m_parent;
        QPtrQueue<K2sendPlayerCommand> command_queue;
        QMutex     command_mutex;
        QPtrQueue<QString> file_queue;
        QMutex     file_mutex;
        QWaitCondition file_cond;
        QWaitCondition command_cond;

};
#endif
