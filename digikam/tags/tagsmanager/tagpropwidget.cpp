/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-03
 * Description : Tag Properties widget to display tag properties
 *               when a tag or multiple tags are selected
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "tagpropwidget.moc"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <kkeysequencewidget.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kmessagebox.h>
#include <kdialog.h>
// local includes

#include "searchtextbar.h"
#include "album.h"
#include "albummanager.h"
#include "tagsactionmngr.h"
#include "syncjob.h"

namespace Digikam
{

class TagPropWidget::Private
{
public:

    Private()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        saveButton      = 0;
        discardButton   = 0;
        topLabel        = 0;
        keySeqWidget    = 0;
        changed         = false;
    }

    QLabel*             topLabel;

    QString             icon;

    QPushButton*        iconButton;
    QPushButton*        resetIconButton;
    QPushButton*        saveButton;
    QPushButton*        discardButton;
    QList<TAlbum*>      selectedAlbums;
    KKeySequenceWidget* keySeqWidget;
    SearchTextBar*      titleEdit;
    bool                changed;

};

TagPropWidget::TagPropWidget(QWidget* const parent)
    : QWidget(parent), d(new Private())
{
    QGridLayout* const grid = new QGridLayout(this);
    QLabel* const logo      = new QLabel(this);

    logo->setPixmap(KIcon("tag-properties").pixmap(30,30));
    d->topLabel = new QLabel(this);
    d->topLabel->setText(i18n("Tag Properties"));
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator* const line = new KSeparator(Qt::Horizontal, this);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(this);
    titleLabel->setText(i18n("&Title:"));
    titleLabel->setMargin(KDialog::marginHint());
    titleLabel->setIndent(KDialog::spacingHint());

    d->titleEdit = new SearchTextBar(this, "TagEditDlgTitleEdit",
                                     i18n("Enter tag name here"));
    d->titleEdit->setCaseSensitive(false);
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const tipLabel = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setMargin(KDialog::marginHint());
    tipLabel->setIndent(KDialog::spacingHint());

    QLabel* const iconTextLabel = new QLabel(this);
    iconTextLabel->setText(i18n("&Icon:"));
    iconTextLabel->setMargin(KDialog::marginHint());
    iconTextLabel->setIndent(KDialog::spacingHint());

    d->iconButton         = new QPushButton(this);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);

    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), this);

    QLabel* const kscTextLabel = new QLabel(this);
    kscTextLabel->setText(i18n("&Shortcut:"));
    kscTextLabel->setMargin(KDialog::marginHint());
    kscTextLabel->setIndent(KDialog::spacingHint());

    d->keySeqWidget      = new KKeySequenceWidget(this);
    kscTextLabel->setBuddy(d->keySeqWidget);
    d->keySeqWidget->setCheckActionCollections(TagsActionMngr::defaultManager()->actionCollections());

    QLabel* const tipLabel2 = new QLabel(this);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note:</b> <i>This shortcut can be used "
                            "to assign or unassign tag to items.</i></p>"));
    tipLabel2->setMargin(KDialog::marginHint() );
    tipLabel2->setIndent(KDialog::spacingHint());

    d->saveButton = new QPushButton(i18n("Save"));
    d->discardButton = new QPushButton(i18n("Discard"));

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 1, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 0, 1, 4);
    grid->addWidget(tipLabel,           2, 0, 1, 4);
    grid->addWidget(titleLabel,         3, 0, 1, 1);
    grid->addWidget(d->titleEdit,       3, 1, 1, 3);
    grid->addWidget(iconTextLabel,      4, 0, 1, 1);
    grid->addWidget(d->iconButton,      4, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 2, 1, 1);
    grid->addWidget(kscTextLabel,       5, 0, 1, 1);
    grid->addWidget(d->keySeqWidget,    5, 1, 1, 3);
    grid->addWidget(tipLabel2,          6, 0, 1, 4);
    grid->addWidget(d->saveButton,      7, 0, 1, 1);
    grid->addWidget(d->discardButton,   7, 1, 1, 1);
    grid->setRowStretch(8, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(KDialog::marginHint());
    grid->setVerticalSpacing(KDialog::spacingHint());

    adjustSize();

    connect(d->iconButton, SIGNAL(clicked()),
            this, SLOT(slotIconChanged()));

    connect(d->titleEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotDataChanged()));

    connect(d->resetIconButton, SIGNAL(clicked()),
            this,SLOT(slotIconResetClicked()));

    connect(d->keySeqWidget, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(slotDataChanged()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveChanges()));

    connect(d->discardButton, SIGNAL(clicked()),
            this, SLOT(slotDiscardChanges()));

    enableItems(TagPropWidget::DisabledAll);
}

TagPropWidget::~TagPropWidget()
{
    delete d;
}

