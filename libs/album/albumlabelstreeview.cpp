/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View.
 *
 * Copyright (C) 2014-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumlabelstreeview.h"

// QT includes

#include <QTreeWidget>
#include <QPainter>
#include <QApplication>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "coredbsearchxml.h"
#include "searchtabheader.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "coredbconstants.h"
#include "imagelister.h"
#include "statesavingobject.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "tagscache.h"
#include "applicationsettings.h"
#include "dnotificationwrapper.h"
#include "digikamapp.h"
#include "ratingwidget.h"
#include "dbjobsmanager.h"

namespace Digikam
{

class AlbumLabelsTreeView::Private
{
public:

    Private() :
        ratings(0),
        picks(0),
        colors(0),
        isCheckableTreeView(false),
        isLoadingState(false),
        iconSizeFromSetting(0)
    {
    }

    QFont                      regularFont;
    QSize                      iconSize;

    QTreeWidgetItem*           ratings;
    QTreeWidgetItem*           picks;
    QTreeWidgetItem*           colors;

    bool                       isCheckableTreeView;
    bool                       isLoadingState;
    int                        iconSizeFromSetting;

    QHash<Labels, QList<int> > selectedLabels;

    static const QString       configRatingSelectionEntry;
    static const QString       configPickSelectionEntry;
    static const QString       configColorSelectionEntry;
    static const QString       configExpansionEntry;
};

const QString AlbumLabelsTreeView::Private::configRatingSelectionEntry(QLatin1String("RatingSelection"));
const QString AlbumLabelsTreeView::Private::configPickSelectionEntry(QLatin1String("PickSelection"));
const QString AlbumLabelsTreeView::Private::configColorSelectionEntry(QLatin1String("ColorSelection"));
const QString AlbumLabelsTreeView::Private::configExpansionEntry(QLatin1String("Expansion"));

AlbumLabelsTreeView::AlbumLabelsTreeView(QWidget* const parent, bool setCheckable) :
    QTreeWidget(parent),
    StateSavingObject(this),
    d(new Private)
{
    d->regularFont         = ApplicationSettings::instance()->getTreeViewFont();
    d->iconSizeFromSetting = ApplicationSettings::instance()->getTreeViewIconSize();
    d->iconSize            = QSize(d->iconSizeFromSetting, d->iconSizeFromSetting);
    d->isCheckableTreeView = setCheckable;

    setHeaderLabel(i18nc("@title", "Labels"));
    setUniformRowHeights(false);
    initTreeView();

    if (d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            if ((*it)->parent())
            {
                (*it)->setFlags((*it)->flags()|Qt::ItemIsUserCheckable);
                (*it)->setCheckState(0, Qt::Unchecked);
            }

            ++it;
        }
    }
    else
    {
        setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSettingsChanged()));
}

AlbumLabelsTreeView::~AlbumLabelsTreeView()
{
    delete d;
}

bool AlbumLabelsTreeView::isCheckable() const
{
    return d->isCheckableTreeView;
}

bool AlbumLabelsTreeView::isLoadingState() const
{
    return d->isLoadingState;
}

QPixmap AlbumLabelsTreeView::goldenStarPixmap(bool fillin) const
{
    QPixmap pixmap = QPixmap(60, 60);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);

    if (fillin)
        p1.setBrush(qApp->palette().color(QPalette::Link));

    QPen pen(palette().color(QPalette::Active, foregroundRole()));
    p1.setPen(pen);

    QMatrix matrix;
    matrix.scale(4, 4);     // 60px/15px (RatingWidget::starPolygon() size is 15*15px)
    p1.setMatrix(matrix);

    p1.drawPolygon(RatingWidget::starPolygon(), Qt::WindingFill);
    p1.end();

    return pixmap;
}

QPixmap AlbumLabelsTreeView::colorRectPixmap(const QColor& color) const
{
    QRect rect(8, 8, 48, 48);
    QPixmap pixmap = QPixmap(60, 60);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(color);
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawRect(rect);
    p1.end();

    return pixmap;
}

