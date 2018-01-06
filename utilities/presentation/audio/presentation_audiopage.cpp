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

#include "presentation_audiopage.h"

// Qt includes

#include <QPointer>
#include <QTime>
#include <QIcon>
#include <QMessageBox>
#include <QDialogButtonBox>

// Local includes

#include "presentationaudiowidget.h"
#include "presentation_mainpage.h"
#include "presentationcontainer.h"
#include "digikam_debug.h"
#include "dfiledialog.h"

namespace Digikam
{

SoundtrackPreview::SoundtrackPreview(QWidget* const parent, const QList<QUrl>& urls, PresentationContainer* const sharedData)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18n("Soundtrack preview"));

    m_playbackWidget                  = new PresentationAudioWidget(this, urls, sharedData);
    QDialogButtonBox* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);

    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->addWidget(m_playbackWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

SoundtrackPreview::~SoundtrackPreview()
{
}

// ------------------------------------------------------------------------------------

class PresentationAudioPage::Private
{
public:

    Private()
    {
        tracksTime = 0;
        sharedData = 0;
        soundItems = 0;
        timeMutex  = 0;
    }

    QList<QUrl>             urlList;
    PresentationContainer*        sharedData;
    QTime                   totalTime;
    QTime                   imageTime;
    QMap<QUrl, QTime>*      tracksTime;
    QMap<QUrl, PresentationAudioListItem*>* soundItems;
    QMutex*                 timeMutex;
};

PresentationAudioPage::PresentationAudioPage(QWidget* const parent, PresentationContainer* const sharedData)
    : QWidget(parent),
      d(new Private)
{
    setupUi(this);

    d->sharedData = sharedData;
    d->totalTime  = QTime(0, 0, 0);
    d->imageTime  = QTime(0, 0, 0);
    d->tracksTime = new QMap<QUrl, QTime>();
    d->soundItems = new QMap<QUrl, PresentationAudioListItem*>();
    d->timeMutex  = new QMutex();

    m_soundtrackTimeLabel->setText(d->totalTime.toString());
    m_previewButton->setEnabled(false);

    m_rememberSoundtrack->setToolTip(i18n("If set, the soundtrack for the current album "
                                          "will be saved and restored automatically on the next startup."));

    // --------------------------------------------------------

    m_SoundFilesButtonUp->setIcon(QIcon::fromTheme(QString::fromLatin1("go-up")));
    m_SoundFilesButtonDown->setIcon(QIcon::fromTheme(QString::fromLatin1("go-down")));
    m_SoundFilesButtonAdd->setIcon(QIcon::fromTheme(QString::fromLatin1("list-add")));
    m_SoundFilesButtonDelete->setIcon(QIcon::fromTheme(QString::fromLatin1("list-remove")));
    m_SoundFilesButtonLoad->setIcon(QIcon::fromTheme(QString::fromLatin1("document-open")));
    m_SoundFilesButtonSave->setIcon(QIcon::fromTheme(QString::fromLatin1("document-save")));
    m_SoundFilesButtonReset->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-clear")));

    m_SoundFilesButtonUp->setText(QString());
    m_SoundFilesButtonDown->setText(QString());
    m_SoundFilesButtonAdd->setText(QString());
    m_SoundFilesButtonDelete->setText(QString());
    m_SoundFilesButtonLoad->setText(QString());
    m_SoundFilesButtonSave->setText(QString());
    m_SoundFilesButtonReset->setText(QString());

    m_SoundFilesButtonUp->setToolTip(i18n("Move the selected track up in the playlist."));
    m_SoundFilesButtonDown->setToolTip(i18n("Move the selected track down in the playlist."));
    m_SoundFilesButtonAdd->setToolTip(i18n("Add new tracks to the playlist."));
    m_SoundFilesButtonDelete->setToolTip(i18n("Delete the selected track from the playlist."));
    m_SoundFilesButtonLoad->setToolTip(i18n("Load playlist from a file."));
    m_SoundFilesButtonSave->setToolTip(i18n("Save playlist to a file."));
    m_SoundFilesButtonReset->setToolTip(i18n("Clear the playlist."));

    // --------------------------------------------------------

    connect( m_SoundFilesListBox, SIGNAL(currentRowChanged(int)),
             this, SLOT(slotSoundFilesSelected(int)) );

    connect( m_SoundFilesListBox, SIGNAL(signalAddedDropItems(QList<QUrl>)),
             this, SLOT(slotAddDropItems(QList<QUrl>)));

    connect( m_SoundFilesButtonAdd, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonAdd()) );

    connect( m_SoundFilesButtonDelete, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonDelete()) );

    connect( m_SoundFilesButtonUp, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonUp()) );

    connect( m_SoundFilesButtonDown, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonDown()) );

    connect( m_SoundFilesButtonLoad, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonLoad()) );

    connect( m_SoundFilesButtonSave, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonSave()) );

    connect( m_SoundFilesButtonReset, SIGNAL(clicked()),
             this, SLOT(slotSoundFilesButtonReset()) );

    connect( m_previewButton, SIGNAL(clicked()),
             this, SLOT(slotPreviewButtonClicked()));

    connect( d->sharedData->mainPage, SIGNAL(signalTotalTimeChanged(QTime)),
             this, SLOT(slotImageTotalTimeChanged(QTime)));
}

