/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date  2010-09-02
 * @brief Wrapper class for face recognition database
 *
 * @author Copyright (C) 2010-2013 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 * @author Copyright (C) 2010 by Aditya Bhatt
 *         <a href="mailto:adityabhatt1991 at gmail dot com">adityabhatt1991 at gmail dot com</a>
 * @author Copyright (C) 2010-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef FACESENGINE_RECOGNITIONDATABASE_H
#define FACESENGINE_RECOGNITIONDATABASE_H

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QImage>
#include <QList>
#include <QMap>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "identity.h"
#include "dataproviders.h"

namespace FacesEngine
{

/**
 * Performs face recognition.
 * Persistent data about identities and training data will be stored
 * under a given or the default configuration path.
 *
 * The class guarantees
 * - deferred creation: The backend is created only when used first.
 * - only one instance per configuration path is created
 * - an instance of this class is thread-safe
 *   (this class is also reentrant, for different objects and paths)
 */
class DIGIKAM_DATABASE_EXPORT RecognitionDatabase
{

public:

    enum TrainingCostHint
    {
        /// Training is so cheap that new photos for training can be passed any time
        TrainingIsCheap,
        /// Training is significantly optimized if new images are received in batches
        /// instead training single images multiple times
        TrainingLikesBatches,
        /// Training is a computing intensive operation.
        /// By choice of the application, it may be manually triggered by the user.
        TrainingIsExpensive
    };

public:

    RecognitionDatabase();
    ~RecognitionDatabase();

    // ------------ Identity management --------------

    /// For the documentation of standard attributes, see identity.h

    /**
     * Returns all identities known to the database
     */
    QList<Identity> allIdentities()  const;
    Identity        identity(int id) const;

    /**
     * Finds the first identity with matching attribute - value.
     * Returns a null identity if no match is found or attribute is empty.
     */
    Identity findIdentity(const QString& attribute, const QString& value) const;

    /**
     * Finds the identity matching the given attributes.
     * Attributes are first checked with knowledge of their meaning.
     * Secondly, all unknown attributes are used.
     * Returns a null Identity if no match is possible or the map is empty.
     */
    Identity findIdentity(const QMap<QString, QString>& attributes) const;

    /**
     * Adds a new identity with the specified attributes.
     * Please note that a UUID is automatically generated.
     */
    Identity addIdentity(const QMap<QString, QString>& attributes);

    /**
     * Adds or sets, resp., the attributes of an identity.
     */
    void addIdentityAttributes(int id, const QMap<QString, QString>& attributes);
    void addIdentityAttribute(int id, const QString& attribute, const QString& value);
    void setIdentityAttributes(int id, const QMap<QString, QString>& attributes);

    // ------------ backend parameters --------------

    /**
     * A textual, informative identifier of the backend in use.
     */
    QString backendIdentifier() const;

    /**
     * Tunes backend parameters.
     * Available parameters:
     * "accuracy", synonymous: "threshold", range: 0-1, type: float
     * Determines recognition threshold, 0->accept very unsecure recognitions, 1-> be very sure about a recognition.
     */
    void        setParameter(const QString& parameter, const QVariant& value);
    void        setParameters(const QVariantMap& parameters);
    QVariantMap parameters() const;

    // ------------ Recognition, clustering and training --------------

    /**
     * Returns the recommended size if you want to scale face images for recognition.
     * Larger images can be passed, but may be downscaled.
     */
    int recommendedImageSize(const QSize& availableSize = QSize()) const;

    /**
     * Performs recognition.
     * The face details to be recognized are passed by the provider.
     * For each entry in the provider, in 1-to-1 mapping,
     * a recognized identity or the null identity is returned.
     */
    QList<Identity> recognizeFaces(ImageListProvider* const images);
    QList<Identity> recognizeFaces(const QList<QImage>& images);
    Identity        recognizeFace(const QImage& image);

    /**
     * Gives a hint about the complexity of training for the current backend.
     */
    TrainingCostHint trainingCostHint() const;

    /**
     * Performs training.
     * The identities which have new images to be trained are given.
     * An empty list means that all identities are checked.
     *
     * All needed data will be queried from the provider.
     *
     * An identifier for the current training context is given,
     * which can identify the application or group of collections.
     * (It is assumed that training from different contexts is based on
     * non-overlapping collections of images. Keep it always constant for your app.)
     */
    void train(const QList<Identity>& identitiesToBeTrained, TrainingDataProvider* const data,
               const QString& trainingContext);
    void train(const Identity& identityToBeTrained, TrainingDataProvider* const data,
               const QString& trainingContext);

    /**
     * Performs training by using image data directly.
     *
     * These are convenience functions for simple setups.
     * If you want good performance and/or a more versatile implementation, be sure to
     * implement your own TrainingDataProvider and use one of the above functions.
     */
    void train(const Identity& identityToBeTrained, const QImage& image,
               const QString& trainingContext);
    void train(const Identity& identityToBeTrained, const QList<QImage>& images,
               const QString& trainingContext);

    /**
     * Deletes the training data for all identities,
     * leaving the identities as such in the database.
     */
    void clearAllTraining(const QString& trainingContext = QString());

    /**
     * Deletes the training data for the given identity,
     * leaving the identity as such in the database.
     */
    void clearTraining(const QList<Identity>& identitiesToClean, const QString& trainingContext = QString());

    /**
     * Deletes an identity from the database.
     */
    void deleteIdentity(const Identity& identityToBeDeleted);

public:

    // Declared as public to please with CLang compiler, due to use as argument with static methods.
    class Private;

private:

    Private* const d;
};

} // namespace FacesEngine

#endif // FACESENGINE_RECOGNITIONDATABASE_H
