/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setup.moc"

// Qt includes

#include <QPointer>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <kdebug.h>

#ifdef HAVE_KIPI

// Libkipi includes

#include <libkipi/configwidget.h>

using namespace KIPI;

#endif /* HAVE_KIPI */

// Local includes

#include "applicationsettings.h"
#include "thumbsgenerator.h"
#include "setupalbumview.h"
#include "setupcamera.h"
#include "setupcategory.h"
#include "setupcollections.h"
#include "setupraw.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setupiofiles.h"
#include "setuplighttable.h"
#include "setupmetadata.h"
#include "setupmime.h"
#include "setupmisc.h"
#include "setupslideshow.h"
#include "setupimagequalitysorter.h"
#include "setuptooltip.h"
#include "setupdatabase.h"
#include "setupversioning.h"
#include "importsettings.h"

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
        page_category(0),
        page_mime(0),
        page_lighttable(0),
        page_editor(0),
        page_raw(0),
        page_iofiles(0),
        page_slideshow(0),
        page_imagequalitysorter(0),
        page_icc(0),
        page_camera(0),
        page_misc(0),

#ifdef HAVE_KIPI
        page_plugins(0),
#endif /* HAVE_KIPI */

        page_versioning(0),
        databasePage(0),
        collectionsPage(0),
        albumViewPage(0),
        tooltipPage(0),
        metadataPage(0),
        templatePage(0),
        categoryPage(0),
        mimePage(0),
        lighttablePage(0),
        editorPage(0),
        rawPage(0),
        iofilesPage(0),
        slideshowPage(0),
        imageQualitySorterPage(0),
        iccPage(0),
        cameraPage(0),
        miscPage(0),

#ifdef HAVE_KIPI
        pluginsPage(0),
#endif /* HAVE_KIPI */

        versioningPage(0),
        pluginFilter(0)
    {
    }

    KPageWidgetItem*         page_database;
    KPageWidgetItem*         page_collections;
    KPageWidgetItem*         page_albumView;
    KPageWidgetItem*         page_tooltip;
    KPageWidgetItem*         page_metadata;
    KPageWidgetItem*         page_template;
    KPageWidgetItem*         page_category;
    KPageWidgetItem*         page_mime;
    KPageWidgetItem*         page_lighttable;
    KPageWidgetItem*         page_editor;
    KPageWidgetItem*         page_raw;
    KPageWidgetItem*         page_iofiles;
    KPageWidgetItem*         page_slideshow;
    KPageWidgetItem*         page_imagequalitysorter;
    KPageWidgetItem*         page_icc;
    KPageWidgetItem*         page_camera;
    KPageWidgetItem*         page_misc;

#ifdef HAVE_KIPI
    KPageWidgetItem*         page_plugins;
#endif /* HAVE_KIPI */

    KPageWidgetItem*         page_versioning;

    SetupDatabase*           databasePage;
    SetupCollections*        collectionsPage;
    SetupAlbumView*          albumViewPage;
    SetupToolTip*            tooltipPage;
    SetupMetadata*           metadataPage;
    SetupTemplate*           templatePage;
    SetupCategory*           categoryPage;
    SetupMime*               mimePage;
    SetupLightTable*         lighttablePage;
    SetupEditor*             editorPage;
    SetupRaw*                rawPage;
    SetupIOFiles*            iofilesPage;
    SetupSlideShow*          slideshowPage;
    SetupImageQualitySorter* imageQualitySorterPage;
    SetupICC*                iccPage;
    SetupCamera*             cameraPage;
    SetupMisc*               miscPage;

#ifdef HAVE_KIPI
    ConfigWidget*            pluginsPage;
#endif /* HAVE_KIPI */

    SetupVersioning*         versioningPage;

    SearchTextBar*           pluginFilter;

public:

    KPageWidgetItem* pageItem(Setup::Page page) const;
};

