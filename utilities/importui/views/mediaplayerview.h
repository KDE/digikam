/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-08-22
 * Description : A view to embed Phonon media player in import interface.
 *
 * Copyright (C) 2012 Islam Wazery <wazery at ubuntu dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMPORTMEDIAPLAYERVIEW_H
#define IMPORTMEDIAPLAYERVIEW_H

// Qt includes

#include <QStackedWidget>

// KDE includes

#include <kurl.h>
#include <phonon/mediaobject.h>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

class ImportStackedView;

class ImportMediaPlayerMouseClickFilter : public QObject
{
    Q_OBJECT

public:

    ImportMediaPlayerMouseClickFilter(QObject* const parent);

protected:

    bool eventFilter(QObject* obj, QEvent* event);

private:

    QObject* m_parent;
};

// --------------------------------------------------------

class ImportMediaPlayerView : public QStackedWidget
{
    Q_OBJECT

public:

    ImportMediaPlayerView(ImportStackedView* const parent);
    ~ImportMediaPlayerView();

    void setCamItemInfo(const CamItemInfo& info     = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next     = CamItemInfo());
    void escapePreview();

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalEscapePreview();

public Q_SLOTS:

    void slotEscapePressed();

private Q_SLOTS:

    void slotPlayerFinished();
    void slotThemeChanged();
    void slotPlayerstateChanged(Phonon::State newState, Phonon::State oldState);

private:

    int  previewMode();
    void setPreviewMode(int mode);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // IMPORTMEDIAPLAYERVIEW_H
