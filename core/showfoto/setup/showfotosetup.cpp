/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : showFoto setup dialog.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosetup.h"

// Qt includes

#include <QPointer>
#include <QPushButton>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "setupeditoriface.h"
#include "setupicc.h"
#include "setupiofiles.h"
#include "setupslideshow.h"
#include "showfotosetupraw.h"
#include "showfotosetupmisc.h"
#include "showfotosetupmetadata.h"
#include "showfotosetuptooltip.h"
#include "dxmlguiwindow.h"

namespace ShowFoto
{

class Setup::Private
{
public:

    Private() :
        page_editorIface(0),
        page_metadata(0),
        page_tooltip(0),
        page_raw(0),
        page_iofiles(0),
        page_slideshow(0),
        page_icc(0),
        page_misc(0),
        metadataPage(0),
        toolTipPage(0),
        miscPage(0),
        rawPage(0),
        editorIfacePage(0),
        iofilesPage(0),
        slideshowPage(0),
        iccPage(0)
    {
    }

    DConfigDlgWdgItem*         page_editorIface;
    DConfigDlgWdgItem*         page_metadata;
    DConfigDlgWdgItem*         page_tooltip;
    DConfigDlgWdgItem*         page_raw;
    DConfigDlgWdgItem*         page_iofiles;
    DConfigDlgWdgItem*         page_slideshow;
    DConfigDlgWdgItem*         page_icc;
    DConfigDlgWdgItem*         page_misc;

    SetupMetadata*             metadataPage;
    SetupToolTip*              toolTipPage;
    SetupMisc*                 miscPage;
    SetupRaw*                  rawPage;

    Digikam::SetupEditorIface* editorIfacePage;
    Digikam::SetupIOFiles*     iofilesPage;
    Digikam::SetupSlideShow*   slideshowPage;
    Digikam::SetupICC*         iccPage;

public:

    DConfigDlgWdgItem* pageItem(Setup::Page page) const;
};

Setup::Setup(QWidget* const parent, Setup::Page page)
    : DConfigDlg(parent),
      d(new Private)
{
    setWindowTitle(i18n("Configure"));
    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button(QDialogButtonBox::Ok)->setDefault(true);
    setFaceType(DConfigDlg::List);
    setModal(true);

    d->editorIfacePage     = new Digikam::SetupEditorIface();
    d->page_editorIface    = addPage(d->editorIfacePage, i18n("Editor Window"));
    d->page_editorIface->setHeader(i18n("<qt>Editor Window Settings<br/>"
                                   "<i>Customize editor window behavior</i></qt>"));
    d->page_editorIface->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    d->metadataPage   = new SetupMetadata();
    d->page_metadata  = addPage(d->metadataPage, i18n("Metadata"));
    d->page_metadata->setHeader(i18n("<qt>Embedded Image Information Management<br/>"
                                     "<i>Setup relations between images and metadata</i></qt>"));
    d->page_metadata->setIcon(QIcon::fromTheme(QLatin1String("format-text-code"))); // krazy:exclude=iconnames

    d->toolTipPage    = new SetupToolTip();
    d->page_tooltip   = addPage(d->toolTipPage, i18n("Tool Tip"));
    d->page_tooltip->setHeader(i18n("<qt>Thumbbar Items Tool-Tip Settings<br/>"
                                    "<i>Customize information in tool-tips</i></qt>"));
    d->page_tooltip->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));

    d->rawPage        = new SetupRaw();
    d->page_raw       = addPage(d->rawPage, i18n("RAW Decoding"));
    d->page_raw->setHeader(i18n("<qt>RAW Files Decoding Settings<br/>"
                                  "<i>Customize default RAW decoding settings</i></qt>"));
    d->page_raw->setIcon(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));

    d->iccPage        = new Digikam::SetupICC(buttonBox());
    d->page_icc       = addPage(d->iccPage, i18n("Color Management"));
    d->page_icc->setHeader(i18n("<qt>Settings for Color Management<br/>"
                                "<i>Customize color management settings</i></qt>"));
    d->page_icc->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")));

    d->iofilesPage    = new Digikam::SetupIOFiles();
    d->page_iofiles   = addPage(d->iofilesPage, i18n("Save Images"));
    d->page_iofiles->setHeader(i18n("<qt>Settings for Saving Image Files<br/>"
                                    "<i>Set default configuration used to save images</i></qt>"));
    d->page_iofiles->setIcon(QIcon::fromTheme(QLatin1String("document-save-all")));

    d->slideshowPage  = new Digikam::SetupSlideShow();
    d->page_slideshow = addPage(d->slideshowPage, i18n("Slide Show"));
    d->page_slideshow->setHeader(i18n("<qt>Slide Show Settings<br/>"
                                      "<i>Customize slideshow settings</i></qt>"));
    d->page_slideshow->setIcon(QIcon::fromTheme(QLatin1String("view-presentation")));

    d->miscPage       = new SetupMisc();
    d->page_misc      = addPage(d->miscPage, i18n("Miscellaneous"));
    d->page_misc->setHeader(i18n("<qt>Miscellaneous Settings<br/>"
                                 "<i>Customize behavior of the other parts of Showfoto</i></qt>"));
    d->page_misc->setIcon(QIcon::fromTheme(QLatin1String("preferences-other")));

    for (int i = 0; i != SetupPageEnumLast; ++i)
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

    connect(buttonBox()->button(QDialogButtonBox::Ok),
            &QPushButton::clicked, this, &Setup::slotOkClicked);

    connect(buttonBox(), SIGNAL(helpRequested()),
            this, SLOT(slotHelp()));

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));

    if (page != LastPageUsed)
    {
        showPage(page);
    }
    else
    {
        showPage((Page)group.readEntry(QLatin1String("Setup Page"), (int)EditorPage));
    }

    Digikam::DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