PresentationAudioPage::~PresentationAudioPage()
{
    delete d->tracksTime;
    delete d->soundItems;
    delete d->timeMutex;
    delete d;
}

void PresentationAudioPage::readSettings()
{
    m_rememberSoundtrack->setChecked(d->sharedData->soundtrackRememberPlaylist);
    m_loopCheckBox->setChecked(d->sharedData->soundtrackLoop);

    connect( d->sharedData->mainPage, SIGNAL(signalTotalTimeChanged(QTime)),
             this, SLOT(slotImageTotalTimeChanged(QTime)) );

    // if tracks are already set in d->sharedData, add them now
    if (!d->sharedData->soundtrackUrls.isEmpty())
        addItems(d->sharedData->soundtrackUrls);

    updateFileList();
    updateTracksNumber();
}

void PresentationAudioPage::saveSettings()
{
    d->sharedData->soundtrackRememberPlaylist = m_rememberSoundtrack->isChecked();
    d->sharedData->soundtrackLoop             = m_loopCheckBox->isChecked();
    d->sharedData->soundtrackUrls             = d->urlList;
}

void PresentationAudioPage::addItems(const QList<QUrl>& fileList)
{
    if (fileList.isEmpty())
        return;

    QList<QUrl> Files = fileList;

    for (QList<QUrl>::ConstIterator it = Files.constBegin(); it != Files.constEnd(); ++it)
    {
        QUrl currentFile              = *it;
        d->sharedData->soundtrackPath = currentFile;
        PresentationAudioListItem* const item         = new PresentationAudioListItem(m_SoundFilesListBox, currentFile);
        item->setName(currentFile.fileName());
        m_SoundFilesListBox->insertItem(m_SoundFilesListBox->count() - 1, item);

        d->soundItems->insert(currentFile, item);

        connect(d->soundItems->value(currentFile), SIGNAL(signalTotalTimeReady(QUrl,QTime)),
                this, SLOT(slotAddNewTime(QUrl,QTime)));

        d->urlList.append(currentFile);
    }

    m_SoundFilesListBox->setCurrentItem(m_SoundFilesListBox->item(m_SoundFilesListBox->count() - 1)) ;

    slotSoundFilesSelected(m_SoundFilesListBox->currentRow());
    m_SoundFilesListBox->scrollToItem(m_SoundFilesListBox->currentItem());
    m_previewButton->setEnabled(true);
}

void PresentationAudioPage::updateTracksNumber()
{
    QTime displayTime(0, 0, 0);
    int number = m_SoundFilesListBox->count();

    if ( number > 0 )
    {
        displayTime = displayTime.addMSecs(1000 * (number - 1));
        

        for (QMap<QUrl, QTime>::iterator it = d->tracksTime->begin(); it != d->tracksTime->end(); ++it)
        {
            int hours = it.value().hour()   + displayTime.hour();
            int mins  = it.value().minute() + displayTime.minute();
            int secs  = it.value().second() + displayTime.second();

            /* QTime doesn't get a overflow value in input. They need
             * to be cut down to size.
             */

            mins        = mins + (int)(secs / 60);
            secs        = secs % 60;
            hours       = hours + (int)(mins / 60);
            displayTime = QTime(hours, mins, secs);
        }
    }

    m_timeLabel->setText(i18ncp("number of tracks and running time", "1 track [%2]", "%1 tracks [%2]", number, displayTime.toString()));

    m_soundtrackTimeLabel->setText(displayTime.toString());

    d->totalTime = displayTime;

    compareTimes();
}

void PresentationAudioPage::updateFileList()
{
    d->urlList = m_SoundFilesListBox->fileUrls();

    m_SoundFilesButtonUp->setEnabled(!d->urlList.isEmpty());
    m_SoundFilesButtonDown->setEnabled(!d->urlList.isEmpty());
    m_SoundFilesButtonDelete->setEnabled(!d->urlList.isEmpty());
    m_SoundFilesButtonSave->setEnabled(!d->urlList.isEmpty());
    m_SoundFilesButtonReset->setEnabled(!d->urlList.isEmpty());

    d->sharedData->soundtrackPlayListNeedsUpdate = true;
}

