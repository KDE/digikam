/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * Copyright (C) 2005-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "resizetool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCustomEvent>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "dimgbuiltinfilter.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationfilter.h"
#include "greycstorationsettings.h"

using namespace KDcrawIface;

namespace DigikamTransformImagePlugin
{

// -------------------------------------------------------------

class ResizeTool::Private
{
public:

    Private() :
        orgWidth(0),
        orgHeight(0),
        prevW(0),
        prevH(0),
        prevWP(0.0),
        prevHP(0.0),
        restorationTips(0),
        preserveRatioBox(0),
        useGreycstorationBox(0),
        mainTab(0),
        cimgLogoLabel(0),
        previewWidget(0),
        wInput(0),
        hInput(0),
        wpInput(0),
        hpInput(0),
        gboxSettings(0),
        settingsWidget(0)
    {}

    static const QString    configGroupName;
    static const QString    configFastApproxEntry;
    static const QString    configInterpolationEntry;
    static const QString    configAmplitudeEntry;
    static const QString    configSharpnessEntry;
    static const QString    configAnisotropyEntry;
    static const QString    configAlphaEntry;
    static const QString    configSigmaEntry;
    static const QString    configGaussPrecEntry;
    static const QString    configDlEntry;
    static const QString    configDaEntry;
    static const QString    configIterationEntry;
    static const QString    configTileEntry;
    static const QString    configBTileEntry;

    int                     orgWidth;
    int                     orgHeight;
    int                     prevW;
    int                     prevH;

    double                  prevWP;
    double                  prevHP;

    QLabel*                 restorationTips;

    QCheckBox*              preserveRatioBox;
    QCheckBox*              useGreycstorationBox;

    KTabWidget*             mainTab;

    KUrlLabel*              cimgLogoLabel;

    ImageGuideWidget*       previewWidget;

    RIntNumInput*           wInput;
    RIntNumInput*           hInput;

    RDoubleNumInput*        wpInput;
    RDoubleNumInput*        hpInput;

    EditorToolSettings*     gboxSettings;
    GreycstorationSettings* settingsWidget;
};

const QString ResizeTool::Private::configGroupName("resize Tool");
const QString ResizeTool::Private::configFastApproxEntry("FastApprox");
const QString ResizeTool::Private::configInterpolationEntry("Interpolation");
const QString ResizeTool::Private::configAmplitudeEntry("Amplitude");
const QString ResizeTool::Private::configSharpnessEntry("Sharpness");
const QString ResizeTool::Private::configAnisotropyEntry("Anisotropy");
const QString ResizeTool::Private::configAlphaEntry("Alpha");
const QString ResizeTool::Private::configSigmaEntry("Sigma");
const QString ResizeTool::Private::configGaussPrecEntry("GaussPrec");
const QString ResizeTool::Private::configDlEntry("Dl");
const QString ResizeTool::Private::configDaEntry("Da");
const QString ResizeTool::Private::configIterationEntry("Iteration");
const QString ResizeTool::Private::configTileEntry("Tile");
const QString ResizeTool::Private::configBTileEntry("BTile");

// -------------------------------------------------------------

ResizeTool::ResizeTool(QObject* const parent)
    : EditorToolThreaded(parent), d(new Private)
{
    setObjectName("resizeimage");
    setToolName(i18n("Resize Image"));
    setToolIcon(SmallIcon("transform-scale"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode, Qt::red, 1, false);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Try|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Cancel);

    ImageIface iface;
    d->orgWidth  = iface.originalSize().width();
    d->orgHeight = iface.originalSize().height();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    d->mainTab               = new KTabWidget();
    QWidget* const firstPage = new QWidget(d->mainTab);
    QGridLayout* const grid  = new QGridLayout(firstPage);

    d->mainTab->addTab(firstPage, i18n("New Size"));

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    QLabel* const label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput            = new RIntNumInput(firstPage);
    d->wInput->setRange(1, qMax(d->orgWidth * 10, 9999), 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setSliderEnabled(true);
    d->wInput->setObjectName("wInput");
    d->wInput->setWhatsThis( i18n("Set here the new image width in pixels."));

    QLabel* const label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput            = new RIntNumInput(firstPage);
    d->hInput->setRange(1, qMax(d->orgHeight * 10, 9999), 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setSliderEnabled(true);
    d->hInput->setObjectName("hInput");
    d->hInput->setWhatsThis( i18n("New image height in pixels (px)."));

    QLabel* const label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput           = new RDoubleNumInput(firstPage);
    d->wpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName("wpInput");
    d->wpInput->setWhatsThis( i18n("New image width in percent (%)."));

    QLabel* const label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput           = new RDoubleNumInput(firstPage);
    d->hpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName("hpInput");
    d->hpInput->setWhatsThis( i18n("New image height in percent (%)."));

    d->cimgLogoLabel = new KUrlLabel(firstPage);
    d->cimgLogoLabel->setText(QString());
    d->cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    d->cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    d->cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("Enable this option to scale-up an image to a huge size. "
                                           "<b>Warning</b>: This process can take some time."));

    d->restorationTips = new QLabel(i18n("<b>Note:</b> use Restoration Mode to scale-up an image to a huge size. "
                                         "This process can take some time."), firstPage);
    d->restorationTips->setWordWrap(true);

    grid->addWidget(d->preserveRatioBox,       0, 0, 1, 3);
    grid->addWidget(label1,                    1, 0, 1, 1);
    grid->addWidget(d->wInput,                 1, 1, 1, 2);
    grid->addWidget(label2,                    2, 0, 1, 1);
    grid->addWidget(d->hInput,                 2, 1, 1, 2);
    grid->addWidget(label3,                    3, 0, 1, 1);
    grid->addWidget(d->wpInput,                3, 1, 1, 2);
    grid->addWidget(label4,                    4, 0, 1, 1);
    grid->addWidget(d->hpInput,                4, 1, 1, 2);
    grid->addWidget(new KSeparator(firstPage), 5, 0, 1, 3);
    grid->addWidget(d->cimgLogoLabel,          6, 0, 3, 1);
    grid->addWidget(d->useGreycstorationBox,   6, 1, 1, 2);
    grid->addWidget(d->restorationTips,        7, 1, 1, 2);
    grid->setRowStretch(8, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationSettings(d->mainTab);

    // -------------------------------------------------------------

    QGridLayout* grid2 = new QGridLayout();
    grid2->addWidget(d->mainTab, 0, 1, 1, 1);
    grid2->setMargin(d->gboxSettings->spacingHint());
    grid2->setSpacing(d->gboxSettings->spacingHint());
    grid2->setRowStretch(1, 10);
    d->gboxSettings->plainPage()->setLayout(grid2);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->cimgLogoLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(processCImgUrl(QString)));

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->useGreycstorationBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRestorationToggled(bool)) );

