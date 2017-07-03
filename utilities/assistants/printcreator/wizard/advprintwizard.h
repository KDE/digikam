/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * Copyright (C) 2008-2012 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef ADV_PRINT_WIZARD_H
#define ADV_PRINT_WIZARD_H

// QT incudes

#include <QPainter>
#include <QIcon>
#include <QPrinter>
#include <QDialog>

// Local includes

#include "dimageslist.h"
#include "dinfointerface.h"
#include "dwizarddlg.h"
#include "digikam_export.h"

namespace Digikam
{

class TemplateIcon;
class AdvPrintPhoto;

typedef struct _AdvPrintPhotoSize
{
    QString       label;
    int           dpi;
    bool          autoRotate;
    QList<QRect*> layouts;     // first element is page size
    QIcon         icon;
} AdvPrintPhotoSize;

// ---------------------------------------------------------------------------

class DIGIKAM_EXPORT AdvPrintWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit AdvPrintWizard(QWidget* const, DInfoInterface* const iface = 0);
    ~AdvPrintWizard();

    void setItemsList(const QList<QUrl>& fileList = QList<QUrl>());

public Q_SLOTS:

    virtual void accept();
    virtual void pageChanged(int curr);
    virtual void captionChanged(const QString& text);
    virtual void saveCaptionSettings();
    virtual void BtnPrintOrderUp_clicked();
    virtual void BtnPrintOrderDown_clicked();

    virtual void BtnPreviewPageDown_clicked();
    virtual void BtnPreviewPageUp_clicked();
    virtual void BtnCropRotateLeft_clicked();
    virtual void BtnCropRotateRight_clicked();
    virtual void BtnCropNext_clicked();
    virtual void BtnCropPrev_clicked();
    virtual void BtnSaveAs_clicked();
    virtual void ListPhotoSizes_selected();

    virtual void reject();
    virtual void crop_selection(int);
    virtual void slotPageRemoved(int);

    virtual void pagesetupclicked();
    virtual void pagesetupdialogexit();
    virtual void imageSelected(QTreeWidgetItem*);
    virtual void decreaseCopies();
    virtual void increaseCopies();
    virtual void infopage_updateCaptions();

    virtual void slotAddItems(const QList<QUrl>&);
    virtual void slotRemovingItem(int);
    virtual void slotContextMenuRequested();
    virtual void slotXMLSaveItem(QXmlStreamWriter&, int);
    virtual void slotXMLLoadElement(QXmlStreamReader&);
    virtual void slotXMLCustomElement(QXmlStreamWriter&);
    virtual void slotXMLCustomElement(QXmlStreamReader&);

private:

    /// Initialize page layout to the given pageSize in mm
    void initPhotoSizes(const QSizeF& pageSize);
    void previewPhotos();

    void infopage_blockCaptionButtons(bool block=true);
    void infopage_setCaptionButtons();
    void infopage_readCaptionSettings();

    /// To parse template file with 'fn' as filename, and 'pageSize' in mm.
    void parseTemplateFile( const QString& fn, const QSizeF& pageSize );

    void updateCaption(AdvPrintPhoto* const);
    void updateCropFrame(AdvPrintPhoto* const, int);
    void setBtnCropEnabled();
    void removeGimpFiles();
    void printPhotos(const QList<AdvPrintPhoto*>& photos,
                     const QList<QRect*>& layouts,
                     QPrinter& printer);
    QStringList printPhotosToFile(const QList<AdvPrintPhoto*>& photos,
                                  const QString& baseFilename,
                                  AdvPrintPhotoSize* const layouts);

    int     getPageCount()                        const;
    QRect*  getLayout(int photoIndex)             const;
    QString captionFormatter(AdvPrintPhoto* const photo) const;
    void    printCaption(QPainter& p,
                         AdvPrintPhoto* photo,
                         int captionW,
                         int captionH,
                         const QString& caption);

    bool paintOnePage(QPainter& p, const QList<AdvPrintPhoto*>& photos, const QList<QRect*>& layouts,
                      int& current, bool cropDisabled, bool useThumbnails = false);

    void manageBtnPreviewPage();

    /// Create a MxN grid of photos, fitting on the page
    void createPhotoGrid(AdvPrintPhotoSize* const p,
                         int pageWidth,
                         int pageHeight,
                         int rows,
                         int columns,
                         TemplateIcon* const iconpreview);

    double getMaxDPI(const QList<AdvPrintPhoto*>& photos,
                     const QList<QRect*>& layouts,
                     int current);

    /// Fix caption group layout according to captions combobox text
    void enableCaptionGroup(const QString& text);

    void saveSettings(const QString& pageName);
    void readSettings(const QString& pageName);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_WIZARD_H