void PresentationAudioPage::compareTimes()
{
    QFont statusBarFont = m_statusBarLabel->font();

    if ( d->imageTime > d->totalTime )
    {
        m_statusBarLabel->setText(i18n("Slide time is greater than soundtrack time. Suggestion: add more sound files."));


        QPalette paletteStatusBar = m_statusBarLabel->palette();
        paletteStatusBar.setColor(QPalette::WindowText, Qt::red);
        m_statusBarLabel->setPalette(paletteStatusBar);

        QPalette paletteTimeLabel = m_soundtrackTimeLabel->palette();
        paletteTimeLabel.setColor(QPalette::WindowText, Qt::red);
        m_soundtrackTimeLabel->setPalette(paletteTimeLabel);

        statusBarFont.setItalic(true);
    }
    else
    {
        m_statusBarLabel->setText(QString::fromLatin1(""));

        QPalette paletteStatusBar = m_statusBarLabel->palette();
        paletteStatusBar.setColor(QPalette::WindowText, Qt::red);
        m_statusBarLabel->setPalette(paletteStatusBar);

        QPalette paletteTimeLabel = m_soundtrackTimeLabel->palette();

        if ( d->imageTime < d->totalTime )
            paletteTimeLabel.setColor(QPalette::WindowText, Qt::black);
        else
            paletteTimeLabel.setColor(QPalette::WindowText, Qt::green);

        m_soundtrackTimeLabel->setPalette(paletteTimeLabel);

        statusBarFont.setItalic(false);
    }

    m_statusBarLabel->setFont(statusBarFont);
}

void PresentationAudioPage::slotAddNewTime(const QUrl& url, const QTime& trackTime)
{
    d->timeMutex->lock();
    d->tracksTime->insert(url, trackTime);
    updateTracksNumber();
    d->timeMutex->unlock();
}

void PresentationAudioPage::slotSoundFilesSelected( int row )
{
    QListWidgetItem* const item = m_SoundFilesListBox->item(row);

    if ( !item || m_SoundFilesListBox->count() == 0 )
    {
        return;
    }
}

void PresentationAudioPage::slotAddDropItems(const QList<QUrl>& filesUrl)
{
    if (!filesUrl.isEmpty())
    {
        addItems(filesUrl);
        updateFileList();
    }
}

void PresentationAudioPage::slotSoundFilesButtonAdd()
{
    QPointer<DFileDialog> dlg = new DFileDialog(this,
                                                i18n("Select sound files"),
                                                d->sharedData->soundtrackPath.adjusted(QUrl::RemoveFilename).toLocalFile());

    QStringList atm;
    atm << QLatin1String("audio/mp3");
    atm << QLatin1String("audio/wav");
    atm << QLatin1String("audio/ogg");
    atm << QLatin1String("audio/flac");
    dlg->setMimeTypeFilters(atm);
    dlg->setAcceptMode(DFileDialog::AcceptOpen);
    dlg->setFileMode(DFileDialog::ExistingFiles);
    dlg->exec();

    QList<QUrl> urls = dlg->selectedUrls();

    if (!urls.isEmpty())
    {
        addItems(urls);
        updateFileList();
    }

    delete dlg;
}

void PresentationAudioPage::slotSoundFilesButtonDelete()
{
    int Index = m_SoundFilesListBox->currentRow();

    if( Index < 0 )
       return;

    PresentationAudioListItem* const pitem = static_cast<PresentationAudioListItem*>(m_SoundFilesListBox->takeItem(Index));
    d->urlList.removeAll(pitem->url());
    d->soundItems->remove(pitem->url());
    d->timeMutex->lock();
    d->tracksTime->remove(pitem->url());
    updateTracksNumber();
    d->timeMutex->unlock();
    delete pitem;
    slotSoundFilesSelected(m_SoundFilesListBox->currentRow());

    if (m_SoundFilesListBox->count() == 0)
        m_previewButton->setEnabled(false);

    updateFileList();
}

void PresentationAudioPage::slotSoundFilesButtonUp()
{
    int Cpt = 0;

    for (int i = 0 ; i < m_SoundFilesListBox->count() ; ++i)
    {
        if (m_SoundFilesListBox->currentRow() == i)
            ++Cpt;
    }

    if (Cpt == 0)
    {
        return;
    }

    if (Cpt > 1)
    {
        QMessageBox::critical(this, QString(), i18n("You can only move image files up one at a time."));
        return;
    }

    unsigned int Index = m_SoundFilesListBox->currentRow();

    if (Index == 0)
    {
        return;
    }

    PresentationAudioListItem* const pitem = static_cast<PresentationAudioListItem*>(m_SoundFilesListBox->takeItem(Index));

    m_SoundFilesListBox->insertItem(Index - 1, pitem);
    m_SoundFilesListBox->setCurrentItem(pitem);

    updateFileList();
}

