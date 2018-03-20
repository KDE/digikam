/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Album
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Album Public License for more details.
 *
 * ============================================================ */

#include "setup.h"

// Qt includes

#include <QPointer>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "thumbsgenerator.h"
#include "setupalbumview.h"
#include "setupcamera.h"
#include "setupcollections.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setuplighttable.h"
#include "setupmetadata.h"
#include "setupmisc.h"
#include "setupslideshow.h"
#include "setupimagequalitysorter.h"
#include "setuptooltip.h"
#include "setupdatabase.h"
#include "importsettings.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Setup::Private
{
public:

    Private() :
        page_database(0),
        page_collections(0),
        page_albumView(0),
        page_tooltip(0),
        page_metadata(0),
        page_template(0),
        page_lighttable(0),
        page_editor(0),
        page_slideshow(0),
        page_imagequalitysorter(0),
        page_icc(0),
        page_camera(0),

        page_misc(0),
        databasePage(0),
        collectionsPage(0),
        albumViewPage(0),
        tooltipPage(0),
        metadataPage(0),
        templatePage(0),
        lighttablePage(0),
        editorPage(0),
        slideshowPage(0),
        imageQualitySorterPage(0),
        iccPage(0),
        cameraPage(0),

        miscPage(0)
    {
    }

    DConfigDlgWdgItem*       page_database;
    DConfigDlgWdgItem*       page_collections;
    DConfigDlgWdgItem*       page_albumView;
    DConfigDlgWdgItem*       page_tooltip;
    DConfigDlgWdgItem*       page_metadata;
    DConfigDlgWdgItem*       page_template;
    DConfigDlgWdgItem*       page_lighttable;
    DConfigDlgWdgItem*       page_editor;
    DConfigDlgWdgItem*       page_slideshow;
    DConfigDlgWdgItem*       page_imagequalitysorter;
    DConfigDlgWdgItem*       page_icc;
    DConfigDlgWdgItem*       page_camera;
    DConfigDlgWdgItem*       page_misc;
    SetupDatabase*           databasePage;
    SetupCollections*        collectionsPage;
    SetupAlbumView*          albumViewPage;
    SetupToolTip*            tooltipPage;
    SetupMetadata*           metadataPage;
    SetupTemplate*           templatePage;
    SetupLightTable*         lighttablePage;
    SetupEditor*             editorPage;
    SetupSlideShow*          slideshowPage;
    SetupImageQualitySorter* imageQualitySorterPage;
    SetupICC*                iccPage;
    SetupCamera*             cameraPage;
    SetupMisc*               miscPage;

public:

    DConfigDlgWdgItem* pageItem(Setup::Page page) const;
};

