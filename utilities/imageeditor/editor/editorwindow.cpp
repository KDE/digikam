/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright 2006 by Gilles Caulier
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

// KDE includes.

#include <kapplication.h>
#include <kprinter.h>
#include <kkeydialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>
#include <kmessagebox.h>

// Local includes.

#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "savingcontextcontainer.h"
#include "editorwindowprivate.h"
#include "editorwindow.h"

namespace Digikam
{

EditorWindow::EditorWindow(const char *name)
            : KMainWindow(0, name, WType_TopLevel|WDestructiveClose)
{
    d = new EditorWindowPriv;
    m_canvas     = 0;
    m_undoAction = 0;
    m_redoAction = 0;
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete d;
}

void EditorWindow::printImage(KURL url)
{
    uchar* ptr      = DImgInterface::instance()->getImage();
    int w           = DImgInterface::instance()->origWidth();
    int h           = DImgInterface::instance()->origHeight();
    bool hasAlpha   = DImgInterface::instance()->hasAlpha();
    bool sixteenBit = DImgInterface::instance()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    KPrinter printer;
    QString appName = KApplication::kApplication()->aboutData()->appName();
    printer.setDocName( url.filename() );
    printer.setCreator( appName );
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, (appName.append(" page")).ascii() ));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        ImagePrint printOperations(image, printer, url.filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(url.filename()));
        }
    }
}

void EditorWindow::slotImagePluginsHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "digikamimageplugins" );
}

void EditorWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );
    ImagePluginLoader* loader = ImagePluginLoader::instance();
    
    for (ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next())
    {
        if (plugin)
        {
            dialog.insert( plugin->actionCollection(), plugin->name() );
        }
    }
    
    dialog.configure();
}

void EditorWindow::slotResize()
{
    int width  = m_canvas->imageWidth();
    int height = m_canvas->imageHeight();

    Digikam::ImageResizeDlg dlg(this, &width, &height);
    
    if (dlg.exec() == QDialog::Accepted &&
        (width != m_canvas->imageWidth() ||
         height != m_canvas->imageHeight()))
        m_canvas->resizeImage(width, height);
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_undoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_redoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

}  // namespace Digikam

#include "editorwindow.moc"
