///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2023,2025 Mihai Ursu                                            //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

/*
AudioSource.cpp
This file contains the sources for the audio source.
*/

#include "AudioSource.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>

#include "NoisePwrSpectrum.h"


//!************************************************************************
//! Constructor
//!************************************************************************
AudioSource::AudioSource
    (
    const QAudioFormat& aFormat,                //!< audio format
    const uint32_t      aBufferLengthSeconds    //!< audio buffer length [seconds]
    )
    : mAudioFormat( aFormat )
    , mAudioBufferLengthSeconds( aBufferLengthSeconds )
    , mBufferPos( 0 )
{
    srand( time( NULL ) );
}


//!************************************************************************
//! Get the number of bytes that are available for reading
//! see QIODevice::bytesAvailable()
//!
//! @returns: Number of available bytes
//!************************************************************************
qint64 AudioSource::bytesAvailable() const
{
    return mAudioBuffer.size() + QIODevice::bytesAvailable();
}


//!************************************************************************
//! Fill the audio buffer with generated data
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::fillDataBuffer()
{
    const int CHANNEL_BYTES = mAudioFormat.bytesPerSample();
    qint64 bufferLength = mAudioFormat.bytesForDuration( mAudioBufferLengthSeconds * 1000000 );
    std::vector<double> totalNoiseBuffer( bufferLength );

    for( size_t k = 0; k < mSignalsVector.size(); k++ )
    {
        if( SignalItem::SIGNAL_TYPE_NOISE == mSignalsVector.at( k )->getType() )
        {
            std::vector<double> crtNoiseBuffer( bufferLength );

            for( size_t i = 0; i < bufferLength; i++ )
            {
                double time = static_cast<double>( i % mAudioFormat.sampleRate() ) / mAudioFormat.sampleRate();
                time += static_cast<size_t>( i / mAudioFormat.sampleRate() );
                crtNoiseBuffer.at( i ) = getSignalValueNoise( mSignalsVector.at( k )->getSignalDataNoise(), time );
            }

            SignalItem::SignalNoise sig = mSignalsVector.at( k )->getSignalDataNoise();

            if( 0 == sig.gamma ) // white noise
            {
                for( size_t i = 0; i < bufferLength; i++ )
                {
                    totalNoiseBuffer.at( i ) += crtNoiseBuffer.at( i );
                }
            }
            else // any value in [-2..2] except 0
            {
                NoisePwrSpectrum noisePwrSpectrum( sig.gamma );
                std::vector<double> filteredNoiseBuffer( bufferLength );
                noisePwrSpectrum.filterData( crtNoiseBuffer, filteredNoiseBuffer );

                for( size_t i = 0; i < bufferLength; i++ )
                {
                    totalNoiseBuffer.at( i ) += filteredNoiseBuffer.at( i );
                }
            }
        }
    }

    mAudioBuffer.resize( bufferLength );
    unsigned char* bufferData = reinterpret_cast<unsigned char *>( mAudioBuffer.data() );

    const bool SAVE_TO_RAW_FILE = false;
    std::ofstream outputFile;

    if( SAVE_TO_RAW_FILE )
    {
        outputFile.open( "out_raw.txt" );
    }

    for( size_t i = 0; bufferLength > 0; i++ )
    {
        double time = static_cast<double>( i % mAudioFormat.sampleRate() ) / mAudioFormat.sampleRate();
        time += static_cast<size_t>( i / mAudioFormat.sampleRate() );
        double yGenerated = getSignalValue( time );
        yGenerated += totalNoiseBuffer.at( i );

        for( int j = 0; j < mAudioFormat.channelCount(); j++ )
        {
            if( 0 == j )
            {
                if( SAVE_TO_RAW_FILE && outputFile.is_open() )
                {
                    QString line = QString::number( time ) + "\t" + QString::number( yGenerated ) + "\n";
                    outputFile << line.toStdString();
                }
            }

            switch( mAudioFormat.sampleFormat() )
            {
                case QAudioFormat::UInt8:
                    *reinterpret_cast<uint8_t*>( bufferData ) = static_cast<uint8_t>( 255 * ( 1.0 + yGenerated ) / 2 );
                    break;

                case QAudioFormat::Int16:
                    *reinterpret_cast<int16_t*>( bufferData ) = static_cast<int16_t>( yGenerated * 32767 );
                    break;

                case QAudioFormat::Int32:
                    *reinterpret_cast<int32_t*>( bufferData ) = static_cast<int32_t>( yGenerated * std::numeric_limits<int32_t>::max() );
                    break;

                case QAudioFormat::Float:
                    *reinterpret_cast<float*>( bufferData ) = yGenerated;
                    break;

                default:
                    break;
            }

            bufferData += CHANNEL_BYTES;
            bufferLength -= CHANNEL_BYTES;
        }
    }    

    if( SAVE_TO_RAW_FILE && outputFile.is_open() )
    {
        outputFile.close();
    }
}


