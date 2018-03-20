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

#ifndef PRESENTATION_AUDIO_WIDGET_H
#define PRESENTATION_AUDIO_WIDGET_H

// Qt includes

#include <QUrl>
#include <QKeyEvent>

// QtAV includes

#include <QtAV/AVError.h>
#include <QtAV/AVPlayer.h>

// Local includes

#include "ui_presentationaudiowidget.h"

namespace Digikam
{

class PresentationContainer;

class PresentationAudioWidget : public QWidget, public Ui::PresentationAudioWidget
{
    Q_OBJECT

public:

    PresentationAudioWidget(QWidget* const, const QList<QUrl>&, PresentationContainer* const);
    ~PresentationAudioWidget();

    void enqueue(const QList<QUrl>&);
    bool canHide() const;
    bool isPaused() const;
    void setPaused(bool);
    void keyPressEvent(QKeyEvent*);

Q_SIGNALS:

    void signalPlay();
    void signalPause();

private Q_SLOTS:

    void slotPlay();
    void slotStop();
    void slotPrev();
    void slotNext();
    void slotTimeUpdaterTimeout();
    void slotError();
    void slotSetVolume(int);
    void slotMediaStateChanged(QtAV::MediaStatus);
    void slotPlayerStateChanged(QtAV::AVPlayer::State);
    void slotPlayerError(const QtAV::AVError&);

private:

    void checkSkip();
    void setZeroTime();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PRESENTATION_AUDIO_WIDGET_H
