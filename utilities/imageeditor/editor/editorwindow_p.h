/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *               private data.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EDITORWINDOWPRIVATE_H
#define EDITORWINDOWPRIVATE_H

// Qt includes

#include <QList>
#include <QString>
#include <QPropertyAnimation>

// KDE includes

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>

// Local includes

#include "daboutdata.h"
#include "editorwindow.h"
#include "versionmanager.h"

class QDialog;
class QEventLoop;
class QLabel;
class QSignalMapper;
class QToolButton;
class QWidgetAction;

class KAction;
class KActionCollection;
class KSqueezedTextLabel;
class KToggleAction;

namespace Digikam
{

class ActionCategorizedView;
class DZoomBar;
class EditorToolIface;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class PreviewToolBar;

class EditorWindow::EditorWindowPriv
{

public:

    EditorWindowPriv() :
        removeFullScreenButton(false),
        fullScreenHideToolBar(false),
        cmViewIndicator(0),
        underExposureIndicator(0),
        overExposureIndicator(0),
        infoLabel(0),
        imagepluginsActionCollection(0),
        copyAction(0),
        cropAction(0),
        filePrintAction(0),
        flipHorizAction(0),
        flipVertAction(0),
        libsInfoAction(0),
        rotateLeftAction(0),
        rotateRightAction(0),
        selectAllAction(0),
        selectNoneAction(0),
        slideShowAction(0),
        softProofOptionsAction(0),
        zoomFitToSelectAction(0),
        zoomMinusAction(0),
        zoomPlusAction(0),
        zoomTo100percents(0),
        undoSignalMapper(0),
        redoSignalMapper(0),
        formatMenuActionMapper(0),
        waitingLoop(0),
        currentWindowModalDialog(0),
        zoomFitToWindowAction(0),
        viewCMViewAction(0),
        viewSoftProofAction(0),
        viewUnderExpoAction(0),
        viewOverExpoAction(0),
        showMenuBarAction(0),
        selectToolsActionView(0),
        ICCSettings(0),
        zoomBar(0),
        previewToolBar(0),
        exposureSettings(0),
        toolIface(0),
        about(0)
    {
    }

    ~EditorWindowPriv()
    {
    }

    static const QString       configAutoZoomEntry;
    static const QString       configBackgroundColorEntry;
    static const QString       configFullScreenEntry;
    static const QString       configFullScreenHideThumbBarEntry;
    static const QString       configFullScreenHideToolBarEntry;
    static const QString       configJpeg2000CompressionEntry;
    static const QString       configJpeg2000LossLessEntry;
    static const QString       configJpegCompressionEntry;
    static const QString       configJpegSubSamplingEntry;
    static const QString       configPgfCompressionEntry;
    static const QString       configPgfLossLessEntry;
    static const QString       configPngCompressionEntry;
    static const QString       configSplitterStateEntry;
    static const QString       configTiffCompressionEntry;
    static const QString       configUnderExposureColorEntry;
    static const QString       configUnderExposureIndicatorEntry;
    static const QString       configUnderExposurePercentsEntry;
    static const QString       configOverExposureColorEntry;
    static const QString       configOverExposureIndicatorEntry;
    static const QString       configOverExposurePercentsEntry;
    static const QString       configExpoIndicatorModeEntry;
    static const QString       configUseRawImportToolEntry;
    static const QString       configUseThemeBackgroundColorEntry;
    static const QString       configVerticalSplitterSizesEntry;
    static const QString       configVerticalSplitterStateEntry;

    bool                       removeFullScreenButton;
    bool                       fullScreenHideToolBar;

    QToolButton*               cmViewIndicator;
    QToolButton*               underExposureIndicator;
    QToolButton*               overExposureIndicator;

    KSqueezedTextLabel*        infoLabel;

    KActionCollection*         imagepluginsActionCollection;

    KAction*                   contributeAction;
    KAction*                   copyAction;
    KAction*                   cropAction;
    KAction*                   donateMoneyAction;
    KAction*                   filePrintAction;
    KAction*                   flipHorizAction;
    KAction*                   flipVertAction;
    KAction*                   libsInfoAction;
    KAction*                   rawCameraListAction;
    KAction*                   rotateLeftAction;
    KAction*                   rotateRightAction;
    KAction*                   selectAllAction;
    KAction*                   selectNoneAction;
    KAction*                   slideShowAction;
    KAction*                   softProofOptionsAction;
    KAction*                   zoomFitToSelectAction;
    KAction*                   zoomMinusAction;
    KAction*                   zoomPlusAction;
    KAction*                   zoomTo100percents;

    QSignalMapper*             undoSignalMapper;
    QSignalMapper*             redoSignalMapper;
    QSignalMapper*             formatMenuActionMapper;

    QEventLoop*                waitingLoop;
    QDialog*                   currentWindowModalDialog;

