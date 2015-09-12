/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2006-10-11
 * Description : a plugin to edit pictures metadata
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011-2012 by Victor Dodon <dodonvictor at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "plugin_metadataedit.h"

// Qt includes

#include <QAction>
#include <QPointer>
#include <QApplication>
#include <QMenu>

// KDE includes

#include <kio/global.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <klibloader.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

// Libkipi includes

#include <KIPI/ImageCollection>
#include <KIPI/Interface>
#include <KIPI/PluginLoader>

// Local includes

#include "metadataedit.h"
#include "dmetadata.h"
#include "kipiplugins_debug.h"



namespace Digikam
{

K_PLUGIN_FACTORY( MetadataEditFactory, registerPlugin<Plugin_MetadataEdit>(); )

class Plugin_MetadataEdit::Private
{
public:

    Private()
    {
        actionMetadataEdit = 0;
        interface          = 0;
        parentWidget       = 0;
    }

    QWidget*   parentWidget;

    QAction*   actionMetadataEdit;

    Interface* interface;

    QUrl       lastSelectedDirectory;
};

Plugin_MetadataEdit::Plugin_MetadataEdit(QObject* const parent, const QVariantList&)
    : Plugin(parent, "MetadataEdit"),
      d(new Private)
{
    qCDebug(KIPIPLUGINS_LOG) << "Plugin_MetadataEdit plugin loaded";

    setUiBaseName("kipiplugin_metadataeditui.rc");
    setupXML();
}

Plugin_MetadataEdit::~Plugin_MetadataEdit()
{
    delete d;
}

void Plugin_MetadataEdit::setup(QWidget* const widget)
{
    d->parentWidget = widget;
    Plugin::setup(widget);

    setupActions();

    d->interface = interface();
    if (!d->interface)
    {
        qCCritical(KIPIPLUGINS_LOG) << "Kipi interface is null!";
        return;
    }

    ImageCollection selection = d->interface->currentSelection();
    d->actionMetadataEdit->setEnabled( selection.isValid() && !selection.images().isEmpty() );

    connect(d->interface, SIGNAL(selectionChanged(bool)),
            d->actionMetadataEdit, SLOT(setEnabled(bool)));
}

void Plugin_MetadataEdit::setupActions()
{
    setDefaultCategory(ImagesPlugin);

    // NOTE This is a workaround, maybe a better solution?
    if (PluginLoader::instance()->disabledPluginActions().contains("metadataedit"))
        return;

    d->actionMetadataEdit = actionCollection()->addAction("metadataedit");
    d->actionMetadataEdit->setText(i18n("&Metadata"));
    d->actionMetadataEdit->setIcon(QIcon::fromTheme(QLatin1String("kipi-metadataedit")));
    d->actionMetadataEdit->setEnabled(false);

    QMenu* metadataEditMenu = new QMenu(d->parentWidget);
    d->actionMetadataEdit->setMenu(metadataEditMenu);

    QAction * metadataEdit = new QAction(this);
    metadataEdit->setText(i18n("Edit &All Metadata..."));
    connect(metadataEdit, SIGNAL(triggered(bool)),
            this,SLOT(slotEditAllMetadata()));
    metadataEditMenu->addAction(metadataEdit);

    addAction("editallmetadata", metadataEdit);

    // ---------------------------------------------------

    d->actionMetadataEdit->menu()->addSeparator();

    QAction * importEXIF = new QAction(this);
    importEXIF->setText(i18n("Import EXIF..."));
    connect(importEXIF, SIGNAL(triggered(bool)),
            this, SLOT(slotImportExif()));
    metadataEditMenu->addAction(importEXIF);

    addAction("importexif", importEXIF);

    QAction * importIPTC = new QAction(this);
    importIPTC->setText(i18n("Import IPTC..."));
    connect(importIPTC, SIGNAL(triggered(bool)),
            this, SLOT(slotImportIptc()));
    metadataEditMenu->addAction(importIPTC);

    addAction("importiptc", importIPTC);

    QAction * importXMP = new QAction(this);
    importXMP->setText(i18n("Import XMP..."));
    connect(importXMP, SIGNAL(triggered(bool)),
            this, SLOT(slotImportXmp()));
    metadataEditMenu->addAction(importXMP);

    addAction("importxmp", importXMP);
}

void Plugin_MetadataEdit::slotEditAllMetadata()
{
    ImageCollection images = d->interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(), images.images());
    dialog->exec();

    delete dialog;
}

void Plugin_MetadataEdit::slotImportExif()
{
    ImageCollection images = d->interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( d->lastSelectedDirectory.isEmpty() )
    {
        d->lastSelectedDirectory = KIO::upUrl(images.images().first());
    }

    QUrl importEXIFFile = KFileDialog::getOpenUrl(d->lastSelectedDirectory,
                                                  QString(), QApplication::activeWindow(),
                                                  i18n("Select File to Import EXIF metadata") );
    if( importEXIFFile.isEmpty() )
       return;

    d->lastSelectedDirectory = KIO::upUrl(importEXIFFile);

    DMetadata meta;
    if (!meta.load(importEXIFFile.path()))
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importEXIFFile.fileName()),
                           i18n("Import EXIF Metadata"));
        return;
    }