//!************************************************************************
//! Generate a random number
//! adapted from Knuth, D.E. - The Art of Computer Programming
//!                            Volume 2, Seminumerical Algorithms
//!                            3rd Ed, Addison-Wesley, 1997
//!
//! @returns: a random double in [0..1]
//!************************************************************************
double AudioSource::generateRandomDek
    (
    int32_t*   pIdum        //!< seed value
    ) const
{
    const int32_t MBIG = 1000000000;
    const int32_t MSEED = 161803398;
    const int32_t MZ = 0;
    const double FAC = 1.0 / MBIG;

    static int32_t inext;
    static int32_t inextp;
    static int32_t ma[56];
    static int32_t iff = 0;
    int32_t mj;
    int32_t mk;
    int32_t i;
    int32_t ii;
    int32_t k;

    if( *pIdum < 0 || iff == 0 )
    {
        iff = 1;
        mj = labs( MSEED - labs( *pIdum ) );
        mj %= MBIG;
        ma[55] = mj;
        mk = 1;

        for( i = 1; i <= 54; i++ )
        {
            ii = ( 21 * i ) % 55;
            ma[ii] = mk;
            mk = mj - mk;

            if( mk < MZ )
            {
                mk += MBIG;
            }

            mj = ma[ii];
        }

        for( k = 1; k <= 4; k++ )
        {
            for( i = 1; i <= 55; i++ )
            {
                ma[i] -= ma[ 1 + ( i + 30 ) % 55 ];

                if( ma[i] < MZ )
                {
                    ma[i] += MBIG;
                }
            }
        }

        inext = 0;
        inextp = 31;
        *pIdum = 1;
    }

    if( ++inext == 56 )
    {
        inext = 1;
    }

    if( ++inextp == 56 )
    {
        inextp = 1;
    }

    mj = ma[inext] - ma[inextp];

    if( mj < MZ )
    {
        mj += MBIG;
    }

    ma[inext] = mj;

    return ( double )( mj * FAC );
}


//!************************************************************************
//! Generate a random number
//! adapted from Press, W.H. et al - Numerical Recipes in C. The Art of Scientific Computing
//!                                  2nd Ed, Cambridge Univ. Press, 1992
//!
//! see ran4(), subchapter 7.5, pp. 303
//!
//! @returns: a random double in [0..1]
//!************************************************************************
double AudioSource::generateRandomNag
    (
    int32_t*   pIdum        //!< seed value
    ) const
{
    const uint32_t JFLONE = 0x3f800000;
    const uint32_t JFLMSK = 0x007fffff;
    static int32_t idums = 0;

    if( *pIdum < 0 )
    {
        idums = -( *pIdum );
        *pIdum = 1;
    }

    uint32_t irword = *pIdum;
    uint32_t lword = idums;
    pseudoDes( &lword, &irword );
    uint32_t itemp = JFLONE | ( JFLMSK & irword );
    ++( *pIdum );

    return ( *reinterpret_cast< float* >( &itemp ) - 1.0 );
}