    KToggleAction*             zoomFitToWindowAction;
    KToggleAction*             viewCMViewAction;
    KToggleAction*             viewSoftProofAction;
    KToggleAction*             viewUnderExpoAction;
    KToggleAction*             viewOverExpoAction;
    KToggleAction*             showMenuBarAction;

    ActionCategorizedView*     selectToolsActionView;

    QList<int>                 fullscreenSizeBackup;

    ICCSettingsContainer*      ICCSettings;

    DZoomBar*                  zoomBar;
    PreviewToolBar*            previewToolBar;

    ExposureSettingsContainer* exposureSettings;

    EditorToolIface*           toolIface;

    VersionManager             defaultVersionManager;

    DAboutData*                about;

    void legacyUpdateSplitterState(KConfigGroup& group)
    {

        // Check if the thumbnail size in the config file is splitter based (the
        // old method), and convert to dock based if needed.
        if (group.hasKey(configSplitterStateEntry))
        {
            // Read splitter state from config file
            QByteArray state;
            state = QByteArray::fromBase64(group.readEntry(configSplitterStateEntry, state));

            // Do a cheap check: a splitter state with 3 windows is always 34 bytes.
            if (state.count() == 34)
            {
                // Read the state in streamwise fashion.
                QDataStream stream(state);

                // The first 8 bytes are resp. the magic number and the version
                // (which should be 0, otherwise it's not saved with an older
                // digiKam version). Then follows the list of window sizes.
                qint32 marker;
                qint32 version = -1;
                QList<int> sizesList;

                stream >> marker;
                stream >> version;

                if (version == 0)
                {
                    stream >> sizesList;

                    if (sizesList.count() == 3)
                    {
                        kDebug() << "Found splitter based config, converting to dockbar";
                        // Remove the first entry (the thumbbar) and write the rest
                        // back. Then it should be fine.
                        sizesList.removeFirst();
                        QByteArray newData;
                        QDataStream newStream(&newData, QIODevice::WriteOnly);
                        newStream << marker;
                        newStream << version;
                        newStream << sizesList;
                        char s[24];
                        int numBytes = stream.readRawData(s, 24);
                        newStream.writeRawData(s, numBytes);
                        group.writeEntry(configSplitterStateEntry, newData.toBase64());
                    }
                }
            }
        }

    }

    void plugNewVersionInFormatAction(EditorWindow* q, KActionMenu* menuAction, const QString& text, const QString& format);
};

const QString EditorWindow::EditorWindowPriv::configAutoZoomEntry("AutoZoom");
const QString EditorWindow::EditorWindowPriv::configBackgroundColorEntry("BackgroundColor");
const QString EditorWindow::EditorWindowPriv::configFullScreenEntry("FullScreen");
const QString EditorWindow::EditorWindowPriv::configFullScreenHideThumbBarEntry("FullScreenHideThumbBar");
const QString EditorWindow::EditorWindowPriv::configFullScreenHideToolBarEntry("FullScreen Hide ToolBar");
const QString EditorWindow::EditorWindowPriv::configJpeg2000CompressionEntry("JPEG2000Compression");
const QString EditorWindow::EditorWindowPriv::configJpeg2000LossLessEntry("JPEG2000LossLess");
const QString EditorWindow::EditorWindowPriv::configJpegCompressionEntry("JPEGCompression");
const QString EditorWindow::EditorWindowPriv::configJpegSubSamplingEntry("JPEGSubSampling");
const QString EditorWindow::EditorWindowPriv::configPgfCompressionEntry("PGFCompression");
const QString EditorWindow::EditorWindowPriv::configPgfLossLessEntry("PGFLossLess");
const QString EditorWindow::EditorWindowPriv::configPngCompressionEntry("PNGCompression");
const QString EditorWindow::EditorWindowPriv::configSplitterStateEntry("SplitterState");
const QString EditorWindow::EditorWindowPriv::configTiffCompressionEntry("TIFFCompression");
const QString EditorWindow::EditorWindowPriv::configUnderExposureColorEntry("UnderExposureColor");
const QString EditorWindow::EditorWindowPriv::configUnderExposureIndicatorEntry("UnderExposureIndicator");
const QString EditorWindow::EditorWindowPriv::configUnderExposurePercentsEntry("UnderExposurePercentsEntry");
const QString EditorWindow::EditorWindowPriv::configOverExposureColorEntry("OverExposureColor");
const QString EditorWindow::EditorWindowPriv::configOverExposureIndicatorEntry("OverExposureIndicator");
const QString EditorWindow::EditorWindowPriv::configOverExposurePercentsEntry("OverExposurePercentsEntry");
const QString EditorWindow::EditorWindowPriv::configExpoIndicatorModeEntry("ExpoIndicatorMode");
const QString EditorWindow::EditorWindowPriv::configUseRawImportToolEntry("UseRawImportTool");
const QString EditorWindow::EditorWindowPriv::configUseThemeBackgroundColorEntry("UseThemeBackgroundColor");
const QString EditorWindow::EditorWindowPriv::configVerticalSplitterSizesEntry("Vertical Splitter Sizes");
const QString EditorWindow::EditorWindowPriv::configVerticalSplitterStateEntry("Vertical Splitter State");

// -----------------------------------------------------------------------------------------------------------------

class ActionCategorizedView : public KCategorizedView
{
public:

