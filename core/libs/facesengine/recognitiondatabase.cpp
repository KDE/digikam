/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2010-06-16
 * Description : The recognition database wrapper
 *
 * Copyright (C)      2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_config.h"

// OpenCV includes need to show up before Qt includes
#include "opencvfisherfacerecognizer.h"
#include "opencveigenfacerecognizer.h"
#include "opencvlbphfacerecognizer.h"
#include "funnelreal.h"

#ifdef HAVE_FACESENGINE_DNN
#   include "opencvdnnfacerecognizer.h"
#endif

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QUuid>
#include <QDir>
#include <QStandardPaths>

// Local includes

#include "simpletrainingdataprovider.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"
#include "facedbaccess.h"
#include "facedboperationgroup.h"
#include "dataproviders.h"
#include "recognitiondatabase.h"            // krazy:exclude=includes
#include "algs.h"
#include "facedb.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN RecognitionDatabase::Private
{
public:

    bool                                    dbAvailable;
    QMutex                                  mutex;
    QVariantMap                             parameters;
    QHash<int, Identity>                    identityCache;
    RecognitionDatabase::RecognizeAlgorithm recognizeAlgorithm;

public:

    explicit Private();
    ~Private();

public:

    template <class T>
    T* getObjectOrCreate(T* &ptr) const
    {
        if (!ptr)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "create recognizer";
            ptr = new T();
        }

        return ptr;
    }

public:

    OpenCVLBPHFaceRecognizer*   lbph()               { return getObjectOrCreate(opencvlbph);   }
    OpenCVLBPHFaceRecognizer*   lbphConst() const    { return opencvlbph;                      }

    OpenCVEIGENFaceRecognizer*  eigen()              { return getObjectOrCreate(opencveigen);  }
    OpenCVEIGENFaceRecognizer*  eigenConst() const   { return opencveigen;                     }

    OpenCVFISHERFaceRecognizer* fisher()             { return getObjectOrCreate(opencvfisher); }
    OpenCVFISHERFaceRecognizer* fisherConst() const  { return opencvfisher;                    }

#ifdef HAVE_FACESENGINE_DNN
    OpenCVDNNFaceRecognizer*    dnn()                { return getObjectOrCreate(opencvdnn);    }
    OpenCVDNNFaceRecognizer*    dnnConst() const     { return opencvdnn;                       }
#endif

    FunnelReal*                 aligner()            { return getObjectOrCreate(funnel);       }
    FunnelReal*                 alignerConst() const { return funnel;                          }

public:

    void applyParameters();

    void train(OpenCVLBPHFaceRecognizer* const r, const QList<Identity>& identitiesToBeTrained,
               TrainingDataProvider* const data, const QString& trainingContext);
    void train(OpenCVEIGENFaceRecognizer* const r, const QList<Identity>& identitiesToBeTrained,
               TrainingDataProvider* const data, const QString& trainingContext);

#ifdef HAVE_FACESENGINE_DNN
    void train(OpenCVDNNFaceRecognizer* const r, const QList<Identity>& identitiesToBeTrained,
               TrainingDataProvider* const data, const QString& trainingContext);
#endif

    void clear(OpenCVLBPHFaceRecognizer* const, const QList<int>& idsToClear, const QString& trainingContext);
    void clear(OpenCVEIGENFaceRecognizer* const, const QList<int>& idsToClear, const QString& trainingContext);
    void clear(OpenCVFISHERFaceRecognizer* const, const QList<int>& idsToClear, const QString& trainingContext);

#ifdef HAVE_FACESENGINE_DNN
    void clear(OpenCVDNNFaceRecognizer* const, const QList<int>& idsToClear, const QString& trainingContext);
#endif

    cv::Mat preprocessingChain(const QImage& image);
    cv::Mat preprocessingChainRGB(const QImage& image);

public:

    bool     identityContains(const Identity& identity, const QString& attribute, const QString& value) const;
    Identity findByAttribute(const QString& attribute, const QString& value) const;
    Identity findByAttributes(const QString& attribute, const QMap<QString, QString>& valueMap) const;

private:

    OpenCVFISHERFaceRecognizer* opencvfisher;
    OpenCVEIGENFaceRecognizer*  opencveigen;
    OpenCVLBPHFaceRecognizer*   opencvlbph;

