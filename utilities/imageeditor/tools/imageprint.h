/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-13
 * Description : image editor printing interface.
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef IMAGEPRINT_H
#define IMAGEPRINT_H

// Qt lib includes

#include <qwidget.h>
#include <qstring.h>

// KDE lib includes

#include <kurl.h>
#include <kprinter.h>
#include <kdeprint/kprintdialogpage.h>

namespace Digikam
{

class ImagePrintPrivate;

class ImagePrint
{
public:

    ImagePrint(DImg& image, KPrinter& printer, const QString& fileName);
    ~ImagePrint();

    bool printImageWithQt();

private:

    QString minimizeString(QString text, const QFontMetrics& metrics, int maxWidth);
    void readSettings();
    
private:

    KPrinter&          m_printer;

    ImagePrintPrivate *d;
};

///////////////////////////////////////////////////////////////////////////////////

class ImageEditorPrintDialogPagePrivate;

class ImageEditorPrintDialogPage : public KPrintDialogPage
{
    Q_OBJECT

public:

    ImageEditorPrintDialogPage( QWidget *parent = 0L, const char *name = 0 );
    ~ImageEditorPrintDialogPage();

    virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
    virtual void setOptions(const QMap<QString,QString>& opts);

private slots:

    void toggleScaling( bool enable );
    void slotSetupDlg();
    void slotAlertSettings(bool t);

private:

    void readSettings();

private:

    ImageEditorPrintDialogPagePrivate *d;
};

}  // namespace Digikam

#endif // IMAGEPRINT_H