Setup::Setup(QWidget* const parent)
    : KPageDialog(parent), d(new Private)
{
    setCaption(i18n("Configure"));
    setButtons(Help | Ok | Cancel);
    setDefaultButton(Ok);
    setHelp("setupdialog.anchor", "digikam");
    setFaceType(List);
    setModal(true);

    d->databasePage     = new SetupDatabase(this);
    d->page_database    = addPage(d->databasePage, i18n("Database"));
    d->page_database->setHeader(i18n("<qt>Database Settings<br/>"
                                     "<i>Customize database settings</i></qt>"));
    d->page_database->setIcon(KIcon("server-database"));

    d->collectionsPage  = new SetupCollections(this);
    d->page_collections = addPage(d->collectionsPage, i18n("Collections"));
    d->page_collections->setHeader(i18n("<qt>Collections Settings<br/>"
                                        "<i>Set root albums and database locations</i></qt>"));
    d->page_collections->setIcon(KIcon("folder-image"));

    d->albumViewPage  = new SetupAlbumView();
    d->page_albumView = addPage(d->albumViewPage, i18n("Album View"));
    d->page_albumView->setHeader(i18n("<qt>Album View Settings<br/>"
                                      "<i>Customize the look of the albums list</i></qt>"));
    d->page_albumView->setIcon(KIcon("view-list-icons"));

    d->categoryPage  = new SetupCategory();
    d->page_category = addPage(d->categoryPage, i18n("Album Category"));
    d->page_category->setHeader(i18n("<qt>Album Category Settings<br/>"
                                     "<i>Assign categories to albums used to sort them</i></qt>"));
    d->page_category->setIcon(KIcon("view-calendar-list"));

    d->tooltipPage  = new SetupToolTip();
    d->page_tooltip = addPage(d->tooltipPage, i18n("Tool-Tip"));
    d->page_tooltip->setHeader(i18n("<qt>Album Items Tool-Tip Settings<br/>"
                                    "<i>Customize information in tool-tips</i></qt>"));
    d->page_tooltip->setIcon(KIcon("dialog-information"));

    d->metadataPage  = new SetupMetadata();
    d->page_metadata = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                     "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(KIcon("exifinfo")); // krazy:exclude=iconnames

    d->templatePage  = new SetupTemplate();
    d->page_template = addPage(d->templatePage, i18n("Templates"));
    d->page_template->setHeader(i18n("<qt>Metadata templates<br/>"
                                     "<i>Manage your collection of metadata templates</i></qt>"));
    d->page_template->setIcon(KIcon("user-identity"));

    d->mimePage  = new SetupMime();
    d->page_mime = addPage(d->mimePage, i18n("MIME Types"));
    d->page_mime->setHeader(i18n("<qt>Supported File Settings<br/>"
                                 "<i>Add new file types to show as album items</i></qt>"));
    d->page_mime->setIcon(KIcon("system-file-manager"));

    d->editorPage  = new SetupEditor();
    d->page_editor = addPage(d->editorPage, i18n("Editor Window"));
    d->page_editor->setHeader(i18n("<qt>Image Editor Window Settings<br/>"
                                   "<i>Customize the image editor window</i></qt>"));
    d->page_editor->setIcon(KIcon("editimage"));

    d->versioningPage  = new SetupVersioning();
    d->page_versioning = addPage(d->versioningPage, i18n("Editing Images"));
    d->page_versioning->setHeader(i18n("<qt>Editing Images<br/>"
                                       "<i>Configure non-destructive editing and versioning</i></qt>"));
    d->page_versioning->setIcon(KIcon("view-catalog"));

    d->rawPage  = new SetupRaw();
    d->page_raw = addPage(d->rawPage, i18n("RAW Decoding"));
    d->page_raw->setHeader(i18n("<qt>Image Editor: RAW File Decoding<br/>"
                                "<i>Configure RAW decoding settings of the image editor</i></qt>"));
    d->page_raw->setIcon(KIcon("kdcraw"));

    d->iofilesPage  = new SetupIOFiles();
    d->page_iofiles = addPage(d->iofilesPage, i18n("Saving Images"));
    d->page_iofiles->setHeader(i18n("<qt>Image Editor: Settings for Saving Image Files<br/>"
                                    "<i>Set default configuration used to save images with the image editor</i></qt>"));
    d->page_iofiles->setIcon(KIcon("document-save-all"));

    d->iccPage  = new SetupICC(this);
    d->page_icc = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Settings for Color Management<br/>"
                                "<i>Customize the color management settings</i></qt>"));
    d->page_icc->setIcon(KIcon("colormanagement"));

    d->lighttablePage  = new SetupLightTable();
    d->page_lighttable = addPage(d->lighttablePage, i18n("Light Table"));
    d->page_lighttable->setHeader(i18n("<qt>Light Table Settings<br/>"
                                       "<i>Customize tool used to compare images</i></qt>"));
    d->page_lighttable->setIcon(KIcon("lighttable"));

    d->slideshowPage  = new SetupSlideShow();
    d->page_slideshow = addPage(d->slideshowPage, i18n("Slide Show"));
    d->page_slideshow->setHeader(i18n("<qt>Slide Show Settings<br/>"
                                      "<i>Customize slideshow settings</i></qt>"));
    d->page_slideshow->setIcon(KIcon("view-presentation"));

    d->imageQualitySorterPage = new SetupImageQualitySorter();
    d->page_imagequalitysorter = addPage(d->imageQualitySorterPage, i18n("Image Quality Sorter"));
    d->page_imagequalitysorter->setHeader(i18n("<qt>Image Quality Sorter Settings<br/>"));
    d->page_imagequalitysorter->setIcon(KIcon("flag-green"));

    d->cameraPage  = new SetupCamera();
    d->page_camera = addPage(d->cameraPage, i18n("Cameras"));
    d->page_camera->setHeader(i18n("<qt>Camera Settings<br/>"
                                   "<i>Manage your camera devices</i></qt>"));
    d->page_camera->setIcon(KIcon("camera-photo"));

    connect(d->cameraPage, SIGNAL(signalUseFileMetadataChanged(bool)),
            d->tooltipPage, SLOT(slotUseFileMetadataChanged(bool)));

