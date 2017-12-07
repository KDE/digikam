/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
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

#ifndef PRESENTATION_AUDIO_PAGE_H
#define PRESENTATION_AUDIO_PAGE_H

// Qt includes

#include <QTime>
#include <QMutex>
#include <QUrl>
#include <QDialog>

// Local includes

#include "ui_presentation_audiopage.h"
#include "presentationaudiowidget.h"
#include "presentationaudiolist.h"

namespace Digikam
{

class PresentationAudioWidget;
class PresentationContainer;

class SoundtrackPreview : public QDialog
{

public :

    SoundtrackPreview(QWidget* const, const QList<QUrl>&, PresentationContainer* const);
    ~SoundtrackPreview();

private :

    PresentationAudioWidget* m_playbackWidget;
};

// ----------------------------------------------------------------------

class PresentationAudioPage : public QWidget, public Ui::PresentationAudioPage
{
    Q_OBJECT

public:

    PresentationAudioPage(QWidget* const parent, PresentationContainer* const sharedData);
    ~PresentationAudioPage();

    void readSettings();
    void saveSettings();

private:

    void addItems(const QList<QUrl>& fileList);
    void updateTracksNumber();
    void updateFileList();
    void compareTimes();

private Q_SLOTS:

    void slotAddDropItems(const QList<QUrl>& filesUrl);
    void slotSoundFilesButtonAdd();
    void slotSoundFilesButtonDelete();
    void slotSoundFilesButtonUp();
    void slotSoundFilesButtonDown();
    void slotSoundFilesButtonLoad();
    void slotSoundFilesButtonSave();
    void slotSoundFilesButtonReset();
    void slotSoundFilesSelected(int);
    void slotPreviewButtonClicked();
    void slotImageTotalTimeChanged(const QTime&);
    void slotAddNewTime(const QUrl&, const QTime&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PRESENTATION_AUDIO_PAGE_H
