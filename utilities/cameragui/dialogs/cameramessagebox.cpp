/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-04
 * Description : a message box to manage camera items
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QScrollArea>
#include <QScrollBar>

// KDE includes

#include <kglobalsettings.h>
#include <knotification.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <ksqueezedtextlabel.h>
#include <kwindowsystem.h>

namespace Digikam
{

class CameraItem::CameraItemPriv
{

public:

    CameraItemPriv()
    {
        hasThumb = false;
    }

    bool        hasThumb;

    CamItemInfo info;
};

CameraItem::CameraItem(QTreeWidget* parent, const CamItemInfo& info)
    : QTreeWidgetItem(parent), d(new CameraItemPriv)
{
    d->info = info;
    setThumb(SmallIcon("image-x-generic", parent->iconSize().width(), KIconLoader::DisabledState), false);
    setText(1, d->info.name );
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
    QPixmap pixmap(iconSize+2, iconSize+2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2)  - (pix.width()/2),
                 (pixmap.height()/2) - (pix.height()/2), pix);

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

class CameraItemList::CameraItemListPriv
{

public:

    CameraItemListPriv()
        : iconSize(64)
    {
        ctrl = 0;
    }

    const int         iconSize;

    CameraThumbsCtrl* ctrl;
};

CameraItemList::CameraItemList(QWidget* parent)
    : QTreeWidget(parent), d(new CameraItemListPriv)
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

void CameraItemList::setThumbCtrl(CameraThumbsCtrl* ctrl)
{
    d->ctrl = ctrl;

    connect(d->ctrl, SIGNAL(signalThumbInfoReady(const CamItemInfo&)),
            this, SLOT(slotThumbnailLoaded(const CamItemInfo&)));
}

void CameraItemList::slotThumbnailLoaded(const CamItemInfo& info)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        CameraItem* item = dynamic_cast<CameraItem*>(*it);
        if (item && item->info().url() == info.url())
        {
            CachedItem citem;
            bool valid = d->ctrl->getThumbInfo(info, citem);
            item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
            return;
        }
        ++it;
    }
}

void CameraItemList::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    CameraItem* item = dynamic_cast<CameraItem*>(itemFromIndex(index));
    if (item && !item->hasValidThumbnail())
    {
        CachedItem citem;
        bool valid = d->ctrl->getThumbInfo(item->info(), citem);
        item->setThumb(citem.second.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio), valid);
    }
    QTreeWidget::drawRow(p, opt, index);
}

// --------------------------------------------------------------------------------------------------------
/** For these methods, see KMessageBox class implementation
 */

static QIcon themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch (icon) {
    case QMessageBox::NoIcon:
        return QIcon();
        break;
    case QMessageBox::Information:
        icon_name = "dialog-information";
        break;
    case QMessageBox::Warning:
        icon_name = "dialog-warning";
        break;
    case QMessageBox::Critical:
        icon_name = "dialog-error";
        break;
    default:
        break;
    }

   QIcon ret = KIconLoader::global()->loadIcon(icon_name, KIconLoader::NoGroup, KIconLoader::SizeHuge, KIconLoader::DefaultState, QStringList(), 0, true);

   if (ret.isNull()) {
       return QMessageBox::standardIcon(icon);
   } else {
       return ret;
   }
}

static void sendNotification( QString message, //krazy:exclude=passbyvalue
                              const QStringList& strlist,
                              QMessageBox::Icon icon,
                              WId parent_id )
{
    // create the message for KNotify
    QString messageType;
    switch (icon) {
    case QMessageBox::Warning:
        messageType = "messageWarning";
        break;
    case QMessageBox::Critical:
        messageType = "messageCritical";
        break;
    case QMessageBox::Question:
        messageType = "messageQuestion";
        break;
    default:
        messageType = "messageInformation";
        break;
    }

    if ( !strlist.isEmpty() ) {
        for ( QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it ) {
            message += '\n' + *it;
        }
    }

    if ( !message.isEmpty() ) {
        KNotification::event( messageType, message, QPixmap(), QWidget::find( parent_id ),
                              KNotification::DefaultEvent | KNotification::CloseOnTimeout );
    }
}

void CameraMessageBox::informationList(CameraThumbsCtrl* ctrl, QWidget* parent, const QString& text, 
                                       const CamItemInfoList& items,
                                       const QString& caption, const QString& dontShowAgainName)
{
    if (!KMessageBox::shouldBeShownContinue(dontShowAgainName))
    {
        return;
    }

    KDialog* dialog = new KDialog(parent, Qt::Dialog);
    dialog->setCaption(caption.isEmpty() ? i18n("Information") : caption);
    dialog->setButtons(KDialog::Ok );
    dialog->setObjectName("information");
    dialog->setModal(true);
    dialog->setDefaultButton(KDialog::Ok);
    dialog->setEscapeButton(KDialog::Ok);

    bool checkboxResult = false;

    createMessageBox(ctrl, dialog, themedMessageBoxIcon(QMessageBox::Information), text, items,
                     dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                     &checkboxResult, KMessageBox::Notify);

    if (checkboxResult)
    {
        KMessageBox::saveDontShowAgainContinue(dontShowAgainName);
    }
}