#ifdef HAVE_FACESENGINE_DNN
    OpenCVDNNFaceRecognizer*    opencvdnn;
#endif

    FunnelReal*                 funnel;
};

// ----------------------------------------------------------------------------------------------

/// Training where the train method takes one identity and one image
/*
template <class Recognizer>
static void trainSingle(Recognizer* const r, const Identity& identity, TrainingDataProvider* const data,
                        const QString& trainingContext, RecognitionDatabase::Private* const d)
{
    ImageListProvider* const images = data->newImages(identity);

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Training " << images->size() << " images for identity " << identity.id();

    for (; !images->atEnd(); images->proceed())
    {
        try
        {
            r->train(identity.id(), d->preprocessingChain(images->image()), trainingContext);
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception:" << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        }
    }
}
*/

/** Training where the train method takes a list of identities and images,
 *  and updating per-identity is non-inferior to updating all at once.
 */
static void trainIdentityBatchLBPH(OpenCVLBPHFaceRecognizer* const r,
                                   const QList<Identity>& identitiesToBeTrained,
                                   TrainingDataProvider* const data,
                                   const QString& trainingContext,
                                   RecognitionDatabase::Private* const d)
{
    foreach (const Identity& identity, identitiesToBeTrained)
    {
        std::vector<int>     labels;
        std::vector<cv::Mat> images;

        ImageListProvider* const imageList = data->newImages(identity);
        images.reserve(imageList->size());

        for (; !imageList->atEnd() ; imageList->proceed())
        {
            try
            {
                cv::Mat cvImage     = d->preprocessingChain(imageList->image());
                cv::Mat cvImage_rgb = d->preprocessingChainRGB(imageList->image());

                labels.push_back(identity.id());
                images.push_back(cvImage);
            }
            catch (cv::Exception& e)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception preparing image for LBPH:" << e.what();
            }
            catch (...)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
            }
        }

        qCDebug(DIGIKAM_FACESENGINE_LOG) << "LBPH Training " << images.size() << " images for identity " << identity.id();

        try
        {
            r->train(images, labels, trainingContext);
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception training LBPH Recognizer:" << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        }
    }
}

static void trainIdentityBatchEIGEN(OpenCVEIGENFaceRecognizer* const r,
                                    const QList<Identity>& identitiesToBeTrained,
                                    TrainingDataProvider* const data,
                                    const QString& trainingContext,
                                    RecognitionDatabase::Private* const d)
{
    foreach (const Identity& identity, identitiesToBeTrained)
    {
        std::vector<int>     labels;
        std::vector<cv::Mat> images;
        std::vector<cv::Mat> images_rgb;

        ImageListProvider* const imageList = data->newImages(identity);
        images.reserve(imageList->size());

        for (; !imageList->atEnd() ; imageList->proceed())
        {
            try
            {
                cv::Mat cvImage     = d->preprocessingChain(imageList->image());
                cv::Mat cvImage_rgb = d->preprocessingChainRGB(imageList->image());

                labels.push_back(identity.id());
                images.push_back(cvImage);
                images_rgb.push_back(cvImage_rgb);
            }
            catch (cv::Exception& e)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception preparing image for Eigen:" << e.what();
            }
            catch (...)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
            }
        }

        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Eigen Training " << images.size() << " images for identity " << identity.id();

        try
        {
            r->train(images, labels, trainingContext, images_rgb);
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception training Eigen Recognizer:" << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        }
    }
}

#ifdef HAVE_FACESENGINE_DNN
static void trainIdentityBatchDNN(OpenCVDNNFaceRecognizer* const r,
                                  const QList<Identity>& identitiesToBeTrained,
                                  TrainingDataProvider* const data,
                                  const QString& trainingContext,
                                  RecognitionDatabase::Private* const d)
{
    foreach (const Identity& identity, identitiesToBeTrained)
    {
        std::vector<int>     labels;
        std::vector<cv::Mat> images;
        std::vector<cv::Mat> images_rgb;

        ImageListProvider* const imageList = data->newImages(identity);
        images.reserve(imageList->size());

        for (; !imageList->atEnd() ; imageList->proceed())
        {
            try
            {
                cv::Mat cvImage     = d->preprocessingChain(imageList->image());
                cv::Mat cvImage_rgb = d->preprocessingChainRGB(imageList->image());

                labels.push_back(identity.id());
                images.push_back(cvImage);
                images_rgb.push_back(cvImage_rgb);
            }
            catch (cv::Exception& e)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception preparing image for DNN:" << e.what();
            }
            catch (...)
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
            }
        }

        qCDebug(DIGIKAM_FACESENGINE_LOG) << "DNN Training " << images.size() << " images for identity " << identity.id();

        try
        {
            r->train(images, labels, trainingContext, images_rgb);
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception training DNN Recognizer:" << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        }
    }
}
#endif

