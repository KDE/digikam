/* ============================================================
 * File  : cameraguiclient.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-17
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CAMERAGUICLIENT_H
#define CAMERAGUICLIENT_H

#include <qobject.h>
#include <guiclient.h>

class KAction;
class KActionMenu;

class CameraGUIClient : public QObject, public Digikam::GUIClient
{
    Q_OBJECT

public:
    
    CameraGUIClient(QWidget *parent);

    virtual QStringList guiDefinition() const;
    
signals:

    void signalDownloadSelected();
    void signalDownloadAll();
    void signalDeleteSelected();
    void signalDeleteAll();
    void signalCancel();
    void signalExit();

    void signalSelectAll();
    void signalSelectNone();
    void signalSelectInvert();

private slots:

    void slotHelp();
    void slotAboutApp();
    void slotAboutKDE();

private:

    KAction     *m_cancelAction;
    KActionMenu *m_downloadAction;
    KActionMenu *m_deleteAction;
    KAction     *m_downloadSelAction;
    KAction     *m_deleteSelAction;
    
    friend class CameraUI;
};

#endif /* CAMERAGUICLIENT_H */
