/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : image sharpen batch tool.
 *
 * Copyright (C) 2009 by Matthias Welwarsky <matze at welwarsky dot de>
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

#ifndef SHARPEN_H_
#define SHARPEN_H_

#include "batchtool.h"

namespace KDcrawIface
{
class RIntNumInput;
class RComboBox;
class RDoubleNumInput;
}

class QStackedWidget;

namespace Digikam
{

class Sharpen : public BatchTool
{
    Q_OBJECT
    
public:

    Sharpen(QObject *parent=0);
    ~Sharpen();
    
    BatchToolSettings defaultSettings();

private:

    enum SharpenType {
	SimpleSharp,
	UnsharpMask,
	Refocus
    };
	
    void assignSettings2Widget();
    bool toolOperations();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotSharpMethodChanged(int);

private:
    QStackedWidget               *m_stack;

    KDcrawIface::RComboBox       *m_sharpMethod;
	
	KDcrawIface::RIntNumInput    *m_radiusInput;
    KDcrawIface::RIntNumInput    *m_radiusInput2;
    KDcrawIface::RIntNumInput    *m_matrixSize;

    KDcrawIface::RDoubleNumInput *m_radius;
    KDcrawIface::RDoubleNumInput *m_gauss;
    KDcrawIface::RDoubleNumInput *m_correlation;
    KDcrawIface::RDoubleNumInput *m_noise;
    KDcrawIface::RDoubleNumInput *m_amountInput;
    KDcrawIface::RDoubleNumInput *m_thresholdInput;
    
};

}

#endif /* SHARPEN_H_ */