// ----------------------------------------------------------------------------------------------

RecognitionDatabase::Private::Private()
    : mutex(QMutex::Recursive),
      opencvfisher(0),
      opencveigen(0),
      opencvlbph(0),
#ifdef HAVE_FACESENGINE_DNN
      opencvdnn(0),
#endif
      funnel(0)
{
    DbEngineParameters params = CoreDbAccess::parameters().faceParameters();
    params.setFaceDatabasePath(CoreDbAccess::parameters().faceParameters().getFaceDatabaseNameOrDir());
    FaceDbAccess::setParameters(params);
    dbAvailable               = FaceDbAccess::checkReadyForUse(0);
    recognizeAlgorithm        = RecognizeAlgorithm::LBP;

    if (dbAvailable)
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Face database ready for use";

        foreach (const Identity& identity, FaceDbAccess().db()->identities())
        {
            identityCache[identity.id()] = identity;
        }
    }
    else
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Failed to initialize face database";
    }
}

RecognitionDatabase::Private::~Private()
{
    delete opencvfisher;
    delete opencveigen;
    delete opencvlbph;
#ifdef HAVE_FACESENGINE_DNN
    delete opencvdnn;
#endif
    delete funnel;
}

// NOTE: Takes care that there may be multiple values of attribute in identity's attributes
bool RecognitionDatabase::Private::identityContains(const Identity& identity, const QString& attribute, const QString& value) const
{
    const QMap<QString, QString> map          = identity.attributesMap();
    QMap<QString, QString>::const_iterator it = map.constFind(attribute);

    for (; it != map.constEnd() && it.key() == attribute; ++it)
    {
        if (it.value() == value)
        {
            return true;
        }
    }

    return false;
}

Identity RecognitionDatabase::Private::findByAttribute(const QString& attribute, const QString& value) const
{
    foreach (const Identity& identity, identityCache)
    {
        if (identityContains(identity, attribute, value))
        {
            return identity;
        }
    }

    return Identity();
}

// NOTE: Takes care that there may be multiple values of attribute in valueMap
Identity RecognitionDatabase::Private::findByAttributes(const QString& attribute, const QMap<QString, QString>& valueMap) const
{
    QMap<QString, QString>::const_iterator it = valueMap.find(attribute);

    for (; it != valueMap.end() && it.key() == attribute ; ++it)
    {
        foreach (const Identity& identity, identityCache)
        {
            if (identityContains(identity, attribute, it.value()))
            {
                return identity;
            }
        }
    }

    return Identity();
}

void RecognitionDatabase::Private::applyParameters()
{
    if (lbphConst()   ||
        eigenConst()  ||
#ifdef HAVE_FACESENGINE_DNN
        dnnConst()    ||
#endif
        fisherConst())
    {
        for (QVariantMap::const_iterator it = parameters.constBegin() ; it != parameters.constEnd() ; ++it)
        {
            if (it.key() == QLatin1String("threshold") || it.key() == QLatin1String("accuracy"))
            {
                if (recognizeAlgorithm == RecognitionDatabase::RecognizeAlgorithm::LBP)
                {
                    lbph()->setThreshold(it.value().toFloat());
                }
                else if (recognizeAlgorithm == RecognitionDatabase::RecognizeAlgorithm::EigenFace)
                {
                    eigen()->setThreshold(it.value().toFloat());
                }
                else if (recognizeAlgorithm == RecognitionDatabase::RecognizeAlgorithm::FisherFace)
                {
                    fisher()->setThreshold(it.value().toFloat());
                }
#ifdef HAVE_FACESENGINE_DNN
                else if (recognizeAlgorithm == RecognitionDatabase::RecognizeAlgorithm::DNN)
                {
                    dnn()->setThreshold(it.value().toFloat());
                }
#endif
                else
                {
                    qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
                }
            }
        }
    }
}