void PresentationAudioPage::slotSoundFilesButtonDown()
{
    int Cpt = 0;

    for (int i = 0 ; i < m_SoundFilesListBox->count() ; ++i)
    {
        if (m_SoundFilesListBox->currentRow() == i)
            ++Cpt;
    }

    if (Cpt == 0)
    {
        return;
    }

    if (Cpt > 1)
    {
        QMessageBox::critical(this, QString(), i18n("You can only move files down one at a time."));
        return;
    }

    int Index = m_SoundFilesListBox->currentRow();

    if (Index == m_SoundFilesListBox->count())
    {
        return;
    }

    PresentationAudioListItem* const pitem = static_cast<PresentationAudioListItem*>(m_SoundFilesListBox->takeItem(Index));

    m_SoundFilesListBox->insertItem(Index + 1, pitem);
    m_SoundFilesListBox->setCurrentItem(pitem);

    updateFileList();
}

void PresentationAudioPage::slotSoundFilesButtonLoad()
{
    QPointer<DFileDialog> dlg = new DFileDialog(this, i18n("Load playlist"),
                                                QString(), i18n("Playlist (*.m3u)"));
    dlg->setAcceptMode(DFileDialog::AcceptOpen);
    dlg->setFileMode(DFileDialog::ExistingFile);

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    QString filename = dlg->selectedFiles().isEmpty() ? QString() : dlg->selectedFiles().at(0);

    if (!filename.isEmpty())
    {
        QFile file(filename);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            QList<QUrl> playlistFiles;

            while (!in.atEnd())
            {
                QString line = in.readLine();

                // we ignore the extended information of the m3u playlist file
                if (line.startsWith(QLatin1Char('#')) || line.isEmpty())
                    continue;

                QUrl fUrl = QUrl::fromLocalFile(line);

                if (fUrl.isValid() && fUrl.isLocalFile())
                {
                    playlistFiles << fUrl;
                }
            }

            if (!playlistFiles.isEmpty())
            {
                m_SoundFilesListBox->clear();
                addItems(playlistFiles);
                updateFileList();
            }
        }
    }

    delete dlg;
}

void PresentationAudioPage::slotSoundFilesButtonSave()
{
    QPointer<DFileDialog> dlg = new DFileDialog(this, i18n("Save playlist"),
                                                QString(), i18n("Playlist (*.m3u)"));
    dlg->setAcceptMode(DFileDialog::AcceptSave);
    dlg->setFileMode(DFileDialog::AnyFile);

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    QString filename = dlg->selectedFiles().isEmpty() ? QString() : dlg->selectedFiles().at(0);

    if (!filename.isEmpty())
    {
        QFile file(filename);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            QList<QUrl> playlistFiles = m_SoundFilesListBox->fileUrls();

            for (int i = 0; i < playlistFiles.count(); ++i)
            {
                QUrl fUrl(playlistFiles.at(i));

                if (fUrl.isValid() && fUrl.isLocalFile())
                {
                    out << fUrl.toLocalFile() << endl;
                }
            }

            file.close();
        }
    }

    delete dlg;
}

void PresentationAudioPage::slotSoundFilesButtonReset()
{
    m_SoundFilesListBox->clear();
    updateFileList();
}

void PresentationAudioPage::slotPreviewButtonClicked()
{
    QList<QUrl> urlList;

    for (int i = 0 ; i < m_SoundFilesListBox->count() ; ++i)
    {
        PresentationAudioListItem* const pitem = dynamic_cast<PresentationAudioListItem*>(m_SoundFilesListBox->item(i));

        if (pitem)
        {
            QString path           = pitem->url().toLocalFile();

            if (!QFile::exists(path))
            {
                QMessageBox::critical(this, QString(), i18n("Cannot access file \"%1\". Please check the path is correct.", path));
                return;
            }

            urlList << pitem->url();
        }
    }

    if ( urlList.isEmpty() )
    {
        QMessageBox::critical(this, QString(), i18n("Cannot create a preview of an empty file list."));
        return;
    }

    // Update PresentationContainer from interface
    saveSettings();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Tracks : " << urlList;

    QPointer<SoundtrackPreview> preview = new SoundtrackPreview(this, urlList, d->sharedData);
    preview->exec();

    delete preview;
    return;
}

void PresentationAudioPage::slotImageTotalTimeChanged( const QTime& imageTotalTime )
{
    d->imageTime = imageTotalTime;
    m_slideTimeLabel->setText(imageTotalTime.toString());
    compareTimes();
}

} // namespace Digikam