QHash<AlbumLabelsTreeView::Labels, QList<int> > AlbumLabelsTreeView::selectedLabels()
{
    QHash<Labels, QList<int> > selectedLabelsHash;
    QList<int> selectedRatings;
    QList<int> selectedPicks;
    QList<int> selectedColors;

    if (d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);

        while(*it)
        {
            QTreeWidgetItem* const item = (*it);

            if(item->parent() == d->ratings)
                selectedRatings << indexFromItem(item).row();
            else if(item->parent() == d->picks)
                selectedPicks << indexFromItem(item).row();
            else
                selectedColors << indexFromItem(item).row();
            ++it;
        }
    }
    else
    {
        foreach (QTreeWidgetItem* const item, selectedItems())
        {
            if(item->parent() == d->ratings)
                selectedRatings << indexFromItem(item).row();
            else if(item->parent() == d->picks)
                selectedPicks << indexFromItem(item).row();
            else
                selectedColors << indexFromItem(item).row();
        }
    }

    selectedLabelsHash[Ratings] = selectedRatings;
    selectedLabelsHash[Picks]   = selectedPicks;
    selectedLabelsHash[Colors]  = selectedColors;

    return selectedLabelsHash;
}

void AlbumLabelsTreeView::doLoadState()
{
    d->isLoadingState                = true;
    KConfigGroup configGroup         = getConfigGroup();
    const QList<int> expansion       = configGroup.readEntry(entryName(d->configExpansionEntry),       QList<int>());
    const QList<int> selectedRatings = configGroup.readEntry(entryName(d->configRatingSelectionEntry), QList<int>());
    const QList<int> selectedPicks   = configGroup.readEntry(entryName(d->configPickSelectionEntry),   QList<int>());
    const QList<int> selectedColors  = configGroup.readEntry(entryName(d->configColorSelectionEntry),  QList<int>());

    d->ratings->setExpanded(true);
    d->picks->setExpanded(true);
    d->colors->setExpanded(true);

    foreach (int parent, expansion)
    {
        switch (parent)
        {
            case 1:
                d->ratings->setExpanded(false);
                break;
            case 2:
                d->picks->setExpanded(false);
                break;
            case 3:
                d->colors->setExpanded(false);
            default:
                break;
        }
    }

    foreach (int rating, selectedRatings)
    {
        if (d->isCheckableTreeView)
            d->ratings->child(rating)->setCheckState(0, Qt::Checked);
        else
            d->ratings->child(rating)->setSelected(true);
    }

    foreach (int pick, selectedPicks)
    {
        if (d->isCheckableTreeView)
            d->picks->child(pick)->setCheckState(0, Qt::Checked);
        else
            d->picks->child(pick)->setSelected(true);
    }

    foreach (int color, selectedColors)
    {
        if (d->isCheckableTreeView)
            d->colors->child(color)->setCheckState(0, Qt::Checked);
        else
            d->colors->child(color)->setSelected(true);
    }

    d->isLoadingState = false;
}

void AlbumLabelsTreeView::doSaveState()
{
    KConfigGroup configGroup = getConfigGroup();
    QList<int> expansion;

    if (!d->ratings->isExpanded())
    {
        expansion << 1;
    }

    if (!d->picks->isExpanded())
    {
        expansion << 2;
    }

    if (!d->colors->isExpanded())
    {
        expansion << 3;
    }

    QHash<Labels, QList<int> > labels = selectedLabels();

    configGroup.writeEntry(entryName(d->configExpansionEntry),       expansion);
    configGroup.writeEntry(entryName(d->configRatingSelectionEntry), labels[Ratings]);
    configGroup.writeEntry(entryName(d->configPickSelectionEntry),   labels[Picks]);
    configGroup.writeEntry(entryName(d->configColorSelectionEntry),  labels[Colors]);
}

void AlbumLabelsTreeView::setCurrentAlbum()
{
    emit signalSetCurrentAlbum();
}

void AlbumLabelsTreeView::initTreeView()
{
    setIconSize(QSize(d->iconSizeFromSetting*5,d->iconSizeFromSetting));
    initRatingsTree();
    initPicksTree();
    initColorsTree();
    expandAll();
    setRootIsDecorated(false);
}