cv::Mat RecognitionDatabase::Private::preprocessingChain(const QImage& image)
{
    try
    {
        cv::Mat cvImage;

        if (recognizeAlgorithm == RecognizeAlgorithm::LBP)
        {
            cvImage = lbph()->prepareForRecognition(image);
        }
        else if (recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
        {
            cvImage = eigen()->prepareForRecognition(image);
        }
        else if (recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
        {
            cvImage = fisher()->prepareForRecognition(image);
        }
#ifdef HAVE_FACESENGINE_DNN
        else if (recognizeAlgorithm == RecognizeAlgorithm::DNN)
        {
            cvImage = dnn()->prepareForRecognition(image);
        }
#endif
        else
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
        }
/*
        cvImage         = aligner()->align(cvImage);
        TanTriggsPreprocessor preprocessor;
        cvImage         = preprocessor.preprocess(cvImage);
*/
        return cvImage;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception:" << e.what();
        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        return cv::Mat();
    }
}

cv::Mat RecognitionDatabase::Private::preprocessingChainRGB(const QImage& image)
{
    try
    {
        cv::Mat cvImage;

        if (recognizeAlgorithm == RecognizeAlgorithm::LBP)
        {
            cvImage = lbph()->prepareForRecognition(image);
        }
        else if (recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
        {
            cvImage = eigen()->prepareForRecognition(image);
        }
        else if (recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
        {
            cvImage = fisher()->prepareForRecognition(image);
        }
#ifdef HAVE_FACESENGINE_DNN
        else if (recognizeAlgorithm == RecognizeAlgorithm::DNN)
        {
            cvImage = dnn()->prepareForRecognition(image);
        }
#endif
        else
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
        }
/*
        cvImage         = aligner()->align(cvImage);
        TanTriggsPreprocessor preprocessor;
        cvImage         = preprocessor.preprocess(cvImage);
*/
        return cvImage;
    }
    catch (cv::Exception& e)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception:" << e.what();
        return cv::Mat();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        return cv::Mat();
    }
}

void RecognitionDatabase::Private::train(OpenCVLBPHFaceRecognizer* const r,
                                         const QList<Identity>& identitiesToBeTrained,
                                         TrainingDataProvider* const data,
                                         const QString& trainingContext)
{
    trainIdentityBatchLBPH(r, identitiesToBeTrained, data, trainingContext, this);
}

void RecognitionDatabase::Private::train(OpenCVEIGENFaceRecognizer* const r,
                                         const QList<Identity>& identitiesToBeTrained,
                                         TrainingDataProvider* const data,
                                         const QString& trainingContext)
{
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Training using opencv eigenfaces";
    trainIdentityBatchEIGEN(r, identitiesToBeTrained, data, trainingContext, this);
}

#ifdef HAVE_FACESENGINE_DNN
void RecognitionDatabase::Private::train(OpenCVDNNFaceRecognizer* const r,
                                         const QList<Identity>& identitiesToBeTrained,
                                         TrainingDataProvider* const data,
                                         const QString& trainingContext)
{
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Training using opencv DNN";
    trainIdentityBatchDNN(r, identitiesToBeTrained, data, trainingContext, this);
}
#endif

void RecognitionDatabase::Private::clear(OpenCVLBPHFaceRecognizer* const,
                                         const QList<int>& idsToClear,
                                         const QString& trainingContext)
{
    // force later reload
    delete opencvlbph;
    opencvlbph = 0;

    if (idsToClear.isEmpty())
    {
        FaceDbAccess().db()->clearLBPHTraining(trainingContext);
    }
    else
    {
        FaceDbAccess().db()->clearLBPHTraining(idsToClear, trainingContext);
    }
}

void RecognitionDatabase::Private::clear(OpenCVEIGENFaceRecognizer* const,
                                         const QList<int>& idsToClear,
                                         const QString& trainingContext)
{
    // force later reload
    delete opencveigen;
    opencveigen = 0;

    if (idsToClear.isEmpty())
    {
        FaceDbAccess().db()->clearEIGENTraining(trainingContext);
    }
    else
    {
        FaceDbAccess().db()->clearEIGENTraining(idsToClear, trainingContext);
    }
}

void RecognitionDatabase::Private::clear(OpenCVFISHERFaceRecognizer* const,
                                         const QList<int>&,
                                         const QString&)
{
    // force later reload
    delete opencvfisher;
    opencvfisher = 0;
}

#ifdef HAVE_FACESENGINE_DNN
void RecognitionDatabase::Private::clear(OpenCVDNNFaceRecognizer* const,
                                         const QList<int>&,
                                         const QString&)
{
    // force later reload
    delete opencvdnn;
    opencvdnn = 0;
}
#endif

// -------------------------------------------------------------------------------------------------

RecognitionDatabase::RecognitionDatabase()
    : d(new Private)
{
}

RecognitionDatabase::~RecognitionDatabase()
{
    delete d;
}

RecognitionDatabase::RecognitionDatabase(const RecognitionDatabase& other)
    : d(other.d)
{
}

QList<Identity> RecognitionDatabase::allIdentities() const
{
    if (!d || !d->dbAvailable)
        return QList<Identity>();

    QMutexLocker lock(&d->mutex);

    return (d->identityCache.values());
}

Identity RecognitionDatabase::identity(int id) const
{
    if (!d || !d->dbAvailable)
    {
        return Identity();
    }

    QMutexLocker lock(&d->mutex);

    return (d->identityCache.value(id));
}

Identity RecognitionDatabase::findIdentity(const QString& attribute, const QString& value) const
{
    if (!d || !d->dbAvailable || attribute.isEmpty())
    {
        return Identity();
    }

    QMutexLocker lock(&d->mutex);

    return (d->findByAttribute(attribute, value));
}

Identity RecognitionDatabase::findIdentity(const QMap<QString, QString>& attributes) const
{
    if (!d || !d->dbAvailable || attributes.isEmpty())
    {
        return Identity();
    }

    QMutexLocker lock(&d->mutex);

    Identity match;

    // First and foremost, UUID
    QString uuid = attributes.value(QLatin1String("uuid"));
    match        = d->findByAttribute(QLatin1String("uuid"), uuid);

    if (!match.isNull())
    {
        return match;
    }

    // A negative UUID match, with a given UUID, precludes any further search
    if (!uuid.isNull())
    {
        return Identity();
    }

    // full name
    match = d->findByAttributes(QLatin1String("fullName"), attributes);

    if (!match.isNull())
    {
        return match;
    }

    // name
    match = d->findByAttributes(QLatin1String("name"), attributes);

    if (!match.isNull())
    {
        return match;
    }

    QMap<QString, QString>::const_iterator it;

    for (it = attributes.begin() ; it != attributes.end() ; ++it)
    {
        if (it.key() == QLatin1String("uuid")     ||
            it.key() == QLatin1String("fullName") ||
            it.key() == QLatin1String("name"))
        {
            continue;
        }

        match = d->findByAttribute(it.key(), it.value());

        if (!match.isNull())
        {
            return match;
        }
    }

    return Identity();
}

Identity RecognitionDatabase::addIdentity(const QMap<QString, QString>& attributes)
{
    if (!d || !d->dbAvailable)
    {
        return Identity();
    }

    QMutexLocker lock(&d->mutex);

    if (attributes.contains(QLatin1String("uuid")))
    {
        Identity matchByUuid = findIdentity(QLatin1String("uuid"), attributes.value(QLatin1String("uuid")));

        if (!matchByUuid.isNull())
        {
            // This situation is not well defined.

            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Called addIdentity with a given UUID, and there is such a UUID already in the database."
                                             << "The existing identity is returned without adjusting properties!";

            return matchByUuid;
        }
    }

    Identity identity;
    {
        FaceDbOperationGroup group;
        int id = FaceDbAccess().db()->addIdentity();
        identity.setId(id);
        identity.setAttributesMap(attributes);
        identity.setAttribute(QLatin1String("uuid"), QUuid::createUuid().toString());
        FaceDbAccess().db()->updateIdentity(identity);
    }

    d->identityCache[identity.id()] = identity;

    return identity;
}

void RecognitionDatabase::addIdentityAttributes(int id, const QMap<QString, QString>& attributes)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    QHash<int, Identity>::iterator it = d->identityCache.find(id);

    if (it != d->identityCache.end())
    {
        QMap<QString, QString> map = it->attributesMap();
        map.unite(attributes);
        it->setAttributesMap(map);
        FaceDbAccess().db()->updateIdentity(*it);
    }
}

