/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#include "expoblendingdlg.h"

// C ANSI includes

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QPointer>
#include <QCloseEvent>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QGroupBox>
#include <QWindow>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "expoblendingthread.h"
#include "bracketstack.h"
#include "enfusebinary.h"
#include "enfusesettings.h"
#include "enfusestack.h"
#include "dmessagebox.h"
#include "dpreviewmanager.h"
#include "dsavesettingswidget.h"
#include "expoblendingmanager.h"
#include "dfileoperations.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class ExpoBlendingDlg::Private
{
public:

    explicit Private()
      : templateFileName(0),
        previewWidget(0),
        enfuseSettingsBox(0),
        saveSettingsBox(0),
        bracketStack(0),
        enfuseStack(0),
        mngr(0),
        firstImageDisplayed(false),
        buttonBox(0),
        previewButton(0),
        startButton(0),
        propagateReject(true)
    {
    }

    QString               inputFileName;
    QString               output;

    QLineEdit*            templateFileName;

    DPreviewManager*      previewWidget;

    EnfuseSettingsWidget* enfuseSettingsBox;

    DSaveSettingsWidget*  saveSettingsBox;

    BracketStackList*     bracketStack;
    EnfuseStackList*      enfuseStack;

    ExpoBlendingManager*  mngr;

    bool                  firstImageDisplayed;

    QDialogButtonBox*     buttonBox;
    QPushButton*          previewButton;
    QPushButton*          startButton;

    bool                  propagateReject;
};

