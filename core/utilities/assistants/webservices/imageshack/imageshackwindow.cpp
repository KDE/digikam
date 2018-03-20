/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "imageshackwindow.h"

// Qt includes

#include <QWindow>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_debug.h"
#include "imageshacksession.h"
#include "imageshackwidget_p.h"
#include "imageshacktalker.h"
#include "dprogresswdg.h"
#include "wslogindialog.h"
#include "imageshacknewalbumdlg.h"

namespace Digikam
{

class ImageShackWindow::Private
{
public:

    explicit Private()
    {
        imagesCount = 0;
        imagesTotal = 0;
        session     = 0;
        widget      = 0;
        talker      = 0;
        albumDlg    = 0;
        iface       = 0;
    }

    unsigned int           imagesCount;
    unsigned int           imagesTotal;
    QString                newAlbmTitle;

    QList<QUrl>            transferQueue;

    ImageShackSession*     session;
    ImageShackWidget*      widget;
    ImageShackTalker*      talker;

    ImageShackNewAlbumDlg* albumDlg;

    DInfoInterface*        iface;
};

ImageShackWindow::ImageShackWindow(DInfoInterface* const iface, 
                                   QWidget* const parent)
    : WSToolDialog(parent),
      d(new Private)
{
    d->session = new ImageShackSession();
    d->iface   = iface;
    d->widget  = new ImageShackWidget(this,
                                      d->session,
                                      d->iface,
                                      QString::fromLatin1("ImageShack"));
    d->widget->setMinimumSize(700, 500);

    setMainWidget(d->widget);
    setWindowTitle(i18n("Export to ImageShack"));
    setModal(true);

    d->albumDlg = new ImageShackNewAlbumDlg(this, QString::fromLatin1("ImageShack"));

    connect(d->widget->d->chgRegCodeBtn, SIGNAL(clicked(bool)),
            this, SLOT(slotChangeRegistrantionCode()));

    startButton()->setText(i18n("Upload"));
    startButton()->setToolTip(i18n("Start upload to ImageShack web service"));
    startButton()->setEnabled(false);

    connect(d->widget->d->imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    // -----------------------------------------------------------

    connect(this, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    d->talker = new ImageShackTalker(d->session);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalJobInProgress(int,int,QString)),
            this, SLOT(slotJobInProgress(int,int,QString)));

    connect(d->talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(d->talker, SIGNAL(signalGetGalleriesDone(int,QString)),
            this, SLOT(slotGetGalleriesDone(int,QString)));

    connect(d->talker, SIGNAL(signalUpdateGalleries(QStringList,QStringList)),
            d->widget, SLOT(slotGetGalleries(QStringList,QStringList)));

    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(d->widget, SIGNAL(signalReloadGalleries()),
            this, SLOT(slotGetGalleries()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    readSettings();

    QTimer::singleShot(20, this, SLOT(authenticate()));
}

ImageShackWindow::~ImageShackWindow()
{
    delete d->session;
    delete d;
}

void ImageShackWindow::slotImageListChanged()
{
    startButton()->setEnabled(!d->widget->d->imgList->imageUrls().isEmpty());
}

void ImageShackWindow::slotFinished()
{
    saveSettings();
    d->widget->d->progressBar->progressCompleted();
    d->widget->d->imgList->listView()->clear();
}

void ImageShackWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void ImageShackWindow::readSettings()
{
    winId();
    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    if (group.readEntry("Private", false))
    {
        d->widget->d->privateImagesChb->setChecked(true);
    }

    if (group.readEntry("Rembar", false))
    {
        d->widget->d->remBarChb->setChecked(true);
    }
    else
    {
        d->widget->d->remBarChb->setChecked(false);
    }
}

void ImageShackWindow::saveSettings()
{
    KConfig config;
    KConfigGroup group = config.group("ImageShack Settings");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    group.writeEntry("Private", d->widget->d->privateImagesChb->isChecked());
    group.writeEntry("Rembar",  d->widget->d->remBarChb->isChecked());
    group.sync();
}

void ImageShackWindow::slotStartTransfer()
{
    d->widget->d->imgList->clearProcessedStatus();
    d->transferQueue = d->widget->d->imgList->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Transfer started!";

    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->d->progressBar->setFormat(i18n("%v / %m"));
    d->widget->d->progressBar->setMaximum(d->imagesTotal);
    d->widget->d->progressBar->setValue(0);
    d->widget->d->progressBar->setVisible(true);
    d->widget->d->progressBar->progressScheduled(i18n("Image Shack Export"), false, true);
    d->widget->d->progressBar->progressThumbnailChanged(QIcon(QLatin1String("imageshack")).pixmap(22, 22));

    uploadNextItem();
}

void ImageShackWindow::slotCancelClicked()
{
    d->talker->cancel();
    d->transferQueue.clear();
    d->widget->d->imgList->cancelProcess();
    d->widget->d->progressBar->setVisible(false);
    d->widget->d->progressBar->progressCompleted();
}

void ImageShackWindow::slotChangeRegistrantionCode()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Change registration code";
    authenticate();
}

void ImageShackWindow::authenticate()
{
    emit signalBusy(true);
    d->widget->progressBar()->show();
    d->widget->d->progressBar->setValue(0);
    d->widget->d->progressBar->setMaximum(4);
    d->widget->progressBar()->setFormat(i18n("Authenticating..."));

    WSLoginDialog* const dlg = new WSLoginDialog(this, QString::fromLatin1("ImageShack"));

    if (dlg->exec() == QDialog::Accepted)
    {
        d->session->setEmail(dlg->login());
        d->session->setPassword(dlg->password());
        d->talker->authenticate();
    }
}

void ImageShackWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->d->chgRegCodeBtn->setEnabled(false);
        startButton()->setEnabled(false);
        setRejectButtonMode(QDialogButtonBox::Cancel);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->d->chgRegCodeBtn->setEnabled(true);
        startButton()->setEnabled(d->session->loggedIn() &&
                                  !d->widget->imagesList()->imageUrls().isEmpty());
        setRejectButtonMode(QDialogButtonBox::Close);
    }
}

void ImageShackWindow::slotJobInProgress(int step, int maxStep, const QString &format)
{
    if (maxStep > 0)
    {
        d->widget->d->progressBar->setMaximum(maxStep);
    }

    d->widget->d->progressBar->setValue(step);

    if (!format.isEmpty())
    {
        d->widget->d->progressBar->setFormat(format);
    }
}

void ImageShackWindow::slotLoginDone(int errCode, const QString& errMsg)
{
    d->widget->updateLabels();

    if (!errCode && d->session->loggedIn())
    {
        d->session->saveSettings();
        startButton()->setEnabled(!d->widget->imagesList()->imageUrls().isEmpty());
        d->talker->getGalleries();
    }
    else
    {
        QMessageBox::critical(this, QString(), i18n("Login failed: %1\n", errMsg));
        startButton()->setEnabled(false);
        d->widget->d->progressBar->setVisible(false);
        slotBusy(false);
    }
}

void ImageShackWindow::slotGetGalleriesDone(int errCode, const QString &errMsg)
{
    slotBusy(false);
    d->widget->d->progressBar->setVisible(false);

    if (errCode)
    {
        QMessageBox::critical(this, QString(), i18n("Failed to get galleries list: %1\n", errMsg));
    }
}

void ImageShackWindow::uploadNextItem()
{
    if (d->transferQueue.empty())
    {
        d->widget->d->progressBar->hide();
        return;
    }

    d->widget->d->imgList->processing(d->transferQueue.first());
    QString imgPath = d->transferQueue.first().toLocalFile();

    d->widget->d->progressBar->setMaximum(d->imagesTotal);
    d->widget->d->progressBar->setValue(d->imagesCount);

    QMap<QString, QString> opts;

    if (d->widget->d->privateImagesChb->isChecked())
    {
        opts[QString::fromLatin1("public")] = QString::fromLatin1("no");
    }

    if (d->widget->d->remBarChb->isChecked())
    {
        opts[QString::fromLatin1("rembar")] = QString::fromLatin1("yes");
    }

    // tags
    if (!d->widget->d->tagsFld->text().isEmpty())
    {
        QString str = d->widget->d->tagsFld->text();
        QStringList tagsList;
        tagsList = str.split(QRegExp(QString::fromLatin1("\\W+")), QString::SkipEmptyParts);
        opts[QString::fromLatin1("tags")] = tagsList.join(QString::fromLatin1(","));
    }

    opts[QString::fromLatin1("auth_token")] = d->session->authToken();

    int gidx = d->widget->d->galleriesCob->currentIndex();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Album ID is "<< d->widget->d->galleriesCob->itemData(gidx).toString();

    switch(gidx)
    {
        case 0:
            d->talker->uploadItem(imgPath, opts);
            break;
        case 1:
            opts[QString::fromLatin1("album")] = d->newAlbmTitle;
            d->talker->uploadItemToGallery(imgPath, d->newAlbmTitle, opts);
            break;
        default:
            opts[QString::fromLatin1("album")] = d->widget->d->galleriesCob->itemData(gidx).toString();
            d->talker->uploadItemToGallery(imgPath, d->widget->d->galleriesCob->itemData(gidx).toString(), opts);
    }
}

void ImageShackWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    d->widget->d->imgList->processed(d->transferQueue.first(), (errCode == 0));

    if (!errCode)
    {
        d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
        d->transferQueue.pop_front();
        d->imagesCount++;
    }
    else
    {
        if (QMessageBox::question(this, i18n("Uploading Failed"),
                                  i18n("Failed to upload photo into ImageShack: %1\n"
                                       "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            d->widget->d->progressBar->setVisible(false);
            d->transferQueue.clear();
            return;
        }
    }

    uploadNextItem();
}

void ImageShackWindow::slotGetGalleries()
{
    d->widget->d->progressBar->setVisible(true);
    d->talker->getGalleries();
}

void ImageShackWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        d->newAlbmTitle = d->albumDlg->getAlbumTitle();
    }
}

} // namespace Digikam
