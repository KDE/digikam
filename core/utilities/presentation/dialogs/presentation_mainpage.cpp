/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
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

#define ICONSIZE 256

#include "presentation_mainpage.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QHeaderView>
#include <QPainter>
#include <QVBoxLayout>
#include <QUrl>
#include <QIcon>
#include <QMessageBox>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationcontainer.h"
#include "presentation_advpage.h"
#include "presentation_captionpage.h"
#include "thumbnailloadthread.h"
#include "dimageslist.h"
#include "dmetadata.h"
#include "presentationwidget.h"

#ifdef HAVE_OPENGL
#   include "presentationgl.h"
#   include "presentationkb.h"
#endif

namespace Digikam
{

class PresentationMainPage::Private
{

public:

    Private()
    {
        sharedData         = 0;
        imagesFilesListBox = 0;
    }

    PresentationContainer* sharedData;
    QTime                  totalTime;
    DImagesList*           imagesFilesListBox;
};

PresentationMainPage::PresentationMainPage(QWidget* const parent, PresentationContainer* const sharedData)
    : QWidget(parent),
      d(new Private)
{
    setupUi(this);

    d->sharedData = sharedData;

    // --------------------------------------------------------

    QVBoxLayout* const listBoxContainerLayout = new QVBoxLayout;
    d->imagesFilesListBox                     = new DImagesList(m_ImagesFilesListBoxContainer, 32);
    d->imagesFilesListBox->setObjectName(QLatin1String("Presentation ImagesList"));
    d->imagesFilesListBox->listView()->header()->hide();
    d->imagesFilesListBox->enableControlButtons(true);
    d->imagesFilesListBox->enableDragAndDrop(true);

    listBoxContainerLayout->addWidget(d->imagesFilesListBox);
    listBoxContainerLayout->setContentsMargins(QMargins());
    listBoxContainerLayout->setSpacing(0);
    m_ImagesFilesListBoxContainer->setLayout(listBoxContainerLayout);

    // --------------------------------------------------------

    m_previewLabel->setMinimumWidth(ICONSIZE);
    m_previewLabel->setMinimumHeight(ICONSIZE);

#ifdef HAVE_OPENGL
    m_openglCheckBox->setEnabled(true);
#else
    m_openglCheckBox->setEnabled(false);
#endif
}

PresentationMainPage::~PresentationMainPage()
{
    delete d;
}

void PresentationMainPage::readSettings()
{
#ifdef HAVE_OPENGL
    m_openglCheckBox->setChecked(d->sharedData->opengl);
#endif

    m_delaySpinBox->setValue(d->sharedData->delay);
    m_printNameCheckBox->setChecked(d->sharedData->printFileName);
    m_printProgressCheckBox->setChecked(d->sharedData->printProgress);
    m_printCommentsCheckBox->setChecked(d->sharedData->printFileComments);
    m_loopCheckBox->setChecked(d->sharedData->loop);
    m_shuffleCheckBox->setChecked(d->sharedData->shuffle);

    m_delaySpinBox->setValue(d->sharedData->useMilliseconds ? d->sharedData->delay
                                                            : d->sharedData->delay / 1000 );

    slotUseMillisecondsToggled();

    // --------------------------------------------------------

    setupConnections();
    slotOpenGLToggled();
    slotPrintCommentsToggled();
    slotEffectChanged();

    addItems(d->sharedData->urlList);
}

void PresentationMainPage::saveSettings()
{
#ifdef HAVE_OPENGL
    d->sharedData->opengl                = m_openglCheckBox->isChecked();
#endif

    d->sharedData->delay                 = d->sharedData->useMilliseconds ? m_delaySpinBox->value()
                                                                          : m_delaySpinBox->value() * 1000;

    d->sharedData->printFileName         = m_printNameCheckBox->isChecked();
    d->sharedData->printProgress         = m_printProgressCheckBox->isChecked();
    d->sharedData->printFileComments     = m_printCommentsCheckBox->isChecked();
    d->sharedData->loop                  = m_loopCheckBox->isChecked();
    d->sharedData->shuffle               = m_shuffleCheckBox->isChecked();

    if (!m_openglCheckBox->isChecked())
    {

        QString effect;
        QMap<QString, QString> effectNames = PresentationWidget::effectNamesI18N();
        QMap<QString, QString>::ConstIterator it;

        for (it = effectNames.constBegin(); it != effectNames.constEnd(); ++it)
        {
            if (it.value() == m_effectsComboBox->currentText())
            {
                effect = it.key();
                break;
            }
        }

        d->sharedData->effectName = effect;
    }
#ifdef HAVE_OPENGL
    else
    {
        QMap<QString, QString> effects;
        QMap<QString, QString> effectNames;
        QMap<QString, QString>::ConstIterator it;

        // Load slideshowgl effects
        effectNames = PresentationGL::effectNamesI18N();

        for (it = effectNames.constBegin(); it != effectNames.constEnd(); ++it)
        {
            effects.insert(it.key(), it.value());
        }

        // Load Ken Burns effect
        effectNames = PresentationKB::effectNamesI18N();

        for (it = effectNames.constBegin(); it != effectNames.constEnd(); ++it)
        {
            effects.insert(it.key(), it.value());
        }

        QString effect;

        for (it = effects.constBegin(); it != effects.constEnd(); ++it)
        {
            if ( it.value() == m_effectsComboBox->currentText())
            {
                effect = it.key();
                break;
            }
        }

        d->sharedData->effectNameGL = effect;
    }
#endif
}

void PresentationMainPage::showNumberImages()
{
    int numberOfImages = d->imagesFilesListBox->imageUrls().count();
    QTime totalDuration(0, 0, 0);

    int transitionDuration = 2000;

#ifdef HAVE_OPENGL
    if ( m_openglCheckBox->isChecked() )
        transitionDuration += 500;
#endif

    if (numberOfImages != 0)
    {
        if (d->sharedData->useMilliseconds)
        {
            totalDuration = totalDuration.addMSecs(numberOfImages * m_delaySpinBox->text().toInt());
        }
        else
        {
            totalDuration = totalDuration.addSecs(numberOfImages * m_delaySpinBox->text().toInt());
        }

        totalDuration = totalDuration.addMSecs((numberOfImages - 1) * transitionDuration);
    }

    d->totalTime = totalDuration;

    // Notify total time is changed
    emit signalTotalTimeChanged(d->totalTime);

    m_label6->setText(i18np("%1 image [%2]", "%1 images [%2]", numberOfImages, totalDuration.toString()));
}

void PresentationMainPage::loadEffectNames()
{
    m_effectsComboBox->clear();

    QMap<QString, QString> effectNames = PresentationWidget::effectNamesI18N();
    QStringList effects;

    QMap<QString, QString>::Iterator it;

    for (it = effectNames.begin(); it != effectNames.end(); ++it)
    {
        effects.append(it.value());
    }

    m_effectsComboBox->insertItems(0, effects);

    for (int i = 0; i < m_effectsComboBox->count(); ++i)
    {
        if (effectNames[d->sharedData->effectName] == m_effectsComboBox->itemText(i))
        {
            m_effectsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void PresentationMainPage::loadEffectNamesGL()
{
#ifdef HAVE_OPENGL
    m_effectsComboBox->clear();

    QStringList effects;
    QMap<QString, QString> effectNames;
    QMap<QString, QString>::Iterator it;

    // Load slideshowgl effects
    effectNames = PresentationGL::effectNamesI18N();

    // Add Ken Burns effect
    effectNames.unite(PresentationKB::effectNamesI18N());

    for (it = effectNames.begin(); it != effectNames.end(); ++it)
    {
        effects.append(it.value());
    }

    // Update GUI

    effects.sort();

    m_effectsComboBox->insertItems(0, effects);

    for (int i = 0; i < m_effectsComboBox->count(); ++i)
    {
        if (effectNames[d->sharedData->effectNameGL] == m_effectsComboBox->itemText(i))
        {
            m_effectsComboBox->setCurrentIndex(i);
            break;
        }
    }
#endif
}

bool PresentationMainPage::updateUrlList()
{
    d->sharedData->urlList.clear();
    QTreeWidgetItemIterator it(d->imagesFilesListBox->listView());

    while (*it)
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*it);

        if (!item)
            continue;

        if (!QFile::exists(item->url().toLocalFile()))
        {
            QMessageBox::critical(this, i18n("Error"), i18n("Cannot access file %1. Please check the path is correct.",
                                                            item->url().toLocalFile()));
            return false;
        }

        if (!d->sharedData->commentsMap.contains(item->url()))
        {
            DMetadata meta(item->url().toLocalFile());
            d->sharedData->commentsMap.insert(item->url(), meta.getImageComments()[QLatin1String("x-default")].caption);
        }

        d->sharedData->urlList.append(item->url());  // Input images files.
        ++it;
    }

    return true;
}

void PresentationMainPage::slotImagesFilesSelected(QTreeWidgetItem* item)
{
    if (!item || d->imagesFilesListBox->imageUrls().isEmpty())
    {
        m_previewLabel->setPixmap(QPixmap());
        m_label7->setText(QLatin1String(""));
        return;
    }

    DImagesListViewItem* const pitem = dynamic_cast<DImagesListViewItem*>(item);

    if (!pitem)
        return;

    connect(ThumbnailLoadThread::defaultThread(), SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    ThumbnailLoadThread::defaultThread()->find(ThumbnailIdentifier(pitem->url().toLocalFile()));

    QModelIndex index = d->imagesFilesListBox->listView()->currentIndex();

    if (index.isValid())
    {
        int rowindex = index.row();
        m_label7->setText(i18nc("Image number %1", "Image #%1", rowindex + 1));
    }
}

void PresentationMainPage::addItems(const QList<QUrl>& fileList)
{
    if (fileList.isEmpty())
        return;

    QList<QUrl> files = fileList;

    d->imagesFilesListBox->slotAddImages(files);
    slotImagesFilesSelected(d->imagesFilesListBox->listView()->currentItem());
}

void PresentationMainPage::slotOpenGLToggled()
{
    if (m_openglCheckBox->isChecked())
    {
        loadEffectNamesGL();
    }
    else
    {
        loadEffectNames();
    }

    showNumberImages();
    slotEffectChanged();
}

void PresentationMainPage::slotEffectChanged()
{
    bool isKB = m_effectsComboBox->currentText() == i18n("Ken Burns");

    m_printNameCheckBox->setEnabled(!isKB);
    m_printProgressCheckBox->setEnabled(!isKB);
    m_printCommentsCheckBox->setEnabled(!isKB);
#ifdef HAVE_OPENGL
    d->sharedData->advancedPage->m_openGlFullScale->setEnabled(!isKB && m_openglCheckBox->isChecked());
#endif
    d->sharedData->captionPage->setEnabled((!isKB) && m_printCommentsCheckBox->isChecked());
}

void PresentationMainPage::slotDelayChanged( int delay )
{
    d->sharedData->delay = d->sharedData->useMilliseconds ? delay : delay * 1000;
    showNumberImages();
}

void PresentationMainPage::slotUseMillisecondsToggled()
{
    int delay = d->sharedData->delay;

    if ( d->sharedData->useMilliseconds )
    {
        m_delayLabel->setText(i18n("Delay between images (ms):"));

        m_delaySpinBox->setRange(d->sharedData->delayMsMinValue, d->sharedData->delayMsMaxValue);
        m_delaySpinBox->setSingleStep(d->sharedData->delayMsLineStep);
    }
    else
    {
        m_delayLabel->setText(i18n("Delay between images (s):"));

        m_delaySpinBox->setRange(d->sharedData->delayMsMinValue / 100, d->sharedData->delayMsMaxValue / 1000  );
        m_delaySpinBox->setSingleStep(d->sharedData->delayMsLineStep / 100);
        delay /= 1000;
    }

    m_delaySpinBox->setValue(delay);
}

void PresentationMainPage::slotPortfolioDurationChanged(int)
{
    showNumberImages();
    emit signalTotalTimeChanged( d->totalTime );
}

void PresentationMainPage::slotThumbnail(const LoadingDescription& /*desc*/, const QPixmap& pix)
{
    if (pix.isNull())
    {
        m_previewLabel->setPixmap(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(ICONSIZE, QIcon::Disabled));
    }
    else
    {
        m_previewLabel->setPixmap(pix.scaled(ICONSIZE, ICONSIZE, Qt::KeepAspectRatio));
    }

    disconnect(ThumbnailLoadThread::defaultThread(), 0,
               this, 0);
}

void PresentationMainPage::slotPrintCommentsToggled()
{
    d->sharedData->printFileComments =  m_printCommentsCheckBox->isChecked();
    d->sharedData->captionPage->setEnabled(m_printCommentsCheckBox->isChecked());
}

void PresentationMainPage::slotImageListChanged()
{
    showNumberImages();
    slotImagesFilesSelected(d->imagesFilesListBox->listView()->currentItem());
}

void PresentationMainPage::setupConnections()
{
    connect(d->sharedData->advancedPage, SIGNAL(useMillisecondsToggled()),
            this, SLOT(slotUseMillisecondsToggled()));

    connect(m_printCommentsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotPrintCommentsToggled()));

    connect(m_openglCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotOpenGLToggled()));

    connect(m_delaySpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(slotDelayChanged(int)));

    connect(m_effectsComboBox, SIGNAL(activated(int)),
            this, SLOT(slotEffectChanged()));

    connect(d->imagesFilesListBox, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->imagesFilesListBox, SIGNAL(signalItemClicked(QTreeWidgetItem*)),
            this, SLOT(slotImagesFilesSelected(QTreeWidgetItem*)));
}

}  // namespace Digikam