void AlbumLabelsTreeView::initRatingsTree()
{
    d->ratings = new QTreeWidgetItem(this);
    d->ratings->setText(0, i18n("Rating"));
    d->ratings->setFont(0, d->regularFont);
    d->ratings->setFlags(Qt::ItemIsEnabled);

    QTreeWidgetItem* const noRate = new QTreeWidgetItem(d->ratings);
    noRate->setText(0, i18n("No Rating"));
    noRate->setFont(0, d->regularFont);
    QPixmap pix(goldenStarPixmap().size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(palette().color(QPalette::Active, foregroundRole()));
    p.drawPixmap(0, 0, goldenStarPixmap(false));
    noRate->setIcon(0, QIcon(pix));
    noRate->setSizeHint(0, d->iconSize);

    for (int rate = 1 ; rate <= 5 ; rate++)
    {
        QTreeWidgetItem* const rateWidget = new QTreeWidgetItem(d->ratings);

        QPixmap pix(goldenStarPixmap().width()*rate, goldenStarPixmap().height());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        int offset = 0;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(palette().color(QPalette::Active, foregroundRole()));

        for (int i = 0 ; i < rate ; ++i)
        {
            p.drawPixmap(offset, 0, goldenStarPixmap());
            offset += goldenStarPixmap().width();
        }

        rateWidget->setIcon(0, QIcon(pix));
        rateWidget->setSizeHint(0, d->iconSize);
    }
}

void AlbumLabelsTreeView::initPicksTree()
{
    d->picks = new QTreeWidgetItem(this);
    d->picks->setText(0, i18n("Pick"));
    d->picks->setFont(0, d->regularFont);
    d->picks->setFlags(Qt::ItemIsEnabled);

    QStringList pickSetNames;
    pickSetNames << i18n("No Pick")
                 << i18n("Rejected Item")
                 << i18n("Pending Item")
                 << i18n("Accepted Item");

    QStringList pickSetIcons;
    pickSetIcons << QLatin1String("flag-black")
                 << QLatin1String("flag-red")
                 << QLatin1String("flag-yellow")
                 << QLatin1String("flag-green");

    foreach (QString pick, pickSetNames)
    {
        QTreeWidgetItem* const pickWidgetItem = new QTreeWidgetItem(d->picks);
        pickWidgetItem->setText(0, pick);
        pickWidgetItem->setFont(0, d->regularFont);
        pickWidgetItem->setIcon(0, QIcon::fromTheme(pickSetIcons.at(pickSetNames.indexOf(pick))));
    }
}

void AlbumLabelsTreeView::initColorsTree()
{
    d->colors = new QTreeWidgetItem(this);
    d->colors->setText(0, i18n("Color"));
    d->colors->setFont(0, d->regularFont);
    d->colors->setFlags(Qt::ItemIsEnabled);

    QTreeWidgetItem* noColor = new QTreeWidgetItem(d->colors);
    noColor->setText(0, i18n("No Color"));
    noColor->setFont(0, d->regularFont);
    noColor->setIcon(0, QIcon::fromTheme(QLatin1String("emblem-unmounted")));

    QStringList colorSet;
    colorSet << QLatin1String("red")      << QLatin1String("orange")
             << QLatin1String("yellow")   << QLatin1String("darkgreen")
             << QLatin1String("darkblue") << QLatin1String("magenta")
             << QLatin1String("darkgray") << QLatin1String("black")
             << QLatin1String("white");

    QStringList colorSetNames;
    colorSetNames << i18n("Red")    << i18n("Orange")
                  << i18n("Yellow") << i18n("Green")
                  << i18n("Blue")   << i18n("Magenta")
                  << i18n("Gray")   << i18n("Black")
                  << i18n("White");

    foreach (QString color, colorSet)
    {
        QTreeWidgetItem* const colorWidgetItem = new QTreeWidgetItem(d->colors);
        colorWidgetItem->setText(0, colorSetNames.at(colorSet.indexOf(color)));
        colorWidgetItem->setFont(0, d->regularFont);
        QPixmap colorIcon = colorRectPixmap(QColor(color));
        colorWidgetItem->setIcon(0, QIcon(colorIcon));
        colorWidgetItem->setSizeHint(0, d->iconSize);
    }
}

void AlbumLabelsTreeView::slotSettingsChanged()
{
    if (d->iconSizeFromSetting != ApplicationSettings::instance()->getTreeViewIconSize())
    {
        d->iconSizeFromSetting = ApplicationSettings::instance()->getTreeViewIconSize();
        setIconSize(QSize(d->iconSizeFromSetting*5, d->iconSizeFromSetting));
        d->iconSize            = QSize(d->iconSizeFromSetting, d->iconSizeFromSetting);
        QTreeWidgetItemIterator it(this);

        while(*it)
        {
            if (*it)
            {
                (*it)->setSizeHint(0, d->iconSize);
            }

            ++it;
        }
    }

    if (d->regularFont != ApplicationSettings::instance()->getTreeViewFont())
    {
        d->regularFont = ApplicationSettings::instance()->getTreeViewFont();
        QTreeWidgetItemIterator it(this);

        while(*it)
        {
            if (*it)
            {
                (*it)->setFont(0, d->regularFont);
            }

            ++it;
        }
    }
}

void AlbumLabelsTreeView::restoreSelectionFromHistory(QHash<Labels, QList<int> > neededLabels)
{
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);

    while(*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    foreach (int rateItemIndex, neededLabels[Ratings])
    {
        d->ratings->child(rateItemIndex)->setSelected(true);
    }

    foreach (int pickItemIndex, neededLabels[Picks])
    {
        d->picks->child(pickItemIndex)->setSelected(true);
    }

    foreach (int colorItemIndex, neededLabels[Colors])
    {
        d->colors->child(colorItemIndex)->setSelected(true);
    }
}

// -------------------------------------------------------------------------------

class AlbumLabelsSearchHandler::Private
{
public:

    Private() :
        treeWidget(0),
        dbJobThread(0),
        restoringSelectionFromHistory(0),
        currentXmlIsEmpty(0),
        albumForSelectedItems(0)
    {
    }

    AlbumLabelsTreeView*  treeWidget;
    SearchesDBJobsThread* dbJobThread;
    bool                  restoringSelectionFromHistory;
    bool                  currentXmlIsEmpty;
    QString               oldXml;
    Album*                albumForSelectedItems;
    QString               generatedAlbumName;
    QList<QUrl>           urlListForSelectedAlbum;
};

AlbumLabelsSearchHandler::AlbumLabelsSearchHandler(AlbumLabelsTreeView* const treeWidget)
    : d(new Private)
{
    d->treeWidget = treeWidget;

    if (!d->treeWidget->isCheckable())
    {
        connect(d->treeWidget, SIGNAL(itemSelectionChanged()),
                this, SLOT(slotSelectionChanged()));

        connect(d->treeWidget, SIGNAL(signalSetCurrentAlbum()),
                this, SLOT(slotSetCurrentAlbum()));
    }
    else
    {
        connect(d->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
                this, SLOT(slotCheckStateChanged()));
    }
}

AlbumLabelsSearchHandler::~AlbumLabelsSearchHandler()
{
    delete d;
}

Album *AlbumLabelsSearchHandler::albumForSelectedItems() const
{
    return d->albumForSelectedItems;
}

QList<QUrl> AlbumLabelsSearchHandler::imagesUrls() const
{
    return d->urlListForSelectedAlbum;
}

QString AlbumLabelsSearchHandler::generatedName() const
{
    return d->generatedAlbumName;
}

void AlbumLabelsSearchHandler::restoreSelectionFromHistory(const QHash<AlbumLabelsTreeView::Labels, QList<int> >& neededLabels)
{
    d->restoringSelectionFromHistory = true;
    d->treeWidget->restoreSelectionFromHistory(neededLabels);
    d->restoringSelectionFromHistory = false;
    slotSelectionChanged();
}

bool AlbumLabelsSearchHandler::isRestoringSelectionFromHistory() const
{
    return d->restoringSelectionFromHistory;
}

QString AlbumLabelsSearchHandler::createXMLForCurrentSelection(const QHash<AlbumLabelsTreeView::Labels, QList<int> >& selectedLabels)
{
    SearchXmlWriter writer;
    writer.setFieldOperator(SearchXml::standardFieldOperator());
    QList<int>      ratings;
    QList<int>      colorsAndPicks;

    foreach (int rate, selectedLabels[AlbumLabelsTreeView::Ratings])
    {
        if (rate == 0)
        {
            ratings << -1;
        }
        ratings << rate;
    }

    foreach (int color, selectedLabels[AlbumLabelsTreeView::Colors])
    {
        colorsAndPicks << TagsCache::instance()->tagForColorLabel(color);
    }

    foreach (int pick, selectedLabels[AlbumLabelsTreeView::Picks])
    {
        colorsAndPicks << TagsCache::instance()->tagForPickLabel(pick);
    }

    d->currentXmlIsEmpty = (ratings.isEmpty() && colorsAndPicks.isEmpty()) ? true : false;

    if (!ratings.isEmpty() && !colorsAndPicks.isEmpty())
    {
        foreach (int val, ratings)
        {
            writer.writeGroup();
            writer.writeField(QLatin1String("rating"), SearchXml::Equal);
            writer.writeValue(val);
            writer.finishField();

            writer.writeField(QLatin1String("tagid"), SearchXml::InTree);
            writer.writeValue(colorsAndPicks);
            writer.finishField();

            writer.finishGroup();
        }
    }
    else if (!ratings.isEmpty())
    {
        foreach (int rate, ratings)
        {
            writer.writeGroup();
            writer.writeField(QLatin1String("rating"), SearchXml::Equal);
            writer.writeValue(rate);
            writer.finishField();
            writer.finishGroup();
        }
    }
    else if (!colorsAndPicks.isEmpty())
    {
        writer.writeGroup();
        writer.writeField(QLatin1String("tagid"), SearchXml::InTree);
        writer.writeValue(colorsAndPicks);
        writer.finishField();
        writer.finishGroup();
    }
    else
    {
        writer.writeGroup();
        writer.finishGroup();
    }

    writer.finish();

    generateAlbumNameForExporting(selectedLabels[AlbumLabelsTreeView::Ratings],
                                  selectedLabels[AlbumLabelsTreeView::Colors],
                                  selectedLabels[AlbumLabelsTreeView::Picks]);
    return writer.xml();
}

SAlbum* AlbumLabelsSearchHandler::search(const QString& xml) const
{
    SAlbum* album = 0;
    int id;

    if (!d->treeWidget->isCheckable())
    {
        album = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch));

        if (album)
        {
            id = album->id();
            CoreDbAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                              SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }
        else
        {
            id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }

        album = new SAlbum(getDefaultTitle(), id);
    }
    else
    {
        album = AlbumManager::instance()->findSAlbum(getDefaultTitle());

        if (album)
        {
            id = album->id();
            CoreDbAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                              getDefaultTitle(), xml);
        }
        else
        {
            id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                getDefaultTitle(), xml);
        }

        album = new SAlbum(d->generatedAlbumName, id);
    }

    if (!album->isUsedByLabelsTree())
        album->setUsedByLabelsTree(true);

    return album;
}