#if KEXIV2_VERSION >= 0x010000
    QByteArray exifData = meta.getExifEncoded();
#else
    QByteArray exifData = meta.getExif();
#endif

    if (exifData.isEmpty())
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("\"%1\" does not have EXIF metadata.", importEXIFFile.fileName()),
                           i18n("Import EXIF Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     QApplication::activeWindow(),
                     i18n("EXIF metadata from the currently selected pictures will be permanently "
                          "replaced by the EXIF content of \"%1\".\n"
                          "Do you want to continue?", importEXIFFile.fileName()),
                     i18n("Import EXIF Metadata")) != KMessageBox::Yes)
        return;

    QList<QUrl>  imageURLs = images.images();
    QList<QUrl>  updatedURLs;
    QStringList errorFiles;

    for( QList<QUrl>::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        QUrl url = *it;
        bool ret = false;

        if (DMetadata::canWriteExif(url.path()))
        {
            ret = true;
            DMetadata meta;

            ret &= meta.load(url.path());
            ret &= meta.setExif(exifData);
            ret &= meta.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    d->interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    QApplication::activeWindow(),
                    i18n("Unable to set EXIF metadata from:"),
                    errorFiles,
                    i18n("Import EXIF Metadata"));
    }
}

void Plugin_MetadataEdit::slotImportIptc()
{
    ImageCollection images = d->interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( d->lastSelectedDirectory.isEmpty() )
    {
        d->lastSelectedDirectory = KIO::upUrl(images.images().first());
    }

    QUrl importIPTCFile = KFileDialog::getOpenUrl(d->lastSelectedDirectory,
                                                  QString(), QApplication::activeWindow(),
                                                  i18n("Select File to Import IPTC metadata") );
    if( importIPTCFile.isEmpty() )
       return;

    d->lastSelectedDirectory = KIO::upUrl(importIPTCFile);

    DMetadata meta;
    if (!meta.load(importIPTCFile.path()))
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importIPTCFile.fileName()),
                           i18n("Import IPTC Metadata"));
        return;
    }

    QByteArray iptcData = meta.getIptc();
    if (iptcData.isEmpty())
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("\"%1\" does not have IPTC metadata.", importIPTCFile.fileName()),
                           i18n("Import IPTC Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     QApplication::activeWindow(),
                     i18n("IPTC metadata from the currently selected pictures will be permanently "
                          "replaced by the IPTC content of \"%1\".\n"
                          "Do you want to continue?", importIPTCFile.fileName()),
                     i18n("Import IPTC Metadata")) != KMessageBox::Yes)
        return;

    QList<QUrl>  imageURLs = images.images();
    QList<QUrl>  updatedURLs;
    QStringList errorFiles;

    for( QList<QUrl>::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        QUrl url = *it;
        bool ret = false;

        if (DMetadata::canWriteIptc(url.path()))
        {
            ret = true;
            DMetadata meta;

            ret &= meta.load(url.path());
            ret &= meta.setIptc(iptcData);
            ret &= meta.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    d->interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    QApplication::activeWindow(),
                    i18n("Unable to set IPTC metadata from:"),
                    errorFiles,
                    i18n("Import IPTC Metadata"));
    }
}

void Plugin_MetadataEdit::slotImportXmp()
{
    ImageCollection images = d->interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( d->lastSelectedDirectory.isEmpty() )
    {
        d->lastSelectedDirectory = KIO::upUrl(images.images().first());
    }
    QUrl importXMPFile = KFileDialog::getOpenUrl(d->lastSelectedDirectory,
                                                 QString(), QApplication::activeWindow(),
                                                 i18n("Select File to Import XMP metadata") );
    if( importXMPFile.isEmpty() )
       return;

    d->lastSelectedDirectory = KIO::upUrl(importXMPFile);

    DMetadata meta;
    if (!meta.load(importXMPFile.path()))
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importXMPFile.fileName()),
                           i18n("Import XMP Metadata"));
        return;
    }

    QByteArray xmpData = meta.getXmp();
    if (xmpData.isEmpty())
    {
        KMessageBox::error(QApplication::activeWindow(),
                           i18n("\"%1\" does not have XMP metadata.", importXMPFile.fileName()),
                           i18n("Import XMP Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     QApplication::activeWindow(),
                     i18n("XMP metadata from the currently selected pictures will be permanently "
                          "replaced by the XMP content of \"%1\".\n"
                          "Do you want to continue?", importXMPFile.fileName()),
                     i18n("Import XMP Metadata")) != KMessageBox::Yes)
        return;

    QList<QUrl>  imageURLs = images.images();
    QList<QUrl>  updatedURLs;
    QStringList errorFiles;

    for( QList<QUrl>::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        QUrl url = *it;
        bool ret = false;

        if (DMetadata::canWriteXmp(url.path()))
        {
            ret = true;
            DMetadata meta;

            ret &= meta.load(url.path());
            ret &= meta.setXmp(xmpData);
            ret &= meta.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    d->interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    QApplication::activeWindow(),
                    i18n("Unable to set XMP metadata from:"),
                    errorFiles,
                    i18n("Import XMP Metadata"));
    }
}

} // namespace Digikam

#include "plugin_metadataedit.moc"
