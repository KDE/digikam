/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a dialog to edit EXIF,IPTC and XMP metadata
 *
 * Copyright (C) 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadataedit.h"

// Qt includes

#include <QCloseEvent>
#include <QKeyEvent>
#include <QPointer>
#include <QObject>
#include <QApplication>
#include <QMenu>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QImage>
#include <QBuffer>
#include <QPainter>
#include <QPalette>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "exifeditwidget.h"
#include "iptceditwidget.h"
#include "xmpeditwidget.h"
#include "thumbnailloadthread.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class MetadataEditDialog::Private
{
public:

    Private()
    {
        isReadOnly      = false;
        tabWidget       = 0;
        buttons         = 0;
        tabExif         = 0;
        tabIptc         = 0;
        tabXmp          = 0;
        catcher         = 0;
    }

    bool                   isReadOnly;

    QString                preview;

    QList<QUrl>            urls;
    QList<QUrl>::iterator  currItem;

    QTabWidget*            tabWidget;

    QDialogButtonBox*      buttons;

    EXIFEditWidget*        tabExif;
    IPTCEditWidget*        tabIptc;
    XMPEditWidget*         tabXmp;

    ThumbnailImageCatcher* catcher;
};

MetadataEditDialog::MetadataEditDialog(QWidget* const parent, const QList<QUrl>& urls)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Metadata Editor"));
    setModal(true);

    ThumbnailLoadThread* const thread = new ThumbnailLoadThread;
    thread->setThumbnailSize(48);
    thread->setPixmapRequested(false);
    d->catcher                        = new ThumbnailImageCatcher(thread, this);

    d->urls     = urls;
    d->currItem = d->urls.begin();
    updatePreview();

    QDialogButtonBox::StandardButtons btns = QDialogButtonBox::Ok    |
                                             QDialogButtonBox::Apply |
                                             QDialogButtonBox::Close |
                                             QDialogButtonBox::No    |   // NextPrevious item
                                             QDialogButtonBox::Yes;      // Previous item

    d->buttons = new QDialogButtonBox(btns, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(false);

    if (d->urls.count() > 1)
    {
        d->buttons->button(QDialogButtonBox::No)->setText(i18nc("@action:button",  "Next"));
        d->buttons->button(QDialogButtonBox::No)->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
        d->buttons->button(QDialogButtonBox::Yes)->setText(i18nc("@action:button", "Previous"));
        d->buttons->button(QDialogButtonBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    }
    else
    {
        d->buttons->button(QDialogButtonBox::No)->setVisible(false);
        d->buttons->button(QDialogButtonBox::Yes)->setVisible(false);
    }

    d->tabWidget = new QTabWidget(this);
    d->tabExif   = new EXIFEditWidget(this);
    d->tabIptc   = new IPTCEditWidget(this);
    d->tabXmp    = new XMPEditWidget(this);
    d->tabWidget->addTab(d->tabExif, i18n("Edit EXIF"));
    d->tabWidget->addTab(d->tabIptc, i18n("Edit IPTC"));
    d->tabWidget->addTab(d->tabXmp,  i18n("Edit XMP"));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->tabWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    //----------------------------------------------------------

    connect(d->tabExif, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabIptc, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabXmp, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabExif, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabIptc, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabXmp, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotApply()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(slotClose()));

    connect(d->buttons->button(QDialogButtonBox::No), SIGNAL(clicked()),
            this, SLOT(slotNext()));

    connect(d->buttons->button(QDialogButtonBox::Yes), SIGNAL(clicked()),
            this, SLOT(slotPrevious()));

    //----------------------------------------------------------

    readSettings();
    slotItemChanged();
}

MetadataEditDialog::~MetadataEditDialog()
{
    d->catcher->thread()->stopAllTasks();
    d->catcher->cancel();

    delete d->catcher->thread();
    delete d->catcher;
    delete d;
}

QString MetadataEditDialog::currentItemTitleHeader(const QString& title) const
{
    QString start = QLatin1String("<qt><table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">");
    QString end   = QLatin1String("</table></qt>");
    return QString::fromLatin1("%1<tr><td>%2</td><td>%3</td></tr>%4").arg(start).arg(d->preview).arg(title).arg(end);
}

void MetadataEditDialog::updatePreview()
{
    d->catcher->setActive(true);

    d->catcher->thread()->find(ThumbnailIdentifier(d->currItem->toLocalFile()));
    d->catcher->enqueue();
    QList<QImage> images = d->catcher->waitForThumbnails();

    QImage img(48, 48, QImage::Format_ARGB32);
    QImage thm = images.first();
    QPainter p(&img);
    p.fillRect(img.rect(), QPalette().window());
    p.setPen(Qt::black);
    p.drawRect(img.rect().left(), img.rect().top(), img.rect().right()-1, img.rect().bottom()-1);
    p.drawImage((img.width() - thm.width())/2, (img.height() - thm.height())/2, thm);

    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    d->preview = QString::fromLatin1("<img src=\"data:image/png;base64,%1\">  ").arg(QString::fromLatin1(byteArray.toBase64().data()));

    d->catcher->setActive(false);
}

QList<QUrl>::iterator MetadataEditDialog::currentItem() const
{
    return d->currItem;
}

void MetadataEditDialog::slotModified()
{
    bool modified = false;

    switch (d->tabWidget->currentIndex())
    {
        case 0:
            modified = d->tabExif->isModified();
            break;

        case 1:
            modified = d->tabIptc->isModified();
            break;

        case 2:
            modified = d->tabXmp->isModified();
            break;
    }

    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(modified);
}

void MetadataEditDialog::slotOk()
{
    slotApply();
    saveSettings();
    accept();
}

void MetadataEditDialog::slotClose()
{
    saveSettings();
    close();
}

void MetadataEditDialog::slotApply()
{
    d->tabExif->apply();
    d->tabIptc->apply();
    d->tabXmp->apply();
    slotItemChanged();
}

void MetadataEditDialog::slotNext()
{
    slotApply();
    d->currItem++;
    slotItemChanged();
}

void MetadataEditDialog::slotPrevious()
{
    slotApply();
    d->currItem--;
    slotItemChanged();
}

void MetadataEditDialog::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Metadata Edit Dialog"));
    d->tabWidget->setCurrentIndex(group.readEntry(QLatin1String("Tab Index"), 0));

    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

void MetadataEditDialog::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Metadata Edit Dialog"));
    group.writeEntry(QLatin1String("Tab Index"), d->tabWidget->currentIndex());
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    d->tabExif->saveSettings();
    d->tabIptc->saveSettings();
    d->tabXmp->saveSettings();
}

void MetadataEditDialog::slotItemChanged()
{
    updatePreview();
    d->tabExif->slotItemChanged();
    d->tabIptc->slotItemChanged();
    d->tabXmp->slotItemChanged();

    setWindowTitle(i18n("%1 (%2/%3) - Edit Metadata",
        (*d->currItem).fileName(),
        d->urls.indexOf(*(d->currItem))+1,
        d->urls.count()));

    d->buttons->button(QDialogButtonBox::No)->setEnabled(*(d->currItem) != d->urls.last());
    d->buttons->button(QDialogButtonBox::Yes)->setEnabled(*(d->currItem) != d->urls.first());
    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(!d->isReadOnly);
}

bool MetadataEditDialog::eventFilter(QObject*, QEvent* e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent* const k = (QKeyEvent*)e;

        if (k->modifiers() == Qt::ControlModifier &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if (d->buttons->button(QDialogButtonBox::No)->isEnabled())
                slotNext();

            return true;
        }
        else if (k->modifiers() == Qt::ShiftModifier &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if (d->buttons->button(QDialogButtonBox::Yes)->isEnabled())
                slotPrevious();

            return true;
        }

        return false;
    }

    return false;
}

void MetadataEditDialog::closeEvent(QCloseEvent* e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void MetadataEditDialog::slotSetReadOnly(bool state)
{
    d->isReadOnly = state;
}

}  // namespace Digikam