#ifdef HAVE_KIPI
    d->pluginsPage  = new ConfigWidget();
    d->pluginFilter = new SearchTextBar(d->pluginsPage, "PluginsSearchBar");
    d->pluginsPage->setFilterWidget(d->pluginFilter);
    d->page_plugins = addPage(d->pluginsPage, i18n("Kipi Plugins"));
    d->page_plugins->setHeader(i18n("<qt>Main Interface Plug-in Settings<br/>"
                                    "<i>Set which plugins will be accessible from the main interface</i></qt>"));
    d->page_plugins->setIcon(KIcon("kipi"));

    connect(d->pluginFilter, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->pluginsPage, SIGNAL(signalSearchResult(bool)),
            d->pluginFilter, SLOT(slotSearchResult(bool)));
#endif /* HAVE_KIPI */

    d->miscPage  = new SetupMisc();
    d->page_misc = addPage(d->miscPage, i18n("Miscellaneous"));
    d->page_misc->setHeader(i18n("<qt>Miscellaneous Settings<br/>"
                                 "<i>Customize behavior of the other parts of digiKam</i></qt>"));
    d->page_misc->setIcon(KIcon("preferences-other"));

    for (int i = 0; i != SetupPageEnumLast; ++i)
    {
        KPageWidgetItem* const item = d->pageItem((Page)i);

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

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
    restoreDialogSize(group);

    show();
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("Setup Dialog"));
    group.writeEntry("Setup Page", (int)activePageIndex());
    saveDialogSize(group);
    config->sync();
    delete d;
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
    QSize hint          = KPageDialog::sizeHint();
    int maxHintHeight   = 0;
    int maxWidgetHeight = 0;

    for (int page = 0; page != SetupPageEnumLast; ++page)
    {
        // only take tabs into account here that should better be displayed without scrolling
        if (page == CollectionsPage ||
            page == AlbumViewPage   ||
            page == TemplatePage    ||
            page == MimePage        ||
            page == LightTablePage  ||
            page == EditorPage      ||
            page == IOFilesPage     ||
            page == RawPage         ||
            page == MiscellaneousPage)
        {
            KPageWidgetItem* const item   = d->pageItem((Page)page);

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

bool Setup::exec(Page page)
{
    return exec(0, page);
}

bool Setup::exec(QWidget* const parent, Page page)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(page);
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
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
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

bool Setup::execTemplateEditor(QWidget* const parent, const Template& t)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(TemplatePage);
    setup->setFaceType(Plain);
    setup->setTemplate(t);
    bool success          = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

bool Setup::execMetadataFilters(QWidget* const parent, int tab)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(MetadataPage);
    setup->setFaceType(Plain);

    KPageWidgetItem* const cur  = setup->currentPage();
    if (!cur) return false;

    SetupMetadata* const widget = dynamic_cast<SetupMetadata*>(cur->widget());
    if (!widget) return false;

    widget->setActiveMainTab(SetupMetadata::Display);
    widget->setActiveSubTab(tab);

    bool success                = setup->KPageDialog::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

void Setup::slotSearchTextChanged(const SearchTextSettings& settings)
{
#ifdef HAVE_KIPI
    d->pluginsPage->slotSetFilter(settings.text, settings.caseSensitive);
#else
    Q_UNUSED(settings);
#endif /* HAVE_KIPI */
}

void Setup::slotButtonClicked(int button)
{
    if (button == KDialog::Ok)
    {
        okClicked();
    }
    else
    {
        KDialog::slotButtonClicked(button);
    }
}

void Setup::okClicked()
{
    if (!d->cameraPage->checkSettings())
    {
        showPage(CameraPage);
        return;
    }

    kapp->setOverrideCursor(Qt::WaitCursor);

    d->cameraPage->applySettings();
    d->databasePage->applySettings();
    d->collectionsPage->applySettings();
    d->albumViewPage->applySettings();
    d->tooltipPage->applySettings();
    d->metadataPage->applySettings();
    d->templatePage->applySettings();
    d->categoryPage->applySettings();
    d->mimePage->applySettings();
    d->lighttablePage->applySettings();
    d->editorPage->applySettings();
    d->rawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->imageQualitySorterPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();

#ifdef HAVE_KIPI
    d->pluginsPage->apply();
#endif /* HAVE_KIPI */

    //d->faceTagsPage->applySettings();
    d->versioningPage->applySettings();

    ApplicationSettings::instance()->emitSetupChanged();
    ImportSettings::instance()->emitSetupChanged();

    kapp->restoreOverrideCursor();

    if (d->metadataPage->exifAutoRotateHasChanged())
    {
        QString msg = i18n("The Exif auto-rotate thumbnails option has been changed.\n"
                           "Do you want to rebuild all albums' items' thumbnails now?\n\n"
                           "Note: thumbnail processing can take a while. You can start "
                           "this job later from the \"Tools-Maintenance\" menu.");
        int result = KMessageBox::warningYesNo(this, msg);

        if (result != KMessageBox::Yes)
        {
            return;
        }

        new ThumbsGenerator(true, -1);
    }

    accept();
}

void Setup::showPage(Setup::Page page)
{
    KPageWidgetItem* item = 0;

    if (page == LastPageUsed)
    {
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group(QString("Setup Dialog"));

        item = d->pageItem((Page)group.readEntry("Setup Page", (int)CollectionsPage));
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
    KPageWidgetItem* const cur = currentPage();

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

    if (cur == d->page_category)
    {
        return CategoryPage;
    }

    if (cur == d->page_mime)
    {
        return MimePage;
    }

    if (cur == d->page_lighttable)
    {
        return LightTablePage;
    }

    if (cur == d->page_editor)
    {
        return EditorPage;
    }

    if (cur == d->page_raw)
    {
        return RawPage;
    }

    if (cur == d->page_iofiles)
    {
        return IOFilesPage;
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

    if (cur == d->page_versioning)
    {
        return VersioningPage;
    }

#ifdef HAVE_KIPI
    if (cur == d->page_plugins)
    {
        return KipiPluginsPage;
    }
#endif /* HAVE_KIPI */

    return DatabasePage;
}

KPageWidgetItem* Setup::Private::pageItem(Setup::Page page) const
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

        case Setup::CategoryPage:
            return page_category;

        case Setup::MimePage:
            return page_mime;

        case Setup::LightTablePage:
            return page_lighttable;

        case Setup::EditorPage:
            return page_editor;

        case Setup::RawPage:
            return page_raw;

        case Setup::IOFilesPage:
            return page_iofiles;

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

        case Setup::VersioningPage:
            return page_versioning;

#ifdef HAVE_KIPI
        case Setup::KipiPluginsPage:
            return page_plugins;
#endif /* HAVE_KIPI */

        default:
            return 0;
    }
}

}  // namespace Digikam
