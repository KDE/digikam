/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PRESENTATION_AUDIO_LIST_H
#define PRESENTATION_AUDIO_LIST_H

// Qt includes

#include <QTime>
#include <QWidget>
#include <QString>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QUrl>

// QtAV includes

#include <QtAV/AVError.h>

namespace Digikam
{

class PresentationAudioListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:

    PresentationAudioListItem(QListWidget* const parent, const QUrl& url);
    ~PresentationAudioListItem();

    QUrl    url()       const;
    QString artist()    const;
    QString title()     const;
    QTime   totalTime() const;
    void    setName(const QString& text);

Q_SIGNALS:

    void signalTotalTimeReady(const QUrl&, const QTime&);

private Q_SLOTS:

    void slotMediaStateChanged(QtAV::MediaStatus);
    void slotPlayerError(const QtAV::AVError&);

private:

    void showErrorDialog(const QString& err);

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------

class PresentationAudioList : public QListWidget
{
    Q_OBJECT

public:

    explicit PresentationAudioList(QWidget* const parent = 0);

public:

    QList<QUrl> fileUrls();

Q_SIGNALS:

    void signalAddedDropItems(const QList<QUrl>& filesUrl);

protected:

    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
};

} // namespace Digikam

#endif // PRESENTATION_AUDIO_LIST_H
