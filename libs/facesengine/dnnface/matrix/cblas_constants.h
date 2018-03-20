/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-08-08
 * Description : Base functions for dnn module, can be used for face recognition, 
 *               all codes are ported from dlib library (http://dlib.net/)
 *
 * Copyright (C) 2006-2016 by Davis E. King <davis at dlib dot net>
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DLIB_CBLAS_CONSTAnTS_Hh_
#define DLIB_CBLAS_CONSTAnTS_Hh_

#ifndef CBLAS_H
//namespace dlib
//{
    namespace blas_bindings
    {
        enum CBLAS_ORDER {CblasRowMajor=101, CblasColMajor=102};
        enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
        enum CBLAS_UPLO {CblasUpper=121, CblasLower=122};
        enum CBLAS_DIAG {CblasNonUnit=131, CblasUnit=132};
        enum CBLAS_SIDE {CblasLeft=141, CblasRight=142};

    }
//}
#endif // if not CBLAS_H

#endif // DLIB_CBLAS_CONSTAnTS_Hh_