ExpoBlendingDlg::ExpoBlendingDlg(ExpoBlendingManager* const mngr, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    d->mngr = mngr;

    setModal(false);
    setWindowTitle(i18n("Exposure Blending"));

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->buttonBox                     = new QDialogButtonBox(QDialogButtonBox::Close, this);
    d->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    d->startButton                   = new QPushButton(this);
    d->startButton->setText(i18nc("@action:button", "&Save"));
    d->startButton->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->startButton->setToolTip(i18nc("@info:tooltip", "Process and save selected items."));
    d->buttonBox->addButton(d->startButton, QDialogButtonBox::ActionRole);

    d->previewButton                 = new QPushButton(this);
    d->previewButton->setText(i18nc("@action:button", "&Preview"));
    d->previewButton->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->previewButton->setToolTip(i18nc("@info:tooltip", "Process a preview of bracketed images stack with current settings."));
    d->buttonBox->addButton(d->previewButton, QDialogButtonBox::ActionRole);

    QPushButton* const defaultButton = new QPushButton(this);
    defaultButton->setText(i18nc("@action:button", "&Default"));
    defaultButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    defaultButton->setToolTip(i18nc("@info:tooltip", "Revert current settings to default values."));
    d->buttonBox->addButton(defaultButton, QDialogButtonBox::ResetRole);

    // ---------------------------------------------------------------

    d->previewWidget                 = new DPreviewManager(this);
    d->previewWidget->setButtonText(i18nc("@action:button", "Details..."));

    // ---------------------------------------------------------------

    QScrollArea* const rightColumn   = new QScrollArea(this);
    QWidget* const rightPanel        = new QWidget(rightColumn->viewport());
    rightColumn->setWidget(rightPanel);
    rightColumn->setWidgetResizable(true);
    rightColumn->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout* const panel         = new QVBoxLayout(rightPanel);

    d->bracketStack                  = new BracketStackList(rightPanel);
    panel->addWidget(d->bracketStack, 1);

    // ---------------------------------------------------------------

    QGroupBox* const enfuse          = new QGroupBox(rightPanel);
    enfuse->setTitle(i18n("Enfuse Settings"));
    QVBoxLayout* const elay          = new QVBoxLayout(enfuse);
    enfuse->setLayout(elay);

    d->enfuseSettingsBox             = new EnfuseSettingsWidget(enfuse);
    elay->addWidget(d->enfuseSettingsBox);

    panel->addWidget(enfuse, 1);

    // ---------------------------------------------------------------

    QGroupBox* const save            = new QGroupBox(rightPanel);
    save->setTitle(i18n("Save Settings"));
    QVBoxLayout* const slay = new QVBoxLayout(save);
    save->setLayout(slay);

    d->saveSettingsBox               = new DSaveSettingsWidget(save);
    slay->addWidget(d->saveSettingsBox);

    QHBoxLayout* const hbox          = new QHBoxLayout(save);

    QLabel* const customLabel        = new QLabel(save);
    customLabel->setText(i18nc("@label:textbox", "File Name Template: "));
    hbox->addWidget(customLabel);

    d->templateFileName              = new QLineEdit(save);
    d->templateFileName->setClearButtonEnabled(true);
    hbox->addWidget(d->templateFileName);

    d->saveSettingsBox->setCustomSettingsWidget(d->saveSettingsBox);
    slay->addLayout(hbox);

    panel->addWidget(save, 1);

    // ---------------------------------------------------------------

    d->enfuseStack = new EnfuseStackList(rightPanel);
    panel->addWidget(d->enfuseStack, 1);

    rightPanel->setLayout(panel);
    panel->setContentsMargins(QMargins());

    // ---------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->previewWidget, 0, 0, 3, 1);
    grid->addWidget(rightColumn,      0, 1, 3, 1);
    grid->addWidget(d->buttonBox,     4, 0, 1, 2);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(1, 5);
    setLayout(grid);

    // ---------------------------------------------------------------

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    connect(defaultButton, SIGNAL(clicked()),
            this, SLOT(slotDefault()));

    connect(d->startButton, SIGNAL(clicked()),
            this, SLOT(slotProcess()));

    connect(d->previewButton, SIGNAL(clicked()),
            this, SLOT(slotPreview()));

    connect(d->buttonBox, &QDialogButtonBox::rejected,
            this, &ExpoBlendingDlg::slotCloseClicked);

    connect(d->mngr->thread(), SIGNAL(starting(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    connect(d->mngr->thread(), SIGNAL(finished(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    connect(d->bracketStack, SIGNAL(signalAddItems(const QList<QUrl>&)),
            this, SLOT(slotAddItems(const QList<QUrl>&)));

    connect(d->previewWidget, SIGNAL(signalButtonClicked()),
            this, SLOT(slotPreviewButtonClicked()));

    connect(d->enfuseStack, SIGNAL(signalItemClicked(QUrl)),
            this, SLOT(slotLoadProcessed(QUrl)));

    connect(d->templateFileName, SIGNAL(textChanged(QString)),
            this, SLOT(slotFileFormatChanged()));

    connect(d->saveSettingsBox, SIGNAL(signalSaveFormatChanged()),
            this, SLOT(slotFileFormatChanged()));

    // ---------------------------------------------------------------

    busy(false);
    readSettings();
    loadItems(d->mngr->itemsList());
}

ExpoBlendingDlg::~ExpoBlendingDlg()
{
    delete d;
}

void ExpoBlendingDlg::slotFinished()
{
    d->mngr->thread()->cancel();
    d->mngr->cleanUp();
    saveSettings();
}

void ExpoBlendingDlg::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void ExpoBlendingDlg::slotCancelClicked()
{
    d->mngr->thread()->cancel();
}

void ExpoBlendingDlg::slotFileFormatChanged()
{
    d->enfuseStack->setTemplateFileName(d->saveSettingsBox->fileFormat(), d->templateFileName->text());
}

void ExpoBlendingDlg::slotPreviewButtonClicked()
{
    DMessageBox::showInformationList(QMessageBox::Information,
                                     qApp->activeWindow(),
                                     qApp->applicationName(),
                                     i18nc("@title:window", "Enfuse Processing Messages"),
                                     d->output.split(QLatin1String("\n")));
}

void ExpoBlendingDlg::loadItems(const QList<QUrl>& urls)
{
    d->bracketStack->clear();
    d->bracketStack->addItems(urls);
}

void ExpoBlendingDlg::slotAddItems(const QList<QUrl>& urls)
{
    if (!urls.empty())
    {
        d->mngr->thread()->identifyFiles(urls);

        if (!d->mngr->thread()->isRunning())
            d->mngr->thread()->start();
    }
}

void ExpoBlendingDlg::slotLoadProcessed(const QUrl& url)
{
    d->mngr->thread()->loadProcessed(url);

    if (!d->mngr->thread()->isRunning())
        d->mngr->thread()->start();
}

void ExpoBlendingDlg::setIdentity(const QUrl& url, const QString& identity)
{
    BracketStackItem* const item = d->bracketStack->findItem(url);

    if (item)
        item->setExposure(identity);
}

void ExpoBlendingDlg::busy(bool val)
{
    d->enfuseSettingsBox->setEnabled(!val);
    d->saveSettingsBox->setEnabled(!val);
    d->bracketStack->setEnabled(!val);

    d->startButton->setEnabled(!val ? !d->enfuseStack->settingsList().isEmpty() : false);
    d->previewButton->setEnabled(!val);
    setRejectButtonMode(val ? QDialogButtonBox::Cancel : QDialogButtonBox::Close);

    if (val)
        d->previewWidget->setButtonVisible(false);
}

void ExpoBlendingDlg::slotDefault()
{
    d->enfuseSettingsBox->resetToDefault();
    d->saveSettingsBox->resetToDefault();
    d->templateFileName->setText(QLatin1String("enfuse"));
}

void ExpoBlendingDlg::readSettings()
{
    KConfig config;
    KConfigGroup group = config.group("ExpoBlending Settings");

    d->enfuseSettingsBox->readSettings(group);
    d->saveSettingsBox->readSettings(group);

    d->templateFileName->setText(group.readEntry("Template File Name", QString::fromLatin1("enfuse")));

    winId();
    KConfigGroup group2 = config.group("ExpoBlending Dialog");
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group2);
    resize(windowHandle()->size());
}

void ExpoBlendingDlg::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group("ExpoBlending Settings");

    d->enfuseSettingsBox->writeSettings(group);
    d->saveSettingsBox->writeSettings(group);

    group.writeEntry("Template File Name", d->templateFileName->text());

    KConfigGroup group2 = config.group("ExpoBlending Dialog");
    DXmlGuiWindow::saveWindowSize(windowHandle(), group2);
    config.sync();
}

void ExpoBlendingDlg::slotPreview()
{
    QList<QUrl> selectedUrl = d->bracketStack->urls();

    if (selectedUrl.isEmpty())
        return;

    ExpoBlendingItemUrlsMap map = d->mngr->preProcessedMap();
    QList<QUrl> preprocessedList;

    foreach(const QUrl& url, selectedUrl)
    {
        ExpoBlendingItemPreprocessedUrls preprocessedUrls = map.value(url);
        preprocessedList.append(preprocessedUrls.previewUrl);
    }

    EnfuseSettings settings = d->enfuseSettingsBox->settings();
    settings.inputUrls      = d->bracketStack->urls();
    settings.outputFormat   = d->saveSettingsBox->fileFormat();
    d->mngr->thread()->enfusePreview(preprocessedList, d->mngr->itemsList()[0], settings, d->mngr->enfuseBinary().path());

    if (!d->mngr->thread()->isRunning())
        d->mngr->thread()->start();
}

void ExpoBlendingDlg::slotProcess()
{
    QList<EnfuseSettings> list = d->enfuseStack->settingsList();

    if (list.isEmpty())
        return;

    ExpoBlendingItemUrlsMap map = d->mngr->preProcessedMap();
    QList<QUrl> preprocessedList;

    foreach(const EnfuseSettings& settings, list)
    {
        preprocessedList.clear();

        foreach(const QUrl& url, settings.inputUrls)
        {
            ExpoBlendingItemPreprocessedUrls preprocessedUrls = map.value(url);
            preprocessedList.append(preprocessedUrls.preprocessedUrl);
        }

        d->mngr->thread()->enfuseFinal(preprocessedList, d->mngr->itemsList()[0], settings, d->mngr->enfuseBinary().path());

        if (!d->mngr->thread()->isRunning())
            d->mngr->thread()->start();
    }
}

void ExpoBlendingDlg::saveItem(const QUrl& temp, const EnfuseSettings& settings)
{
    QUrl newUrl = QUrl::fromLocalFile(temp.adjusted(QUrl::RemoveFilename).toLocalFile() + settings.targetFileName);

    if (d->saveSettingsBox->conflictRule() != FileSaveConflictBox::OVERWRITE)
    {
        newUrl = DFileOperations::getUniqueFileUrl(newUrl);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Renaming " << temp << " to " << newUrl;

    if (!newUrl.isEmpty())
    {
        // remove newUrl file if it exist
        if (temp.toLocalFile() != newUrl.toLocalFile() && QFile::exists(temp.toLocalFile()) && QFile::exists(newUrl.toLocalFile()))
        {
            QFile::remove(newUrl.toLocalFile());
        }

        if (!QFile::rename(temp.toLocalFile(), newUrl.toLocalFile()))
        {
            QMessageBox::critical(this, QString(), i18n("Failed to save image to %1.", QDir::toNativeSeparators(newUrl.toLocalFile())));
            d->enfuseStack->setOnItem(settings.previewUrl, false);
            d->enfuseStack->processedItem(settings.previewUrl, false);
            return;
        }
        else
        {
            d->enfuseStack->removeItem(settings.previewUrl);
        }
    }

    if (d->enfuseStack->settingsList().isEmpty())
    {
        d->startButton->setEnabled(false);
        busy(false);
        d->previewWidget->setBusy(false);
    }
}

void ExpoBlendingDlg::slotExpoBlendingAction(const Digikam::ExpoBlendingActionData& ad)
{
    QString text;

    if (ad.starting)            // Something have been started...
    {
        switch (ad.action)
        {
            case(EXPOBLENDING_IDENTIFY):
            {
                break;
            }
            case(EXPOBLENDING_LOAD):
            {
                busy(true);
                break;
            }
            case(EXPOBLENDING_ENFUSEPREVIEW):
            {
                busy(true);
                d->previewWidget->setBusy(true, i18n("Processing preview of bracketed images..."));
                break;
            }
            case(EXPOBLENDING_ENFUSEFINAL):
            {
                busy(true);
                d->previewWidget->setBusy(true, i18n("Processing output of bracketed images..."));
                d->enfuseStack->processingItem(ad.enfuseSettings.previewUrl, true);
                break;
            }
            default:
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                break;
            }
        }
    }
    else
    {
        if (!ad.success)        // Something is failed...
        {
            switch (ad.action)
            {
                case(EXPOBLENDING_IDENTIFY):
                {
                    setIdentity(ad.inUrls[0], ad.message);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_LOAD):
                {
                    d->previewWidget->setText(i18n("Failed to load processed image."), Qt::red);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_ENFUSEPREVIEW):
                {
                    d->output = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewWidget->setButtonVisible(true);
                    d->previewWidget->setText(i18n("Failed to process preview of bracketed images."), Qt::red);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_ENFUSEFINAL):
                {
                    slotCancelClicked();
                    d->output = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewWidget->setButtonVisible(true);
                    d->previewWidget->setText(i18n("Failed to process output of bracketed images."), Qt::red);
                    d->enfuseStack->processingItem(ad.enfuseSettings.previewUrl, false);
                    d->enfuseStack->setOnItem(ad.enfuseSettings.previewUrl, false);
                    busy(false);
                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case(EXPOBLENDING_IDENTIFY):
                {
                    setIdentity(ad.inUrls[0], ad.message);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_LOAD):
                {
                    d->previewWidget->setImage(ad.image, !d->firstImageDisplayed);
                    d->firstImageDisplayed |= true;
                    d->enfuseStack->setThumbnail(ad.inUrls[0], ad.image);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_ENFUSEPREVIEW):
                {
                    d->enfuseStack->addItem(ad.outUrls[0], ad.enfuseSettings);
                    busy(false);
                    break;
                }
                case(EXPOBLENDING_ENFUSEFINAL):
                {
                    d->enfuseStack->processingItem(ad.enfuseSettings.previewUrl, false);
                    saveItem(ad.outUrls[0], ad.enfuseSettings);
                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                    break;
                }
            }
        }
    }
}

void ExpoBlendingDlg::setRejectButtonMode(QDialogButtonBox::StandardButton button)
{
    if (button == QDialogButtonBox::Close)
    {
        d->buttonBox->button(QDialogButtonBox::Close)->setText(i18n("Close"));
        d->buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QString::fromLatin1("window-close")));
        d->buttonBox->button(QDialogButtonBox::Close)->setToolTip(i18n("Close window"));
        d->propagateReject = true;
    }
    else if (button == QDialogButtonBox::Cancel)
    {
        d->buttonBox->button(QDialogButtonBox::Close)->setText(i18n("Cancel"));
        d->buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QString::fromLatin1("dialog-cancel")));
        d->buttonBox->button(QDialogButtonBox::Close)->setToolTip(i18n("Cancel current operation"));
        d->propagateReject = false;
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Unexpected button mode passed";
    }
}

void ExpoBlendingDlg::slotCloseClicked()
{
    if (d->propagateReject)
    {
        reject();
    }
    else
    {
        emit cancelClicked();
    }
}

} // namespace Digikam