void RecognitionDatabase::addIdentityAttribute(int id, const QString& attribute, const QString& value)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);
    QHash<int, Identity>::iterator it = d->identityCache.find(id);

    if (it != d->identityCache.end())
    {
        QMap<QString, QString> map = it->attributesMap();
        map.insertMulti(attribute, value);
        it->setAttributesMap(map);
        FaceDbAccess().db()->updateIdentity(*it);
    }
}

void RecognitionDatabase::setIdentityAttributes(int id, const QMap<QString, QString>& attributes)
{
    if (!d || !d->dbAvailable)
    {
            return;
    }

    QMutexLocker lock(&d->mutex);
    QHash<int, Identity>::iterator it = d->identityCache.find(id);

    if (it != d->identityCache.end())
    {
        it->setAttributesMap(attributes);
        FaceDbAccess().db()->updateIdentity(*it);
    }
}

QString RecognitionDatabase::backendIdentifier() const
{
    if (d->recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
    {
        return QLatin1String("eigenfaces");
    }
#ifdef HAVE_FACESENGINE_DNN
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::DNN)
    {
        return QLatin1String("dnn");
    }
#endif
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
    {
        return QLatin1String("fisherfaces");
    }

    // d->recognizeAlgorithm == RecognizeAlgorithm::LPB
    return QLatin1String("opencvlbph");
}

