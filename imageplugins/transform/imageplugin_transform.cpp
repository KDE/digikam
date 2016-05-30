/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to transform image geometry.
 *
 * Copyright (C) 2005-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_transform.h"

// Qt includes

#include <QKeySequence>
#include <QAction>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "perspectivetool.h"
#include "freerotationtool.h"
#include "sheartool.h"
#include "resizetool.h"
#include "ratiocroptool.h"

#ifdef HAVE_LIBLQR_1
#include "contentawareresizetool.h"
#endif /* HAVE_LIBLQR_1 */

namespace DigikamTransformImagePlugin
{

K_PLUGIN_FACTORY( TransformFactory, registerPlugin<ImagePlugin_Transform>(); )

class ImagePlugin_Transform::Private
{
public:

    Private() :
        aspectRatioCropAction(0),
        resizeAction(0),
        contentAwareResizingAction(0),
        sheartoolAction(0),
        freerotationAction(0),
        perspectiveAction(0)
    {
    }

    QAction* aspectRatioCropAction;
    QAction* resizeAction;
    QAction* contentAwareResizingAction;
    QAction* sheartoolAction;
    QAction* freerotationAction;
    QAction* perspectiveAction;
};

ImagePlugin_Transform::ImagePlugin_Transform(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, QLatin1String("ImagePlugin_Transform")),
      d(new Private)
{
    // to load the rc file from digikam's installation path
    setComponentName(QLatin1String("digikam"), i18nc("to be displayed in shortcuts dialog", "Transformation plugins"));

    KActionCollection *ac = actionCollection();

    d->perspectiveAction = new QAction(QIcon::fromTheme(QLatin1String("perspective")), i18n("Perspective Adjustment..."), this);
    ac->addAction(QLatin1String("imageplugin_perspective"), d->perspectiveAction);
    connect(d->perspectiveAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPerspective()));

    d->sheartoolAction = new QAction(QIcon::fromTheme(QLatin1String("transform-shear-left")), i18n("Shear..."), this);
    ac->addAction(QLatin1String("imageplugin_sheartool"), d->sheartoolAction);
    connect(d->sheartoolAction, SIGNAL(triggered(bool)),
            this, SLOT(slotShearTool()));

    d->resizeAction = new QAction(QIcon::fromTheme(QLatin1String("transform-scale")), i18n("&Resize..."), this);
    ac->addAction(QLatin1String("imageplugin_resize"), d->resizeAction);
    connect(d->resizeAction, SIGNAL(triggered()),
            this, SLOT(slotResize()));

    d->aspectRatioCropAction = new QAction(QIcon::fromTheme(QLatin1String("transform-crop")), i18n("Aspect Ratio Crop..."), this);
    ac->addAction(QLatin1String("imageplugin_ratiocrop"), d->aspectRatioCropAction);
    connect(d->aspectRatioCropAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRatioCrop()));

#ifdef HAVE_LIBLQR_1

    d->contentAwareResizingAction = new QAction(QIcon::fromTheme(QLatin1String("transform-scale")), i18n("Liquid Rescale..."), this);
    ac->addAction(QLatin1String("imageplugin_contentawareresizing"), d->contentAwareResizingAction);
    //ac->setDefaultShortcut(d->contentAwareResizingAction, Qt::CTRL + Qt::SHIFT + Qt::Key_C);
    connect(d->contentAwareResizingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotContentAwareResizing()));

#endif /* HAVE_LIBLQR_1 */

    //-----------------------------------------------------------------------------------

    d->freerotationAction = new QAction(QIcon::fromTheme(QLatin1String("transform-rotate")), i18n("Free Rotation..."), this);
    ac->addAction(QLatin1String("imageplugin_freerotation"), d->freerotationAction );
    connect(d->freerotationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFreeRotation()));

    QAction* point1Action = new QAction(i18n("Set Point 1"), this);
    ac->addAction(QLatin1String("imageplugin_freerotation_point1"), point1Action);
    ac->setDefaultShortcut(point1Action, Qt::CTRL + Qt::SHIFT + Qt::Key_1);
    connect(point1Action, SIGNAL(triggered(bool)),
            this, SIGNAL(signalPoint1Action()));

    QAction* const point2Action = new QAction(i18n("Set Point 2"), this);
    ac->addAction(QLatin1String("imageplugin_freerotation_point2"), point2Action);
    ac->setDefaultShortcut(point2Action, Qt::CTRL + Qt::SHIFT + Qt::Key_2);
    connect(point2Action, SIGNAL(triggered(bool)),
            this, SIGNAL(signalPoint2Action()));

    QAction* const autoAdjustAction = new QAction(i18n("Auto Adjust"), this);
    ac->addAction(QLatin1String("imageplugin_freerotation_autoadjust"), autoAdjustAction);
    ac->setDefaultShortcut(autoAdjustAction, Qt::CTRL + Qt::SHIFT + Qt::Key_R);
    connect(autoAdjustAction, SIGNAL(triggered(bool)),
            this, SIGNAL(signalAutoAdjustAction()));

    setActionCategory(i18n("Transform"));
    setXMLFile(QLatin1String("digikamimageplugin_transform_ui.rc"));

    qCDebug(DIGIKAM_IMAGEPLUGINS_LOG) << "ImagePlugin_Transform plugin loaded";
}

ImagePlugin_Transform::~ImagePlugin_Transform()
{
    delete d;
}

void ImagePlugin_Transform::setEnabledActions(bool b)
{
    d->resizeAction->setEnabled(b);
    d->perspectiveAction->setEnabled(b);
    d->freerotationAction->setEnabled(b);
    d->sheartoolAction->setEnabled(b);
    d->aspectRatioCropAction->setEnabled(b);

#ifdef HAVE_LIBLQR_1
    d->contentAwareResizingAction->setEnabled(b);
#endif /* HAVE_LIBLQR_1 */
}

void ImagePlugin_Transform::slotPerspective()
{
    loadTool(new PerspectiveTool(this));
}

void ImagePlugin_Transform::slotShearTool()
{
    loadTool(new ShearTool(this));
}

void ImagePlugin_Transform::slotResize()
{
    loadTool(new ResizeTool(this));
}

void ImagePlugin_Transform::slotRatioCrop()
{
    loadTool(new RatioCropTool(this));
}

void ImagePlugin_Transform::slotContentAwareResizing()
{
#ifdef HAVE_LIBLQR_1
    loadTool(new ContentAwareResizeTool(this));
#endif /* HAVE_LIBLQR_1 */
}

void ImagePlugin_Transform::slotFreeRotation()
{
    FreeRotationTool* const tool = new FreeRotationTool(this);

    connect(this, SIGNAL(signalPoint1Action()),
            tool, SLOT(slotAutoAdjustP1Clicked()));

    connect(this, SIGNAL(signalPoint2Action()),
            tool, SLOT(slotAutoAdjustP2Clicked()));

    connect(this, SIGNAL(signalAutoAdjustAction()),
            tool, SLOT(slotAutoAdjustClicked()));

    loadTool(tool);
}

} // namespace DigikamTransformImagePlugin

#include "imageplugin_transform.moc"