void AlbumLabelsSearchHandler::generateAlbumNameForExporting(const QList<int>& ratings,
                                                             const QList<int>& colorsList,
                                                             const QList<int>& picksList)
{
    QString name;
    QString ratingsString;
    QString picksString;
    QString colorsString;

    if (!ratings.isEmpty())
    {
        ratingsString += i18n("Rating: ");

        QListIterator<int> it(ratings);

        while (it.hasNext())
        {
            int rating = it.next();

            if (rating == -1)
            {
                ratingsString += i18n("No Rating");
            }
            else
            {
                ratingsString += QString::number(rating);
            }

            if (it.hasNext())
            {
                ratingsString += QLatin1String(", ");
            }
        }
    }

    if (!colorsList.isEmpty())
    {
        colorsString += i18n("Colors: ");

        QListIterator<int> it(colorsList);

        while(it.hasNext())
        {
            switch (it.next())
            {
                case NoColorLabel:
                    colorsString += i18n("No Color");
                    break;
                case RedLabel:
                    colorsString += i18n("Red");
                    break;
                case OrangeLabel:
                    colorsString += i18n("Orange");
                    break;
                case YellowLabel:
                    colorsString += i18n("Yellow");
                    break;
                case GreenLabel:
                    colorsString += i18n("Green");
                    break;
                case BlueLabel:
                    colorsString += i18n("Blue");
                    break;
                case MagentaLabel:
                    colorsString += i18n("Magenta");
                    break;
                case GrayLabel:
                    colorsString += i18n("Gray");
                    break;
                case BlackLabel:
                    colorsString += i18n("Black");
                    break;
                case WhiteLabel:
                    colorsString += i18n("White");
                    break;
                default:
                    break;
            }

            if (it.hasNext())
            {
                colorsString += QLatin1String(", ");
            }
        }
    }

    if (!picksList.isEmpty())
    {
        picksString += i18n("Picks: ");

        QListIterator<int> it(picksList);

        while(it.hasNext())
        {
            switch (it.next())
            {
                case NoPickLabel:
                    picksString += i18n("No Pick");
                    break;
                case RejectedLabel:
                    picksString += i18n("Rejected");
                    break;
                case PendingLabel:
                    picksString += i18n("Pending");
                    break;
                case AcceptedLabel:
                    picksString += i18n("Accepted");
                    break;
                default:
                    break;
            }

            if (it.hasNext())
            {
                picksString += QLatin1String(", ");
            }
        }
    }

    if (ratingsString.isEmpty() && picksString.isEmpty())
    {
        name = colorsString;
    }
    else if (ratingsString.isEmpty() && colorsString.isEmpty())
    {
        name = picksString;
    }
    else if (colorsString.isEmpty() && picksString.isEmpty())
    {
        name = ratingsString;
    }
    else if (ratingsString.isEmpty())
    {
        name = picksString + QLatin1String(" | ") + colorsString;
    }
    else if (picksString.isEmpty())
    {
        name = ratingsString + QLatin1String(" | ") + colorsString;
    }
    else if (colorsString.isEmpty())
    {
        name = ratingsString + QLatin1String(" | ") + picksString;
    }
    else
    {
        name = ratingsString + QLatin1String(" | ") + picksString + QLatin1String(" | ") + colorsString;
    }

    d->generatedAlbumName = name;
}