void TagPropWidget::slotSelectionChanged(QList<Album*> albums)
{

    if(albums.isEmpty())
    {
        enableItems(TagPropWidget::DisabledAll);
        return;
    }

    if(d->changed)
    {
        int rez = KMessageBox::questionYesNo(this,
                                             i18n("Previous tags were changed. "
                                                "Save changes? "));
        if(rez == KMessageBox::Yes)
        {
            slotSaveChanges();
        }

        d->changed = false;
    }

    if(albums.size() == 1)
    {
        TAlbum* const album = dynamic_cast<TAlbum*>(albums.first());

        if(!album)
        {
            return;
        }

        QString Seq = album->property(TagPropertyName::tagKeyboardShortcut());
        d->selectedAlbums.clear();
        d->selectedAlbums.append(album);
        d->titleEdit->setText(album->title());
        d->icon     = album->icon();
        d->iconButton->setIcon(SyncJob::getTagThumbnail(album));
        d->keySeqWidget->setKeySequence(Seq);

        if(album->isRoot())
            enableItems(TagPropWidget::DisabledAll);
        else
            enableItems(TagPropWidget::EnabledAll);
    }
    else
    {
        d->selectedAlbums.clear();
        QList<Album*>::iterator it;
        bool containsRoot = false;

        for(it = albums.begin(); it != albums.end(); ++it)
        {
            TAlbum* const temp = dynamic_cast<TAlbum*>(*it);

            if (temp)
            {
                d->selectedAlbums.append(temp);

                if(temp->isRoot())
                    containsRoot = true;
            }
        }

        d->titleEdit->clear();
        d->icon.clear();
        d->iconButton->setIcon(KIcon());
        d->keySeqWidget->clearKeySequence();
        if(containsRoot)
            enableItems(TagPropWidget::DisabledAll);
        else
            enableItems(TagPropWidget::IconOnly);
    }

    d->changed = false;
}

void TagPropWidget::slotIconResetClicked()
{
    if(d->icon.isEmpty() || d->icon == QString("tag"))
    {
        return;
    }

    d->changed = true;
    d->icon    = QString("tag");
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}
void TagPropWidget::slotIconChanged()
{
    d->changed   = true;
    KIconDialog dlg(this);
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Application, false,
              20, false, false, false);
    QString icon = dlg.openDialog();

    if (icon.isEmpty() || icon == d->icon)
    {
        return;
    }

    d->icon = icon;
    d->iconButton->setIcon(KIconLoader::global()->loadIcon(d->icon, KIconLoader::NoGroup, 20));
}

void TagPropWidget::slotDataChanged()
{
    d->changed = true;
}

void TagPropWidget::slotSaveChanges()
{
    if(d->selectedAlbums.size() == 1)
    {
        QString title     = d->titleEdit->text();
        TAlbum* const tag = d->selectedAlbums.first();
        QString icon      = d->icon;
        QKeySequence ks   = d->keySeqWidget->keySequence();

        if (tag && tag->title() != title)
        {
            QString errMsg;

            if (!AlbumManager::instance()->renameTAlbum(tag, title, errMsg))
            {
                KMessageBox::error(0, errMsg);
            }
        }

        if (tag && tag->icon() != icon)
        {
            QString errMsg;

            if (!AlbumManager::instance()->updateTAlbumIcon(tag, icon, 0, errMsg))
            {
                KMessageBox::error(0, errMsg);
            }
        }

        if (tag && tag->property(TagPropertyName::tagKeyboardShortcut()) != ks.toString())
        {
            TagsActionMngr::defaultManager()->updateTagShortcut(tag->id(), ks);
        }
    }
    else
    {
        QList<TAlbum*>::iterator it;

        for(it = d->selectedAlbums.begin(); it != d->selectedAlbums.end(); ++it)
        {
            TAlbum* const tag = *it;

            if (tag && tag->icon() != d->icon)
            {
                QString errMsg;

                if (!AlbumManager::instance()->updateTAlbumIcon(tag, d->icon, 0, errMsg))
                {
                    KMessageBox::error(0, errMsg);
                }
            }
        }
    }

    d->changed = false;
}

void TagPropWidget::slotDiscardChanges()
{
    if(d->selectedAlbums.size() == 1)
    {
        TAlbum* const album = d->selectedAlbums.first();
        QString Seq         = album->property(TagPropertyName::tagKeyboardShortcut());

        d->titleEdit->setText(album->title());
        d->icon             = album->icon();
        d->iconButton->setIcon(SyncJob::getTagThumbnail(album));
        d->keySeqWidget->setKeySequence(Seq);
    }
    else
    {
        d->icon.clear();
    }

    d->changed = false;
}

void TagPropWidget::enableItems(TagPropWidget::ItemsEnable value)
{
    bool val        = false;
    bool iconEnable = false;

    if(value == TagPropWidget::DisabledAll)
    {
        val        = false;
        iconEnable = false;
    }

    if(value == TagPropWidget::EnabledAll)
    {
        val        = true;
        iconEnable = true;
    }

    if(value == TagPropWidget::IconOnly)
    {
        val        = false;
        iconEnable = true;
    }

    d->titleEdit->setEnabled(val);
    d->keySeqWidget->setEnabled(val);
    d->resetIconButton->setEnabled(iconEnable);
    d->iconButton->setEnabled(iconEnable);
    d->saveButton->setEnabled(iconEnable);
    d->discardButton->setEnabled(iconEnable);
}

} // namespace Digikam
