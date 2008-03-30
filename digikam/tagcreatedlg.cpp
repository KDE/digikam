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

#include <qcombobox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>

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

TagEditDlg::TagEditDlg(QWidget *parent, TAlbum* album, bool create)
          : KDialogBase(Plain, 0, Help|Ok|Cancel, Ok, parent, 0, true, true)
{
    d = new TagEditDlgPriv;
    setHelp("tagscreation.anchor", "digikam");
    if (create) setCaption(i18n("New Tag"));
    else        setCaption(i18n("Edit Tag"));

    QGridLayout* grid = new QGridLayout(plainPage(), 1, 1, 0, spacingHint());
    QLabel *logo      = new QLabel(plainPage());
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 96, KIcon::DefaultState, 0, true));    

    QVBoxLayout *topLayout = new QVBoxLayout(spacingHint());

    QLabel *topLabel = new QLabel(plainPage());
    QString tagName  = album->prettyURL();
    if (tagName.endsWith("/")) tagName.truncate(tagName.length()-1);

    if (create) topLabel->setText( i18n("<qt><b>Create New Tag in <i>\"%1\"</i></b></qt>").arg(tagName));
    else        topLabel->setText( i18n("<qt><b>Tag <i>\"%1\"</i> Properties </b></qt>").arg(tagName));

    topLabel->setAlignment(Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine);
    topLayout->addWidget(topLabel);

    // --------------------------------------------------------

    QFrame *topLine = new QFrame( plainPage() );
    topLine->setFrameShape( QFrame::HLine );
    topLine->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( topLine );

    // --------------------------------------------------------

    QGridLayout *gl = new QGridLayout(topLayout, spacingHint());

    QLabel *titleLabel = new QLabel(plainPage());
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new KLineEdit(plainPage());
    d->titleEdit->setText(album->title());
    titleLabel->setBuddy(d->titleEdit);
    setFocusProxy(d->titleEdit);

    QLabel *iconTextLabel = new QLabel(plainPage());
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton = new QPushButton(plainPage());
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);
    
    d->resetIconButton = new QPushButton(i18n("Reset"), plainPage());
    if (create) d->resetIconButton->hide();

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    
    gl->addWidget(titleLabel,            0, 0);
    gl->addMultiCellWidget(d->titleEdit, 0, 0, 1, 3);
    gl->addWidget(iconTextLabel,         1, 0);
    gl->addWidget(d->iconButton,         1, 1);
    gl->addWidget(d->resetIconButton,    1, 2);
    gl->addItem(spacer, 1, 3);
    
    grid->addMultiCellWidget(logo,      0, 0, 0, 0);
    grid->addMultiCellLayout(topLayout, 0, 1, 1, 1);
    grid->setRowStretch(1, 10);

    // --------------------------------------------------------

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChange()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this, SLOT(slotIconResetClicked()));
            
    connect(d->titleEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTitleChanged(const QString&)));

    // --------------------------------------------------------

    // In create mode, by default assign the icon of the parent (if not root) to this new tag.
    if (create && !album->isRoot())
        d->icon = album->icon();
    else
        d->icon = album->icon();

    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));

    enableButtonOK(!d->titleEdit->text().isEmpty());
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
    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));
}
    
void TagEditDlg::slotIconChange()
{
#if KDE_IS_VERSION(3,3,0)
    KIconDialog dlg(this);
    dlg.setup(KIcon::NoGroup, KIcon::Application, false, 20, false, false, false);
    QString icon = dlg.openDialog();
#else
    QString icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 20);
    if (icon.startsWith("/"))
        return;
#endif
    
    if (icon.isEmpty() || icon == d->icon)
        return;

    d->icon = icon;
    d->iconButton->setIconSet(SyncJob::getTagThumbnail(d->icon, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    enableButtonOK(!newtitle.isEmpty());
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
