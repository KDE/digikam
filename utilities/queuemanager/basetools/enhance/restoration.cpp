/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-19
 * Description : Restoration batch tool.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "restoration.h"
#include "restoration.moc"

// Qt includes

#include <QWidget>
#include <QLabel>

// KDE includes

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kstandarddirs.h>
#include <kurllabel.h>

// Local includes

#include "dimg.h"
#include "greycstorationsettings.h"

namespace Digikam
{

Restoration::Restoration(QObject* parent)
           : BatchTool("Restoration", BaseTool, parent), m_cimgIface(0)
{
    setToolTitle(i18n("Restoration"));
    setToolDescription(i18n("A tool to restore photographs based on Greystoration."));
    setToolIcon(KIcon(SmallIcon("restoration")));

    KVBox *vbox   = new KVBox;

    KUrlLabel *cimgLogoLabel = new KUrlLabel(vbox);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    new QLabel(i18n("Filter:"), vbox);
    m_comboBox = new KComboBox(vbox);
    m_comboBox->insertItem(ReduceUniformNoise,  i18n("Reduce Uniform Noise"));
    m_comboBox->insertItem(ReduceJPEGArtefacts, i18n("Reduce JPEG Artifacts"));
    m_comboBox->insertItem(ReduceTexturing,     i18n("Reduce Texturing"));
    m_comboBox->setWhatsThis( i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                   "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                   "<b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br/>"
                                   "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br/>"
                                   "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns "
                                   "on scanned images.</p>"));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
}

Restoration::~Restoration()
{
}

BatchToolSettings Restoration::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("RestorationMethod", ReduceUniformNoise);
    return settings;
}

void Restoration::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["RestorationMethod"].toInt());
}

void Restoration::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("RestorationMethod", (int)m_comboBox->currentIndex());
    setSettings(settings);
}

bool Restoration::toolOperations()
{
    if (!loadToDImg()) return false;

    int type = settings()["RestorationMethod"].toInt();

    GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();

    switch(type)
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    m_cimgIface = new GreycstorationIface(this);
    m_cimgIface->setMode(GreycstorationIface::Restore);
    m_cimgIface->setOriginalImage(image());
    m_cimgIface->setSettings(settings);
    m_cimgIface->setup();
    m_cimgIface->startFilterDirectly();

    if (isCancelled()) return false;

    DImg trg    = m_cimgIface->getTargetImage();
    image().putImageData(trg.bits());
    delete m_cimgIface;
    m_cimgIface = 0;

    return (savefromDImg());
}

void Restoration::cancel()
{
    if (m_cimgIface)
        m_cimgIface->cancelFilter();
    BatchTool::cancel();
}

}  // namespace Digikam