Setup::~Setup()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Setup Dialog"));
    group.writeEntry(QLatin1String("Setup Page"), (int)activePageIndex());
    Digikam::DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    config->sync();
    delete d;
}

void Setup::slotHelp()
{
    Digikam::DXmlGuiWindow::openHandbook();
}

void Setup::slotOkClicked()
{
    d->editorIfacePage->applySettings();
    d->metadataPage->applySettings();
    d->toolTipPage->applySettings();
    d->rawPage->applySettings();
    d->iofilesPage->applySettings();
    d->slideshowPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();
    close();
}

void Setup::showPage(Setup::Page page)
{
    switch (page)
    {
        case ToolTipPage:
            setCurrentPage(d->page_tooltip);
            break;
        case RawPage:
            setCurrentPage(d->page_raw);
            break;
        case IOFilesPage:
            setCurrentPage(d->page_iofiles);
            break;
        case SlideshowPage:
            setCurrentPage(d->page_slideshow);
            break;
        case ICCPage:
            setCurrentPage(d->page_icc);
            break;
        case MetadataPage:
            setCurrentPage(d->page_metadata);
            break;
        case MiscellaneousPage:
            setCurrentPage(d->page_misc);
            break;
        default:
            setCurrentPage(d->page_editorIface);
            break;
    }
}

Setup::Page Setup::activePageIndex()
{
    DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_tooltip)
    {
        return ToolTipPage;
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

    if (cur == d->page_icc)
    {
        return ICCPage;
    }

    if (cur == d->page_metadata)
    {
        return MetadataPage;
    }

    if (cur == d->page_misc)
    {
        return MiscellaneousPage;
    }

    return EditorPage;
}

DConfigDlgWdgItem* Setup::Private::pageItem(Setup::Page page) const
{
    switch (page)
    {
        case Setup::EditorPage:
            return page_editorIface;
        case Setup::MetadataPage:
            return page_metadata;
        case Setup::ToolTipPage:
            return page_tooltip;
        case Setup::RawPage:
            return page_raw;
        case Setup::IOFilesPage:
            return page_iofiles;
        case Setup::SlideshowPage:
            return page_slideshow;
        case Setup::ICCPage:
            return page_icc;
        case Setup::MiscellaneousPage:
            return page_misc;
        default:
            return 0;
    }
}

bool Setup::execMetadataFilters(QWidget* const parent, int tab)
{
    QPointer<Setup> setup = new Setup(parent);
    setup->showPage(MetadataPage);
    setup->setFaceType(Plain);

    DConfigDlgWdgItem* const cur  = setup->currentPage();

    if (!cur)
        return false;

    SetupMetadata* const widget = dynamic_cast<SetupMetadata*>(cur->widget());

    if (!widget)
        return false;

    widget->setActiveTab((SetupMetadata::MetadataTab)tab);

    bool success                = setup->DConfigDlg::exec() == QDialog::Accepted;
    delete setup;
    return success;
}

}   // namespace ShowFoto
