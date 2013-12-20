#include "showfotosettings.h"

// KDE includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "setupmisc.h"

namespace ShowFoto {

class ShowfotoSettings::Private
{

public:
    Private() :
        drawFormatOverThumbnail(false)
    {
    }

    static const QString                   configGroupDefault;

    bool                                   drawFormatOverThumbnail;

    KSharedConfigPtr                       config;
};

const QString ShowfotoSettings::Private::configGroupDefault("ImageViewer Settings");


// -------------------------------------------------------------------------------------------------

class ShowfotoSettingsCreator
{
public:

    ShowfotoSettings object;
};

K_GLOBAL_STATIC(ShowfotoSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ShowfotoSettings* ShowfotoSettings::instance()
{
    return &creator->object;
}

ShowfotoSettings::ShowfotoSettings()
    : QObject(), d(new Private)
{
    d->config = KGlobal::config();
    init();
    readSettings();
}

ShowfotoSettings::~ShowfotoSettings()
{
    delete d;
}

void ShowfotoSettings::init()
{
    d->drawFormatOverThumbnail           = false;
}

void ShowfotoSettings::readSettings()
{
    KSharedConfigPtr config         = d->config;

    KConfigGroup group              = config->group(d->configGroupDefault);

    d->drawFormatOverThumbnail      = group.readEntry("ShowMimeOverImage",false);
}

bool ShowfotoSettings::getShowFormatOverThumbnail()
{
    return d->drawFormatOverThumbnail;
}

void ShowfotoSettings::saveSettings()
{
    KSharedConfigPtr config         = d->config;

    KConfigGroup group              = config->group(d->configGroupDefault);

    group.writeEntry("ShowMimeOverImage",getShowFormatOverThumbnail());
}

}//namespace Showfoto
