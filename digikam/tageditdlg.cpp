/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-01
 * Description : dialog to edit and create digiKam Tags
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QLabel>
#include <QLayout>
#include <QGridLayout>

// KDE includes.

#include <klineedit.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kseparator.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "syncjob.h"
#include "searchtextbar.h"
#include "tageditdlg.h"
#include "tageditdlg.moc"

namespace Digikam
{

class TagEditDlgPriv
{
public:

    TagEditDlgPriv()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        mainRootAlbum   = 0;
        topLabel        = 0;
        create          = false;
    }

    bool           create;

    QLabel        *topLabel;

    QString        icon;

    QPushButton   *iconButton;
    QPushButton   *resetIconButton;

    TAlbum        *mainRootAlbum;
    SearchTextBar *titleEdit;
};

TagEditDlg::TagEditDlg(QWidget *parent, TAlbum* album, bool create)
          : KDialog(parent)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("tagscreation.anchor", "digikam");

    if (create) setCaption(i18n("New Tag"));
    else        setCaption(i18n("Edit Tag"));

    d = new TagEditDlgPriv;
    d->mainRootAlbum = album;
    d->create        = create;

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    // --------------------------------------------------------

    QGridLayout* grid = new QGridLayout(page);

    QLabel *logo            = new QLabel(page);
    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIconLoader::NoGroup, 96));

    d->topLabel = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator *line = new KSeparator(Qt::Horizontal, page);

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new SearchTextBar(page, i18n("Enter tag name here..."));
    titleLabel->setBuddy(d->titleEdit);
    setFocusProxy(d->titleEdit);

    if (d->create) 
    {
        AlbumList tList = AlbumManager::instance()->allTAlbums();
        for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
        {
            TAlbum *tag = dynamic_cast<TAlbum*>(*it);
            d->titleEdit->completionObject()->addItem(tag->tagPath());
        }
    }
    else
    {
        d->titleEdit->setText(d->mainRootAlbum->title());
    }

    QLabel *iconTextLabel = new QLabel(page);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(page);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    if (create && !album->isRoot())
        d->icon = album->icon();
    else
        d->icon = album->icon();

    d->iconButton->setIcon(SyncJob::getTagThumbnail(album));

    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), page);
    if (create) d->resetIconButton->hide();

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 4, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 1, 1, 4);
    grid->addWidget(titleLabel,         2, 1, 1, 1);
    grid->addWidget(d->titleEdit,       2, 2, 1, 3);
    grid->addWidget(iconTextLabel,      3, 1, 1, 1);
    grid->addWidget(d->iconButton,      3, 2, 1, 1);
    grid->addWidget(d->resetIconButton, 3, 3, 1, 1);
    grid->setColumnStretch(4, 10);
    grid->setRowStretch(4, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));

    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    // --------------------------------------------------------

    resize(400, 200);
    slotTitleChanged(d->titleEdit->text());
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
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotIconChanged()
{
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();

    if (icon.isEmpty() || icon == d->icon)
        return;

    d->icon = icon;
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    QString tagName = d->mainRootAlbum->tagPath();
    if (tagName.endsWith("/") && !d->mainRootAlbum->isRoot()) 
        tagName.truncate(tagName.length()-1);

    if (d->create) 
        d->topLabel->setText(i18n("<qt><b>Create New Tag in<br>"
                                  "<i>\"%1\"</i></b></qt>", tagName));
    else
        d->topLabel->setText(i18n("<qt><b>Properties of Tag<br>"
                                  "<i>\"%1\"</i></b></qt>", tagName));

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

bool TagEditDlg::tagCreate(QWidget *parent, TAlbum* album, QString& title, QString& icon)
{
    TagEditDlg dlg(parent, album, true);

    bool valRet = dlg.exec();
    if (valRet == QDialog::Accepted)
    {
        title = dlg.title();
        icon  = dlg.icon();
    }

    return valRet;
}

}  // namespace Digikam
