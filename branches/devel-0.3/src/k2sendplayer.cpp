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

// $Id: k2sendplayer.cpp 2 2004-06-19 09:38:31Z optixx $
//
// $HeadURL$
//
// $LastChangedBy: optixx $


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include <qstring.h>
#include <qtimer.h>
#include <qfile.h>
#include <qptrqueue.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>

#include <taglib.h>
#include <tag.h>
#include <tstring.h>
#include <mpegproperties.h>
#include <mpegfile.h>

#include "k2sendplayer.h"
#include "k2sendwidget.h"




K2sendPlayer::K2sendPlayer (k2sendWidget * p, int vol, int filt):
loud_filt (filt),
volume (vol),
m_parent (p)
{
    is_connected = FALSE;
    start_ticks = last_ticks = getTime ();
}


K2sendPlayer::~K2sendPlayer ()
{
    if (running ()) {
        terminate ();
        wait ();
    }
    if (is_connected)
        blueClose ();
}

void
K2sendPlayer::addCommand (K2sendPlayerCommand * cmd)
{
    command_mutex.lock ();
    command_queue.enqueue (cmd);
    command_mutex.unlock ();
    command_cond.wakeOne ();
    if (cmd)
        kdDebug (200010) << "K2sendPlayer::addCommand cmd=" << cmd->command () << endl;
}

K2sendPlayerCommand *
K2sendPlayer::command ()
{
    K2sendPlayerCommand *cmd = 0;
    command_mutex.lock ();
    cmd = command_queue.dequeue ();
    command_mutex.unlock ();
    if (cmd)
        kdDebug (200010) << "K2sendPlayer::command cmd=" << cmd->command () << endl;
    return cmd;
}

void
K2sendPlayer::addFile (QString * file)
{
    if (!file)
        return;
    file_mutex.lock ();
    file_queue.enqueue (file);
    file_mutex.unlock ();
    file_cond.wakeOne ();
    kdDebug (200010) << "K2sendPlayer::addFile file=" << *file << endl;
}

QString *
K2sendPlayer::file ()
{
    kdDebug (200010) << "K2sendPlayer::file get file from queue" << endl;
    QString *file = 0;
    file_mutex.lock ();
    file = file_queue.dequeue ();
    file_mutex.unlock ();
    if (file)
        kdDebug (200010) << "K2sendPlayer::file file=" << *file << endl;
    return file;
}

void
K2sendPlayer::run ()
{
    QString *filename;
    K2sendPlayerCommand *cmd = 0;
    K2sendStatusEvent *se = 0;
    kdDebug (200010) << "K2sendPlayer::run" << endl;

    while (TRUE) {
        if (!cmd)
            cmd = command ();
        if (cmd) {
            kdDebug (200010) << "K2sendPlayer::run cmd->value " << cmd->command () << endl;

            switch (cmd->command ()) {
            case K2sendPlayerCommand::Skip:
            case K2sendPlayerCommand::Play:

                filename = file ();

                if (filename) {
                    kdDebug (200010) << "K2sendPlayer::run got filename" << *filename << endl;
                    if (!connected ())
                        blueOpen ();

                    if (connected ()) {
                        se = new K2sendStatusEvent (K2sendStatusEvent::EventPlay, 0, 0);
                        QApplication::postEvent (m_parent, se);
                        bluePlay (filename);
                    }
                    if (filename)
                        delete filename;
                    filename = 0;
                    if (cmd)
                        delete cmd;
                    cmd = 0;

                }
                else {
                    kdDebug (200010) << "K2sendPlayer::run request new file " << endl;
                    se = new K2sendStatusEvent (K2sendStatusEvent::EventEnqueue, 0, 0);
                    //kdDebug(200010) << "post 1" << endl;
                    QApplication::postEvent (m_parent, se);
                    file_cond.wait (1000);
                    kdDebug (200010) << "K2sendPlayer::run wake up " << endl;

                }
                break;
            case K2sendPlayerCommand::Stop:
                if (connected ())
                    blueClose ();
                delete cmd;
                cmd = 0;
                se = new K2sendStatusEvent (K2sendStatusEvent::EventStop);
                QApplication::postEvent (m_parent, se);
                break;
            case K2sendPlayerCommand::Volume:
            case K2sendPlayerCommand::Loudness:
            case K2sendPlayerCommand::Length:
                se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, "Not available", 2000);
                //kdDebug(200010) << "post 2" << endl;
                QApplication::postEvent (m_parent, se);

                delete cmd;
                cmd = 0;
                break;

            default:
                break;
            }

        }
        else {
            kdDebug (200010) << "K2sendPlayer::run no cmd going to sleep" << endl;

            command_cond.wait ();
        }
    }
}



