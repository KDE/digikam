/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "textfilter.h"

// Qt includes

#include <QAction>
#include <QToolButton>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class TextFilter::Private
{
public:

    Private()
    {
        itemNameAction        = 0;
        itemTitleAction       = 0;
        itemCommentAction     = 0;
        tagNameAction         = 0;
        albumNameAction       = 0;
        optionsBtn            = 0;
        optionsMenu           = 0;
        searchTextBar         = 0;
        itemAspectRatioAction = 0;
        itemPixelSizeAction   = 0;
        clearAllAction        = 0;
        selAllAction          = 0;
    }

    QAction*       itemNameAction;
    QAction*       itemTitleAction;
    QAction*       itemCommentAction;
    QAction*       tagNameAction;
    QAction*       albumNameAction;
    QAction*       itemAspectRatioAction;
    QAction*       itemPixelSizeAction;
    QAction*       clearAllAction;
    QAction*       selAllAction;

    QToolButton*   optionsBtn;

    QMenu*         optionsMenu;

    SearchTextBar* searchTextBar;
};

TextFilter::TextFilter(QWidget* const parent)
    : DHBox(parent), d(new Private)
{
    d->searchTextBar = new SearchTextBar(this, QLatin1String("AlbumIconViewFilterSearchTextBar"));
    d->searchTextBar->setTextQueryCompletion(true);
    d->searchTextBar->setToolTip(i18n("Text quick filter (search)"));
    d->searchTextBar->setWhatsThis(i18n("Enter search patterns to quickly filter this view on "
                                        "item names, captions (comments), and tags"));

    d->optionsBtn = new QToolButton(this);
    d->optionsBtn->setToolTip( i18n("Text Search Fields"));
    d->optionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->optionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->optionsBtn->setWhatsThis(i18n("Defines which fields to search for the text in."));

    d->optionsMenu           = new QMenu(d->optionsBtn);
    d->itemNameAction        = d->optionsMenu->addAction(i18n("Item Name"));
    d->itemNameAction->setCheckable(true);
    d->itemTitleAction       = d->optionsMenu->addAction(i18n("Item Title"));
    d->itemTitleAction->setCheckable(true);
    d->itemCommentAction     = d->optionsMenu->addAction(i18n("Item Comment"));
    d->itemCommentAction->setCheckable(true);
    d->tagNameAction         = d->optionsMenu->addAction(i18n("Tag Name"));
    d->tagNameAction->setCheckable(true);
    d->albumNameAction       = d->optionsMenu->addAction(i18n("Album Name"));
    d->albumNameAction->setCheckable(true);
    d->itemAspectRatioAction = d->optionsMenu->addAction(i18n("Item Aspect Ratio"));
    d->itemAspectRatioAction->setCheckable(true);
    d->itemPixelSizeAction   = d->optionsMenu->addAction(i18n("Item Pixel Size"));
    d->itemPixelSizeAction->setCheckable(true);
    d->optionsMenu->addSeparator();
    d->clearAllAction        = d->optionsMenu->addAction(i18n("Clear All"));
    d->clearAllAction->setCheckable(false);
    d->selAllAction          = d->optionsMenu->addAction(i18n("Select All"));
    d->selAllAction->setCheckable(false);
    d->optionsBtn->setMenu(d->optionsMenu);

    setContentsMargins(QMargins());
    setSpacing(0);

    connect(d->searchTextBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextFieldsChanged()));

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSearchFieldsChanged(QAction*)));
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

    if (d->itemNameAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImageName;
    }
    if (d->itemTitleAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImageTitle;
    }
    if (d->itemCommentAction->isChecked())
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
    if (d->itemAspectRatioAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImageAspectRatio;
    }
    if (d->itemPixelSizeAction->isChecked())
    {
        fields |= SearchTextFilterSettings::ImagePixelSize;
    }

    return (SearchTextFilterSettings::TextFilterFields)fields;
}

void TextFilter::setsearchTextFields(SearchTextFilterSettings::TextFilterFields fields)
{
    d->itemNameAction->setChecked(fields & SearchTextFilterSettings::ImageName);
    d->itemTitleAction->setChecked(fields & SearchTextFilterSettings::ImageTitle);
    d->itemCommentAction->setChecked(fields & SearchTextFilterSettings::ImageComment);
    d->tagNameAction->setChecked(fields & SearchTextFilterSettings::TagName);
    d->albumNameAction->setChecked(fields & SearchTextFilterSettings::AlbumName);
    d->itemAspectRatioAction->setChecked(fields & SearchTextFilterSettings::ImageAspectRatio);
    d->itemPixelSizeAction->setChecked(fields & SearchTextFilterSettings::ImagePixelSize);
}

void TextFilter::checkMenuActions(bool checked)
{
    d->itemNameAction->setChecked(checked);
    d->itemTitleAction->setChecked(checked);
    d->itemCommentAction->setChecked(checked);
    d->tagNameAction->setChecked(checked);
    d->albumNameAction->setChecked(checked);
    d->itemAspectRatioAction->setChecked(checked);
    d->itemPixelSizeAction->setChecked(checked);
}

void TextFilter::slotSearchFieldsChanged(QAction* action)
{
    if (action == d->clearAllAction)
    {
        checkMenuActions(false);
    }
    if (action == d->selAllAction)
    {
        checkMenuActions(true);
    }

    slotSearchTextFieldsChanged();
}

void TextFilter::slotSearchTextFieldsChanged()
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
