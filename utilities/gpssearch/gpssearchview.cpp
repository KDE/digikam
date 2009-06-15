/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search sidebar tab contents.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gpssearchview.h"
#include "gpssearchview.moc"

// Qt includes

#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfo.h"
#include "searchxml.h"
#include "searchtextbar.h"
#include "gpssearchwidget.h"
#include "gpssearchfolderview.h"

namespace Digikam
{

class GPSSearchViewPriv
{

public:

    GPSSearchViewPriv()
    {
        gpsSearchWidget     = 0;
        saveBtn             = 0;
        nameEdit            = 0;
        searchGPSBar        = 0;
        gpsSearchFolderView = 0;
    }

    QToolButton            *saveBtn;

    KLineEdit              *nameEdit;

    SearchTextBar          *searchGPSBar;

    GPSSearchFolderView    *gpsSearchFolderView;

    GPSSearchWidget        *gpsSearchWidget;
};

GPSSearchView::GPSSearchView(QWidget *parent)
             : QWidget(parent), d(new GPSSearchViewPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    // ---------------------------------------------------------------

    QVBoxLayout *vlay  = new QVBoxLayout(this);

    QFrame *mapPanel   = new QFrame(this);
    QVBoxLayout *vlay2 = new QVBoxLayout(mapPanel);
    d->gpsSearchWidget = new GPSSearchWidget(mapPanel);

    mapPanel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mapPanel->setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    vlay2->addWidget(d->gpsSearchWidget);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    KHBox *hbox = new KHBox(this);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    d->nameEdit = new KLineEdit(hbox);
    d->nameEdit->setClearButtonShown(true);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current map search to save in the "
                                   "\"My Map Searches\" view."));

    d->saveBtn  = new QToolButton(hbox);
    d->saveBtn->setIcon(SmallIcon("document-save"));
    d->saveBtn->setEnabled(false);
    d->saveBtn->setToolTip(i18n("Save current map search to a new virtual album."));
    d->saveBtn->setWhatsThis(i18n("If this button is pressed, the current map search "
                                  "will be saved to a new search "
                                  "virtual album using the name "
                                  "set on the left side."));

    // ---------------------------------------------------------------

    d->gpsSearchFolderView = new GPSSearchFolderView(this);
    d->searchGPSBar        = new SearchTextBar(this, "GPSSearchViewSearchGPSBar");

    // ---------------------------------------------------------------

    vlay->addWidget(mapPanel);
    vlay->addWidget(hbox);
    vlay->addWidget(d->gpsSearchFolderView);
    vlay->addWidget(d->searchGPSBar);
    vlay->setStretchFactor(mapPanel, 10);
    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());

    readConfig();

    // ---------------------------------------------------------------

    connect(d->gpsSearchFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->gpsSearchFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->gpsSearchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchGPSBar, SLOT(slotSearchResult(bool)));

    connect(d->searchGPSBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->gpsSearchFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

    connect(d->saveBtn, SIGNAL(clicked()),
            this, SLOT(slotSaveGPSSAlbum()));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditGPSConditions()));

    connect(d->nameEdit, SIGNAL(returnPressed(const QString&)),
            d->saveBtn, SLOT(animateClick()));

    connect(d->gpsSearchWidget, SIGNAL(signalNewSelectionFromMap()),
            this, SLOT(slotSelectionChanged()));

    // ---------------------------------------------------------------

    slotCheckNameEditGPSConditions();
}

GPSSearchView::~GPSSearchView()
{
    writeConfig();
    delete d;
}

void GPSSearchView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("GPSSearch SideBar"));

    d->gpsSearchWidget->zoomView(group.readEntry("Zoom Level", 5));
    // Default GPS location : Paris
    d->gpsSearchWidget->setCenterLongitude(group.readEntry("Longitude", 2.3455810546875));
    d->gpsSearchWidget->setCenterLatitude(group.readEntry("Latitude", 48.850258199721495));
}

void GPSSearchView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("GPSSearch SideBar"));
    group.writeEntry("Zoom Level", d->gpsSearchWidget->zoom());
    group.writeEntry("Longitude",  d->gpsSearchWidget->centerLongitude());
    group.writeEntry("Latitude",   d->gpsSearchWidget->centerLatitude());
    group.sync();
}

GPSSearchFolderView* GPSSearchView::folderView() const
{
    return d->gpsSearchFolderView;
}

SearchTextBar* GPSSearchView::searchBar() const
{
    return d->searchGPSBar;
}

void GPSSearchView::setActive(bool val)
{
    if (d->gpsSearchFolderView->selectedItem())
    {
        d->gpsSearchFolderView->setActive(val);
    }
    else if (val)
    {
        // TODO
    }
}

void GPSSearchView::slotSaveGPSSAlbum()
{
    QString name = d->nameEdit->text();
    if (!checkName(name))
        return;

    createNewGPSSearchAlbum(name);
}

void GPSSearchView::slotSelectionChanged()
{
    slotCheckNameEditGPSConditions();
    createNewGPSSearchAlbum(GPSSearchFolderView::currentGPSSearchName());
}

void GPSSearchView::createNewGPSSearchAlbum(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (!d->gpsSearchWidget->asSelection())
        return;

    // We query database here

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    QList<double> coordinates = d->gpsSearchWidget->selectionCoordinates();

    kDebug(50003) << "West, North, East, South: " << coordinates;

    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField("position", SearchXml::Inside);
    writer.writeAttribute("type", "rectangle");
    writer.writeValue(coordinates);
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::MapSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
}

void GPSSearchView::slotAlbumSelected(SAlbum* salbum)
{
    if (!salbum)
        return;

    AlbumManager::instance()->setCurrentAlbum(salbum);

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type = reader.attributes().value("type");

    if (type == "rectangle")
    {
        QList<double> list;
        list << reader.valueToDoubleList();
        d->gpsSearchWidget->setSelectionCoordinates(list);
        slotCheckNameEditGPSConditions();
    }
}

bool GPSSearchView::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked)
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool GPSSearchView::checkAlbum(const QString& name) const
{
    const AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void GPSSearchView::slotCheckNameEditGPSConditions()
{
    if (d->gpsSearchWidget->asSelection())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
            d->saveBtn->setEnabled(true);
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveBtn->setEnabled(false);
    }
}

void GPSSearchView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    if (salbum->title() == GPSSearchFolderView::currentGPSSearchName())
        return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)",oldName),
                                         i18n("Enter new album name:"),
                                         oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

}  // namespace Digikam