    // -------------------------------------------------------------

    GreycstorationContainer defaults;
    defaults.setResizeDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

ResizeTool::~ResizeTool()
{
    delete d;
}

void ResizeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationContainer prm;
    GreycstorationContainer defaults;
    defaults.setResizeDefaultSettings();

    prm.fastApprox = group.readEntry(d->configFastApproxEntry,    defaults.fastApprox);
    prm.interp     = group.readEntry(d->configInterpolationEntry, defaults.interp);
    prm.amplitude  = group.readEntry(d->configAmplitudeEntry,     (double)defaults.amplitude);
    prm.sharpness  = group.readEntry(d->configSharpnessEntry,     (double)defaults.sharpness);
    prm.anisotropy = group.readEntry(d->configAnisotropyEntry,    (double)defaults.anisotropy);
    prm.alpha      = group.readEntry(d->configAlphaEntry,         (double)defaults.alpha);
    prm.sigma      = group.readEntry(d->configSigmaEntry,         (double)defaults.sigma);
    prm.gaussPrec  = group.readEntry(d->configGaussPrecEntry,     (double)defaults.gaussPrec);
    prm.dl         = group.readEntry(d->configDlEntry,            (double)defaults.dl);
    prm.da         = group.readEntry(d->configDaEntry,            (double)defaults.da);
    prm.nbIter     = group.readEntry(d->configIterationEntry,     defaults.nbIter);
    prm.tile       = group.readEntry(d->configTileEntry,          defaults.tile);
    prm.btile      = group.readEntry(d->configBTileEntry,         defaults.btile);
    d->settingsWidget->setSettings(prm);
}

void ResizeTool::writeSettings()
{
    GreycstorationContainer prm = d->settingsWidget->settings();
    KConfigGroup group          = KGlobal::config()->group(d->configGroupName);

    group.writeEntry(d->configFastApproxEntry,    prm.fastApprox);
    group.writeEntry(d->configInterpolationEntry, prm.interp);
    group.writeEntry(d->configAmplitudeEntry,     (double)prm.amplitude);
    group.writeEntry(d->configSharpnessEntry,     (double)prm.sharpness);
    group.writeEntry(d->configAnisotropyEntry,    (double)prm.anisotropy);
    group.writeEntry(d->configAlphaEntry,         (double)prm.alpha);
    group.writeEntry(d->configSigmaEntry,         (double)prm.sigma);
    group.writeEntry(d->configGaussPrecEntry,     (double)prm.gaussPrec);
    group.writeEntry(d->configDlEntry,            (double)prm.dl);
    group.writeEntry(d->configDaEntry,            (double)prm.da);
    group.writeEntry(d->configIterationEntry,     prm.nbIter);
    group.writeEntry(d->configTileEntry,          prm.tile);
    group.writeEntry(d->configBTileEntry,         prm.btile);
    group.writeEntry("RestorePhotograph",         d->useGreycstorationBox->isChecked());
    group.sync();
}

