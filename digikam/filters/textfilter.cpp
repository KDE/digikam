/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "textfilter.moc"

// Qt includes

#include <QAction>
#include <QToolButton>

// KDE includes

#include <klocale.h>
#include <kmenu.h>
#include <kiconloader.h>

namespace Digikam
{

class TextFilter::TextFilterPriv
{
public:

    TextFilterPriv()
    {
        imageNameAction    = 0;
        imageCommentAction = 0;
        tagNameAction      = 0;
        albumNameAction    = 0;
        optionsBtn         = 0;
        optionsMenu        = 0;
        searchTextBar      = 0;
    }

    QAction*       imageNameAction;
    QAction*       imageCommentAction;
    QAction*       tagNameAction;
    QAction*       albumNameAction;

    QToolButton*   optionsBtn;

    KMenu*         optionsMenu;

    SearchTextBar* searchTextBar;
};

TextFilter::TextFilter(QWidget* parent)
    : KHBox(parent), d(new TextFilterPriv)
{
    d->searchTextBar = new SearchTextBar(this, "AlbumIconViewFilterSearchTextBar");
    d->searchTextBar->setTextQueryCompletion(true);
    d->searchTextBar->setToolTip(i18n("Text quick filter (search)"));
    d->searchTextBar->setWhatsThis(i18n("Enter search patterns to quickly filter this view on "
                                        "file names, captions (comments), and tags"));

    d->optionsBtn = new QToolButton(this);
    d->optionsBtn->setToolTip( i18n("Text Search Fields"));
    d->optionsBtn->setIcon(KIconLoader::global()->loadIcon("configure", KIconLoader::Toolbar));
    d->optionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->optionsBtn->setWhatsThis(i18n("Defines where text must be search in fields"));

    d->optionsMenu        = new KMenu(d->optionsBtn);
    d->imageNameAction    = d->optionsMenu->addAction(i18n("Image Name"));
    d->imageNameAction->setCheckable(true);
    d->imageCommentAction = d->optionsMenu->addAction(i18n("Image Comment"));
    d->imageCommentAction->setCheckable(true);
    d->tagNameAction      = d->optionsMenu->addAction(i18n("Tag Name"));
    d->tagNameAction->setCheckable(true);
    d->albumNameAction    = d->optionsMenu->addAction(i18n("Album Name"));
    d->albumNameAction->setCheckable(true);
    d->optionsBtn->setMenu(d->optionsMenu);

    setMargin(0);
    setSpacing(0);

    connect(d->searchTextBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchFieldsChanged()));

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSearchFieldsChanged()));
}

TextFilter::~TextFilter()
{
    delete d;
}

SearchTextBar* TextFilter::searchTextBar() const
{
    return d->searchTextBar;
}

SearchTextFilterSettings::TextFilterFields TextFilter::searchTextFields()
{
    int fields = SearchTextFilterSettings::None;

    if (d->imageNameAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImageName;
    }
    if (d->imageCommentAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImageComment;
    }
    if (d->tagNameAction->isChecked())
    {
        fields |= SearchTextFilterSettings::TagName;
    }
    if (d->albumNameAction->isChecked())
    {
        fields |= SearchTextFilterSettings::AlbumName;
    }

    return (SearchTextFilterSettings::TextFilterFields)fields;
}

void TextFilter::setsearchTextFields(SearchTextFilterSettings::TextFilterFields fields)
{
    d->imageNameAction->setChecked(fields & SearchTextFilterSettings::ImageName);
    d->imageCommentAction->setChecked(fields & SearchTextFilterSettings::ImageComment);
    d->tagNameAction->setChecked(fields & SearchTextFilterSettings::TagName);
    d->albumNameAction->setChecked(fields & SearchTextFilterSettings::AlbumName);
}

void TextFilter::slotSearchFieldsChanged()
{
    SearchTextFilterSettings settings(d->searchTextBar->searchTextSettings());
    settings.textFields = searchTextFields();

    emit signalSearchTextFilterSettings(settings);
}

void TextFilter::reset()
{
    d->searchTextBar->setText(QString());
    setsearchTextFields(SearchTextFilterSettings::All);
}

}  // namespace Digikam