    ActionCategorizedView(QWidget* parent = 0)
        : KCategorizedView(parent)
    {
        m_horizontalScrollAnimation = new QPropertyAnimation(horizontalScrollBar(), "value", this);
        m_verticalScrollAnimation   = new QPropertyAnimation(verticalScrollBar(),   "value", this);
    }

    void setupIconMode()
    {
        setViewMode(QListView::IconMode);
        setMovement(QListView::Static);
#if KDE_IS_VERSION(4,5,0)
        setCategoryDrawer(new KCategoryDrawerV3(this)); // deprecated, but needed for KDE 4.4 compatibility
#else
        setCategoryDrawer(new KCategoryDrawerV2);       // deprecated, but needed for KDE 4.4 compatibility
#endif
        setSelectionMode(QAbstractItemView::SingleSelection);

        setMouseTracking(true);
        viewport()->setAttribute(Qt::WA_Hover);

        setFrameShape(QFrame::NoFrame);
    }

    void adjustGridSize()
    {
        // Find a suitable grid size. The delegate's size hint does never word-wrap.
        // To keep a suitable width, we want to word wrap.
        setWordWrap(true);
        int maxSize = viewOptions().decorationSize.width() * 4;
        QFontMetrics fm(viewOptions().font);
        QSize grid;

        for (int i = 0; i < model()->rowCount(); ++i)
        {
            const QModelIndex index = model()->index(i, 0);
            const QSize size        = sizeHintForIndex(index);

            if (size.width() > maxSize)
            {
                QString text        = index.data(Qt::DisplayRole).toString();
                QRect unwrappedRect = fm.boundingRect(QRect(0, 0, size.width(), size.height()), Qt::AlignLeft, text);
                QRect wrappedRect   = fm.boundingRect(QRect(0, 0, maxSize, maxSize), Qt::AlignLeft | Qt::TextWordWrap, text);
                grid                = grid.expandedTo(QSize(maxSize, size.height() + wrappedRect.height() - unwrappedRect.height()));
            }
            else
            {
                grid = grid.expandedTo(size);
            }
        }

        //grid += QSize(KDialog::spacingHint(), KDialog::spacingHint());
        setGridSize(grid);
    }

protected:

    int autoScrollDuration(float relativeDifference, QPropertyAnimation* animation)
    {
        const int minimumTime       = 1000;
        const int maxPixelPerSecond = 1000;

        int pixelToScroll           = qAbs(animation->startValue().toInt() - animation->endValue().toInt());
        int factor                  = qMax(1.0f, relativeDifference * 100); // in [1;15]

        int duration                = 1000 * pixelToScroll / maxPixelPerSecond;
        duration                    *= factor;

        return qMax(minimumTime, duration);
    }

    void autoScroll(float relativePos, QScrollBar* scrollBar, QPropertyAnimation* animation)
    {

        if (scrollBar->minimum() != scrollBar->maximum())
        {
            const float lowerPart = 0.15F;
            const float upperPart = 0.85F;

            if (relativePos > upperPart && scrollBar->value() !=  scrollBar->maximum())
            {
                animation->stop();
                animation->setStartValue(scrollBar->value());
                animation->setEndValue(scrollBar->maximum());
                animation->setDuration(autoScrollDuration(1 - relativePos, animation));
                animation->start();
            }
            else if (relativePos < lowerPart && scrollBar->value() !=  scrollBar->minimum())
            {
                animation->stop();
                animation->setStartValue(scrollBar->value());
                animation->setEndValue(scrollBar->minimum());
                animation->setDuration(autoScrollDuration(relativePos, animation));
                animation->start();
            }
            else
            {
                animation->stop();
            }
        }
    }

    void mouseMoveEvent(QMouseEvent* e)
    {
        KCategorizedView::mouseMoveEvent(e);
        autoScroll(float(e->pos().x()) / viewport()->width(),  horizontalScrollBar(), m_horizontalScrollAnimation);
        autoScroll(float(e->pos().y()) / viewport()->height(), verticalScrollBar(),   m_verticalScrollAnimation);
    }

    void leaveEvent(QEvent* e)
    {
        KCategorizedView::leaveEvent(e);
        m_horizontalScrollAnimation->stop();
        m_verticalScrollAnimation->stop();
    }

protected:

    QPropertyAnimation* m_verticalScrollAnimation;
    QPropertyAnimation* m_horizontalScrollAnimation;
};

}  // namespace Digikam

#endif /* EDITORWINDOWPRIVATE_H */
