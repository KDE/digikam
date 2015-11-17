/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-17
 * Description : Qt item view for images - category drawer
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importcategorydrawer.h"

// Qt includes

#include <QPainter>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "importcategorizedview.h"
#include "camitemsortsettings.h"
#include "importfiltermodel.h"
#include "imagescanner.h"

namespace Digikam
{

class ImportCategoryDrawer::Private
{
public:

    Private()
    {
        lowerSpacing = 0;
        view         = 0;
    }

    QFont                  font;
    QRect                  rect;
    QPixmap                pixmap;
    int                    lowerSpacing;
    ImportCategorizedView* view;
};

ImportCategoryDrawer::ImportCategoryDrawer(ImportCategorizedView* const parent)
    : DCategoryDrawer(0),
      d(new Private)
{
    d->view = parent;
}

ImportCategoryDrawer::~ImportCategoryDrawer()
{
    delete d;
}

int ImportCategoryDrawer::categoryHeight(const QModelIndex& /*index*/, const QStyleOption& /*option*/) const
{
    return d->rect.height() + d->lowerSpacing;
}

int ImportCategoryDrawer::maximumHeight() const
{
    return d->rect.height() + d->lowerSpacing;
}

void ImportCategoryDrawer::setLowerSpacing(int spacing)
{
    d->lowerSpacing = spacing;
}

void ImportCategoryDrawer::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    d->font = option.font;

    if (option.rect.width() != d->rect.width())
    {
        updateRectsAndPixmaps(option.rect.width());
    }
}

void ImportCategoryDrawer::invalidatePaintingCache()
{
    if (d->rect.isEmpty())
    {
        return;
    }

    updateRectsAndPixmaps(d->rect.width());
}

void ImportCategoryDrawer::drawCategory(const QModelIndex& index, int /*sortRole*/,
                                       const QStyleOption& option, QPainter* p) const
{
    if (option.rect.width() != d->rect.width())
    {
        const_cast<ImportCategoryDrawer*>(this)->updateRectsAndPixmaps(option.rect.width());
    }

    p->save();

    p->translate(option.rect.topLeft());

    CamItemSortSettings::CategorizationMode mode =
        (CamItemSortSettings::CategorizationMode)index.data(ImportFilterModel::CategorizationModeRole).toInt();

    p->drawPixmap(0, 0, d->pixmap);

    QFont fontBold(d->font);
    QFont fontNormal(d->font);
    fontBold.setBold(true);
    int fnSize = fontBold.pointSize();

    //    bool usePointSize;
    if (fnSize > 0)
    {
        fontBold.setPointSize(fnSize+2);
        //        usePointSize = true;
    }
    else
    {
        fnSize = fontBold.pixelSize();
        fontBold.setPixelSize(fnSize+2);
        //        usePointSize = false;
    }

    QString header;
    QString subLine;

    switch (mode)
    {
        case CamItemSortSettings::NoCategories:
            break;
        case CamItemSortSettings::CategoryByFolder:
            viewHeaderText(index, &header, &subLine);
            break;
        case CamItemSortSettings::CategoryByFormat:
            textForFormat(index, &header, &subLine);
            break;
        case CamItemSortSettings::CategoryByDate:
            textForDate(index, &header, &subLine);
            break;
    }

    p->setPen(qApp->palette().color(QPalette::HighlightedText));
    p->setFont(fontBold);

    QRect tr;
    p->drawText(5, 5, d->rect.width(), d->rect.height(),
                Qt::AlignLeft | Qt::AlignTop,
                header, &tr);

    int y = tr.height() + 2;

    p->setFont(fontNormal);

    p->drawText(5, y, d->rect.width(), d->rect.height() - y,
                Qt::AlignLeft | Qt::AlignVCenter, subLine);

    p->restore();
}

void ImportCategoryDrawer::viewHeaderText(const QModelIndex& index, QString* header, QString* subLine) const
{
    ImportImageModel* sourceModel = index.data(ImportImageModel::ImportImageModelPointerRole).value<ImportImageModel*>();

    if (!sourceModel)
    {
        return;
    }

    CamItemInfo info = sourceModel->retrieveCamItemInfo(index);

    if (!info.isNull())
    {
        *header      = info.url().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).fileName();
        int count    = d->view->categoryRange(index).height();
        *subLine     = i18np("1 Item", "%1 Items", count);
    }
}

void ImportCategoryDrawer::textForFormat(const QModelIndex& index, QString* header, QString* subLine) const
{
    QString format = index.data(ImportFilterModel::CategoryFormatRole).toString();

    if (!format.isEmpty())
    {
        format     = format.split(QLatin1Char('/')).at(1);
        format     = ImageScanner::formatToString(format);
        *header    = format;
    }
    else
    {
        format     = i18n("Unknown Format");
        *header    = format;
    }

    int count      = d->view->categoryRange(index).height();
    *subLine       = i18np("1 Item", "%1 Items", count);
}

void ImportCategoryDrawer::textForDate(const QModelIndex& index, QString* header, QString* subLine) const
{
    QDate date = index.data(ImportFilterModel::CategoryDateRole).toDate();

    *header    = date.toString(QLatin1String("dd MMM yyyy"));
    int count  = d->view->categoryRange(index).height();
    *subLine   = i18np("1 Item", "%1 Items", count);
}

void ImportCategoryDrawer::updateRectsAndPixmaps(int width)
{
    d->rect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------

    QFont fn(d->font);
    int   fnSize = fn.pointSize();
    bool  usePointSize;

    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, width,
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               QLatin1String("XXX"));
    d->rect.setHeight(tr.height());

    if (usePointSize)
    {
        fn.setPointSize(d->font.pointSize());
    }
    else
    {
        fn.setPixelSize(d->font.pixelSize());
    }

    fn.setBold(false);
    fm = QFontMetrics(fn);
    tr = fm.boundingRect(0, 0, width,
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         QLatin1String("XXX"));

    d->rect.setHeight(d->rect.height() + tr.height() + 10);
    d->rect.setWidth(width);

    d->pixmap = QPixmap(d->rect.width(), d->rect.height());
    d->pixmap.fill(qApp->palette().color(QPalette::Highlight));
}

} // namespace Digikam
