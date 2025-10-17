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
SignalGenerator.h
This file contains the definitions for the signal generator.
*/

#ifndef SignalGenerator_h
#define SignalGenerator_h

#include <QAudioDevice>
#include <QAudioOutput>
#include <QAudioSink>
#include <QMainWindow>
#include <QMediaDevices>
#include <QMessageBox>
#include <QStringListModel>
#include <QTimer>

#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "AudioSource.h"
#include "SignalItem.h"
#include "Smc.h"
#include "./ui_About.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
    class SignalGenerator;
}
QT_END_NAMESPACE


//************************************************************************
// Class for handling the signal generator
//************************************************************************
class SignalGenerator : public QMainWindow
{
    Q_OBJECT

    //************************************************************************
    // constants and types
    //************************************************************************
    private:
        static constexpr double FREQ_MAX_HZ = 20000;                    //!< f_max = 20 kHz
        static constexpr double T_MIN_S = 1.0 / FREQ_MAX_HZ;            //!< T_min = 50 us

        const QString GAMMA_SMALL = QString::fromUtf8( "\u03B3" );      //!< small Greek gamma
        const QString PHI_SMALL = QString::fromUtf8( "\u03C6" );        //!< small Greek phi
        const QString SUBSTR_DELIMITER = ", ";                          //!< parameter delimiter in signal
        const QString NA_STR = "N/A";                                   //!< not available

        static const int TIMER_PER_MS = 1000;                           //!< timer period [ms]

    //************************************************************************
    // functions
    //************************************************************************
    public:
        SignalGenerator
            (
            QWidget*    aParent = nullptr   //!< a parent widget
            );

        ~SignalGenerator();


    private:
        bool checkValidInteger
            (
            const int16_t aIntValue         //!< integer value
            ) const;

        bool checkValidReal
            (
            const double aRealValue         //!< real value
            ) const;

        QString createSignalStringTriangle
            (
            const SignalItem::SignalTriangle    aSignal     //!< a Triangle signal
            ) const;

        QString createSignalStringRectangle
            (
            const SignalItem::SignalRectangle   aSignal     //!< a Rectangle signal
            ) const;

        QString createSignalStringPulse
            (
            const SignalItem::SignalPulse       aSignal     //!< a Pulse signal
            ) const;

        QString createSignalStringRiseFall
            (
            const SignalItem::SignalRiseFall    aSignal     //!< a RiseFall signal
            ) const;

        QString createSignalStringSinDamp
            (
            const SignalItem::SignalSinDamp     aSignal     //!< a SinDamp signal
            ) const;

        QString createSignalStringSinRise
            (
            const SignalItem::SignalSinRise     aSignal     //!< a SinRise signal
            ) const;

        QString createSignalStringWavSin
            (
            const SignalItem::SignalWavSin      aSignal     //!< a WavSin signal
            ) const;

        QString createSignalStringAmSin
            (
            const SignalItem::SignalAmSin       aSignal     //!< a AmSin signal
            ) const;

        QString createSignalStringSinDampSin
            (
            const SignalItem::SignalSinDampSin  aSignal     //!< a SinDampSin signal
            ) const;

        QString createSignalStringTrapDampSin
            (
            const SignalItem::SignalTrapDampSin aSignal     //!< a TrapDampSin signal
            ) const;

        QString createSignalStringNoise
            (
            const SignalItem::SignalNoise       aSignal     //!< a Noise signal
            ) const;


        void createSmcSignal();

        void createTabSignalsMap();

        void fillValuesTriangle();
        void fillValuesRectangle();
        void fillValuesPulse();
        void fillValuesRiseFall();
        void fillValuesSinDamp();
        void fillValuesSinRise();
        void fillValuesWavSin();
        void fillValuesAmSin();
        void fillValuesSinDampSin();
        void fillValuesTrapDampSin();
        void fillValuesNoise();
        void fillValuesSmc();

        bool initializeAudio
            (
            const QAudioDevice&     aDeviceInfo     //!< audio device
            );

        void setAudioData();

        void trim
            (
            std::string&    aString         //!< string to trim
            );

        void updateControls();

    private slots:
        void handleAudioBufferLengthChanged
            (
            double aValue   //!< value
            );

        void handleAbout();

        void handleDeviceChanged
            (
            int aIndex      //!< index
            );

        void handleAddReplaceSignal();
        void handleEditSignal();
        void handleSaveSignal();
        void handleRemoveSignal();

        void handleExit();

        void handleGenerateStart();
        void handleGeneratePauseResume();
        void handleGenerateStop();

        void handleSignalNew();
        void handleSignalOpen();


        void handleSignalChangedTriangleTPeriod();
        void handleSignalChangedTriangleTRise();
        void handleSignalChangedTriangleTDelay();
        void handleSignalChangedTriangleYMax();
        void handleSignalChangedTriangleYMin();

        void handleSignalChangedRectangleTPeriod();
        void handleSignalChangedRectangleFillFactor();
        void handleSignalChangedRectangleTDelay();
        void handleSignalChangedRectangleYMax();
        void handleSignalChangedRectangleYMin();

        void handleSignalChangedPulseTPeriod();
        void handleSignalChangedPulseTRise();
        void handleSignalChangedPulseTWidth();
        void handleSignalChangedPulseTFall();
        void handleSignalChangedPulseTDelay();
        void handleSignalChangedPulseYMax();
        void handleSignalChangedPulseYMin();

        void handleSignalChangedRiseFallTDelay();
        void handleSignalChangedRiseFallTDelayRise();
        void handleSignalChangedRiseFallTRampRise();
        void handleSignalChangedRiseFallTDelayFall();
        void handleSignalChangedRiseFallTRampFall();
        void handleSignalChangedRiseFallYMax();
        void handleSignalChangedRiseFallYMin();