//!************************************************************************
//! Get the value of the entire signal, obtained by superposition
//! through the entire vector *without noise*
//!
//! @returns The value of the signal at a specified moment
//!************************************************************************
double AudioSource::getSignalValue
    (
    const double aTime      //!< time
    ) const
{
    double y = 0;

    for( size_t i = 0; i < mSignalsVector.size(); i++ )
    {
        switch( mSignalsVector.at( i )->getType() )
        {
            case SignalItem::SIGNAL_TYPE_TRIANGLE:
                y += getSignalValueTriangle( mSignalsVector.at( i )->getSignalDataTriangle(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_RECTANGLE:
                y += getSignalValueRectangle( mSignalsVector.at( i )->getSignalDataRectangle(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_PULSE:
                y += getSignalValuePulse( mSignalsVector.at( i )->getSignalDataPulse(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_RISEFALL:
                y += getSignalValueRiseFall( mSignalsVector.at( i )->getSignalDataRiseFall(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMP:
                y += getSignalValueSinDamp( mSignalsVector.at( i )->getSignalDataSinDamp(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_SINRISE:
                y += getSignalValueSinRise( mSignalsVector.at( i )->getSignalDataSinRise(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_WAVSIN:
                y += getSignalValueWavSin( mSignalsVector.at( i )->getSignalDataWavSin(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_AMSIN:
                y += getSignalValueAmSin( mSignalsVector.at( i )->getSignalDataAmSin(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                y += getSignalValueSinDampSin( mSignalsVector.at( i )->getSignalDataSinDampSin(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                y += getSignalValueTrapDampSin( mSignalsVector.at( i )->getSignalDataTrapDampSin(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_SMC:
                y += getSignalValueSmc( mSignalsVector.at( i )->getSignalDataSmc(), aTime );
                break;

            case SignalItem::SIGNAL_TYPE_NOISE: // intentionally skip noise type
            default:
                break;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a Triangle signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueTriangle
    (
    const SignalItem::SignalTriangle    aSignalData,    //!< Triangle signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double dt0 = aTime - aSignalData.tDelay;
        uint32_t kPer = dt0 / aSignalData.tPeriod;
        double tInPer = dt0 - kPer * aSignalData.tPeriod;

        if( tInPer <= aSignalData.tRise )
        {
            y = aSignalData.yMin + ( aSignalData.yMax - aSignalData.yMin ) * tInPer / aSignalData.tRise;
        }
        else
        {
            y = aSignalData.yMax - ( aSignalData.yMax - aSignalData.yMin ) * ( tInPer - aSignalData.tRise ) / aSignalData.tFall;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a Rectangle signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueRectangle
    (
    const SignalItem::SignalRectangle   aSignalData,    //!< Rectangle signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double dt0 = aTime - aSignalData.tDelay;
        uint32_t kPer = dt0 / aSignalData.tPeriod;
        double tInPer = dt0 - kPer * aSignalData.tPeriod;

        if( tInPer <= aSignalData.tPeriod * aSignalData.fillFactor )
        {
            y = aSignalData.yMax;
        }
        else
        {
            y = aSignalData.yMin;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a Pulse signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValuePulse
    (
    const SignalItem::SignalPulse       aSignalData,    //!< Pulse signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double dt0 = aTime - aSignalData.tDelay;
        uint32_t kPer = dt0 / aSignalData.tPeriod;
        double tInPer = dt0 - kPer * aSignalData.tPeriod;

        if( tInPer <= aSignalData.tRise )
        {
            y = aSignalData.yMin + ( aSignalData.yMax - aSignalData.yMin ) * tInPer / aSignalData.tRise;
        }
        else if( tInPer <= aSignalData.tRise + aSignalData.tWidth )
        {
            y = aSignalData.yMax;
        }
        else if( tInPer <= aSignalData.tRise + aSignalData.tWidth + aSignalData.tFall )
        {
            y = aSignalData.yMax - ( aSignalData.yMax - aSignalData.yMin ) * ( tInPer - aSignalData.tRise - aSignalData.tWidth ) / aSignalData.tFall;
        }
        else
        {
            y = aSignalData.yMin;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of an exponential RiseFall signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueRiseFall
    (
    const SignalItem::SignalRiseFall    aSignalData,    //!< RiseFall signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        if( aTime <= aSignalData.tDelayRise )
        {
            y = aSignalData.yMin;
        }
        else if( aTime > aSignalData.tDelayRise
              && aTime <= aSignalData.tDelayFall
               )
        {
            y = aSignalData.yMin
                + ( aSignalData.yMax - aSignalData.yMin ) * ( 1. - exp( -( aTime - aSignalData.tDelayRise ) / aSignalData.tRampRise ) );

        }
        else if( aTime > aSignalData.tDelayFall )
        {
            y = aSignalData.yMin
                + ( aSignalData.yMax - aSignalData.yMin ) * ( 1. - exp( -( aTime - aSignalData.tDelayRise ) / aSignalData.tRampRise ) )
                + ( aSignalData.yMin - aSignalData.yMax ) * ( 1. - exp( -( aTime - aSignalData.tDelayFall ) / aSignalData.tRampFall ) );
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a SinDamp signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueSinDamp
    (
    const SignalItem::SignalSinDamp     aSignalData,    //!< SinDamp signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double dt0 = aTime - aSignalData.tDelay;

        y = aSignalData.offset
            + aSignalData.amplit * sin( 2 * M_PI * aSignalData.freqHz * dt0 + aSignalData.phiRad ) * exp( -aSignalData.damping * dt0 );
    }

    return y;
}


//!************************************************************************
//! Get the value of a SinRise signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueSinRise
    (
    const SignalItem::SignalSinRise     aSignalData,    //!< SinRise signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        if( aTime < aSignalData.tEnd )
        {
            double dtend = aTime - aSignalData.tEnd;

            y = aSignalData.offset
                + aSignalData.amplit * sin( 2 * M_PI * aSignalData.freqHz * dtend + aSignalData.phiRad ) * exp( aSignalData.damping * dtend );
        }
        else
        {
            y = aSignalData.offset;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a WavSin signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueWavSin
    (
    const SignalItem::SignalWavSin      aSignalData,    //!< WavSin signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        uint8_t N = aSignalData.index;

        if( N < 3
         || N % 2 != 1
          )
        {
            N = 3;
        }

        double b = aSignalData.freqHz / N;
        double T = 0.5 / b;
        double dt0 = aTime - aSignalData.tDelay;

        if( aTime < T + aSignalData.tDelay )
        {
            y = aSignalData.offset
                + aSignalData.amplit * sin( 2 * M_PI * b * dt0 ) * sin( 2 * M_PI * aSignalData.freqHz * dt0 );
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a AmSin signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueAmSin
    (
    const SignalItem::SignalAmSin       aSignalData,    //!< AmSin signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.carrierTDelay )
    {
        double dt0 = aTime - aSignalData.carrierTDelay;

        y = aSignalData.carrierOffset
            + aSignalData.carrierAmplitude * sin( 2 * M_PI * aSignalData.carrierFreqHz * dt0 )
            * ( 1 + aSignalData.modulationIndex * cos( 2 * M_PI * aSignalData.modulationFreqHz * dt0 + aSignalData.modulationPhiRad ) );
    }

    return y;
}


//!************************************************************************
//! Get the value of a SinDampSin signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueSinDampSin
    (
    const SignalItem::SignalSinDampSin  aSignalData,    //!< SinDampSin signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double eyeAmplit = aSignalData.amplit;
        double dt0 = aTime - aSignalData.tDelay;
        uint32_t kPer = 1 + dt0 / aSignalData.tPeriodEnv;

        switch( aSignalData.dampingType )
        {
            case 0:
                break;

            case -3:
                eyeAmplit *= exp( kPer - 1.0 );
                break;

            case -2:
            case -1:
            case 1:
            case 2:
                eyeAmplit *= pow( static_cast<double>( kPer ), -aSignalData.dampingType );
                break;

            case 3:
                eyeAmplit *= exp( -( kPer - 1.0 ) );
                break;

            default:
                eyeAmplit = 0;
                break;
        }

        y = aSignalData.offset
                + eyeAmplit * sin( M_PI / aSignalData.tPeriodEnv * dt0 ) * sin( 2 * M_PI * aSignalData.freqSinHz * dt0 );
    }

    return y;
}


//!************************************************************************
//! Get the value of a TrapDampSin signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueTrapDampSin
    (
    const SignalItem::SignalTrapDampSin aSignalData,    //!< TrapDampSin signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aTime >= aSignalData.tDelay )
    {
        double dt0 = aTime - aSignalData.tDelay;
        uint32_t kPer = 1 + dt0 / aSignalData.tPeriod;

        if( aTime >= aSignalData.tCross
       || ( aTime > aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise + aSignalData.tWidth + aSignalData.tFall
         && aTime < aSignalData.tDelay + kPer * aSignalData.tPeriod )
          )
        {
            y = aSignalData.offset;
        }
        else
        {
            double yEnv = 0;

            if( aTime > aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod
            &&  aTime <= aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise
              )
            {
                yEnv = aSignalData.amplit * ( aSignalData.tCross - aSignalData.tDelay - ( kPer - 1 ) * aSignalData.tPeriod - aSignalData.tRise );
                yEnv /= aSignalData.tCross;

                y = ( dt0 - ( kPer - 1 ) * aSignalData.tPeriod ) / aSignalData.tRise;
                y *= yEnv;
            }
            else if( aTime > aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise
                  && aTime <= aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise + aSignalData.tWidth
                   )
            {
                yEnv = aSignalData.amplit * ( aSignalData.tCross - aSignalData.tDelay - ( kPer - 1 ) * aSignalData.tPeriod - aSignalData.tRise );
                yEnv /= aSignalData.tCross;

                y = yEnv - aSignalData.amplit * ( dt0 - ( kPer - 1 ) * aSignalData.tPeriod - aSignalData.tRise ) / aSignalData.tCross;
            }
            else if( aTime > aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise + aSignalData.tWidth
                  && aTime <= aSignalData.tDelay + ( kPer - 1 ) * aSignalData.tPeriod + aSignalData.tRise + aSignalData.tWidth + aSignalData.tFall
                  )
            {
                yEnv = aSignalData.amplit * ( aSignalData.tCross - aSignalData.tDelay - ( kPer - 1 ) * aSignalData.tPeriod - aSignalData.tRise - aSignalData.tWidth );
                yEnv /= aSignalData.tCross;

                y = 1 - ( dt0 - ( kPer - 1 ) * aSignalData.tPeriod - aSignalData.tRise - aSignalData.tWidth ) / aSignalData.tFall;
                y *= yEnv;
            }

            y *= sin( 2 * M_PI * aSignalData.freqHz * ( dt0 - ( kPer - 1 ) * aSignalData.tPeriod ) );
            y += aSignalData.offset;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a white Noise signal
//! An array of such values can be filtered for obtaining violet, blue,
//! pink, or brown noise.
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueNoise
    (
    const SignalItem::SignalNoise       aSignalData,    //!< Noise signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;
    static int32_t seedNag = rand();

    if( aTime >= aSignalData.tDelay )
    {
        switch( aSignalData.noiseType )
        {
            case SignalItem::NOISE_TYPE_DEK:
                {
                    int32_t seedDek = 1;
                    y = generateRandomDek( &seedDek );  // [0..1]
                    y = 2 * y - 1;                      // [-1..1]
                    y *= aSignalData.amplit;            // [-a..a]
                    y += aSignalData.offset;
                }
                break;

            case SignalItem::NOISE_TYPE_NAG:
                {
                    y = generateRandomNag( &seedNag );  // [0..1]
                    y = 2 * y - 1;                      // [-1..1]
                    y *= aSignalData.amplit;            // [-a..a]
                    y += aSignalData.offset;
                }
                break;

            default:
                break;
        }
    }

    return y;
}


//!************************************************************************
//! Get the value of a SMC signal
//!
//! @returns The signal value at a specified moment
//!************************************************************************
double AudioSource::getSignalValueSmc
    (
    const SignalItem::SignalSmc         aSignalData,    //!< SMC signal data
    const double                        aTime           //!< time
    ) const
{
    double y = 0;

    if( aSignalData.sps > 0
     && aSignalData.nrPoints
     && aSignalData.accelDataVec.size()
      )
    {
        double smcSignalDuration = aSignalData.nrPoints / aSignalData.sps;

        if( aTime <= smcSignalDuration )
        {
            double dt = 1.0 / aSignalData.sps;
            uint32_t kSample = std::floor( aTime / dt );
            double tInSample = aTime - kSample * dt;

            if( kSample < aSignalData.accelDataVec.size() )
            {
                double yL = ( kSample > 0 ) ? aSignalData.accelDataVec.at( kSample - 1 ) : 0;
                double yR = aSignalData.accelDataVec.at( kSample );
                y = yL + ( tInSample / dt ) * ( yR - yL );
                y /= aSignalData.MAX_SCALE_ACCEL_MS2;
            }
        }
    }

    return y;
}


//!************************************************************************
//! Check if the audio source is started
//!
//! @returns: If the device is open
//!************************************************************************
bool AudioSource::isStarted() const
{
    return isOpen();
}


//!************************************************************************
//! Pseudo DES (Data Encryption Standard)
//! adapted from Press, W.H. et al - Numerical Recipes in C. The Art of Scientific Computing
//!                                  2nd Ed, Cambridge Univ. Press, 1992
//! see psdes(), subchapter 7.5, page 302
//!
//! This function is used by generateRandomNag()
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::pseudoDes
    (
    uint32_t*   lword,      //!< left word
    uint32_t*   irword      //!< right word
    ) const
{
    const uint8_t NITER = 4;
    const uint32_t C1[NITER] = { 0xbaa96887, 0x1e17d32c, 0x03bcdc3c, 0x0f33d1b2 };
    const uint32_t C2[NITER] = { 0x4b0f3b58, 0xe874f0c3, 0x6955c5a6, 0x55a7ca46 };
    uint32_t iswap = 0;

    for( uint8_t i = 0; i < NITER; i++ )
    {
        uint32_t ia = ( iswap = ( *irword ) ) ^ C1[i];
        uint32_t itmpl = ia & 0xffff;
        uint32_t itmph = ia >> 16;
        uint32_t ib = itmpl * itmpl + ~( itmph * itmph );
        *irword = ( *lword ) ^ ( ( ( ia = ( ib >> 16 ) | ( ( ib & 0xffff ) << 16 ) ) ^ C2[i] ) + itmpl * itmph );
        *lword = iswap;
    }
}


//!************************************************************************
//! Reads up to aLength bytes from the device into aData
//! see QIODevice::readData()
//!
//! @returns: Number of bytes read
//!************************************************************************
qint64 AudioSource::readData
    (
    char*   aData,          //!< data content
    qint64  aLength         //!< data length
    )
{
    qint64 bytesRead = 0;

    if( !mAudioBuffer.isEmpty() )
    {
        while( aLength - bytesRead > 0 )
        {
            const qint64 chunk = qMin( ( mAudioBuffer.size() - mBufferPos ), aLength - bytesRead );
            memcpy( aData + bytesRead, mAudioBuffer.constData() + mBufferPos, chunk );
            mBufferPos = ( mBufferPos + chunk ) % mAudioBuffer.size();
            bytesRead += chunk;
        }
    }

    return bytesRead;
}


//!************************************************************************
//! Set the audio buffer length [seconds]
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::setBufferLength
    (
    const double aLength          //!< a length in seconds
    )
{
    mAudioBufferLengthSeconds = aLength;

    mBufferPos = 0;
    close();

    mAudioBuffer.clear();

    if( mAudioFormat.isValid() )
    {
        fillDataBuffer();
    }
}


//!************************************************************************
//! Set the data for entire waveform
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::setData
    (
    const std::vector<SignalItem*>  aSignalsVector  //!< signals vector
    )
{
    mBufferPos = 0;
    close();

    mAudioBuffer.clear();
    mSignalsVector.clear();

    for( size_t i = 0; i < aSignalsVector.size(); i++ )
    {
        mSignalsVector.push_back( aSignalsVector.at( i ) );
    }

    if( mAudioFormat.isValid() )
    {
        fillDataBuffer();
    }
}


//!************************************************************************
//! Start the audio source
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::start()
{
    open( QIODevice::ReadOnly );
}


//!************************************************************************
//! Stop the audio source
//!
//! @returns: nothing
//!************************************************************************
void AudioSource::stop()
{
    mBufferPos = 0;
    close();
}


//!************************************************************************
//! Writes up to aLength bytes from aData to the device
//! see QIODevice::writeData()
//!
//! @returns: Number of bytes written
//!************************************************************************
qint64 AudioSource::writeData
    (
    const char*     aData,      //!< data content
    qint64          aLength     //!< data length
    )
{
    Q_UNUSED( aData );
    Q_UNUSED( aLength );
    return 0;
}
