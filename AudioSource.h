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
AudioSource.h
This file contains the definitions for the audio source.
*/

#ifndef AudioSource_h
#define AudioSource_h

#include <QAudioFormat>
#include <QAudioOutput>
#include <QByteArray>
#include <QIODevice>

#include <cstdint>
#include <vector>

#include "SignalItem.h"


//************************************************************************
// Class for handling the audio source
//************************************************************************
class AudioSource : public QIODevice
{
    Q_OBJECT
    //************************************************************************
    // functions
    //************************************************************************
    public:
        AudioSource
            (
            const QAudioFormat& aFormat,                //!< audio format
            const uint32_t      aBufferLengthSeconds    //!< audio buffer length [seconds]
            );

        qint64 bytesAvailable() const override;

        bool isStarted() const;

        qint64 readData
            (
            char*       aData,              //!< data content
            qint64      aLength             //!< data length
            ) override;

        void setBufferLength
            (
            const double aLength            //!< a length in seconds
            );

        void setData
            (
            const std::vector<SignalItem*>  aSignalsVector  //!< signals vector
            );

        void start();

        void stop();

        qint64 writeData
            (
            const char* aData,              //!< data content
            qint64      aLength             //!< data length
            ) override;


    private:
        void fillDataBuffer();

        double generateRandomDek
            (
            int32_t*   pIdum                //!< seed value
            ) const;

        double generateRandomNag
            (
            int32_t*   pIdum                //!< seed value
            ) const;


        double getSignalValue
            (
            const double         aTime      //!< time
            ) const;


        double getSignalValueTriangle
            (
            const SignalItem::SignalTriangle    aSignalData,    //!< Triangle signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueRectangle
            (
            const SignalItem::SignalRectangle   aSignalData,    //!< Rectangle signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValuePulse
            (
            const SignalItem::SignalPulse       aSignalData,    //!< Pulse signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueRiseFall
            (
            const SignalItem::SignalRiseFall    aSignalData,    //!< RiseFall signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueSinDamp
            (
            const SignalItem::SignalSinDamp     aSignalData,    //!< SinDamp signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueSinRise
            (
            const SignalItem::SignalSinRise     aSignalData,    //!< SinRise signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueWavSin
            (
            const SignalItem::SignalWavSin      aSignalData,    //!< WavSin signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueAmSin
            (
            const SignalItem::SignalAmSin       aSignalData,    //!< AmSin signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueSinDampSin
            (
            const SignalItem::SignalSinDampSin  aSignalData,    //!< SinDampSin signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueTrapDampSin
            (
            const SignalItem::SignalTrapDampSin aSignalData,    //!< TrapDampSin signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueNoise
            (
            const SignalItem::SignalNoise       aSignalData,    //!< Noise signal data
            const double                        aTime           //!< time
            ) const;

        double getSignalValueSmc
            (
            const SignalItem::SignalSmc         aSignalData,    //!< SMC signal data
            const double                        aTime           //!< time
            ) const;


        void pseudoDes
            (
            uint32_t*   lword,      //!< left word
            uint32_t*   irword      //!< right word
            ) const;


    //************************************************************************
    // variables
    //************************************************************************
    private:
        QAudioFormat                mAudioFormat;               //!< audio format
        double                      mAudioBufferLengthSeconds;  //!< length of audio buffer [seconds]
        qint64                      mBufferPos;                 //!< current position in data buffer
        QByteArray                  mAudioBuffer;               //!< audio data buffer
        std::vector<SignalItem*>    mSignalsVector;             //!< signals vector
};

#endif // AudioSource_h