void RecognitionDatabase::setParameter(const QString& parameter, const QVariant& value)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    d->parameters.insert(parameter, value);
    d->applyParameters();
}

void RecognitionDatabase::setParameters(const QVariantMap& parameters)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    for (QVariantMap::const_iterator it = parameters.begin() ; it != parameters.end() ; ++it)
    {
        d->parameters.insert(it.key(), it.value());
    }

    d->applyParameters();
}

QVariantMap RecognitionDatabase::parameters() const
{
    if (!d || !d->dbAvailable)
    {
        return QVariantMap();
    }

    QMutexLocker lock(&d->mutex);

    return d->parameters;
}

int RecognitionDatabase::recommendedImageSize(const QSize& availableSize) const
{
    // hardcoded for now, change when we know better.
    Q_UNUSED(availableSize)

    return 256;
}

Identity RecognitionDatabase::recognizeFace(const QImage& image)
{
    QList<Identity> result = recognizeFaces(QList<QImage>() << image);

    if (result.isEmpty())
        return Identity();

    return result.first();
}

QList<Identity> RecognitionDatabase::recognizeFaces(const QList<QImage>& images)
{
    QListImageListProvider provider(images);

    return recognizeFaces(&provider);
}

void RecognitionDatabase::activeFaceRecognizer(RecognizeAlgorithm algorithmType)
{
    if (algorithmType == RecognizeAlgorithm::EigenFace ||
        algorithmType == RecognizeAlgorithm::FisherFace)
    {
        d->recognizeAlgorithm = RecognizeAlgorithm::LBP;
    }
    else
    {
        d->recognizeAlgorithm = algorithmType;
    }
}

