/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011 by Lukas Krejci <krejci.l at centrum dot cz>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RAJCE_WIDGET_H
#define RAJCE_WIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "rajcesession.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"
#include "dprogresswdg.h"

class QLabel;
class QRadioButton;
class QSpinBox;
class QComboBox;
class QPushButton;

namespace Digikam
{

class RajceTalker;

class RajceWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit RajceWidget(DInfoInterface* const iface, QWidget* const parent);

Q_SIGNALS:

    void loginStatusChanged(bool loggedIn);

public Q_SLOTS:

    void updateLabels(const QString& name = QString(), const QString& url = QString()) Q_DECL_OVERRIDE;
    void reactivate();

    void startUpload();
    void cancelUpload();

    void writeSettings();
    void readSettings();

private Q_SLOTS:

    void changeUserClicked();

    void progressStarted(unsigned);
    void progressFinished(unsigned);
    void progressChange(unsigned, unsigned percent);

    void loadAlbums();
    void createAlbum();
    void closeAlbum();

    void uploadNext();

    void startUploadAfterAlbumOpened();
    void selectedAlbumChanged(const QString&);

private:

    void _setEnabled(bool);

private:

    QLabel*                    m_headerLbl;
    QLabel*                    m_userNameLbl;
    QLabel*                    m_userName;

    QSpinBox*                  m_dimensionSpB;
    QSpinBox*                  m_imageQualitySpB;

    QComboBox*                 m_albumsCoB;

    QPushButton*               m_newAlbumBtn;
    QPushButton*               m_reloadAlbumsBtn;
    QPushButton*               m_changeUserBtn;

    DInfoInterface*            m_iface;
    DImagesList*               m_imgList;
    DProgressWdg*              m_progressBar;

    RajceTalker*               m_talker;

    QList<QString>             m_uploadQueue;
    QList<QString>::Iterator   m_currentUploadImage;

    bool                       m_uploadingPhotos;
    bool                       m_lastLoggedInState;
    QString                    m_currentAlbumName;
};

} // namespace Digikam

#endif // RAJCE_WIDGET_H
