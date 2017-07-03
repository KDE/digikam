/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-08
 * Description : a tool to print images
 *
 * Copyright (C) 2009-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintimagesconfig.h"

// Qt includes

#include <qglobal.h>
#include <QFile>

namespace Digikam
{

class AdvPrintImagesConfigHelper
{
public:

    explicit AdvPrintImagesConfigHelper()
        : q(0)
    {
    }

    ~AdvPrintImagesConfigHelper()
    {
        delete q;
    }

    AdvPrintImagesConfig* q;
};


Q_GLOBAL_STATIC(AdvPrintImagesConfigHelper, s_globalAdvPrintImagesConfig)

AdvPrintImagesConfig* AdvPrintImagesConfig::self()
{
    if (!s_globalAdvPrintImagesConfig()->q)
    {
        new AdvPrintImagesConfig;
        s_globalAdvPrintImagesConfig()->q->read();
    }

    return s_globalAdvPrintImagesConfig()->q;
}

AdvPrintImagesConfig::AdvPrintImagesConfig()
    : KConfigSkeleton(QLatin1String("printcreatorrc"))
{
    Q_ASSERT(!s_globalAdvPrintImagesConfig()->q);

    s_globalAdvPrintImagesConfig()->q = this;
    setCurrentGroup( QLatin1String( "Print" ) );

    KConfigSkeleton::ItemInt* itemPrintPosition = new KConfigSkeleton::ItemInt( currentGroup(), QLatin1String( "PrintPosition" ), mPrintPosition, Qt::AlignHCenter | Qt::AlignVCenter);
    addItem( itemPrintPosition, QLatin1String( "PrintPosition" ) );

    QList<KConfigSkeleton::ItemEnum::Choice> valuesPrintScaleMode;
    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::NoScale");
        valuesPrintScaleMode.append( choice );
    }

    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::ScaleToPage");
        valuesPrintScaleMode.append( choice );
    }

    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::ScaleToCustomSize");
        valuesPrintScaleMode.append( choice );
    }

    KConfigSkeleton::ItemEnum* itemPrintScaleMode = new KConfigSkeleton::ItemEnum(currentGroup(),
                                                        QLatin1String("PrintScaleMode"),
                                                        mPrintScaleMode,
                                                        valuesPrintScaleMode,
                                                        Digikam::AdvPrintOptionsPage::ScaleToPage);
    addItem( itemPrintScaleMode, QLatin1String( "PrintScaleMode" ) );

    KConfigSkeleton::ItemBool* itemPrintEnlargeSmallerImages = new KConfigSkeleton::ItemBool( currentGroup(), QLatin1String( "PrintEnlargeSmallerImages" ), mPrintEnlargeSmallerImages, false );
    addItem( itemPrintEnlargeSmallerImages, QLatin1String( "PrintEnlargeSmallerImages" ) );

    KConfigSkeleton::ItemDouble* itemPrintWidth = new KConfigSkeleton::ItemDouble( currentGroup(), QLatin1String( "PrintWidth" ), mPrintWidth, 15.0 );
    addItem( itemPrintWidth, QLatin1String( "PrintWidth" ) );

    KConfigSkeleton::ItemDouble* itemPrintHeight = new KConfigSkeleton::ItemDouble( currentGroup(), QLatin1String( "PrintHeight" ), mPrintHeight, 10.0 );
    addItem( itemPrintHeight, QLatin1String( "PrintHeight" ) );

    QList<KConfigSkeleton::ItemEnum::Choice> valuesPrintUnit;
    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::Millimeters");
        valuesPrintUnit.append( choice );
    }

    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::Centimeters");
        valuesPrintUnit.append( choice );
    }

    {
        KConfigSkeleton::ItemEnum::Choice choice;
        choice.name = QLatin1String("PrintOptionsPage::Inches");
        valuesPrintUnit.append( choice );
    }

    KConfigSkeleton::ItemEnum* itemPrintUnit = new KConfigSkeleton::ItemEnum( currentGroup(), QLatin1String( "PrintUnit" ), mPrintUnit, valuesPrintUnit, Digikam::AdvPrintOptionsPage::Centimeters );
    addItem( itemPrintUnit, QLatin1String( "PrintUnit" ) );

    KConfigSkeleton::ItemBool* itemPrintKeepRatio = new KConfigSkeleton::ItemBool( currentGroup(), QLatin1String( "PrintKeepRatio" ), mPrintKeepRatio, true );
    addItem( itemPrintKeepRatio, QLatin1String( "PrintKeepRatio" ) );

    KConfigSkeleton::ItemBool*itemPrintAutoRotate = new KConfigSkeleton::ItemBool( currentGroup(), QLatin1String( "PrintAutoRotate" ), mPrintAutoRotate, false );
    addItem( itemPrintAutoRotate, QLatin1String( "PrintAutoRotate" ) );
}

AdvPrintImagesConfig::~AdvPrintImagesConfig()
{
    s_globalAdvPrintImagesConfig()->q = 0;
}

} // namespace Digikam
