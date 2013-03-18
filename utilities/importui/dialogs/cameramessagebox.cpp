/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-04
 * Description : a message box to manage camera items
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameramessagebox.moc"

// Qt includes

#include <QPainter>
#include <QPointer>
#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

// KDE includes

#include <kurl.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

namespace Digikam
{

class CameraItem::Private
{

public:

    Private()
    {
        hasThumb = false;
    }

    bool        hasThumb;

    CamItemInfo info;
};

CameraItem::CameraItem(QTreeWidget* const parent, const CamItemInfo& info)
    : QTreeWidgetItem(parent), d(new Private)
{
    d->info = info;
    setThumb(SmallIcon("image-x-generic", parent->iconSize().width(), KIconLoader::DisabledState), false);
    setText(1, d->info.name);
}

CameraItem::~CameraItem()
{
    delete d;
}

bool CameraItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

CamItemInfo CameraItem::info() const
{
    return d->info;
}

void CameraItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width() / 2)  - (pix.width() / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);

    QIcon icon = QIcon(pixmap);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);

    d->hasThumb = hasThumb;
}

//----------------------------------------------------------------------------

class CameraItemList::Private
{

public:

    Private()
        : iconSize(64)
    {
        ctrl = 0;
    }

    const int         iconSize;

    CameraThumbsCtrl* ctrl;
};

CameraItemList::CameraItemList(QWidget* const parent)
    : QTreeWidget(parent), d(new Private)
{
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18n("Thumb") << i18n("File Name"));
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);
}

CameraItemList::~CameraItemList()
{
    delete d;
}

void CameraItemList::setItems(const CamItemInfoList& items)
{
    foreach(CamItemInfo info, items)
    {
        new CameraItem(this, info);
    }
}

void CameraItemList::setThumbCtrl(CameraThumbsCtrl* const ctrl)
{
    d->ctrl = ctrl;

    connect(d->ctrl, SIGNAL(signalThumbInfoReady(CamItemInfo)),
            this, SLOT(slotThumbnailLoaded(CamItemInfo)));
}

void CameraItemList::slotThumbnailLoaded(const CamItemInfo& info)
{
    QTreeWidgetItemIterator it(this);
    bool                    valid;
    CachedItem              citem;

    while (*it)
    {
        CameraItem* const item = dynamic_cast<CameraItem*>(*it);

        if (item && item->info().url() == info.url())
        {
            valid = d->ctrl->getThumbInfo(info, citem);
            item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
            return;
        }

        ++it;
    }
}

void CameraItemList::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    CameraItem* const item = dynamic_cast<CameraItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        CachedItem citem;
        bool valid = d->ctrl->getThumbInfo(item->info(), citem);
        item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
    }

    QTreeWidget::drawRow(p, opt, index);
}

// --------------------------------------------------------------------------------------------------------

/** These methods are simplified version from KMessageBox class implementation
 */

void CameraMessageBox::informationList(CameraThumbsCtrl* const ctrl,
                                       QWidget* const parent,
                                       const QString& text,
                                       const CamItemInfoList& items,
                                       const QString& caption,
                                       const QString& dontShowAgainName)
{
    if (!KMessageBox::shouldBeShownContinue(dontShowAgainName))
    {
        return;
    }

    KDialog* const dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption(caption.isEmpty() ? i18n("Information") : caption);
    dialog->setButtons(KDialog::Ok);
    dialog->setObjectName("information");
    dialog->setModal(true);
    dialog->setDefaultButton(KDialog::Ok);
    dialog->setEscapeButton(KDialog::Ok);

    bool checkboxResult = false;
    QIcon icon          = KIconLoader::global()->loadIcon("dialog-information", KIconLoader::NoGroup, KIconLoader::SizeHuge,
                                                          KIconLoader::DefaultState, QStringList(), 0, true);

    createMessageBox(ctrl, dialog, icon, text, items,
                     dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                     &checkboxResult);

    if (checkboxResult)
    {
        KMessageBox::saveDontShowAgainContinue(dontShowAgainName);
    }
}

int CameraMessageBox::warningContinueCancelList(CameraThumbsCtrl* const ctrl,
                                                QWidget* const parent,
                                                const QString& text,
                                                const CamItemInfoList& items,
                                                const QString& caption,
                                                const KGuiItem& buttonContinue,
                                                const KGuiItem& buttonCancel,
                                                const QString& dontAskAgainName)
{
    if (!KMessageBox::shouldBeShownContinue(dontAskAgainName))
    {
        return KMessageBox::Continue;
    }

    KDialog* const dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setButtons(KDialog::Yes | KDialog::No);
    dialog->setObjectName("warningYesNo");
    dialog->setModal(true);
    dialog->setButtonGuiItem(KDialog::Yes, buttonContinue);
    dialog->setButtonGuiItem(KDialog::No, buttonCancel);
    dialog->setDefaultButton(KDialog::Yes);
    dialog->setEscapeButton(KDialog::No);

    bool checkboxResult = false;
    QIcon icon          = KIconLoader::global()->loadIcon("dialog-warning", KIconLoader::NoGroup, KIconLoader::SizeHuge,
                                                          KIconLoader::DefaultState, QStringList(), 0, true);
    const int result    = createMessageBox(ctrl, dialog, icon, text, items,
                                           dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                                           &checkboxResult);

    if (result != KDialog::Yes)
    {
        return KMessageBox::Cancel;
    }

    if (checkboxResult)
    {
        KMessageBox::saveDontShowAgainContinue(dontAskAgainName);
    }

    return KMessageBox::Continue;
}

int CameraMessageBox::createMessageBox(CameraThumbsCtrl* const ctrl,
                                       KDialog* const dialog,
                                       const QIcon& icon,
                                       const QString& text,
                                       const CamItemInfoList& items,
                                       const QString& ask,
                                       bool* checkboxReturn
                                      )
{
    QWidget* const mainWidget     = new QWidget(dialog);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(KDialog::spacingHint() * 2); // provide extra spacing
    mainLayout->setMargin(0);

    QHBoxLayout* const hLayout    = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout, 5);

    //--------------------------------------------------------------------------------

    QLabel* const iconLabel       = new QLabel(mainWidget);
    QStyleOption option;
    option.initFrom(mainWidget);
    iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));

    QVBoxLayout* const iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);
    hLayout->addLayout(iconLayout, 0);
    hLayout->addSpacing(KDialog::spacingHint());

    //--------------------------------------------------------------------------------

    QLabel* const messageLabel    = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);
    hLayout->addWidget(messageLabel, 5);

    //--------------------------------------------------------------------------------

    CameraItemList* const listWidget = new CameraItemList(mainWidget);
    listWidget->setThumbCtrl(ctrl);
    listWidget->setItems(items);
    mainLayout->addWidget(listWidget, 50);

    //--------------------------------------------------------------------------------

    QPointer<QCheckBox> checkbox = 0;

    if (!ask.isEmpty())
    {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);

        if (checkboxReturn)
        {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    dialog->setMainWidget(mainWidget);

    //--------------------------------------------------------------------------------

    KDialog::ButtonCode defaultCode = dialog->defaultButton();

    if (defaultCode != KDialog::NoDefault)
    {
        dialog->setButtonFocus(defaultCode);
    }

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<KDialog> guardedDialog = dialog;

    const int result = guardedDialog->exec();

    if (checkbox && checkboxReturn)
    {
        *checkboxReturn = checkbox->isChecked();
    }

    delete(KDialog*) guardedDialog;
    return result;
}

} // namespace Digikam
