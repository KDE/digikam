/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View.
 *
 * Copyright (C) 2014 Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "albumlabelstreeview.moc"

// QT includes

#include <QTreeWidget>
#include <QPainter>
#include <QDebug>

// KDE includes

#include <kapplication.h>
#include <KIconLoader>
#include <KUrl>

// Local includes

#include "searchxml.h"
#include "searchtabheader.h"
#include "albummanager.h"
#include "albumtreeview.h"
#include "databaseconstants.h"
#include "imagelister.h"
#include "statesavingobject.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "tagscache.h"
#include "globals.h"

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
        searchHandler(0)
    {
        starPolygon << QPoint(0,  12);
        starPolygon << QPoint(10, 10);
        starPolygon << QPoint(14,  0);
        starPolygon << QPoint(18, 10);
        starPolygon << QPoint(28, 12);
        starPolygon << QPoint(20, 18);
        starPolygon << QPoint(22, 28);
        starPolygon << QPoint(14, 22);
        starPolygon << QPoint(6,  28);
        starPolygon << QPoint(8,  18);
    }

    QFont                        rootFont;
    QFont                        regularFont;
    QSize                        iconSize;
    QSize                        rootSizeHint;

    QPolygon                     starPolygon;

    QTreeWidgetItem*             ratings;
    QTreeWidgetItem*             picks;
    QTreeWidgetItem*             colors;

    bool                         isCheckableTreeView;
    bool                         isLoadingState;

    QHash<QString, QList<int> >  selectedLabels;

    AlbumLabelsSearchHandler*    searchHandler;

    static const QString         configRatingSelectionEntry;
    static const QString         configPickSelectionEntry;
    static const QString         configColorSelectionEntry;
    static const QString         configExpansionEntry;
};

const QString AlbumLabelsTreeView::Private::configRatingSelectionEntry("RatingSelection");
const QString AlbumLabelsTreeView::Private::configPickSelectionEntry("PickSelection");
const QString AlbumLabelsTreeView::Private::configColorSelectionEntry("ColorSelection");
const QString AlbumLabelsTreeView::Private::configExpansionEntry("Expansion");