int CameraMessageBox::createMessageBox(CameraThumbsCtrl* ctrl,
                                       KDialog* dialog,
                                       const QIcon& icon,
                                       const QString& text,
                                       const CamItemInfoList& items,
                                       const QString& ask,
                                       bool* checkboxReturn,
                                       KMessageBox::Options options,
                                       const QString& details,
                                       QMessageBox::Icon notifyType
                                      )
{
    QWidget *mainWidget = new QWidget(dialog);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(KDialog::spacingHint() * 2); // provide extra spacing
    mainLayout->setMargin(0);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout,5);

    QLabel *iconLabel = new QLabel(mainWidget);

    if (!icon.isNull()) {
        QStyleOption option;
        option.initFrom(mainWidget);
        iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));
    }

    QVBoxLayout *iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);

    hLayout->addLayout(iconLayout,0);
    hLayout->addSpacing(KDialog::spacingHint());

    QLabel *messageLabel = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
    Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard;
    if (options & KMessageBox::AllowLink) {
        flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    }
    messageLabel->setTextInteractionFlags(flags);

    QRect desktop = KGlobalSettings::desktopGeometry(dialog);
    bool usingSqueezedTextLabel = false;
    if (messageLabel->sizeHint().width() > desktop.width() * 0.5) {
        // enable automatic wrapping of messages which are longer than 50% of screen width
        messageLabel->setWordWrap(true);
        // display a text widget with scrollbar if still too wide
        usingSqueezedTextLabel = messageLabel->sizeHint().width() > desktop.width() * 0.85;
        if (usingSqueezedTextLabel)
        {
            delete messageLabel;
            messageLabel = new KSqueezedTextLabel(text, mainWidget);
            messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            messageLabel->setTextInteractionFlags(flags);
        }
    }

    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);


    bool usingScrollArea=desktop.height() / 3 < messageLabel->sizeHint().height();
    if (usingScrollArea)
    {
        QScrollArea* messageScrollArea = new QScrollArea(mainWidget);
        messageScrollArea->setWidget(messageLabel);
        messageScrollArea->setFrameShape(QFrame::NoFrame);
        messageScrollArea->setWidgetResizable(true);
        QPalette scrollPal(messageScrollArea->palette());
        scrollPal.setColor(QPalette::Window, Qt::transparent);
        messageScrollArea->viewport()->setPalette(scrollPal);
        hLayout->addWidget(messageScrollArea,5);
    }
    else
        hLayout->addWidget(messageLabel,5);

    const bool usingListWidget=!items.isEmpty();
    if (usingListWidget) {
        // enable automatic wrapping since the listwidget has already a good initial width
        messageLabel->setWordWrap(true);
        messageLabel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);

        // NOTE: customization here
        CameraItemList* listWidget = new CameraItemList(mainWidget);
        listWidget->setThumbCtrl(ctrl);
        listWidget->setItems(items);
        // end of customization

        mainLayout->addWidget(listWidget,usingScrollArea?10:50);
    }
    else if (!usingScrollArea)
        mainLayout->addStretch(15);

    QPointer<QCheckBox> checkbox = 0;
    if (!ask.isEmpty()) {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);
        if (checkboxReturn) {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    if (!details.isEmpty()) {
        QGroupBox *detailsGroup = new QGroupBox(i18n("Details"));
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
        if (details.length() < 512) {
            QLabel *detailsLabel = new QLabel(details);
            detailsLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard;
            if ( options & KMessageBox::AllowLink )
                flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;;
            detailsLabel->setTextInteractionFlags(flags);
            detailsLabel->setWordWrap(true);
            detailsLayout->addWidget(detailsLabel,50);
        } else {
            KTextEdit *detailTextEdit = new KTextEdit(details);
            detailTextEdit->setReadOnly(true);
            detailTextEdit->setMinimumHeight(detailTextEdit->fontMetrics().lineSpacing() * 11);
            detailsLayout->addWidget(detailTextEdit,50);
        }
        if (!usingListWidget)
            mainLayout->setStretchFactor(hLayout,10);
        dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(mainWidget);
    if (!usingListWidget && !usingScrollArea && !usingSqueezedTextLabel && details.isEmpty())
        dialog->setFixedSize(dialog->sizeHint() + QSize( 10, 10 ));
    else if (!details.isEmpty() && dialog->minimumHeight()<iconLabel->sizeHint().height()*2)//strange bug...
    {
        if (!usingScrollArea)
            dialog->setMinimumSize(300,qMax(150,qMax(iconLabel->sizeHint().height(),messageLabel->sizeHint().height())));
        else
            dialog->setMinimumSize(300,qMax(150,iconLabel->sizeHint().height()));
    }

    if ((options & KMessageBox::Dangerous)) {
        if (dialog->isButtonEnabled(KDialog::Cancel))
            dialog->setDefaultButton(KDialog::Cancel);
        else if (dialog->isButtonEnabled(KDialog::No))
            dialog->setDefaultButton(KDialog::No);
    }

    KDialog::ButtonCode defaultCode = dialog->defaultButton();
    if (defaultCode != KDialog::NoDefault) {
        dialog->setButtonFocus(defaultCode);
    }

/*
#ifndef Q_WS_WIN // FIXME problems with KNotify on Windows
    if ((options & KMessageBox::Notify)) {
        sendNotification(text, strlist, notifyType, dialog->topLevelWidget()->winId());
    }
#endif

    if (KMessageBox_queue) {
        KDialogQueue::queueDialog(dialog);
        return KMessageBox::Cancel; // We have to return something.
    }
*/
    if ((options & KMessageBox::NoExec)) {
        return KMessageBox::Cancel; // We have to return something.
    }

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<KDialog> guardedDialog = dialog;

    const int result = guardedDialog->exec();
    if (checkbox && checkboxReturn) {
        *checkboxReturn = checkbox->isChecked();
    }

    delete (KDialog *) guardedDialog;
    return result;
}

} // namespace Digikam
