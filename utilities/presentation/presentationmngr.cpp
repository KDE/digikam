/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a presentation tool.
 *
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentationmngr.h"

// C ANSI includes

extern "C"
{
#include <sys/time.h>
}

// C++ includes

#include <cstdlib>

// Qt includes

#include <QList>
#include <QPair>
#include <QStringList>
#include <QAction>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationdlg.h"
#include "presentationwidget.h"
#include "presentationcontainer.h"

#ifdef HAVE_OPENGL
#   include "presentationgl.h"
#   include "presentationkb.h"
#endif

namespace Digikam
{

PresentationMngr::PresentationMngr(QObject* const parent)
    : QObject(parent)
{
    m_sharedData = new PresentationContainer();
}

PresentationMngr::~PresentationMngr()
{
    delete m_sharedData;
}

void PresentationMngr::setItems(const QList<QUrl>& urls)
{
    m_sharedData->urlList = urls;
}

void PresentationMngr::showConfigDialog()
{
    PresentationDlg* const dlg = new PresentationDlg(QApplication::activeWindow(), m_sharedData);

    connect(dlg, SIGNAL(buttonStartClicked()),
            this, SLOT(slotSlideShow()));

    dlg->show();
}

void PresentationMngr::slotSlideShow()
{
    KConfig config;
    KConfigGroup grp = config.group("Presentation Settings");
    bool opengl      = grp.readEntry("OpenGL",  false);
    bool shuffle     = grp.readEntry("Shuffle", false);
    bool wantKB      = grp.readEntry("Effect Name (OpenGL)") == QString::fromLatin1("Ken Burns");
    QList<QUrl> urls = m_sharedData->urlList;

    if (urls.isEmpty())
    {
        QMessageBox::information(QApplication::activeWindow(), QString(), i18n("There are no images to show."));
        return;
    }

    QStringList fileList;
    QStringList commentsList;

    for (QList<QUrl>::ConstIterator urlIt = urls.constBegin(); urlIt != urls.constEnd(); ++urlIt)
    {
        fileList.append((*urlIt).toLocalFile());
        commentsList.append(QString());
    }

    urls.clear();

    if (shuffle)
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        srand(tv.tv_sec);

        QStringList::iterator it    = fileList.begin();
        QStringList::iterator it1;

        QStringList::iterator itcom = commentsList.begin();
        QStringList::iterator itcom1;

        for (uint i = 0; i < (uint) fileList.size(); ++i)
        {
            int inc = (int) (float(fileList.count()) * qrand() / (RAND_MAX + 1.0));

            it1  = fileList.begin();
            it1 += inc;

            itcom1  = commentsList.begin();
            itcom1 += inc;

            qSwap(*(it++), *(it1));
            qSwap(*(itcom++), *(itcom1));
        }
    }

    if (!opengl)
    {
        PresentationWidget* const slide = new PresentationWidget(fileList, commentsList, m_sharedData);
        slide->show();
    }
    else
    {
#ifdef HAVE_OPENGL
        if (!QGLFormat::hasOpenGL())
        {
            QMessageBox::critical(QApplication::activeWindow(), QString(),
                                  i18n("OpenGL support is not available on your system."));
        }
        else
        {
            if (wantKB)
            {
                PresentationKB* const slide = new PresentationKB(fileList, commentsList, m_sharedData);
                slide->show();
            }
            else
            {
                PresentationGL* const slide = new PresentationGL(fileList, commentsList, m_sharedData);
                slide->show();
            }
        }
#else
        Q_UNUSED(wantKB);
#endif
    }
}

}  // namespace Digikam