AlbumLabelsTreeView::AlbumLabelsTreeView(QWidget *parent, bool setCheckable) :
    QTreeWidget(parent), StateSavingObject(this), d(new Private)
{
    d->rootFont            = QFont("Times",18,-1,false);
    d->regularFont         = QFont("Times",12,-1,false);
    d->iconSize            = QSize(30 ,30);
    d->rootSizeHint        = QSize(1,40);
    d->isCheckableTreeView = setCheckable;

    setHeaderLabel("Labels");
    setUniformRowHeights(false);
    initTreeView();

    if(d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this);
        while(*it)
        {
            if((*it)->parent())
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
}

AlbumLabelsTreeView::~AlbumLabelsTreeView()
{
    delete d;
}

bool AlbumLabelsTreeView::isCheckable()
{
    return d->isCheckableTreeView;
}

bool AlbumLabelsTreeView::isLoadingState()
{
    return d->isLoadingState;
}

void AlbumLabelsTreeView::initTreeView()
{
    initRatingsTree();
    initPicksTree();
    initColorsTree();
    expandAll();
    setRootIsDecorated(false);
}

QPixmap AlbumLabelsTreeView::goldenStarPixmap()
{
    QPixmap pixmap = QPixmap(30, 30);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(QColor(0xff,0xd7,0x00));
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawPolygon(d->starPolygon, Qt::WindingFill);
    p1.end();

    return pixmap;
}

QPixmap AlbumLabelsTreeView::colorRectPixmap(QColor color)
{
    QRect rect(4,4,24,24);
    QPixmap pixmap = QPixmap(30, 30);
    pixmap.fill(Qt::transparent);

    QPainter p1(&pixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(color);
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawRect(rect);
    p1.end();

    return pixmap;
}

QHash<QString, QList<int> > AlbumLabelsTreeView::selectedLabels()
{
    QHash<QString, QList<int> > selectedLabelsHash;
    QList<int> selectedRatings;
    QList<int> selectedPicks;
    QList<int> selectedColors;

    if(d->isCheckableTreeView)
    {
        QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Checked);
        while(*it)
        {
            QTreeWidgetItem* item = (*it);

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
        foreach (QTreeWidgetItem* item, selectedItems())
        {
            if(item->parent() == d->ratings)
                selectedRatings << indexFromItem(item).row();
            else if(item->parent() == d->picks)
                selectedPicks << indexFromItem(item).row();
            else
                selectedColors << indexFromItem(item).row();
        }
    }

    selectedLabelsHash["ratings"] = selectedRatings;
    selectedLabelsHash["picks"]   = selectedPicks;
    selectedLabelsHash["colors"]  = selectedColors;

    return selectedLabelsHash;
}

void AlbumLabelsTreeView::doLoadState()
{
    d->isLoadingState = true;
    KConfigGroup configGroup         = getConfigGroup();
    const QList<int> expansion       = configGroup.readEntry(entryName(d->configExpansionEntry), QList<int>());
    const QList<int> selectedRatings = configGroup.readEntry(entryName(d->configRatingSelectionEntry), QList<int>());
    const QList<int> selectedPicks   = configGroup.readEntry(entryName(d->configPickSelectionEntry), QList<int>());
    const QList<int> selectedColors  = configGroup.readEntry(entryName(d->configColorSelectionEntry), QList<int>());

    d->ratings->setExpanded(true);
    d->picks->setExpanded(true);
    d->colors->setExpanded(true);

    foreach (int parent, expansion)
    {
        switch (parent) {
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

    foreach (int rating, selectedRatings) {
        d->ratings->child(rating)->setSelected(true);
    }

    foreach (int pick, selectedPicks) {
        d->picks->child(pick)->setSelected(true);
    }

    foreach (int color, selectedColors) {
        d->colors->child(color)->setSelected(true);
    }

    d->isLoadingState = false;
}

void AlbumLabelsTreeView::doSaveState()
{
    KConfigGroup configGroup = getConfigGroup();
    QList<int> expansion;

    if(!d->ratings->isExpanded())
    {
        expansion << 1;
    }

    if(!d->picks->isExpanded())
    {
        expansion << 2;
    }

    if(!d->colors->isExpanded())
    {
        expansion << 3;
    }

    QHash<QString, QList<int> > labels =  selectedLabels();

    configGroup.writeEntry(entryName(d->configExpansionEntry), expansion);
    configGroup.writeEntry(entryName(d->configRatingSelectionEntry), labels["ratings"]);
    configGroup.writeEntry(entryName(d->configPickSelectionEntry), labels["picks"]);
    configGroup.writeEntry(entryName(d->configColorSelectionEntry), labels["colors"]);
}

void AlbumLabelsTreeView::setCurrentAlbum()
{
    emit signalSetCurrentAlbum();
}

void AlbumLabelsTreeView::initRatingsTree()
{
    d->ratings = new QTreeWidgetItem(this);
    d->ratings->setText(0, tr("Rating"));
    d->ratings->setSizeHint(0,d->rootSizeHint);
    d->ratings->setFont(0,d->rootFont);
    d->ratings->setFlags(Qt::ItemIsEnabled);
    setIconSize(QSize(100,20));

    QTreeWidgetItem* noRate = new QTreeWidgetItem(d->ratings);
    noRate->setText(0,tr("No Rating"));
    noRate->setIcon(0,KIconLoader::global()->loadIcon("emblem-unmounted", KIconLoader::NoGroup, 20));
    noRate->setFont(0,d->regularFont);

    QList<int> ratings;
    ratings << 1 << 2 << 3 << 4 << 5;
    foreach(int rate, ratings)
    {
        QTreeWidgetItem* rateWidget = new QTreeWidgetItem(d->ratings);

        QPixmap pix(goldenStarPixmap().width()*rate,goldenStarPixmap().height());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        int offset = 0;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(palette().color(QPalette::Active, foregroundRole()));

        for(int i = 0 ; i < rate ; ++i)
        {
            p.drawPixmap(offset,0,goldenStarPixmap());
            offset += goldenStarPixmap().width();
        }

        rateWidget->setIcon(0,QIcon(pix));
        rateWidget->setSizeHint(0,QSize(1,20));
    }
}

void AlbumLabelsTreeView::initPicksTree()
{
    d->picks = new QTreeWidgetItem(this);
    d->picks->setText(0, i18n("Pick"));
    d->picks->setSizeHint(0,d->rootSizeHint);
    d->picks->setFont(0,d->rootFont);
    d->picks->setFlags(Qt::ItemIsEnabled);

    QStringList pickSetNames;
    pickSetNames << "No Pick" << "Rejected Item" << "Pending Item" << "Accepted Item";

    QStringList pickSetIcons;
    pickSetIcons << "emblem-unmounted" << "flag-red" << "flag-yellow" << "flag-green";

    foreach (QString pick, pickSetNames) {
        QTreeWidgetItem* pickWidgetItem = new QTreeWidgetItem(d->picks);
        pickWidgetItem->setText(0,pick);
        pickWidgetItem->setFont(0,d->regularFont);
        pickWidgetItem->setIcon(0,KIconLoader::global()->loadIcon(pickSetIcons.at(pickSetNames.indexOf(pick)), KIconLoader::NoGroup, 20));
    }
}

void AlbumLabelsTreeView::initColorsTree()
{
    d->colors = new QTreeWidgetItem(this);
    d->colors->setText(0, tr("Colors"));
    d->colors->setSizeHint(0,d->rootSizeHint);
    d->colors->setFont(0,d->rootFont);
    d->colors->setFlags(Qt::ItemIsEnabled);

    QTreeWidgetItem* noColor = new QTreeWidgetItem(d->colors);
    noColor->setText(0,tr("No Color"));
    noColor->setFont(0,d->regularFont);
    noColor->setIcon(0,KIconLoader::global()->loadIcon("emblem-unmounted", KIconLoader::NoGroup, 20));

    QStringList colorSet;
    colorSet << "red" << "orange" << "yellow" << "darkgreen" << "darkblue" << "magenta" << "darkgray" << "black" << "white";

    QStringList colorSetNames;
    colorSetNames << "Red"  << "Orange" << "Yellow" << "Green" << "Blue" << "Magenta" << "Gray" << "Black" << "White";

    foreach (QString color, colorSet) {
        QTreeWidgetItem* colorWidgetItem = new QTreeWidgetItem(d->colors);
        colorWidgetItem->setText(0,colorSetNames.at(colorSet.indexOf(color)));
        colorWidgetItem->setFont(0,d->regularFont);
        QPixmap colorIcon = colorRectPixmap(QColor(color));
        colorWidgetItem->setIcon(0,QIcon(colorIcon));
        colorWidgetItem->setSizeHint(0,QSize(1,20));
    }
}

void AlbumLabelsTreeView::restoreSelectionFromHistory(QHash<QString, QList<int> > neededLabels)
{
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
    while(*it)
    {
        (*it)->setSelected(false);
        ++it;
    }

    foreach (int rateItemIndex, neededLabels["ratings"])
    {
        d->ratings->child(rateItemIndex)->setSelected(true);
    }

    foreach (int pickItemIndex, neededLabels["picks"])
    {
        d->picks->child(pickItemIndex)->setSelected(true);
    }

    foreach (int colorItemIndex, neededLabels["colors"])
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
        restoringSelectionFromHistory(0),
        albumForSelectedItems(0)
    {}


    AlbumLabelsTreeView* treeWidget;
    bool                 restoringSelectionFromHistory;
    bool                 currentXmlIsEmpty;
    QString              oldXml;
    Album*               albumForSelectedItems;
    QString              generatedAlbumName;
    KUrl::List           urlListForSelectedAlbum;
};

AlbumLabelsSearchHandler::AlbumLabelsSearchHandler(AlbumLabelsTreeView *treeWidget) :
    d(new Private)
{
    d->treeWidget = treeWidget;

    if(!d->treeWidget->isCheckable())
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

Album *AlbumLabelsSearchHandler::albumForSelectedItems()
{
    return d->albumForSelectedItems;
}

KUrl::List AlbumLabelsSearchHandler::imagesUrls()
{
    return d->urlListForSelectedAlbum;
}

QString AlbumLabelsSearchHandler::generatedName()
{
    return d->generatedAlbumName;
}

void AlbumLabelsSearchHandler::restoreSelectionFromHistory(QHash<QString, QList<int> > neededLabels)
{
    d->restoringSelectionFromHistory = true;
    d->treeWidget->restoreSelectionFromHistory(neededLabels);
    d->restoringSelectionFromHistory = false;
    slotSelectionChanged();
}

bool AlbumLabelsSearchHandler::isRestoringSelectionFromHistory()
{
    return d->restoringSelectionFromHistory;
}

QString AlbumLabelsSearchHandler::createXMLForCurrentSelection(QHash<QString, QList<int> > selectedLabels)
{
    SearchXmlWriter writer;
    writer.setFieldOperator(SearchXml::standardFieldOperator());
    QList<int>        ratings;
    QList<int>        colorsAndPicks;

    foreach (int rate, selectedLabels["ratings"])
    {
        if(rate == 0)
            ratings << -1;
        else
            ratings << rate;
    }

    foreach (int color, selectedLabels["colors"])
    {
        colorsAndPicks << TagsCache::instance()->tagForColorLabel(color);
    }

    foreach (int pick, selectedLabels["picks"])
    {
        colorsAndPicks << TagsCache::instance()->tagForPickLabel(pick);
    }

    d->currentXmlIsEmpty = (ratings.isEmpty() && colorsAndPicks.isEmpty()) ? true : false;

    if(!ratings.isEmpty() && !colorsAndPicks.isEmpty())
    {
        foreach (int val, ratings)
        {
            writer.writeGroup();
            writer.writeField("rating", SearchXml::Equal);
            writer.writeValue(val);
            writer.finishField();

            writer.writeField("tagid",SearchXml::InTree);
            writer.writeValue(colorsAndPicks);
            writer.finishField();

            writer.finishGroup();
        }
    }
    else if(!ratings.isEmpty())
    {
        foreach (int rate, ratings) {
            writer.writeGroup();
            writer.writeField("rating", SearchXml::Equal);
            writer.writeValue(rate);
            writer.finishField();
            writer.finishGroup();
        }
    }
    else if(!colorsAndPicks.isEmpty())
    {
        writer.writeGroup();
        writer.writeField("tagid",SearchXml::InTree);
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

    generateAlbumNameForExporting(selectedLabels["ratings"], selectedLabels["colors"], selectedLabels["picks"]);
    return writer.xml();
}

SAlbum* AlbumLabelsSearchHandler::search(const QString &xml)
{
    SAlbum* album;
    int id;

    if(!d->treeWidget->isCheckable())
    {
        album = AlbumManager::instance()->findSAlbum(SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch));

        if(album)
        {
            id = album->id();
            DatabaseAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                                SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }
        else
        {
            id = DatabaseAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                  SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), xml);
        }
        album = new SAlbum(getDefaultTitle(false), id);
    }
    else
    {
        album = AlbumManager::instance()->findSAlbum(getDefaultTitle());

        if(album)
        {
            id = album->id();
            DatabaseAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                                getDefaultTitle(true), xml);
        }
        else
        {
            id = DatabaseAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                                  getDefaultTitle(true), xml);
        }
        album = new SAlbum(d->generatedAlbumName, id);
    }

    if(!album->isUsedByLabelsTree())
        album->setUsedByLabelsTree(true);

    return album;
}

void AlbumLabelsSearchHandler::generateAlbumNameForExporting(QList<int> ratings, QList<int> colorsList, QList<int> picksList)
{
    QString name;
    QString ratingsString;
    QString picksString;
    QString colorsString;

    if(!ratings.isEmpty())
    {
        ratingsString += i18n("Rating: ");

        QListIterator<int> it(ratings);

        while (it.hasNext())
        {
            int rating = it.next();
            if(rating == -1)
            {
                ratingsString += i18n("No Rating");
            }
            else
            {
                ratingsString += QString::number(rating);
            }

            if(it.hasNext())
            {
                ratingsString +=", ";
            }
        }
    }

    if(!colorsList.isEmpty())
    {
        colorsString += i18n("Colors: ");

        QListIterator<int> it(colorsList);

        while (it.hasNext())
        {
            switch (it.next()) {
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

            if(it.hasNext())
            {
                colorsString +=", ";
            }
        }
    }

    if(!picksList.isEmpty())
    {
        picksString += i18n("Picks: ");

        QListIterator<int> it(picksList);
        while (it.hasNext())
        {
            switch (it.next()) {
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

            if(it.hasNext())
            {
                picksString +=", ";
            }
        }
    }


    if(ratingsString.isEmpty() && picksString.isEmpty())
    {
        name = colorsString;
    }
    else if(ratingsString.isEmpty() && colorsString.isEmpty())
    {
        name = picksString;
    }
    else if(colorsString.isEmpty() && picksString.isEmpty())
    {
        name = ratingsString;
    }
    else if(ratingsString.isEmpty())
    {
        name = picksString + " | " + colorsString;
    }
    else if(picksString.isEmpty())
    {
        name = ratingsString + " | " + colorsString;
    }
    else if(colorsString.isEmpty())
    {
        name = ratingsString + " | " + picksString;
    }
    else
    {
        name = ratingsString + " | " + picksString + " | " + colorsString;
    }

    d->generatedAlbumName = name;

}

void AlbumLabelsSearchHandler::imagesUrlsForCurrentAlbum()
{
    KUrl url = d->albumForSelectedItems->databaseUrl();
    KIO::TransferJob* job = ImageLister::startListJob(url);
    job->addMetaData("listAlbumsRecursively", "true");

    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotData(KIO::Job*,QByteArray)));
}

QString AlbumLabelsSearchHandler::getDefaultTitle(bool isCheckable)
{
    if(isCheckable)
    {
        return i18n("Exporting Album");
    }
    else
    {
        return i18n("Labels Album");
    }
}

void AlbumLabelsSearchHandler::slotSelectionChanged()
{
    if(d->treeWidget->isLoadingState() || d->restoringSelectionFromHistory)
    {
        return;
    }

    QString xml   = createXMLForCurrentSelection(d->treeWidget->selectedLabels());
    SAlbum* album = search(xml);


    if(album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
        d->albumForSelectedItems = album;
        d->oldXml = xml;
    }
}

void AlbumLabelsSearchHandler::slotCheckStateChanged()
{
    QString currentXml = createXMLForCurrentSelection(d->treeWidget->selectedLabels());

    if(currentXml == d->oldXml)
    {
        return;
    }

    if(d->albumForSelectedItems)
    {
        emit checkStateChanged(d->albumForSelectedItems,Qt::Unchecked);
    }

    SAlbum* album = search(currentXml);

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

        emit checkStateChanged(album,Qt::Checked);
    }

    d->oldXml   = currentXml;
}

void AlbumLabelsSearchHandler::slotSetCurrentAlbum()
{
    slotSelectionChanged();
}

void AlbumLabelsSearchHandler::slotData(KIO::Job *job, QByteArray data)
{
    Q_UNUSED(job);
    if (data.isEmpty())
    {
        return;
    }

    QByteArray    tmp(data);
    QDataStream   ds(&tmp, QIODevice::ReadOnly);
    KUrl::List urlList;

    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        ImageInfo info(record);
        urlList << info.fileUrl();
    }

    d->urlListForSelectedAlbum = urlList;
}

} // namespace Digikam