Setup::Setup(QWidget* const parent)
    : DConfigDlg(parent),
      d(new Private)
{
    setWindowTitle(i18n("Configure"));
    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button(QDialogButtonBox::Ok)->setDefault(true);
    setFaceType(List);
    setModal(true);

    d->databasePage     = new SetupDatabase();
    d->page_database    = addPage(d->databasePage, i18n("Database"));
    d->page_database->setHeader(i18n("<qt>Database Settings<br/>"
                                     "<i>Customize database settings</i></qt>"));
    d->page_database->setIcon(QIcon::fromTheme(QLatin1String("network-server-database")));

    d->collectionsPage  = new SetupCollections();
    d->page_collections = addPage(d->collectionsPage, i18n("Collections"));
    d->page_collections->setHeader(i18n("<qt>Collections Settings<br/>"
                                        "<i>Set root albums locations</i></qt>"));
    d->page_collections->setIcon(QIcon::fromTheme(QLatin1String("folder-pictures")));

    d->albumViewPage  = new SetupAlbumView();
    d->page_albumView = addPage(d->albumViewPage, i18n("Views"));
    d->page_albumView->setHeader(i18n("<qt>Application Views Settings<br/>"
                                      "<i>Customize the look of the views</i></qt>"));
    d->page_albumView->setIcon(QIcon::fromTheme(QLatin1String("view-list-icons")));

    d->tooltipPage  = new SetupToolTip();
    d->page_tooltip = addPage(d->tooltipPage, i18n("Tool-Tip"));
    d->page_tooltip->setHeader(i18n("<qt>Items Tool-Tip Settings<br/>"
                                    "<i>Customize information in item tool-tips</i></qt>"));
    d->page_tooltip->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                     "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(QIcon::fromTheme(QLatin1String("format-text-code"))); // krazy:exclude=iconnames

    d->templatePage  = new SetupTemplate();
    d->page_template = addPage(d->templatePage, i18n("Templates"));
    d->page_template->setHeader(i18n("<qt>Metadata templates<br/>"
                                     "<i>Manage your collection of metadata templates</i></qt>"));
    d->page_template->setIcon(QIcon::fromTheme(QLatin1String("im-user")));

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18n("Image Editor"));
    d->page_editor->setHeader(i18n("<qt>Image Editor Settings<br/>"
                                   "<i>Customize the image editor settings</i></qt>"));
    d->page_editor->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    d->iccPage  = new SetupICC(buttonBox());
    d->page_icc = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Settings for Color Management<br/>"
                                "<i>Customize the color management settings</i></qt>"));
    d->page_icc->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")));

    d->lighttablePage  = new SetupLightTable();
    d->page_lighttable = addPage(d->lighttablePage, i18n("Light Table"));
    d->page_lighttable->setHeader(i18n("<qt>Light Table Settings<br/>"
                                       "<i>Customize tool used to compare images</i></qt>"));
    d->page_lighttable->setIcon(QIcon::fromTheme(QLatin1String("lighttable")));

    d->slideshowPage  = new SetupSlideShow();
    d->page_slideshow = addPage(d->slideshowPage, i18n("Slide Show"));
    d->page_slideshow->setHeader(i18n("<qt>Slide Show Settings<br/>"
                                      "<i>Customize slideshow settings</i></qt>"));
    d->page_slideshow->setIcon(QIcon::fromTheme(QLatin1String("view-presentation")));

    d->imageQualitySorterPage = new SetupImageQualitySorter();
    d->page_imagequalitysorter = addPage(d->imageQualitySorterPage, i18n("Image Quality Sorter"));
    d->page_imagequalitysorter->setHeader(i18n("<qt>Image Quality Sorter Settings<br/>"));
    d->page_imagequalitysorter->setIcon(QIcon::fromTheme(QLatin1String("flag-green")));

    d->cameraPage  = new SetupCamera();
    d->page_camera = addPage(d->cameraPage, i18n("Cameras"));
    d->page_camera->setHeader(i18n("<qt>Camera Settings<br/>"
                                   "<i>Manage your camera devices</i></qt>"));
    d->page_camera->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));

    connect(d->cameraPage, SIGNAL(signalUseFileMetadataChanged(bool)),
            d->tooltipPage, SLOT(slotUseFileMetadataChanged(bool)));

    connect(buttonBox(), SIGNAL(helpRequested()),
            this, SLOT(slotHelp()));

    connect(buttonBox()->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &Setup::slotOkClicked);

    d->miscPage  = new SetupMisc();
    d->page_misc = addPage(d->miscPage, i18n("Miscellaneous"));
    d->page_misc->setHeader(i18n("<qt>Miscellaneous Settings<br/>"
                                 "<i>Customize behavior of the other parts of digiKam</i></qt>"));
    d->page_misc->setIcon(QIcon::fromTheme(QLatin1String("preferences-other")));

    for (int i = 0 ; i != SetupPageEnumLast ; ++i)
    {
        DConfigDlgWdgItem* const item = d->pageItem((Page)i);

        if (!item)
        {
            continue;
        }

        QWidget* const wgt            = item->widget();
        QScrollArea* const scrollArea = qobject_cast<QScrollArea*>(wgt);

        if (scrollArea)
        {
            scrollArea->setFrameShape(QFrame::NoFrame);
        }
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));

    winId();
    windowHandle()->resize(800, 600);
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));
    group.writeEntry(QLatin1String("Setup Page"), (int)activePageIndex());
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    config->sync();
    delete d;
}

void Setup::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

void Setup::setTemplate(const Template& t)
{
    if (d->templatePage)
    {
        d->templatePage->setTemplate(t);
    }
}

QSize Setup::sizeHint() const
{
    // The minimum size is very small. But the default initial size is such
    // that some important tabs get a scroll bar, although the dialog could be larger
    // on a normal display (QScrollArea size hint does not take widget into account)
    // Adjust size hint here so that certain selected tabs are display full per default.
    QSize hint          = DConfigDlg::sizeHint();
    int maxHintHeight   = 0;
    int maxWidgetHeight = 0;

    for (int page = 0 ; page != SetupPageEnumLast ; ++page)
    {
        // only take tabs into account here that should better be displayed without scrolling
        if (page == CollectionsPage ||
            page == AlbumViewPage   ||
            page == TemplatePage    ||
            page == LightTablePage  ||
            page == EditorPage      ||
            page == MiscellaneousPage)
        {
            DConfigDlgWdgItem* const item = d->pageItem((Page)page);

            if (!item)
            {
                continue;
            }

            QWidget* const page           = item->widget();
            maxHintHeight                 = qMax(maxHintHeight, page->sizeHint().height());
            QScrollArea* const scrollArea = qobject_cast<QScrollArea*>(page);

            if (scrollArea)
            {
                maxWidgetHeight = qMax(maxWidgetHeight, scrollArea->widget()->sizeHint().height());
            }
        }
    }

    // The additional 20 is a hack to make it work.
    // Don't know why, the largest page would have scroll bars without this
    if (maxWidgetHeight > maxHintHeight)
    {
        hint.setHeight(hint.height() + (maxWidgetHeight - maxHintHeight) + 20);
    }

    return hint;
}