void
K2sendPlayer::setAddr (QString & str)
{

    baddr = str;
    kdDebug (200010) << "K2sendPlayer::setAddr " << baddr << endl;
}

bool
K2sendPlayer::blueOpen ()
{
    struct sockaddr_l2 rem_addr, loc_addr;
    struct l2cap_options opts;
    //bdaddr_t bdaddr;
    int opt;
    char buffer[128];
    QString msg;
    kdDebug (200010) << "K2sendPlayer::blueOpen called" << endl;
    is_connected = FALSE;
    if (baddr.isEmpty ()) {
        msg = QString ("No address");
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 3" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 4" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    msg = QString ("Try to connect " + baddr);
    K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 30000);
    //kdDebug(200010) << "post 5" << endl;
    QApplication::postEvent (m_parent, se);

    /* create a bluetooth socket */
    if ((blue_sock = socket (PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0) {
        msg = QString ("Can't create socket");
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 6" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 7" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    kdDebug (200010) << "K2sendPlayer::blueOpen got socket" << endl;

    /* setup control structures */
    memset (&loc_addr, 0, sizeof (loc_addr));
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_psm = htobs (MP3_PSM);
    if (bind (blue_sock, (struct sockaddr *) &loc_addr, sizeof (loc_addr)) < 0) {
        msg = QString ("Can't bind socket");
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        QApplication::postEvent (m_parent, se);
        //kdDebug(200010) << "post 8" << endl;
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 9" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    kdDebug (200010) << "K2sendPlayer::blueOpen bind done" << endl;

    memset (&rem_addr, 0, sizeof (rem_addr));
    rem_addr.l2_family = AF_BLUETOOTH;
    baswap (&rem_addr.l2_bdaddr, strtoba (baddr.latin1 ()));
    rem_addr.l2_psm = htobs (MP3_PSM);
    if (connect (blue_sock, (struct sockaddr *) &rem_addr, sizeof (rem_addr)) < 0) {
        msg = QString ("Can't connect.");
        close (blue_sock);
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 10" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 11" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    kdDebug (200010) << "K2sendPlayer::blueOpen connect ok" << endl;

    opt = sizeof (opts);
    if (getsockopt (blue_sock, SOL_L2CAP, L2CAP_OPTIONS, &opts, (socklen_t *) & opt) < 0) {
        msg = QString ("Can't get L2CAP options. ");
        close (blue_sock);
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 12" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 13" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }

    /* use omtu for output buffer size */
    buffer_size = opts.omtu;

    /* ask for firmware version */
    buffer[0] = K2sendPlayerCommand::Fwrev;
    if (write (blue_sock, buffer, 1) != 1) {
        msg = QString ("Failed to write firmware request");
        close (blue_sock);
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 14" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 15" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    kdDebug (200010) << "K2sendPlayer::blueOpen req fw done" << endl;

    if (read (blue_sock, buffer, sizeof (buffer)) < 0) {
        msg = QString ("Failed to read firmware reply");
        close (blue_sock);
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 16" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 17" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    kdDebug (200010) << "K2sendPlayer::blueOpen got response" << endl;

    if (buffer[0] != K2sendPlayerCommand::Fwrev) {
        msg = QString ("Unexpected firware reply");
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
        //kdDebug(200010) << "post 17" << endl;
        QApplication::postEvent (m_parent, se);
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 18" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    is_connected = TRUE;
    msg = QString ("Connected");
    se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
    //kdDebug(200010) << "post 19" << endl;
    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventAddr, baddr);
    //kdDebug(200010) << "post 20" << endl;
    QApplication::postEvent (m_parent, se);
    return TRUE;
}


void
K2sendPlayer::blueClose ()
{
    close (blue_sock);
    is_connected = FALSE;
    QString msg;
    msg = QString ("Disconnected " + baddr);
    K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
    //kdDebug(200010) << "post 21" << endl;
    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventTitle, "");
    //kdDebug(200010) << "post 22" << endl;
    QApplication::postEvent (m_parent, se);
    clearStatus ();
    se = new K2sendStatusEvent (K2sendStatusEvent::EventAddr, "00:00:00:00:00:00");
    //kdDebug(200010) << "post 23" << endl;
    QApplication::postEvent (m_parent, se);

}


bool
K2sendPlayer::setReset (int hard)
{
    char buffer[2];
    if (!is_connected)
        return FALSE;
    kdDebug (200010) << "K2sendPlayer::setReset hard=" << hard << endl;
    buffer[0] = K2sendPlayerCommand::Reset;
    buffer[1] = hard;
    if (write (blue_sock, buffer, 2) != 2) {
        kdDebug (200010) << "K2sendPlayer::reset failed " << endl;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

/* set volume */
bool
K2sendPlayer::setVolume (int volume, bool do_update)
{
    char buffer[3];
    if (!is_connected)
        return FALSE;
    kdDebug (200010) << "K2sendPlayer::setVolume left=" << volume << endl;
    buffer[0] = K2sendPlayerCommand::Volume;
    buffer[1] = 100 - volume;
    buffer[2] = 100 - volume;
    if (write (blue_sock, buffer, 3) != 3) {
        kdDebug (200010) << "K2sendPlayer::setVolume failed " << endl;
        return FALSE;
    }
    else {
        if (do_update) {
            K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventVolume, NULL, 100 - volume);
            //kdDebug(200010) << "post 24" << endl;
            QApplication::postEvent (m_parent, se);
        }
        return TRUE;
    }
}

bool
K2sendPlayer::setLoudness (int filt)
{
    char buffer[2];
    if (!is_connected)
        return FALSE;

    kdDebug (200010) << "K2sendPlayer::loudness filt=" << filt << endl;
    buffer[0] = K2sendPlayerCommand::Loudness;
    buffer[1] = filt;
    if (write (blue_sock, buffer, 2) != 2) {
        kdDebug (200010) << "K2sendPlayer::loudness failed " << endl;
        return FALSE;
    }
    else {
        QString str;
        str.sprintf ("Loud: %i", filt);
        K2sendStatusEvent *se = new K2sendStatusEvent (K2sendStatusEvent::EventLoudness, str);
        //kdDebug(200010) << "post 25" << endl;
        QApplication::postEvent (m_parent, se);
        return TRUE;
    }
}

unsigned long long
K2sendPlayer::getTime (void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return ((tv.tv_sec * 1000ll) + (tv.tv_usec / 1000ll));
}

void
K2sendPlayer::updateStatus (int bytes_sent, int current_bytes_sent, int total, int play_length)
{

    K2sendStatusEvent *se;
    QString str_rate;
    QString str_time;
    long long unsigned diff;
    int rate;
    float progress;
    float t;
    last_ticks = getTime ();
    diff = getTime () - start_ticks;
    if (diff)
        rate = 8 * current_bytes_sent / diff;
    else
        rate = 0;
    progress = (float) (100 * bytes_sent) / total;

    se = new K2sendStatusEvent (K2sendStatusEvent::EventProgress, NULL, (int) progress);
    //kdDebug(200010) << "post 26" << endl;

    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventLength, NULL, (int) progress);
    //kdDebug(200010) << "post 27" << endl;
    QApplication::postEvent (m_parent, se);

    str_rate.sprintf ("%i kbit/s", rate);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventRate, str_rate);
    //kdDebug(200010) << "post 28" << endl;
    QApplication::postEvent (m_parent, se);

    t = (float) (play_length / 100) * progress;
    str_time.sprintf ("%02i:%02i", (int) t / 60, (int) t % 60);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventTime, str_time);
    //kdDebug(200010) << "l:"<< play_length << "p:" << progress <<"t:" << t <<  " " <<  (int)t/60 << " " << (int)t%60 << endl;
    QApplication::postEvent (m_parent, se);
}

void
K2sendPlayer::clearStatus ()
{
    K2sendStatusEvent *se;
    se = new K2sendStatusEvent (K2sendStatusEvent::EventProgress, NULL, 0);
    //kdDebug(200010) << "post 30" << endl;
    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventLength, NULL, 0);
    //kdDebug(200010) << "post 31" << endl;
    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventRate, "0 kbit/s");
    //kdDebug(200010) << "post 32" << endl;
    QApplication::postEvent (m_parent, se);
    se = new K2sendStatusEvent (K2sendStatusEvent::EventTime, "00:00");
    //kdDebug(200010) << "post 33" << endl;
    QApplication::postEvent (m_parent, se);
}

bool
K2sendPlayer::bluePlay (QString * filename)
{

    char buffer[buffer_size];
    int bytes_done;
    int file_len;
    int bytes_sent;
    int current_bytes_sent;
    int rc;
    int play_length;
    bool stop = FALSE;
    unsigned long long now;
    fd_set rfds, wfds;
    struct timeval tv;
    K2sendPlayerCommand *cmd = 0;
    K2sendPlayerCommand *new_cmd = 0;
    K2sendStatusEvent *se = 0;

    TagLib::MPEG::File mp3file (filename->latin1 ());
    play_length = mp3file.audioProperties ()->length ();
    QFile file (*filename);
    kdDebug (200010) << "K2sendPlayer::bluePlay name=" << *filename << " lenght=" << play_length << endl;
    setReset (0);
    setLoudness (loud_filt);
    setVolume (100 - volume);

    if (!file.open (IO_ReadOnly)) {
        QString msg = QString ("Error opening file ");
        se = new K2sendStatusEvent (K2sendStatusEvent::EventError, msg);
        //kdDebug(200010) << "post 34" << endl;
        QApplication::postEvent (m_parent, se);
        return FALSE;
    }
    se = new K2sendStatusEvent (K2sendStatusEvent::EventTitle, *filename);
    //kdDebug(200010) << "post 35" << endl;
    QApplication::postEvent (m_parent, se);

    file_len = file.size ();
    current_bytes_sent = bytes_sent = 0;
    last_ticks = start_ticks = getTime ();

    do {
        /* check if we can send(write) or receive(read) data */
        FD_ZERO (&rfds);
        FD_SET (blue_sock, &rfds);
        FD_ZERO (&wfds);
        FD_SET (blue_sock, &wfds);

        tv.tv_sec = 3;
        tv.tv_usec = 0;

        rc = select (FD_SETSIZE, &rfds, &wfds, NULL, &tv);
        if (rc < 1) {
            kdDebug (200010) << "K2sendPlayer::bluePlay select()" << endl;
            QString msg = QString ("Communication error");
            se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
            //kdDebug(200010) << "post 36" << endl;
            QApplication::postEvent (m_parent, se);
            blueClose ();
            return FALSE;
        }


        /* are we allowed to write? */
        if (FD_ISSET (blue_sock, &wfds)) {
            buffer[0] = K2sendPlayerCommand::Data;

            if (!(bytes_done = file.readBlock (buffer + 1, sizeof (buffer) - 1))) {
                if (file.atEnd ()) {
                    kdDebug (200010) << "K2sendPlayer::bluePlay Error reading file " << endl;
                    QString msg = QString ("Error reading file");
                    se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
                    //kdDebug(200010) << "post 37" << endl;
                    QApplication::postEvent (m_parent, se);

                }
            }
            else {
                if (write (blue_sock, buffer, bytes_done + 1) != bytes_done + 1) {
                    kdDebug (200010) << "K2sendPlayer::bluePlay Error sending" << endl;
                    QString msg = QString ("Error sending data");
                    se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
                    //kdDebug(200010) << "post 38" << endl;
                    QApplication::postEvent (m_parent, se);
                }
            }
            bytes_sent += bytes_done;
            current_bytes_sent += bytes_done;
        }
        if (FD_ISSET (blue_sock, &rfds)) {
            rc = read (blue_sock, buffer, sizeof (buffer));
            kdDebug (200010) << "K2sendPlayer::bluePlay  got data " << rc << " " << buffer[0] << " " << buffer[1] <<
                endl;
            /* handle commands */
            switch (buffer[0]) {
            case K2sendPlayerCommand::Buttons:
                if ((buffer[1] & 1) == 1) {
                    volume += 10;
                    if (volume > 100)
                        volume = 100;
                    //setVolume(100 - volume,TRUE);
                    kdDebug (200010) << "got cmd volume" << endl;
                    new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Volume, volume);
                    addCommand (new_cmd);
                }
                if (buffer[1] & 2) {
                    se = new K2sendStatusEvent (K2sendStatusEvent::EventSkip, NULL, 0);
                    //kdDebug(200010) << "post 39" << endl;
                    kdDebug (200010) << "got cmd skip" << endl;
                    QApplication::postEvent (m_parent, se);
                    new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Skip, 0);
                    addCommand (new_cmd);
                }
                if (buffer[1] & 4) {
                    volume -= 10;
                    if (volume < 0)
                        volume = 0;
                    //setVolume(100 - volume,TRUE);
                    kdDebug (200010) << "got cmd volume" << endl;
                    new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Volume, volume);
                    addCommand (new_cmd);
                }
                if (buffer[1] & 8) {
                    se = new K2sendStatusEvent (K2sendStatusEvent::EventError, "Battery low", 2000);
                    kdDebug (200010) << "battery low" << endl;
                    QApplication::postEvent (m_parent, se);
                    new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Stop, 0);
                    addCommand (new_cmd);
                }
                if (buffer[1] & 16) {
                    new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Loudness, 0);
                    kdDebug (200010) << "got cmd loudness" << endl;
                    addCommand (new_cmd);
                }
                break;
            default:
                QString msg = QString ("Unknown command from BlueMP3");
                se = new K2sendStatusEvent (K2sendStatusEvent::EventMessage, msg, 2000);
                //kdDebug(200010) << "post 41" << endl;
                QApplication::postEvent (m_parent, se);
                break;
            }
        }
        now = getTime ();

        if ((last_ticks) < now - 250) {
            updateStatus (bytes_sent, current_bytes_sent, file_len, play_length);
            last_ticks = now;
        }
        cmd = 0;
        cmd = command ();
        if (cmd) {
            switch (cmd->command ()) {
            case K2sendPlayerCommand::Loudness:
                if (++loud_filt > 8)
                    loud_filt = 0;
                setLoudness (loud_filt);
                delete cmd;
                cmd = 0;
                break;
            case K2sendPlayerCommand::Volume:
                volume = cmd->value ();
                setVolume (100 - volume, FALSE);
                delete cmd;
                cmd = 0;
                break;
            case K2sendPlayerCommand::Play:
                bytes_done = 0;
                stop = TRUE;
                addCommand (cmd);
                break;
            case K2sendPlayerCommand::Stop:
                stop = TRUE;
                /* forward to global handling */
                addCommand (cmd);
                break;
            case K2sendPlayerCommand::Skip:
                stop = TRUE;
                addCommand (cmd);
                break;

            case K2sendPlayerCommand::Length:
                start_ticks = getTime ();
                current_bytes_sent = 0;
                bytes_sent = (file_len / 100) * cmd->value ();
                file.at (bytes_sent);
                delete cmd;
                cmd = 0;
                break;
            default:
                break;
            }
        }

    } while (bytes_sent != file_len && !stop);
    file.close ();

    se = new K2sendStatusEvent (K2sendStatusEvent::EventTitle, "");
    //kdDebug(200010) << "post 42" << endl;
    QApplication::postEvent (m_parent, se);

    this->clearStatus ();
    /* keep playing */
    if (!stop) {
        kdDebug (200010) << "K2sendPlayer::bluePlay keep playing" << endl;
        se = new K2sendStatusEvent (K2sendStatusEvent::EventSkip, 0, 0);
        //kdDebug(200010) << "post 43" << endl;
        QApplication::postEvent (m_parent, se);
        new_cmd = new K2sendPlayerCommand (K2sendPlayerCommand::Play, 0);
        addCommand (new_cmd);
    }
    kdDebug (200010) << "K2sendPlayer::bluePlay done" << endl;
    return TRUE;
}