QList<Identity> RecognitionDatabase::recognizeFaces(ImageListProvider* const images)
{
    if (!d || !d->dbAvailable)
    {
        return QList<Identity>();
    }

    QMutexLocker lock(&d->mutex);

    QList<Identity> result;

    for (; !images->atEnd(); images->proceed())
    {
        int id = -1;

        try
        {
            if (d->recognizeAlgorithm == RecognizeAlgorithm::LBP)
            {
                id = d->lbph()->recognize(d->preprocessingChain(images->image()));
            }
            else if (d->recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
            {
                id = d->eigen()->recognize(d->preprocessingChain(images->image()));
            }
            else if (d->recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
            {
                id = d->fisher()->recognize(d->preprocessingChain(images->image()));
            }
#ifdef HAVE_FACESENGINE_DNN
            else if (d->recognizeAlgorithm == RecognizeAlgorithm::DNN)
            {
                id = d->dnn()->recognize(d->preprocessingChainRGB(images->image()));
            }
#endif
            else
            {
                qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
            }
        }
        catch (cv::Exception& e)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "cv::Exception:" << e.what();
        }
        catch (...)
        {
            qCCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception from OpenCV";
        }

        if (id == -1)
        {
            result << Identity();
        }
        else
        {
            result << d->identityCache.value(id);
        }
    }

    return result;
}

RecognitionDatabase::TrainingCostHint RecognitionDatabase::trainingCostHint() const
{
    return TrainingIsCheap;
}

void RecognitionDatabase::train(const Identity& identityToBeTrained, TrainingDataProvider* const data,
                                const QString& trainingContext)
{
    train(QList<Identity>() << identityToBeTrained, data, trainingContext);
}

void RecognitionDatabase::train(const QList<Identity>& identitiesToBeTrained, TrainingDataProvider* const data,
                                const QString& trainingContext)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    if (d->recognizeAlgorithm == RecognizeAlgorithm::LBP)
    {
        d->train(d->lbph(),  identitiesToBeTrained, data, trainingContext);
    }
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
    {
        d->train(d->eigen(), identitiesToBeTrained, data, trainingContext);
    }
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
    {
        // No method to call
    }
#ifdef HAVE_FACESENGINE_DNN
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::DNN)
    {
        d->train(d->dnn(),   identitiesToBeTrained, data, trainingContext);
    }
#endif
    else
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
    }
}

void RecognitionDatabase::train(const Identity& identityToBeTrained,
                                const QImage& image,
                                const QString& trainingContext)
{
    SimpleTrainingDataProvider* const data = new SimpleTrainingDataProvider(identityToBeTrained,
                                                                            QList<QImage>() << image);
    train(identityToBeTrained, data, trainingContext);
    delete data;
}

void RecognitionDatabase::train(const Identity& identityToBeTrained,
                                const QList<QImage>& images,
                                const QString& trainingContext)
{
    SimpleTrainingDataProvider* const data = new SimpleTrainingDataProvider(identityToBeTrained, images);
    train(identityToBeTrained, data, trainingContext);
    delete data;
}

void RecognitionDatabase::clearAllTraining(const QString& trainingContext)
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    d->clear(d->lbph(),   QList<int>(), trainingContext);
    d->clear(d->eigen(),  QList<int>(), trainingContext);
    d->clear(d->fisher(), QList<int>(), trainingContext);
#ifdef HAVE_FACESENGINE_DNN
    d->clear(d->dnn(),    QList<int>(), trainingContext);
#endif
}

void RecognitionDatabase::clearTraining(const QList<Identity>& identitiesToClean, const QString& trainingContext)
{
    if (!d || !d->dbAvailable || identitiesToClean.isEmpty())
    {
        return;
    }

    QMutexLocker lock(&d->mutex);
    QList<int>   ids;

    foreach (const Identity& id, identitiesToClean)
    {
        ids << id.id();
    }

    if (d->recognizeAlgorithm == RecognizeAlgorithm::LBP)
    {
        d->clear(d->lbph(), ids, trainingContext);
    }
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::EigenFace)
    {
        d->clear(d->eigen(), ids, trainingContext);
    }
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::FisherFace)
    {
        d->clear(d->fisher(), ids, trainingContext);
    }
#ifdef HAVE_FACESENGINE_DNN
    else if (d->recognizeAlgorithm == RecognizeAlgorithm::DNN)
    {
        d->clear(d->dnn(), ids, trainingContext);
    }
#endif
    else
    {
        qCCritical(DIGIKAM_FACESENGINE_LOG) << "No obvious recognize algorithm";
    }
}

void RecognitionDatabase::deleteIdentity(const Identity& identityToBeDeleted)
{
    if (!d || !d->dbAvailable || identityToBeDeleted.isNull())
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    FaceDbAccess().db()->deleteIdentity(identityToBeDeleted.id());
    d->identityCache.remove(identityToBeDeleted.id());
}

bool RecognitionDatabase::integrityCheck()
{
    if (!d || !d->dbAvailable)
    {
        return false;
    }

    QMutexLocker lock(&d->mutex);

    return FaceDbAccess().db()->integrityCheck();
}

void RecognitionDatabase::vacuum()
{
    if (!d || !d->dbAvailable)
    {
        return;
    }

    QMutexLocker lock(&d->mutex);

    return FaceDbAccess().db()->vacuum();
}

} // namespace Digikam