void ResizeTool::slotResetSettings()
{
    GreycstorationContainer prm;
    prm.setResizeDefaultSettings();

    d->settingsWidget->setSettings(prm);
    d->useGreycstorationBox->setChecked(false);
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    blockWidgetSignals(true);

    d->preserveRatioBox->setChecked(true);
    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();

    blockWidgetSignals(false);
}

void ResizeTool::slotValuesChanged()
{
    blockWidgetSignals(true);

    QString s(sender()->objectName());

    if (s == "wInput")
    {
        double val  = d->wInput->value();
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(pval * d->orgHeight / 100);

            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hInput")
    {
        double val  = d->hInput->value();
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(pval * d->orgWidth / 100);

            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if (s == "wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val * d->orgWidth / 100);

        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(val * d->orgHeight / 100);

            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hpInput")
    {
        double val = d->hpInput->value();
        int h = (int)(val * d->orgHeight / 100);

        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(val * d->orgWidth / 100);

            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ResizeTool::preparePreview()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    ImageIface* const iface = d->previewWidget->imageIface();
    DImg* imTemp             = iface->original();

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(new GreycstorationFilter(imTemp,
                                           d->settingsWidget->settings(),
                                           GreycstorationFilter::Resize,
                                           d->wInput->value(), d->hInput->value(),
                                           QImage(),
                                           this));
    }
    else
    {
        // See bug #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        DImgBuiltinFilter resize(DImgBuiltinFilter::Resize, QSize(d->wInput->value(), d->hInput->value()));
        setFilter(resize.createThreadedFilter(imTemp, this));
    }
}

void ResizeTool::prepareFinal()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    d->mainTab->setCurrentIndex(0);

    ImageIface iface;

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(new GreycstorationFilter(iface.original(),
                                           d->settingsWidget->settings(),
                                           GreycstorationFilter::Resize,
                                           d->wInput->value(),
                                           d->hInput->value(),
                                           QImage(),
                                           this));
    }
    else
    {
        // See bug #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        DImgBuiltinFilter resize(DImgBuiltinFilter::Resize, QSize(d->wInput->value(), d->hInput->value()));
        setFilter(resize.createThreadedFilter(iface.original(), this));
    }
}

void ResizeTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    int w                   = iface->previewSize().width();
    int h                   = iface->previewSize().height();
    DImg imTemp             = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha());

    QColor background = toolView()->backgroundRole();
    imDest.fill(DColor(background, filter()->getTargetImage().sixteenBit()));
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest.smoothScale(iface->previewSize()));
    d->previewWidget->updatePreview();
}

void ResizeTool::renderingFinished()
{
    d->settingsWidget->setEnabled(d->useGreycstorationBox->isChecked());
}

void ResizeTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Resize"),
                           filter()->filterAction(),
                           targetImage);
}

void ResizeTool::blockWidgetSignals(bool b)
{
    d->preserveRatioBox->blockSignals(b);
    d->wInput->blockSignals(b);
    d->hInput->blockSignals(b);
    d->wpInput->blockSignals(b);
    d->hpInput->blockSignals(b);
}

void ResizeTool::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->cimgLogoLabel->setEnabled(b);
    toolSettings()->enableButton(EditorToolSettings::Load, b);
    toolSettings()->enableButton(EditorToolSettings::SaveAs, b);
}

void ResizeTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ResizeTool::slotLoadSettings()
{
    KUrl loadBlowupFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                          QString( "*" ), kapp->activeWindow(),
                          QString( i18n("Photograph Resizing Settings File to Load")) );

    if ( loadBlowupFile.isEmpty() )
    {
        return;
    }

    QFile file(loadBlowupFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Resizing settings text file.",
                                    loadBlowupFile.fileName()));
            file.close();
            return;
        }
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Photograph Resizing text file."));
    }

    file.close();
}

void ResizeTool::slotSaveAsSettings()
{
    KUrl saveBlowupFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                          QString( "*" ), kapp->activeWindow(),
                          QString( i18n("Photograph Resizing Settings File to Save")) );

    if ( saveBlowupFile.isEmpty() )
    {
        return;
    }

    QFile file(saveBlowupFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        d->settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Resizing text file."));
    }

    file.close();
}

}  // namespace DigikamTransformImagePlugin