void AlbumLabelsSearchHandler::imagesUrlsForCurrentAlbum()
{
    SearchesDBJobInfo jobInfo;
    jobInfo.setSearchId( d->albumForSelectedItems->id() );
    jobInfo.setRecursive();

    d->dbJobThread = DBJobsManager::instance()->startSearchesJobThread(jobInfo);

    connect(d->dbJobThread, SIGNAL(finished()),
            this, SLOT(slotResult()));

    connect(d->dbJobThread, SIGNAL(data(QList<ImageListerRecord>)),
            this, SLOT(slotData(QList<ImageListerRecord>)));
}

QString AlbumLabelsSearchHandler::getDefaultTitle() const
{
    if (d->treeWidget->isCheckable())
    {
        return i18n("Exported Labels");
    }
    else
    {
        return i18n("Labels Album");
    }
}

void AlbumLabelsSearchHandler::slotSelectionChanged()
{
    if (d->treeWidget->isLoadingState() || d->restoringSelectionFromHistory)
    {
        return;
    }

    QString xml         = createXMLForCurrentSelection(d->treeWidget->selectedLabels());
    SAlbum* const album = search(xml);

    if (album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
        d->albumForSelectedItems = album;
        d->oldXml                = xml;
    }
}

void AlbumLabelsSearchHandler::slotCheckStateChanged()
{
    QString currentXml = createXMLForCurrentSelection(d->treeWidget->selectedLabels());

    if (currentXml == d->oldXml)
    {
        return;
    }

    if (d->albumForSelectedItems)
    {
        emit checkStateChanged(d->albumForSelectedItems, Qt::Unchecked);
    }

    SAlbum* const album = search(currentXml);

    if (album)
    {
        if(!d->currentXmlIsEmpty)
        {
            d->albumForSelectedItems = album;
            imagesUrlsForCurrentAlbum();
        }
        else
        {
            d->albumForSelectedItems = 0;
        }

        emit checkStateChanged(album, Qt::Checked);
    }

    d->oldXml = currentXml;
}

void AlbumLabelsSearchHandler::slotSetCurrentAlbum()
{
    slotSelectionChanged();
}

void AlbumLabelsSearchHandler::slotResult()
{
    if (d->dbJobThread != sender())
    {
        return;
    }

    if (d->dbJobThread->hasErrors())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to list urls: " << d->dbJobThread->errorsList().first();

        // Pop-up a message about the error.
        DNotificationWrapper(QString(),  d->dbJobThread->errorsList().first(),
                             DigikamApp::instance(), DigikamApp::instance()->windowTitle());
    }
}

void AlbumLabelsSearchHandler::slotData(const QList<ImageListerRecord>& data)
{
    if (d->dbJobThread != sender() || data.isEmpty())
        return;

    QList<QUrl> urlList;

    foreach (const ImageListerRecord &record, data)
    {
        ImageInfo info(record);
        urlList << info.fileUrl();
    }

    d->urlListForSelectedAlbum = urlList;
}

} // namespace Digikam
