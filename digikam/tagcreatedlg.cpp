/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klineedit.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>

// Local includes.

#include "album.h"
#include "syncjob.h"
#include "tagcreatedlg.h"
#include "tagcreatedlg.moc"

namespace Digikam
{

class TagCreateDlgPriv
{
public:

    TagCreateDlgPriv()
    {
        titleEdit  = 0;
        iconButton = 0;
        icon       = QString("tag");
    }

    QString      icon;

    QPushButton *iconButton;

    KLineEdit   *titleEdit;
};

TagCreateDlg::TagCreateDlg(QWidget *parent, TAlbum* album)
            : KDialog( parent)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setCaption(i18n("New Tag"));
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");

    d = new TagCreateDlgPriv;

    QWidget *widget = new QWidget(this);
    setMainWidget(widget);

    // --------------------------------------------------------
 
    QGridLayout* grid = new QGridLayout(widget);

    QLabel *logo            = new QLabel(widget);
    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIconLoader::NoGroup, 96));    

    QVBoxLayout *topLayout = new QVBoxLayout();

    QLabel *topLabel = new QLabel(widget);
    QString tagName  = album->prettyUrl();
    if (tagName.endsWith("/")) tagName.truncate(tagName.length()-1);
    topLabel->setText(i18n("<qt><b>Create New Tag in <i>\"%1\"</i></b></qt>", tagName));
    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLabel->setWordWrap(false);

    // --------------------------------------------------------

    QFrame *topLine = new QFrame(widget);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Sunken);

    topLayout->addWidget(topLabel);
    topLayout->addWidget(topLine);
    topLayout->setMargin(KDialog::spacingHint());
    topLayout->setSpacing(0);

    // --------------------------------------------------------

    QGridLayout *gl = new QGridLayout();
    topLayout->addLayout(gl);

    QLabel *titleLabel = new QLabel(widget);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(widget);
    d->titleEdit->setClearButtonShown(true);
    titleLabel->setBuddy(d->titleEdit);

    setFocusProxy(d->titleEdit);

    QLabel *iconTextLabel = new QLabel(widget);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(widget);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // --------------------------------------------------------

    gl->addWidget(titleLabel, 0, 0, 1, 1);
    gl->addWidget(d->titleEdit, 0, 1, 1, 1);
    gl->addWidget(iconTextLabel, 1, 0, 1, 1);
    gl->addWidget(d->iconButton, 1, 1, 1, 1);
    gl->addItem(spacer, 2, 1, 1, 1);
    gl->setMargin(KDialog::spacingHint());
    gl->setSpacing(KDialog::spacingHint());

    grid->addWidget(logo, 0, 0, 1, 1);
    grid->addLayout(topLayout, 0, 1, 2, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChange()));

    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));
    
    // --------------------------------------------------------

    // by default assign the icon of the parent (if not root)
    // to this new tag
    if (!album->isRoot())
        d->icon = album->icon();
    
    d->iconButton->setIcon(SyncJob::getTagThumbnail(d->icon, 20));

    enableButtonOk(!d->titleEdit->text().isEmpty());
    adjustSize();
}

TagCreateDlg::~TagCreateDlg()
{
    delete d;
}

QString TagCreateDlg::title() const
{
    return d->titleEdit->text();
}

QString TagCreateDlg::icon() const
{
    return d->icon;
}

void TagCreateDlg::slotIconChange()
{
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();

    if (icon.isEmpty() || d->icon == icon)
        return;

    d->icon = icon;
    d->iconButton->setIcon(SyncJob::getTagThumbnail(d->icon, 20));
}

void TagCreateDlg::slotTitleChanged(const QString& newtitle)
{
    enableButtonOk(!newtitle.isEmpty());
}

bool TagCreateDlg::tagCreate(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagCreateDlg dlg(parent, album);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

// -------------------------------------------------------------------------------------

class TagEditDlgPriv
{
public:

    TagEditDlgPriv()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
    }

    QString      icon;

    QPushButton *iconButton;
    QPushButton *resetIconButton;

    KLineEdit   *titleEdit;
};

TagEditDlg::TagEditDlg(QWidget *parent, TAlbum* album)
          : KDialog(parent)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setCaption(i18n("Edit Tag"));
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");

    d = new TagEditDlgPriv;

    QWidget *widget = new QWidget(this);
    setMainWidget(widget);

    // --------------------------------------------------------

    QGridLayout* grid = new QGridLayout(widget);

    QLabel *logo            = new QLabel(widget);
    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIconLoader::NoGroup, 96));    

    QVBoxLayout *topLayout = new QVBoxLayout();

    QLabel *topLabel = new QLabel(widget);
    QString tagName  = album->prettyUrl();
    if (tagName.endsWith("/")) tagName.truncate(tagName.length()-1);
    topLabel->setText(i18n("<qt><b>Tag <i>\"%1\"</i> Properties </b></qt>", tagName));
    topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    topLabel->setWordWrap(false);

    // --------------------------------------------------------

    QFrame *topLine = new QFrame(widget);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Sunken);

    topLayout->addWidget(topLabel);
    topLayout->addWidget(topLine);
    topLayout->setMargin(KDialog::spacingHint());
    topLayout->setSpacing(0);

    // --------------------------------------------------------

    QGridLayout *gl = new QGridLayout();
    topLayout->addLayout(gl);

    QLabel *titleLabel = new QLabel(widget);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(widget);
    d->titleEdit->setClearButtonShown(true);
    d->titleEdit->setText(album->title());
    titleLabel->setBuddy(d->titleEdit);
    setFocusProxy(d->titleEdit);

    QLabel *iconTextLabel = new QLabel(widget);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(widget);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);
    
    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), widget);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // --------------------------------------------------------
    
    gl->addWidget(titleLabel, 0, 0, 1, 1);
    gl->addWidget(d->titleEdit, 0, 1, 1, 3);
    gl->addWidget(iconTextLabel, 1, 0, 1, 1);
    gl->addWidget(d->iconButton, 1, 1, 1, 1);
    gl->addWidget(d->resetIconButton, 1, 2, 1, 1);
    gl->addItem(spacer, 1, 3, 1, 1);
    gl->setMargin(KDialog::spacingHint());
    gl->setSpacing(KDialog::spacingHint());
    
    grid->addWidget(logo, 0, 0, 1, 1);
    grid->addLayout(topLayout, 0, 1, 2, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChange()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));
            
    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    // --------------------------------------------------------

    d->icon = album->icon();
    d->iconButton->setIcon(SyncJob::getTagThumbnail(d->icon, 20));

    enableButtonOk(!d->titleEdit->text().isEmpty());
    adjustSize();
}

TagEditDlg::~TagEditDlg()
{
    delete d;
}

QString TagEditDlg::title() const
{
    return d->titleEdit->text();
}

QString TagEditDlg::icon() const
{
    return d->icon;
}

void TagEditDlg::slotIconResetClicked()
{
    d->icon = QString("tag");
    d->iconButton->setIcon(SyncJob::getTagThumbnail(d->icon, 20));
}
    
void TagEditDlg::slotIconChange()
{
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();
    
    if (icon.isEmpty() || icon == d->icon)
        return;

    d->icon = icon;
    d->iconButton->setIcon(SyncJob::getTagThumbnail(d->icon, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    enableButtonOk(!newtitle.isEmpty());
}

bool TagEditDlg::tagEdit(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

}  // namespace Digikam
