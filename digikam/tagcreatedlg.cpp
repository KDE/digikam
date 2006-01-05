/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-01
 * Description :
 *
 * Copyright 2004 by Renchi Raju

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
#include <qlineedit.h>

// KDE includes.

#include <klocale.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kdeversion.h>

// Local includes.

#include "album.h"
#include "tagcreatedlg.h"
#include "syncjob.h"

namespace Digikam
{

TagCreateDlg::TagCreateDlg(TAlbum* parent)
    : KDialogBase( Plain, i18n("Create New Tag"), Help|Ok|Cancel, Ok,
                   0, 0, true, true )
{
    setHelp("tagscreation.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout(plainPage(), 0, spacingHint());

    QLabel *topLabel = new QLabel(plainPage());
    topLabel->setText( i18n("<qt><b>Create New Tag in <i>%1</i></b></qt>").arg(parent->prettyURL()) );
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
    gl->addWidget(titleLabel, 0, 0);

    m_titleEdit = new QLineEdit(plainPage());
    titleLabel->setBuddy(m_titleEdit);
    gl->addWidget(m_titleEdit, 0, 1);

    setFocusProxy(m_titleEdit);

    QLabel *iconTextLabel = new QLabel(plainPage());
    iconTextLabel->setText(i18n("&Icon:"));
    gl->addWidget(iconTextLabel, 1, 0);

    m_iconButton = new QPushButton(plainPage());
    m_iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(m_iconButton);
    gl->addWidget(m_iconButton, 1, 1);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding);
    gl->addItem(spacer, 2, 1);

    connect(m_iconButton, SIGNAL(clicked()),
            SLOT(slotIconChange()));
    connect(m_titleEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotTitleChanged(const QString&)));
    
    // by default assign the icon of the parent (if not root)
    // to this new tag
    if (!parent->isRoot())
    {
        m_icon = parent->icon();
        m_iconButton->setIconSet(SyncJob::getTagThumbnail(m_icon, 20));
    }

    enableButtonOK(!m_titleEdit->text().isEmpty());
    adjustSize();
}

TagCreateDlg::~TagCreateDlg()
{

}

QString TagCreateDlg::title() const
{
    return m_titleEdit->text();
}


QString TagCreateDlg::icon() const
{
    return m_icon;
}

void TagCreateDlg::slotIconChange()
{
#if KDE_IS_VERSION(3,3,0)
    KIconDialog dlg(this);
    dlg.setup(KIcon::NoGroup, KIcon::Application, false, 20, false,
              true, true);
    QString icon = dlg.openDialog();
#else
    QString icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 20);
    if (icon.startsWith("/"))
        return;
#endif

    if (icon.isEmpty() || m_icon == icon)
        return;

    m_icon = icon;
    m_iconButton->setIconSet(SyncJob::getTagThumbnail(m_icon, 20));
}


void TagCreateDlg::slotTitleChanged(const QString& newtitle)
{
    enableButtonOK(!newtitle.isEmpty());
}

bool TagCreateDlg::tagCreate(TAlbum* parent, QString& title,
                             QString& icon)
{
    TagCreateDlg dlg(parent);

    bool ok = dlg.exec() == QDialog::Accepted;

    title    = dlg.title();
    icon     = dlg.icon();

    return ok;
}

// -------------------------------------------------------------------------------------

TagEditDlg::TagEditDlg(TAlbum* album)
    : KDialogBase( Plain, i18n("Edit Tag"), Ok|Cancel, Ok,
                   0, 0, true, true )
{
    QVBoxLayout *topLayout = new QVBoxLayout(plainPage(), 0, spacingHint());

    QLabel *topLabel = new QLabel(plainPage());
    topLabel->setText( i18n("<qt><b><i>%1</i> Properties</b></qt>")
                       .arg(album->prettyURL()) );
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
    gl->addWidget(titleLabel, 0, 0);

    m_titleEdit = new QLineEdit(plainPage());
    m_titleEdit->setText(album->title());
    titleLabel->setBuddy(m_titleEdit);
    gl->addWidget(m_titleEdit, 0, 1);

    setFocusProxy(m_titleEdit);

    QLabel *iconTextLabel = new QLabel(plainPage());
    iconTextLabel->setText(i18n("&Icon:"));
    gl->addWidget(iconTextLabel, 1, 0);

    m_iconButton = new QPushButton(plainPage());
    m_iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(m_iconButton);
    gl->addWidget(m_iconButton, 1, 1);

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding);
    gl->addItem(spacer, 2, 1);

    connect(m_iconButton, SIGNAL(clicked()),
            SLOT(slotIconChange()));
    connect(m_titleEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotTitleChanged(const QString&)));

    m_icon = album->icon();
    m_iconButton->setIconSet(SyncJob::getTagThumbnail(m_icon, 20));

    enableButtonOK(!m_titleEdit->text().isEmpty());
    adjustSize();
}

TagEditDlg::~TagEditDlg()
{

}

QString TagEditDlg::title() const
{
    return m_titleEdit->text();
}

QString TagEditDlg::icon() const
{
    return m_icon;
}

void TagEditDlg::slotIconChange()
{
#if KDE_IS_VERSION(3,3,0)
    KIconDialog dlg(this);
    dlg.setup(KIcon::NoGroup, KIcon::Application, false, 20, false,
              true, true);
    QString icon = dlg.openDialog();
#else
    QString icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 20);
    if (icon.startsWith("/"))
        return;
#endif
    
    if (icon.isEmpty() || icon == m_icon)
        return;

    m_icon = icon;
    m_iconButton->setIconSet(SyncJob::getTagThumbnail(m_icon, 20));
}

void TagEditDlg::slotTitleChanged(const QString& newtitle)
{
    enableButtonOK(!newtitle.isEmpty());
}

bool TagEditDlg::tagEdit(TAlbum* album, QString& title,
                         QString& icon)
{
    TagEditDlg dlg(album);

    bool ok = (dlg.exec() == QDialog::Accepted);

    title    = dlg.title();
    icon     = dlg.icon();

    return ok;
}

}  // namespace Digikam

#include "tagcreatedlg.moc"