bool Setup::execDialog(Page page)
{
    return execDialog(0, page);
}

bool Setup::execDialog(QWidget* const parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    bool success          = (setup->DConfigDlg::exec() == QDialog::Accepted);
    delete setup;
    return success;
}

bool Setup::execSinglePage(Page page)
{
    return execSinglePage(0, page);
}

bool Setup::execSinglePage(QWidget* const parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    setup->setFaceType(Plain);
    bool success          = (setup->DConfigDlg::exec() == QDialog::Accepted);
    delete setup;
    return success;
}

bool Setup::execTemplateEditor(QWidget* const parent, const Template& t)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(TemplatePage);
    setup->setFaceType(Plain);
    setup->setTemplate(t);
    bool success          = (setup->DConfigDlg::exec() == QDialog::Accepted);
    delete setup;
    return success;
}

bool Setup::execMetadataFilters(QWidget* const parent, int tab)
{
    QPointer<Setup> setup       = new Setup(parent);
    setup->showPage(MetadataPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur  = setup->currentPage();
    if (!cur) return false;

    SetupMetadata* const widget = dynamic_cast<SetupMetadata*>(cur->widget());
    if (!widget) return false;

    widget->setActiveMainTab(SetupMetadata::Display);
    widget->setActiveSubTab(tab);

    bool success                = (setup->DConfigDlg::exec() == QDialog::Accepted);
    delete setup;
    return success;
}

void Setup::slotOkClicked()
{
    if (!d->cameraPage->checkSettings())
    {
        showPage(CameraPage);
        return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);

    d->cameraPage->applySettings();
    d->databasePage->applySettings();
    d->collectionsPage->applySettings();
    d->albumViewPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->templatePage->applySettings();
    d->lighttablePage->applySettings();
    d->editorPage->applySettings();
    d->slideshowPage->applySettings();
    d->imageQualitySorterPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();

    ApplicationSettings::instance()->emitSetupChanged();
    ImportSettings::instance()->emitSetupChanged();

    qApp->restoreOverrideCursor();

    if (d->metadataPage->exifAutoRotateHasChanged())
    {
        QString msg = i18n("The Exif auto-rotate thumbnails option has been changed.\n"
                           "Do you want to rebuild all albums' items' thumbnails now?\n\n"
                           "Note: thumbnail processing can take a while. You can start "
                           "this job later from the \"Tools-Maintenance\" menu.");

        int result = QMessageBox::warning(this, qApp->applicationName(), msg,
                                          QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return;
        }

        new ThumbsGenerator(true, -1);
    }

    accept();
}

void Setup::showPage(Setup::Page page)
{
    DConfigDlgWdgItem* item = 0;

    if (page == LastPageUsed)
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));

        item = d->pageItem((Page)group.readEntry(QLatin1String("Setup Page"), (int)CollectionsPage));
    }
    else
    {
        item = d->pageItem(page);
    }

    if (!item)
    {
        item = d->pageItem(CollectionsPage);
    }

    setCurrentPage(item);
}

Setup::Page Setup::activePageIndex() const
{
    DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_collections)
    {
        return CollectionsPage;
    }

    if (cur == d->page_albumView)
    {
        return AlbumViewPage;
    }

    if (cur == d->page_tooltip)
    {
        return ToolTipPage;
    }

    if (cur == d->page_metadata)
    {
        return MetadataPage;
    }

    if (cur == d->page_template)
    {
        return TemplatePage;
    }

    if (cur == d->page_lighttable)
    {
        return LightTablePage;
    }

    if (cur == d->page_editor)
    {
        return EditorPage;
    }

    if (cur == d->page_slideshow)
    {
        return SlideshowPage;
    }

    if (cur == d->page_imagequalitysorter)
    {
        return ImageQualityPage;
    }

    if (cur == d->page_icc)
    {
        return ICCPage;
    }

    if (cur == d->page_camera)
    {
        return CameraPage;
    }

    if (cur == d->page_misc)
    {
        return MiscellaneousPage;
    }

    return DatabasePage;
}

DConfigDlgWdgItem* Setup::Private::pageItem(Setup::Page page) const
{
    switch (page)
    {
        case Setup::DatabasePage:
            return page_database;

        case Setup::CollectionsPage:
            return page_collections;

        case Setup::AlbumViewPage:
            return page_albumView;

        case Setup::ToolTipPage:
            return page_tooltip;

        case Setup::MetadataPage:
            return page_metadata;

        case Setup::TemplatePage:
            return page_template;

        case Setup::LightTablePage:
            return page_lighttable;

        case Setup::EditorPage:
            return page_editor;

        case Setup::SlideshowPage:
            return page_slideshow;

        case Setup::ImageQualityPage:
            return page_imagequalitysorter;

        case Setup::ICCPage:
            return page_icc;

        case Setup::CameraPage:
            return page_camera;

        case Setup::MiscellaneousPage:
            return page_misc;

        default:
            return 0;
    }
}

}  // namespace Digikam
