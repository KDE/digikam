/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * Copyright (C) 2009 by Angelo Naselli <anaselli at linux dot it>
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

#include "printconfig.h"

// Qt includes

#include <qglobal.h>
#include <QFile>

namespace Digikam
{

class PrintConfigHelper
{
    public:

        PrintConfigHelper()
            : q(0)
        {
        }

        ~PrintConfigHelper()
        {
            delete q;
        }

        PrintConfig* q;
};

Q_GLOBAL_STATIC(PrintConfigHelper, s_globalPrintConfig)

// -------------------------------------------------------

PrintConfig* PrintConfig::self()
{
    if (!s_globalPrintConfig()->q)
    {
        new PrintConfig;
        s_globalPrintConfig()->q->read();
    }

    return s_globalPrintConfig()->q;
}

PrintConfig::PrintConfig()
    : KConfigSkeleton(QLatin1String("digikamrc"))
{
    Q_ASSERT(!s_globalPrintConfig()->q);
    s_globalPrintConfig()->q = this;
    setCurrentGroup( QLatin1String( "Print" ) );

    KConfigSkeleton::ItemInt* const itemPrintPosition
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("PrintPosition"),
                                       mPrintPosition, Qt::AlignHCenter | Qt::AlignVCenter);

    addItem( itemPrintPosition, QLatin1String("PrintPosition"));

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

    KConfigSkeleton::ItemEnum* const itemPrintScaleMode
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("PrintScaleMode"),
                                        mPrintScaleMode, valuesPrintScaleMode,
                                        PrintOptionsPage::ScaleToPage);
    addItem( itemPrintScaleMode, QLatin1String( "PrintScaleMode" ) );

    KConfigSkeleton::ItemBool* const itemPrintEnlargeSmallerImages
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String( "PrintEnlargeSmallerImages" ),
                                        mPrintEnlargeSmallerImages, false);
    addItem( itemPrintEnlargeSmallerImages, QLatin1String( "PrintEnlargeSmallerImages" ) );

    KConfigSkeleton::ItemDouble* const itemPrintWidth
        = new KConfigSkeleton::ItemDouble(currentGroup(), QLatin1String( "PrintWidth" ),
                                          mPrintWidth, 15.0);
    addItem( itemPrintWidth, QLatin1String( "PrintWidth" ) );

    KConfigSkeleton::ItemDouble* const itemPrintHeight
        = new KConfigSkeleton::ItemDouble(currentGroup(), QLatin1String( "PrintHeight" ),
                                          mPrintHeight, 10.0);
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

    KConfigSkeleton::ItemEnum* const itemPrintUnit
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String( "PrintUnit" ),
                                        mPrintUnit, valuesPrintUnit, PrintOptionsPage::Centimeters);
    addItem( itemPrintUnit, QLatin1String( "PrintUnit" ) );

    KConfigSkeleton::ItemBool* const itemPrintKeepRatio
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String( "PrintKeepRatio" ),
                                        mPrintKeepRatio, true);
    addItem( itemPrintKeepRatio, QLatin1String( "PrintKeepRatio" ) );

    KConfigSkeleton::ItemBool* const itemPrintColorManaged
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String( "PrintColorManaged" ),
                                        mPrintColorManaged, false);
    addItem( itemPrintColorManaged, QLatin1String( "PrintColorManaged" ) );

    KConfigSkeleton::ItemBool* const itemPrintAutoRotate
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String( "PrintAutoRotate" ),
                                        mPrintAutoRotate, false);
    addItem( itemPrintAutoRotate, QLatin1String( "PrintAutoRotate" ) );
}

PrintConfig::~PrintConfig()
{
    s_globalPrintConfig()->q = 0;
}

} // namespace Digikam