        void handleSignalChangedSinDampFreq();
        void handleSignalChangedSinDampPhi();
        void handleSignalChangedSinDampTDelay();
        void handleSignalChangedSinDampAmplitude();
        void handleSignalChangedSinDampOffset();
        void handleSignalChangedSinDampDamping();

        void handleSignalChangedSinRiseFreq();
        void handleSignalChangedSinRisePhi();
        void handleSignalChangedSinRiseTEnd();
        void handleSignalChangedSinRiseTDelay();
        void handleSignalChangedSinRiseAmplitude();
        void handleSignalChangedSinRiseOffset();
        void handleSignalChangedSinRiseDamping();

        void handleSignalChangedWavSinFreq();
        void handleSignalChangedWavSinPhi();
        void handleSignalChangedWavSinTDelay();
        void handleSignalChangedWavSinAmplitude();
        void handleSignalChangedWavSinOffset();
        void handleSignalChangedWavSinNOrder
            (
            int     aIndex      //!< index
            );

        void handleSignalChangedAmSinCarrierFreq();
        void handleSignalChangedAmSinCarrierAmplitude();
        void handleSignalChangedAmSinCarrierOffset();
        void handleSignalChangedAmSinCarrierTDelay();
        void handleSignalChangedAmSinModulationFreq();
        void handleSignalChangedAmSinModulationPhi();
        void handleSignalChangedAmSinModulationIndex();

        void handleSignalChangedSinDampSinFreq();
        void handleSignalChangedSinDampSinTPeriodEnv();
        void handleSignalChangedSinDampSinTDelay();
        void handleSignalChangedSinDampSinAmplitude();
        void handleSignalChangedSinDampSinOffset();
        void handleSignalChangedSinDampSinDampingType
            (
            int     aIndex      //!< index
            );

        void handleSignalChangedTrapDampSinTPeriod();
        void handleSignalChangedTrapDampSinTRise();
        void handleSignalChangedTrapDampSinTFall();
        void handleSignalChangedTrapDampSinTWidth();
        void handleSignalChangedTrapDampSinTDelay();
        void handleSignalChangedTrapDampSinTCross();
        void handleSignalChangedTrapDampSinFreq();
        void handleSignalChangedTrapDampSinAmplitude();
        void handleSignalChangedTrapDampSinOffset();

        void handleSignalChangedNoiseType
            (
            int     aIndex      //!< index
            );
        void handleSignalChangedNoiseGamma
            (
            double  aValue      //!< value
            );
        void handleSignalChangedNoiseTDelay();
        void handleSignalChangedNoiseAmplitude();
        void handleSignalChangedNoiseOffset();


        void handleSignalTypeChanged();

        void handleSmcOpen();

        void handleVolumeChanged
            (
            int     aValue      //!< index
            );

        void updateAudioBufferTimer();

        void updateAudioDevices();


    //************************************************************************
    // variables
    //************************************************************************
    private:
        Ui::SignalGenerator*            mMainUi;                //!< main UI
        Ui::AboutDialog*                mAboutUi;               //!< about dialog

        std::map<int, std::string>      mTabSignalsMap;         //!< map of tab signals

        bool                            mSignalUndefined;       //!< true if a signal is undefined
        bool                            mSignalReady;           //!< true if a signal is ready
        bool                            mSignalStarted;         //!< true if a signal is currently generated
        bool                            mSignalPaused;          //!< true if a signal is currently paused
        bool                            mSignalIsSmc;           //!< true if the signal is SMC data

        int                             mCurrentSignalType;     //!< current signal type

        SignalItem::SignalTriangle      mSignalTriangle;        //!< current data for Triangle signal
        SignalItem::SignalRectangle     mSignalRectangle;       //!< current data for Rectangle signal
        SignalItem::SignalPulse         mSignalPulse;           //!< current data for Pulse signal
        SignalItem::SignalRiseFall      mSignalRiseFall;        //!< current data for RiseFall signal
        SignalItem::SignalSinDamp       mSignalSinDamp;         //!< current data for SinDamp signal
        SignalItem::SignalSinRise       mSignalSinRise;         //!< current data for SinRise signal
        SignalItem::SignalWavSin        mSignalWavSin;          //!< current data for WavSin signal
        SignalItem::SignalAmSin         mSignalAmSin;           //!< current data for AmSin signal
        SignalItem::SignalSinDampSin    mSignalSinDampSin;      //!< current data for SinDampSin signal
        SignalItem::SignalTrapDampSin   mSignalTrapDampSin;     //!< current data for TrapDampSin signal
        SignalItem::SignalNoise         mSignalNoise;           //!< current data for Noise signal

        std::vector<SignalItem*>        mSignalsVector;         //!< signals vector

        QStringListModel                mSignalsListModel;      //!< signals list model
        QStringList                     mSignalsList;           //!< signals list

        SignalItem*                     mEditedSignal;          //!< signal which is edited
        bool                            mIsSignalEdited;        //!< if a signal is edited

        QMediaDevices*                  mDevices;               //!< media devices
        QScopedPointer<AudioSource>     mAudioSrc;              //!< audio source
        QScopedPointer<QAudioSink>      mAudioOutput;           //!< audio output
        double                          mAudioBufferLength;     //!< audio buffer length [s]

        int                             mAudioBufferProgress;   //!< percentage progress in audio buffer
        QTimer*                         mAudioBufferTimer;      //!< timer for progress in audio buffer
        uint64_t                        mAudioBufferCounter;    //!< counter for the audio buffer

        Smc                             mSmc;                   //!< SMC (Strong-Motion CD) data object
        std::string                     mSmcInputFilename;      //!< SMC file name
};

#endif // SignalGenerator_h
