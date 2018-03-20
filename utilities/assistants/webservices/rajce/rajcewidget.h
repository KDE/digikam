/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QString>

// Local includes

#include "wssettingswidget.h"

namespace Digikam
{

class DInfoInterface;
   
class RajceWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit RajceWidget(DInfoInterface* const iface, QWidget* const parent);
    ~RajceWidget();

Q_SIGNALS:

    void signalLoginStatusChanged(bool loggedIn);

public Q_SLOTS:

    void slotStartUpload();
    
public:

    void reactivate();
    void cancelUpload();
    
    void writeSettings();
    void readSettings();

private Q_SLOTS:

    void slotChangeUserClicked();

    void slotProgressStarted(unsigned);
    void slotProgressFinished(unsigned);
    void slotProgressChanged(unsigned, unsigned percent);

    void slotLoadAlbums();
    void slotCreateAlbum();
    void slotCloseAlbum();

    void slotUploadNext();

    void slotStartUploadAfterAlbumOpened();
    void slotSelectedAlbumChanged(const QString&);

private:

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) Q_DECL_OVERRIDE;
    
    void setEnabledWidgets(bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RAJCE_WIDGET_H
