/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-03
 * Description : Generating random numbers
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "randomnumbergenerator.h"

// Boost includes

// Pragma directives to reduce warnings from Boost header files.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunnamed-type-template-args"
#endif

#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

// Qt includes

#include <QDateTime>
#include <QFile>
#include <QUuid>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

NonDeterministicRandomData::NonDeterministicRandomData(int s)
{
#ifndef Q_OS_WIN
    {
        // Try urandom for UNIX platforms.
        QFile urandom(QLatin1String("/dev/urandom"));

        if (urandom.exists() && urandom.open(QIODevice::ReadOnly))
        {
            resize(s);

            if (urandom.read(data(), s) == s)
            {
                return;
            }
        }
    }
#endif

    /* Fallback, mostly for Windows, where UUID generation is supposed to be very good. */
    if (isEmpty())
    {
        reserve(s);

        while (size() < s)
        {
            append(QByteArray::fromHex(QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}')).remove(QLatin1Char('-')).toLatin1()));
        }

        resize(s);
    }

#if 0
    /**
     * Implementation with boost::random_device.
     * Works on Windows only starting with boost 1.43,
     * before, only urandom is supported, but for that,
     * we have a easy code snippet already.
     */
    const int stepSize = sizeof(boost::random_device::result_type);
    int steps          = s / stepSize;

    if (s % stepSize)
    {
        ++steps;
    }

    resize(steps * stepSize);

    boost::random_device device;
    boost::random_device::result_type* ptr = reinterpret_cast<boost::random_device::result_type*>(data());

    for (int i = 0; i < stepSize; ++i)
    {
        *ptr++ = device();
    }

    resize(s);
#endif
}

// --------------------------------------------------------------------------------------------------

class RandomNumberGenerator::Private
{
public:

    enum { InitialSeed = 5489 }; // guaranteed constant initial seed, do not change

    Private()
        : seed(InitialSeed),
          engine(InitialSeed)
    {
    }

    quint32        seed;
    boost::mt19937 engine;
};

RandomNumberGenerator::RandomNumberGenerator()
    : d(new Private)
{
}

RandomNumberGenerator::~RandomNumberGenerator()
{
    delete d;
}

quint32 RandomNumberGenerator::nonDeterministicSeed()
{
    NonDeterministicRandomData seed(sizeof(quint32));
    return *reinterpret_cast<quint32*>(seed.data());
}

quint32 RandomNumberGenerator::timeSeed()
{
    uint seed;
    seed = quintptr(&seed) + QDateTime::currentDateTime().toTime_t();
    return seed;
}

quint32 RandomNumberGenerator::seedNonDeterministic()
{
    d->seed = nonDeterministicSeed();
    d->engine.seed(d->seed);
    return d->seed;
}

quint32 RandomNumberGenerator::seedByTime()
{
    d->seed = timeSeed();
    d->engine.seed(d->seed);
    return d->seed;
}

void RandomNumberGenerator::seed(quint32 seed)
{
    d->seed = seed;
    d->engine.seed(seed);
}

void RandomNumberGenerator::reseed()
{
    seed(d->seed);
}

quint32 RandomNumberGenerator::currentSeed() const
{
    return d->seed;
}

int RandomNumberGenerator::number(int min, int max)
{
    boost::uniform_smallint<> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_smallint<> > generator(d->engine, distribution);
    return generator();
}

double RandomNumberGenerator::number(double min, double max)
{
    boost::uniform_real<> distribution(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> > generator(d->engine, distribution);
    return generator();
}

bool RandomNumberGenerator::yesOrNo(double p)
{
    boost::bernoulli_distribution<> distribution(p);
    boost::variate_generator<boost::mt19937&, boost::bernoulli_distribution<> > generator(d->engine, distribution);
    return generator();
}

} // namespace Digikam
