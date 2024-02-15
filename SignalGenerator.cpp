///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2023 Mihai Ursu                                                 //
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
SignalGenerator.cpp
This file contains the sources for the signal generator.
*/

#include "SignalGenerator.h"
#include "./ui_SignalGenerator.h"

#include <QFileDialog>

#include <algorithm>
#include <iostream>
#include <fstream>

#include "NoisePwrSpectrum.h"


//!************************************************************************
//! Constructor
//!************************************************************************
SignalGenerator::SignalGenerator
    (
    QWidget*    aParent     //!< a parent widget
    )
    : QMainWindow( aParent )
    , mMainUi( new Ui::SignalGenerator )
    , mAboutUi( new Ui::AboutDialog )
    , mSignalUndefined( true )
    , mSignalReady( false )
    , mSignalStarted( false )
    , mSignalPaused( false )
    , mEditedSignal( nullptr )
    , mIsSignalEdited( false )
    , mAudioBufferLength( 30 )
    , mAudioBufferProgress( 0 )
    , mAudioBufferTimer( new QTimer( this ) )
    , mAudioBufferCounter( 0 )
{
    mMainUi->setupUi( this );

    // exit
    connect( mMainUi->ExitButton, SIGNAL( clicked() ), this, SLOT( handleExit() ) );

    //****************************************
    // signals tabs
    //****************************************
    mCurrentSignalType = SignalItem::SIGNAL_TYPE_FIRST + mMainUi->SignalTypesTab->currentIndex();

    mMainUi->SignalTypesTab->setStyleSheet( "background-color: rgb(240, 240, 240)" );
    createTabSignalsMap();
    connect( mMainUi->SignalTypesTab, &QTabWidget::currentChanged, this, &SignalGenerator::handleSignalTypeChanged );


    // Triangle
    fillValuesTriangle();
    connect( mMainUi->TriangleTPerEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTriangleTPeriod );
    connect( mMainUi->TriangleTRiseEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTriangleTRise );
    connect( mMainUi->TriangleTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTriangleTDelay );
    connect( mMainUi->TriangleYMaxEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTriangleYMax );
    connect( mMainUi->TriangleYMinEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTriangleYMin );

    // Rectangle
    fillValuesRectangle();
    connect( mMainUi->RectangleTPerEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRectangleTPeriod );
    connect( mMainUi->RectangleFillFactorEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRectangleFillFactor );
    connect( mMainUi->RectangleTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRectangleTDelay );
    connect( mMainUi->RectangleYMaxEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRectangleYMax );
    connect( mMainUi->RectangleYMinEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRectangleYMin );

    // Pulse
    fillValuesPulse();
    connect( mMainUi->PulseTPerEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseTPeriod );
    connect( mMainUi->PulseTRiseEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseTRise );
    connect( mMainUi->PulseTWidthEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseTWidth );
    connect( mMainUi->PulseTFallEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseTFall );
    connect( mMainUi->PulseTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseTDelay );
    connect( mMainUi->PulseYMaxEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseYMax );
    connect( mMainUi->PulseYMinEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedPulseYMin );

    // RiseFall
    fillValuesRiseFall();
    connect( mMainUi->RiseFallTDelayRiseEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallTDelayRise );
    connect( mMainUi->RiseFallTRampRiseEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallTRampRise );
    connect( mMainUi->RiseFallTDelayFallEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallTDelayFall );
    connect( mMainUi->RiseFallTRampFallEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallTRampFall );
    connect( mMainUi->RiseFallTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallTDelay );
    connect( mMainUi->RiseFallYMaxEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallYMax );
    connect( mMainUi->RiseFallYMinEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedRiseFallYMin );

    // SinDamp
    fillValuesSinDamp();
    connect( mMainUi->SinDampFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampFreq );
    connect( mMainUi->SinDampPhiEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampPhi );
    connect( mMainUi->SinDampTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampTDelay );
    connect( mMainUi->SinDampAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampAmplitude );
    connect( mMainUi->SinDampOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampOffset );
    connect( mMainUi->SinDampDampingEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampDamping );

    // SinRise
    fillValuesSinRise();
    connect( mMainUi->SinRiseFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseFreq );
    connect( mMainUi->SinRisePhiEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRisePhi );
    connect( mMainUi->SinRiseTEndEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseTEnd );
    connect( mMainUi->SinRiseTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseTDelay );
    connect( mMainUi->SinRiseAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseAmplitude );
    connect( mMainUi->SinRiseOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseOffset );
    connect( mMainUi->SinRiseDampingEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinRiseDamping );

    // WavSin
    fillValuesWavSin();
    connect( mMainUi->WavSinFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedWavSinFreq );
    connect( mMainUi->WavSinPhiEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedWavSinPhi );
    connect( mMainUi->WavSinTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedWavSinTDelay );
    connect( mMainUi->WavSinAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedWavSinAmplitude );
    connect( mMainUi->WavSinOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedWavSinOffset );
    connect( mMainUi->WavSinNOrderSpin, SIGNAL( valueChanged(int) ), this, SLOT( handleSignalChangedWavSinNOrder(int) ) );

    // AmSin
    fillValuesAmSin();
    connect( mMainUi->AmSinCarrierFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinCarrierFreq );
    connect( mMainUi->AmSinCarrierAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinCarrierAmplitude );
    connect( mMainUi->AmSinCarrierOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinCarrierOffset );
    connect( mMainUi->AmSinCarrierTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinCarrierTDelay );
    connect( mMainUi->AmSinModFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinModulationFreq );
    connect( mMainUi->AmSinModPhiEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinModulationPhi );
    connect( mMainUi->AmSinModModEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedAmSinModulationIndex );

    // SinDampSin
    fillValuesSinDampSin();
    connect( mMainUi->SinDampSinFreqSinEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampSinFreq );
    connect( mMainUi->SinDampSinTEnvEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampSinTPeriodEnv );
    connect( mMainUi->SinDampSinTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampSinTDelay );
    connect( mMainUi->SinDampSinAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampSinAmplitude );
    connect( mMainUi->SinDampSinOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedSinDampSinOffset );
    connect( mMainUi->SinDampSinDampingTypeSpin, SIGNAL( valueChanged(int) ), this, SLOT( handleSignalChangedSinDampSinDampingType(int) ) );

    // TrapDampSin
    fillValuesTrapDampSin();
    connect( mMainUi->TrapDampSinTPerEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTPeriod );
    connect( mMainUi->TrapDampSinTRiseEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTRise );
    connect( mMainUi->TrapDampSinTWidthEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTWidth );
    connect( mMainUi->TrapDampSinTFallEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTFall );
    connect( mMainUi->TrapDampSinTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTDelay );
    connect( mMainUi->TrapDampSinTCrossEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinTCross );
    connect( mMainUi->TrapDampSinFreqEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinFreq );
    connect( mMainUi->TrapDampSinAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinAmplitude );
    connect( mMainUi->TrapDampSinOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedTrapDampSinOffset );

    // Noise
    fillValuesNoise();
    connect( mMainUi->NoiseTypeComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleSignalChangedNoiseType(int) ) );

    mMainUi->NoiseGammaLabel->setText( GAMMA_SMALL + " =" );
    connect( mMainUi->NoiseGammaSpin, SIGNAL( valueChanged(double) ), this, SLOT( handleSignalChangedNoiseGamma(double) ) );

    connect( mMainUi->NoiseTDelayEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedNoiseTDelay );
    connect( mMainUi->NoiseAmplitEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedNoiseAmplitude );
    connect( mMainUi->NoiseOffsetEdit, &QLineEdit::editingFinished, this, &SignalGenerator::handleSignalChangedNoiseOffset );


    // Add/Replace button
    connect( mMainUi->SignalItemActionButton, SIGNAL( clicked() ), this, SLOT( handleAddReplaceSignal() ) );


    //****************************************
    // Active signal
    //****************************************
    connect( mMainUi->ActiveSignalEditButton, SIGNAL( clicked() ), this, SLOT( handleEditSignal() ) );
    connect( mMainUi->ActiveSignalSaveButton, SIGNAL( clicked() ), this, SLOT( handleSaveSignal() ) );
    connect( mMainUi->ActiveSignalRemoveButton, SIGNAL( clicked() ), this, SLOT( handleRemoveSignal() ) );

    mSignalsListModel.setStringList( mSignalsList );
    mMainUi->ActiveSignalList->setModel( &mSignalsListModel );

    mMainUi->ActiveSignalList->setEditTriggers( QAbstractItemView::NoEditTriggers );


    //****************************************
    // Generate
    //****************************************
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    mMainUi->GenerateDeviceComboBox->addItem( defaultDeviceInfo.deviceName(), QVariant::fromValue( defaultDeviceInfo ) );

    for( auto &deviceInfo: QAudioDeviceInfo::availableDevices( QAudio::AudioOutput ) )
    {
        if( deviceInfo != defaultDeviceInfo )
        {
             mMainUi->GenerateDeviceComboBox->addItem( deviceInfo.deviceName(), QVariant::fromValue( deviceInfo ) );
        }
    }

    connect( mMainUi->GenerateDeviceComboBox, QOverload<int>::of( &QComboBox::activated ), this, &SignalGenerator::handleDeviceChanged );

    mMainUi->BufferLengthSpin->setRange( 2, 3600 );
    mMainUi->BufferLengthSpin->setValue( mAudioBufferLength );
    connect( mMainUi->BufferLengthSpin, SIGNAL( valueChanged(int) ), this, SLOT( handleAudioBufferLengthChanged(int) ) );

    mMainUi->BufferProgressBar->setRange( 0, 100 );
    mMainUi->BufferProgressBar->setValue( mAudioBufferProgress );

    connect( mAudioBufferTimer, SIGNAL( timeout() ), this, SLOT( updateAudioBufferTimer() ) );

    if( !initializeAudio( QAudioDeviceInfo::defaultOutputDevice() ) )
    {
        QMessageBox::warning( this,
                              "SignalGenerator",
                              "The required audio format is not supported on this system."
                              "\nGenerated waveforms may not be the expected ones.",
                              QMessageBox::Ok
                             );
    }

    connect( mMainUi->GenerateStartButton, SIGNAL( clicked() ), this, SLOT( handleGenerateStart() ) );
    connect( mMainUi->GeneratePauseButton, SIGNAL( clicked() ), this, SLOT( handleGeneratePauseResume() ) );
    connect( mMainUi->GenerateStopButton, SIGNAL( clicked() ), this, SLOT( handleGenerateStop() ) );

    connect( mMainUi->GenerateVolumeSlider, &QSlider::valueChanged, this, &SignalGenerator::handleVolumeChanged );

    //****************************************
    // menus
    //****************************************
    connect( mMainUi->actionNew, &QAction::triggered, this, &SignalGenerator::handleSignalNew );
    connect( mMainUi->actionOpen, &QAction::triggered, this, &SignalGenerator::handleSignalOpen );
    connect( mMainUi->actionExit, &QAction::triggered, this, &SignalGenerator::handleExit );

    connect( mMainUi->actionAbout, &QAction::triggered, this, &SignalGenerator::handleAbout );

#ifdef __unix__
    QFont font = QApplication::font();
    font.setFamily( "Sans Serif" );
    font.setPointSize( 8 );
    QApplication::setFont( font );
#endif

    updateControls();
}


//!************************************************************************
//! Destructor
//!************************************************************************
SignalGenerator::~SignalGenerator()
{
    delete mMainUi;
}


//!************************************************************************
//! Format a string for Triangle signals
//!
//! @returns Triangle signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringTriangle
    (
    const SignalItem::SignalTriangle    aSignal     //!< a Triangle signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tPeriod );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tRise );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tFall );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMax );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMin );

    return lineString;
}

//!************************************************************************
//! Format a string for Rectangle signals
//!
//! @returns Rectangle signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringRectangle
    (
    const SignalItem::SignalRectangle   aSignal     //!< a Rectangle signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tPeriod );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.fillFactor );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMax );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMin );

    return lineString;
}

//!************************************************************************
//! Format a string for Pulse signals
//!
//! @returns Pulse signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringPulse
    (
    const SignalItem::SignalPulse       aSignal     //!< a Pulse signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tPeriod );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tRise );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tWidth );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tFall );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMax );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMin );

    return lineString;
}

//!************************************************************************
//! Format a string for RiseFall signals
//!
//! @returns RiseFall signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringRiseFall
    (
    const SignalItem::SignalRiseFall    aSignal     //!< a RiseFall signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelayRise );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tRampRise );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelayFall );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tRampFall );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMax );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.yMin );

    return lineString;
}

//!************************************************************************
//! Format a string for SinDamp signals
//!
//! @returns SinDamp signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringSinDamp
    (
    const SignalItem::SignalSinDamp     aSignal     //!< a SinDamp signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.freqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.phiRad );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.damping );

    return lineString;
}

//!************************************************************************
//! Format a string for SinRise signals
//!
//! @returns SinRise signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringSinRise
    (
    const SignalItem::SignalSinRise     aSignal     //!< a SinRise signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.freqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.phiRad );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tEnd );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.damping );

    return lineString;
}

//!************************************************************************
//! Format a string for WavSin signals
//!
//! @returns WavSin signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringWavSin
    (
    const SignalItem::SignalWavSin      aSignal     //!< a WavSin signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.freqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.phiRad );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.index );

    return lineString;
}

//!************************************************************************
//! Format a string for AmSin signals
//!
//! @returns AmSin signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringAmSin
    (
    const SignalItem::SignalAmSin       aSignal     //!< a AmSin signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.carrierFreqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.carrierAmplitude );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.carrierOffset );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.carrierTDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.modulationFreqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.modulationPhiRad );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.modulationIndex );

    return lineString;
}

//!************************************************************************
//! Format a string for SinDampSin signals
//!
//! @returns SinDampSin signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringSinDampSin
    (
    const SignalItem::SignalSinDampSin  aSignal     //!< a SinDampSin signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.freqSinHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tPeriodEnv );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.dampingType );

    return lineString;
}

//!************************************************************************
//! Format a string for TrapDampSin signals
//!
//! @returns TrapDampSin signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringTrapDampSin
    (
    const SignalItem::SignalTrapDampSin aSignal     //!< a TrapDampSin signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tPeriod );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tRise );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tWidth );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tFall );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tCross );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.freqHz );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );

    return lineString;
}

//!************************************************************************
//! Format a string for Noise signals
//!
//! @returns Noise signal string with comma separated parameters
//!************************************************************************
QString SignalGenerator::createSignalStringNoise
    (
    const SignalItem::SignalNoise       aSignal     //!< a Noise signal
    ) const
{
    QString lineString = QString::number( aSignal.type );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.noiseType );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.gamma );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.tDelay );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.amplit );
    lineString += SUBSTR_DELIMITER + QString::number( aSignal.offset );

    return lineString;
}


//!************************************************************************
//! Create the map of tab signals
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::createTabSignalsMap()
{
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_INVALID,     "N/A" ) );

    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_TRIANGLE,    "SignalTabTriangle" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_RECTANGLE,   "SignalTabRectangle" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_PULSE,       "SignalTabPulse" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_RISEFALL,    "SignalTabRiseFall" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_SINDAMP,     "SignalTabSinDamp" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_SINRISE,     "SignalTabSinRise" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_WAVSIN,      "SignalTabWavSin" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_AMSIN,       "SignalTabAmSin" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_SINDAMPSIN,  "SignalTabSinDampSin" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_TRAPDAMPSIN, "SignalTabTrapDampSin" ) );
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_NOISE,       "SignalTabNoise" ) );
}


//!************************************************************************
//! Update the field values of Triangle signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesTriangle()
{
    mMainUi->TriangleTPerEdit->setText( QString::number( mSignalTriangle.tPeriod ) );
    mMainUi->TriangleTRiseEdit->setText( QString::number( mSignalTriangle.tRise ) );
    mMainUi->TriangleTFallEdit->setText( QString::number( mSignalTriangle.tFall ) );
    mMainUi->TriangleTDelayEdit->setText( QString::number( mSignalTriangle.tDelay ) );
    mMainUi->TriangleYMaxEdit->setText( QString::number( mSignalTriangle.yMax ) );
    mMainUi->TriangleYMinEdit->setText( QString::number( mSignalTriangle.yMin ) );
}

//!************************************************************************
//! Update the field values of Rectangle signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesRectangle()
{
    mMainUi->RectangleTPerEdit->setText( QString::number( mSignalRectangle.tPeriod ) );
    mMainUi->RectangleFillFactorEdit->setText( QString::number( mSignalRectangle.fillFactor ) );
    mMainUi->RectangleTDelayEdit->setText( QString::number( mSignalRectangle.tDelay ) );
    mMainUi->RectangleYMaxEdit->setText( QString::number( mSignalRectangle.yMax ) );
    mMainUi->RectangleYMinEdit->setText( QString::number( mSignalRectangle.yMin ) );
}

//!************************************************************************
//! Update the field values of Pulse signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesPulse()
{
    mMainUi->PulseTPerEdit->setText( QString::number( mSignalPulse.tPeriod ) );
    mMainUi->PulseTRiseEdit->setText( QString::number( mSignalPulse.tRise ) );
    mMainUi->PulseTWidthEdit->setText( QString::number( mSignalPulse.tWidth ) );
    mMainUi->PulseTFallEdit->setText( QString::number( mSignalPulse.tFall ) );
    mMainUi->PulseTDelayEdit->setText( QString::number( mSignalPulse.tDelay ) );
    mMainUi->PulseYMaxEdit->setText( QString::number( mSignalPulse.yMax ) );
    mMainUi->PulseYMinEdit->setText( QString::number( mSignalPulse.yMin ) );
}

//!************************************************************************
//! Update the field values of RiseFall signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesRiseFall()
{
    mMainUi->RiseFallTDelayRiseEdit->setText( QString::number( mSignalRiseFall.tDelayRise ) );
    mMainUi->RiseFallTRampRiseEdit->setText( QString::number( mSignalRiseFall.tRampRise ) );
    mMainUi->RiseFallTDelayFallEdit->setText( QString::number( mSignalRiseFall.tDelayFall ) );
    mMainUi->RiseFallTRampFallEdit->setText( QString::number( mSignalRiseFall.tRampFall ) );
    mMainUi->RiseFallTDelayEdit->setText( QString::number( mSignalRiseFall.tDelay ) );
    mMainUi->RiseFallYMaxEdit->setText( QString::number( mSignalRiseFall.yMax ) );
    mMainUi->RiseFallYMinEdit->setText( QString::number( mSignalRiseFall.yMin ) );
}

//!************************************************************************
//! Update the field values of SinDamp signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesSinDamp()
{
    mMainUi->SinDampFreqEdit->setText( QString::number( mSignalSinDamp.freqHz ) );
    mMainUi->SinDampPhiEdit->setText( QString::number( mSignalSinDamp.phiRad ) );
    mMainUi->SinDampTDelayEdit->setText( QString::number( mSignalSinDamp.tDelay ) );
    mMainUi->SinDampAmplitEdit->setText( QString::number( mSignalSinDamp.amplit ) );
    mMainUi->SinDampOffsetEdit->setText( QString::number( mSignalSinDamp.offset ) );
    mMainUi->SinDampDampingEdit->setText( QString::number( mSignalSinDamp.damping ) );
}

//!************************************************************************
//! Update the field values of SinRise signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesSinRise()
{
    mMainUi->SinRiseFreqEdit->setText( QString::number( mSignalSinRise.freqHz ) );
    mMainUi->SinRisePhiEdit->setText( QString::number( mSignalSinRise.phiRad ) );
    mMainUi->SinRiseTEndEdit->setText( QString::number( mSignalSinRise.tEnd ) );
    mMainUi->SinRiseTDelayEdit->setText( QString::number( mSignalSinRise.tDelay ) );
    mMainUi->SinRiseAmplitEdit->setText( QString::number( mSignalSinRise.amplit ) );
    mMainUi->SinRiseOffsetEdit->setText( QString::number( mSignalSinRise.offset ) );
    mMainUi->SinRiseDampingEdit->setText( QString::number( mSignalSinRise.damping ) );
}

//!************************************************************************
//! Update the field values of WavSin signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesWavSin()
{
    mMainUi->WavSinFreqEdit->setText( QString::number( mSignalWavSin.freqHz ) );
    mMainUi->WavSinPhiEdit->setText( QString::number( mSignalWavSin.phiRad ) );
    mMainUi->WavSinTDelayEdit->setText( QString::number( mSignalWavSin.tDelay ) );
    mMainUi->WavSinAmplitEdit->setText( QString::number( mSignalWavSin.amplit ) );
    mMainUi->WavSinOffsetEdit->setText( QString::number( mSignalWavSin.offset ) );
    mMainUi->WavSinNOrderSpin->setValue( mSignalWavSin.index );
}

//!************************************************************************
//! Update the field values of AmSin signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesAmSin()
{
    mMainUi->AmSinCarrierFreqEdit->setText( QString::number( mSignalAmSin.carrierFreqHz ) );
    mMainUi->AmSinCarrierAmplitEdit->setText( QString::number( mSignalAmSin.carrierAmplitude ) );
    mMainUi->AmSinCarrierOffsetEdit->setText( QString::number( mSignalAmSin.carrierOffset ) );
    mMainUi->AmSinCarrierTDelayEdit->setText( QString::number( mSignalAmSin.carrierTDelay ) );
    mMainUi->AmSinModFreqEdit->setText( QString::number( mSignalAmSin.modulationFreqHz ) );
    mMainUi->AmSinModPhiEdit->setText( QString::number( mSignalAmSin.modulationPhiRad ) );
    mMainUi->AmSinModModEdit->setText( QString::number( mSignalAmSin.modulationIndex ) );
}

//!************************************************************************
//! Update the field values of SinDampSin signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesSinDampSin()
{
    mMainUi->SinDampSinFreqSinEdit->setText( QString::number( mSignalSinDampSin.freqSinHz ) );
    mMainUi->SinDampSinTEnvEdit->setText( QString::number( mSignalSinDampSin.tPeriodEnv  ) );
    mMainUi->SinDampSinTDelayEdit->setText( QString::number( mSignalSinDampSin.tDelay ) );
    mMainUi->SinDampSinAmplitEdit->setText( QString::number( mSignalSinDampSin.amplit ) );
    mMainUi->SinDampSinOffsetEdit->setText( QString::number( mSignalSinDampSin.offset ) );
    mMainUi->SinDampSinDampingTypeSpin->setValue( mSignalSinDampSin.dampingType );
}

//!************************************************************************
//! Update the field values of TrapDampSin signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesTrapDampSin()
{
    mMainUi->TrapDampSinTPerEdit->setText( QString::number( mSignalTrapDampSin.tPeriod ) );
    mMainUi->TrapDampSinTRiseEdit->setText( QString::number( mSignalTrapDampSin.tRise ) );
    mMainUi->TrapDampSinTWidthEdit->setText( QString::number( mSignalTrapDampSin.tWidth ) );
    mMainUi->TrapDampSinTFallEdit->setText( QString::number( mSignalTrapDampSin.tFall ) );
    mMainUi->TrapDampSinTDelayEdit->setText( QString::number( mSignalTrapDampSin.tDelay ) );
    mMainUi->TrapDampSinTCrossEdit->setText( QString::number( mSignalTrapDampSin.tCross ) );
    mMainUi->TrapDampSinFreqEdit->setText( QString::number( mSignalTrapDampSin.freqHz ) );
    mMainUi->TrapDampSinAmplitEdit->setText( QString::number( mSignalTrapDampSin.amplit ) );
    mMainUi->TrapDampSinOffsetEdit->setText( QString::number( mSignalTrapDampSin.offset ) );
}

//!************************************************************************
//! Update the field values of Noise signal tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesNoise()
{
    mMainUi->NoiseTypeComboBox->setCurrentIndex( mSignalNoise.noiseType );
    mMainUi->NoiseGammaSpin->setValue( mSignalNoise.gamma );
    mMainUi->NoiseTDelayEdit->setText( QString::number( mSignalNoise.tDelay ) );
    mMainUi->NoiseAmplitEdit->setText( QString::number( mSignalNoise.amplit ) );
    mMainUi->NoiseOffsetEdit->setText( QString::number( mSignalNoise.offset ) );
}


//!************************************************************************
//! Handle for changing the audio buffer length (seconds)
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleAudioBufferLengthChanged
    (
    int aValue      //!< value
    )
{
    mAudioBufferLength = aValue;

    if( mAudioSrc )
    {
        mAudioSrc->setBufferLength( mAudioBufferLength );
    }
}


//!************************************************************************
//! About dialog box
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleAbout()
{
    QDialog dialog;
    mAboutUi->setupUi( &dialog );
    connect( mAboutUi->OkButton, SIGNAL( clicked() ), &dialog, SLOT( close() ) );
    dialog.exec();
}


//!************************************************************************
//! Handle for changing the audio device
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::handleDeviceChanged
    (
    int     aIndex      //!< index
    )
{
    mAudioOutput->stop();
    mAudioOutput->disconnect( this );

    if( mAudioSrc )
    {
        mAudioSrc->stop();
    }

    initializeAudio( mMainUi->GenerateDeviceComboBox->itemData( aIndex ).value<QAudioDeviceInfo>() );

    if( mSignalReady )
    {
        setAudioData();
    }
}


//!************************************************************************
//! Handle for adding/replacing the current signal
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleAddReplaceSignal()
{
    if( !mIsSignalEdited ) // add a new signal
    {
        int row = mSignalsListModel.rowCount();
        mSignalsListModel.insertRow( row );

        QModelIndex index = mSignalsListModel.index( row );
        mMainUi->ActiveSignalList->setCurrentIndex( index );

        SignalItem::SignalType sigType = static_cast<SignalItem::SignalType>( mCurrentSignalType );
        SignalItem* crtSignal = nullptr;

        switch( sigType )
        {
            case SignalItem::SIGNAL_TYPE_TRIANGLE:
                crtSignal = new SignalItem( mSignalTriangle );
                break;

            case SignalItem::SIGNAL_TYPE_RECTANGLE:
                crtSignal = new SignalItem( mSignalRectangle );
                break;

            case SignalItem::SIGNAL_TYPE_PULSE:
                crtSignal = new SignalItem( mSignalPulse );
                break;

            case SignalItem::SIGNAL_TYPE_RISEFALL:
                crtSignal = new SignalItem( mSignalRiseFall );
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMP:
                crtSignal = new SignalItem( mSignalSinDamp );
                break;

            case SignalItem::SIGNAL_TYPE_SINRISE:
                crtSignal = new SignalItem( mSignalSinRise );
                break;

            case SignalItem::SIGNAL_TYPE_WAVSIN:
                crtSignal = new SignalItem( mSignalWavSin );
                break;

            case SignalItem::SIGNAL_TYPE_AMSIN:
                crtSignal = new SignalItem( mSignalAmSin );
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                crtSignal = new SignalItem( mSignalSinDampSin );
                break;

            case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                crtSignal = new SignalItem( mSignalTrapDampSin );
                break;

            case SignalItem::SIGNAL_TYPE_NOISE:
                crtSignal = new SignalItem( mSignalNoise );
                break;

            default:
                break;
        }

        if( crtSignal )
        {
            mSignalsVector.push_back( crtSignal );

            switch( sigType )
            {
                case SignalItem::SIGNAL_TYPE_TRIANGLE:
                    {
                        SignalItem::SignalTriangle sigTriangle = crtSignal->getSignalDataTriangle();
                        mSignalsListModel.setData( index, createSignalStringTriangle( sigTriangle ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_RECTANGLE:
                    {
                        SignalItem::SignalRectangle sigRectangle = crtSignal->getSignalDataRectangle();
                        mSignalsListModel.setData( index, createSignalStringRectangle( sigRectangle ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_PULSE:
                    {
                        SignalItem::SignalPulse sigPulse = crtSignal->getSignalDataPulse();
                        mSignalsListModel.setData( index, createSignalStringPulse( sigPulse ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_RISEFALL:
                    {
                        SignalItem::SignalRiseFall sigRiseFall = crtSignal->getSignalDataRiseFall();
                        mSignalsListModel.setData( index, createSignalStringRiseFall( sigRiseFall ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMP:
                    {
                        SignalItem::SignalSinDamp sigSinDamp = crtSignal->getSignalDataSinDamp();
                        mSignalsListModel.setData( index, createSignalStringSinDamp( sigSinDamp ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_SINRISE:
                    {
                        SignalItem::SignalSinRise sigSinRise = crtSignal->getSignalDataSinRise();
                        mSignalsListModel.setData( index, createSignalStringSinRise( sigSinRise ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_WAVSIN:
                    {
                        SignalItem::SignalWavSin sigWavSin = crtSignal->getSignalDataWavSin();
                        mSignalsListModel.setData( index, createSignalStringWavSin( sigWavSin ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_AMSIN:
                    {
                        SignalItem::SignalAmSin sigAmSin = crtSignal->getSignalDataAmSin();
                        mSignalsListModel.setData( index, createSignalStringAmSin( sigAmSin ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                    {
                        SignalItem::SignalSinDampSin sigSinDampSin = crtSignal->getSignalDataSinDampSin();
                        mSignalsListModel.setData( index, createSignalStringSinDampSin( sigSinDampSin ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                    {
                        SignalItem::SignalTrapDampSin sigTrapDampSin = crtSignal->getSignalDataTrapDampSin();
                        mSignalsListModel.setData( index, createSignalStringTrapDampSin( sigTrapDampSin ) );
                    }
                    break;

                case SignalItem::SIGNAL_TYPE_NOISE:
                    {
                        SignalItem::SignalNoise sigNoise = crtSignal->getSignalDataNoise();
                        mSignalsListModel.setData( index, createSignalStringNoise( sigNoise ) );
                    }
                    break;

                default:
                    break;
            }

            if( mSignalUndefined )
            {
                mSignalUndefined = false;
            }

            mSignalReady = false;

            if( mAudioSrc )
            {
                if( mAudioSrc->isOpen() )
                {
                    mAudioSrc->stop();
                }
            }
        }
    }
    else // replace the edited signal
    {
        if( mEditedSignal )
        {
            int crtRow = mMainUi->ActiveSignalList->currentIndex().row();
            QModelIndex index = mSignalsListModel.index( crtRow );

            SignalItem::SignalType sigType = mEditedSignal->getType();            

            switch( sigType )
            {
                case SignalItem::SIGNAL_TYPE_TRIANGLE:
                    *mEditedSignal = SignalItem( mSignalTriangle );
                    mSignalsListModel.setData( index, createSignalStringTriangle( mSignalTriangle ) );
                    break;

                case SignalItem::SIGNAL_TYPE_RECTANGLE:
                    *mEditedSignal = SignalItem( mSignalRectangle );
                    mSignalsListModel.setData( index, createSignalStringRectangle( mSignalRectangle ) );
                    break;

                case SignalItem::SIGNAL_TYPE_PULSE:
                    *mEditedSignal = SignalItem( mSignalPulse );
                    mSignalsListModel.setData( index, createSignalStringPulse( mSignalPulse ) );
                    break;

                case SignalItem::SIGNAL_TYPE_RISEFALL:
                    *mEditedSignal = SignalItem( mSignalRiseFall );
                    mSignalsListModel.setData( index, createSignalStringRiseFall( mSignalRiseFall ) );
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMP:
                    *mEditedSignal = SignalItem( mSignalSinDamp );
                    mSignalsListModel.setData( index, createSignalStringSinDamp( mSignalSinDamp ) );
                    break;

                case SignalItem::SIGNAL_TYPE_SINRISE:
                    *mEditedSignal = SignalItem( mSignalSinRise );
                    mSignalsListModel.setData( index, createSignalStringSinRise( mSignalSinRise ) );
                    break;

                case SignalItem::SIGNAL_TYPE_WAVSIN:
                    *mEditedSignal = SignalItem( mSignalWavSin );
                    mSignalsListModel.setData( index, createSignalStringWavSin( mSignalWavSin ) );
                    break;

                case SignalItem::SIGNAL_TYPE_AMSIN:
                    *mEditedSignal = SignalItem( mSignalAmSin );
                    mSignalsListModel.setData( index, createSignalStringAmSin( mSignalAmSin ) );
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                    *mEditedSignal = SignalItem( mSignalSinDampSin );
                    mSignalsListModel.setData( index, createSignalStringSinDampSin( mSignalSinDampSin ) );
                    break;

                case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                    *mEditedSignal = SignalItem( mSignalTrapDampSin );
                    mSignalsListModel.setData( index, createSignalStringTrapDampSin( mSignalTrapDampSin ) );
                    break;

                case SignalItem::SIGNAL_TYPE_NOISE:
                    *mEditedSignal = SignalItem( mSignalNoise );
                    mSignalsListModel.setData( index, createSignalStringNoise( mSignalNoise ) );
                    break;

                default:
                    break;
            }

            // replace vector item
            mSignalsVector.at( crtRow ) = mEditedSignal;

            mEditedSignal = nullptr;
            mIsSignalEdited = false;

            mSignalReady = false;

            if( mAudioSrc )
            {
                if( mAudioSrc->isOpen() )
                {
                    mAudioSrc->stop();
                }
            }
        }
    }

    updateControls();
}


//!************************************************************************
//! Handle for editing the current signal from the list
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleEditSignal()
{
    int crtRow = mMainUi->ActiveSignalList->currentIndex().row();
    mEditedSignal = mSignalsVector.at( crtRow );

    if( mEditedSignal )
    {
        SignalItem::SignalType sigType = mEditedSignal->getType();
        int crtTab = sigType - SignalItem::SIGNAL_TYPE_FIRST;
        mMainUi->SignalTypesTab->setCurrentIndex( crtTab );

        switch( sigType )
        {
            case SignalItem::SIGNAL_TYPE_TRIANGLE:
                {
                    SignalItem::SignalTriangle sig = mEditedSignal->getSignalDataTriangle();
                    memcpy( &mSignalTriangle, &sig, sizeof( mSignalTriangle ) );
                    fillValuesTriangle();
                }
                break;

            case SignalItem::SIGNAL_TYPE_RECTANGLE:
                {
                    SignalItem::SignalRectangle sig = mEditedSignal->getSignalDataRectangle();
                    memcpy( &mSignalRectangle, &sig, sizeof( mSignalRectangle ) );
                    fillValuesRectangle();
                }
                break;

            case SignalItem::SIGNAL_TYPE_PULSE:
                {
                    SignalItem::SignalPulse sig = mEditedSignal->getSignalDataPulse();
                    memcpy( &mSignalPulse, &sig, sizeof( mSignalPulse ) );
                    fillValuesPulse();
                }
                break;

            case SignalItem::SIGNAL_TYPE_RISEFALL:
                {
                    SignalItem::SignalRiseFall sig = mEditedSignal->getSignalDataRiseFall();
                    memcpy( &mSignalRiseFall, &sig, sizeof( mSignalRiseFall ) );
                    fillValuesRiseFall();
                }
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMP:
                {
                    SignalItem::SignalSinDamp sig = mEditedSignal->getSignalDataSinDamp();
                    memcpy( &mSignalSinDamp, &sig, sizeof( mSignalSinDamp ) );
                    fillValuesSinDamp();
                }
                break;

            case SignalItem::SIGNAL_TYPE_SINRISE:
                {
                    SignalItem::SignalSinRise sig = mEditedSignal->getSignalDataSinRise();
                    memcpy( &mSignalSinRise, &sig, sizeof( mSignalSinRise ) );
                    fillValuesSinRise();
                }
                break;

            case SignalItem::SIGNAL_TYPE_WAVSIN:
                {
                    SignalItem::SignalWavSin sig = mEditedSignal->getSignalDataWavSin();
                    memcpy( &mSignalWavSin, &sig, sizeof( mSignalWavSin ) );
                    fillValuesWavSin();
                }
                break;

            case SignalItem::SIGNAL_TYPE_AMSIN:
                {
                    SignalItem::SignalAmSin sig = mEditedSignal->getSignalDataAmSin();
                    memcpy( &mSignalAmSin, &sig, sizeof( mSignalAmSin ) );
                    fillValuesAmSin();
                }
                break;

            case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                {
                    SignalItem::SignalSinDampSin sig = mEditedSignal->getSignalDataSinDampSin();
                    memcpy( &mSignalSinDampSin, &sig, sizeof( mSignalSinDampSin ) );
                    fillValuesSinDampSin();
                }
                break;

            case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                {
                    SignalItem::SignalTrapDampSin sig = mEditedSignal->getSignalDataTrapDampSin();
                    memcpy( &mSignalTrapDampSin, &sig, sizeof( mSignalTrapDampSin ) );
                    fillValuesTrapDampSin();
                }
                break;

            case SignalItem::SIGNAL_TYPE_NOISE:
                {
                    SignalItem::SignalNoise sig = mEditedSignal->getSignalDataNoise();
                    memcpy( &mSignalNoise, &sig, sizeof( mSignalNoise ) );
                    fillValuesNoise();
                }
                break;

            default:
                break;
        }

        mIsSignalEdited = true;
        updateControls();
    }
}


//!************************************************************************
//! Handle for saving the current signal from the list
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSaveSignal()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName( this,
                                                     "Save active signal",
                                                     "",
                                                     "Text files (*.txt);;All files (*)",
                                                     &selectedFilter,
                                                     QFileDialog::DontUseNativeDialog
                                                    );

    std::string outputFilename = fileName.toStdString();
    std::ofstream outputFile;
    outputFile.open( outputFilename );

    if( outputFile.is_open() )
    {
        for( size_t i = 0; i < mSignalsVector.size(); i++ )
        {
            SignalItem::SignalType sigType = mSignalsVector.at( i )->getType();
            QString lineString;

            switch( sigType )
            {
                case SignalItem::SIGNAL_TYPE_TRIANGLE:
                    lineString = createSignalStringTriangle( mSignalsVector.at( i )->getSignalDataTriangle() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_RECTANGLE:
                    lineString = createSignalStringRectangle( mSignalsVector.at( i )->getSignalDataRectangle() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_PULSE:
                    lineString = createSignalStringPulse( mSignalsVector.at( i )->getSignalDataPulse() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_RISEFALL:
                    lineString = createSignalStringRiseFall( mSignalsVector.at( i )->getSignalDataRiseFall() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMP:
                    lineString = createSignalStringSinDamp( mSignalsVector.at( i )->getSignalDataSinDamp() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_SINRISE:
                    lineString = createSignalStringSinRise( mSignalsVector.at( i )->getSignalDataSinRise() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_WAVSIN:
                    lineString = createSignalStringWavSin( mSignalsVector.at( i )->getSignalDataWavSin() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_AMSIN:
                    lineString = createSignalStringAmSin( mSignalsVector.at( i )->getSignalDataAmSin() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                    lineString = createSignalStringSinDampSin( mSignalsVector.at( i )->getSignalDataSinDampSin() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                    lineString = createSignalStringTrapDampSin( mSignalsVector.at( i )->getSignalDataTrapDampSin() );
                    lineString += "\n";
                    break;

                case SignalItem::SIGNAL_TYPE_NOISE:
                    lineString = createSignalStringNoise( mSignalsVector.at( i )->getSignalDataNoise() );
                    lineString += "\n";
                    break;

                default:
                    break;
            }

            outputFile << lineString.toStdString();
        }

        outputFile.close();
        mSignalReady = true;

        setAudioData();

        updateControls();
    }
    else if( fileName.size() )
    {
        QString msg = "Could not open file \"" + fileName +"\".";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();
    }
}


//!************************************************************************
//! Handle for removing the currently selected signal from the list
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleRemoveSignal()
{
    int crtRow = mMainUi->ActiveSignalList->currentIndex().row();
    mSignalsListModel.removeRow( crtRow );

    mSignalsVector.erase( mSignalsVector.begin() + crtRow );

    mSignalUndefined = mSignalsVector.empty();

    if( mSignalUndefined )
    {
        QString msg = "The list of signal items is now empty";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();
    }

    updateControls();
}


//!************************************************************************
//! Handle for exit event
//!
//! @returns: nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleExit()
{
    bool canExit = true;

    if( !mSignalUndefined && !mSignalReady )
    {
        QMessageBox::StandardButton choice = QMessageBox::question( this,
                                                                   "Exit",
                                                                   "The current signal was not saved.\nExit without saving?",
                                                                   QMessageBox::Yes | QMessageBox::No
                                                                  );

        canExit = ( QMessageBox::Yes == choice );
    }

    if( canExit )
    {
        QApplication::quit();
    }
}


//!************************************************************************
//! Start generating signals
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::handleGenerateStart()
{
    if( QAudio::StoppedState != mAudioOutput->state() )
    {
        mAudioOutput->stop();
    }

    if( mAudioSrc )
    {
        if( mAudioSrc->isStarted() )
        {
            mAudioSrc->stop();
        }

        mAudioSrc->start();
        mAudioOutput->start( mAudioSrc.data() );
    }

    mSignalStarted = ( QAudio::ActiveState == mAudioOutput->state() );
    mSignalPaused = false;

    if( mSignalStarted )
    {
        mAudioBufferCounter = 0;
        mAudioBufferTimer->start( TIMER_PER_MS );
    }

    updateControls();
}


//!************************************************************************
//! Pause generating signals
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::handleGeneratePauseResume()
{
    switch( mAudioOutput->state() )
    {
        case QAudio::ActiveState:
            mAudioOutput->suspend();
            mSignalPaused = true;
            break;

        case QAudio::SuspendedState:
            mAudioOutput->resume();
            mSignalPaused = false;
            break;

        default: // StoppedState, IdleState, InterruptedState
            break;
    }

    updateControls();
}


//!************************************************************************
//! Stop generating signals
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::handleGenerateStop()
{
    mAudioOutput->stop();

    if( mAudioSrc )
    {
        mAudioSrc->stop();
    }

    mSignalStarted = false;
    mSignalPaused = false;

    mAudioBufferCounter = 0;
    updateAudioBufferTimer();
    mAudioBufferTimer->stop();

    updateControls();
}


//!************************************************************************
//! Create a new signal
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalNew()
{
    if( !mSignalUndefined && mSignalStarted )
    {
        QString msg = "Please stop generating the current signal first.";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();
    }
    else
    {
        mSignalUndefined = false;
        mSignalReady = false;
        mSignalStarted = false;
        mSignalPaused = false;
        mIsSignalEdited = false;

        mSignalsListModel.removeRows( 0, mSignalsVector.size() );
        mSignalsVector.clear();

        mAudioOutput->stop();

        if( mAudioSrc )
        {
            mAudioSrc->stop();
        }

        updateControls();
    }
}


//!************************************************************************
//! Open a file with a signal definition
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalOpen()
{
    if( !mSignalUndefined && !mSignalReady )
    {
        QString msg = "Please save the current signal first.";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();
    }
    else if( !mSignalUndefined && mSignalStarted )
    {
        QString msg = "Please stop generating the current signal first.";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();
    }
    else
    {
        mSignalUndefined = true;
        mSignalReady = false;
        mSignalStarted = false;
        mSignalPaused = false;
        mIsSignalEdited = false;

        mSignalsListModel.removeRows( 0, mSignalsVector.size() );
        mSignalsVector.clear();

        mAudioOutput->stop();

        if( mAudioSrc )
        {
            mAudioSrc->stop();
        }

        QString selectedFilter;
        QString fileName = QFileDialog::getOpenFileName( this,
                                                         "Open signal file",
                                                         "",
                                                         "Text files (*.txt);;All files (*)",
                                                         &selectedFilter,
                                                         QFileDialog::DontUseNativeDialog
                                                        );

        std::string inputFilename = fileName.toStdString();
        std::ifstream inputFile( inputFilename );

        if( inputFile.is_open() )
        {
            const std::string DELIM = ", ";
            std::string currentLine;


            while( getline( inputFile, currentLine ) )
            {
                std::vector<QString> substringsVec;
                size_t pos = 0;

                while( ( pos = currentLine.find( DELIM ) ) != std::string::npos )
                {
                    substringsVec.push_back( QString::fromStdString( currentLine.substr( 0, pos ) ) );
                    currentLine.erase( 0, pos + DELIM.length() );
                }

                if( currentLine.size() )
                {
                    substringsVec.push_back( QString::fromStdString( currentLine ) );
                }

                size_t ssCount = substringsVec.size();

                if( ssCount >=2 )
                {
                    bool currentSignalOk = true;
                    size_t expectedParams = 0;

                    QString lineString;

                    double crtDbl = 0;
                    int crtInt = 0;

                    SignalItem::SignalType sigType = static_cast<SignalItem::SignalType>( substringsVec[0].toInt() );
                    SignalItem* crtSignal = nullptr;

                    switch( sigType )
                    {
                        case SignalItem::SIGNAL_TYPE_TRIANGLE:
                            {
                                expectedParams = 6;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalTriangle sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tPeriod = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tRise = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tFall = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMax = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMin = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringTriangle( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_RECTANGLE:
                            {
                                expectedParams = 5;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalRectangle sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tPeriod = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.fillFactor = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMax = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMin = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringRectangle( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_PULSE:
                            {
                                expectedParams = 7;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalPulse sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tPeriod = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tRise = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tWidth = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tFall = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMax = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[7].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMin = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringPulse( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_RISEFALL:
                            {
                                expectedParams = 7;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalRiseFall sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelayRise = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tRampRise = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelayFall = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tRampFall = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMax = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[7].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.yMin = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringRiseFall( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_SINDAMP:
                            {
                                expectedParams = 6;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalSinDamp sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.freqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.phiRad = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.damping = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringSinDamp( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_SINRISE:
                            {
                                expectedParams = 7;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalSinRise sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.freqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.phiRad = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tEnd = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[7].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.damping = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringSinRise( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_WAVSIN:
                            {
                                expectedParams = 6;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalWavSin sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.freqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.phiRad = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtInt = substringsVec[6].toInt( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.index = crtInt;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringWavSin( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_AMSIN:
                            {
                                expectedParams = 7;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalAmSin sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.carrierFreqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.carrierAmplitude = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.carrierOffset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.carrierTDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.modulationFreqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.modulationPhiRad = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[7].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.modulationIndex = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringAmSin( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_SINDAMPSIN:
                            {
                                expectedParams = 6;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalSinDampSin sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.freqSinHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tPeriodEnv = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtInt = substringsVec[6].toInt( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.dampingType = crtInt;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringSinDampSin( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_TRAPDAMPSIN:
                            {
                                expectedParams = 9;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalTrapDampSin sig;

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[1].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tPeriod = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tRise = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tWidth = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tFall = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[6].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tCross = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[7].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.freqHz = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[8].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[9].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringTrapDampSin( sig );
                                }
                            }
                            break;

                        case SignalItem::SIGNAL_TYPE_NOISE:
                            {
                                expectedParams = 5;
                                currentSignalOk = ( expectedParams == ( ssCount - 1 ) );
                                SignalItem::SignalNoise sig;

                                if( currentSignalOk )
                                {
                                    crtInt = substringsVec[1].toInt( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.noiseType = static_cast<SignalItem::NoiseType>( crtInt );
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[2].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.gamma = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[3].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.tDelay = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[4].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.amplit = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtDbl = substringsVec[5].toDouble( &currentSignalOk );

                                    if( currentSignalOk )
                                    {
                                        sig.offset = crtDbl;
                                    }
                                }

                                if( currentSignalOk )
                                {
                                    crtSignal = new SignalItem( sig );
                                    lineString = createSignalStringNoise( sig );
                                }
                            }
                            break;

                        default:
                            break;
                    }

                    if( crtSignal && currentSignalOk )
                    {
                        mSignalsVector.push_back( crtSignal );

                        int row = mSignalsListModel.rowCount();
                        mSignalsListModel.insertRow( row );
                        QModelIndex index = mSignalsListModel.index( row );
                        mSignalsListModel.setData( index, lineString );
                    }
                }
            }

            inputFile.close();

            if( mSignalsVector.size() )
            {
                mSignalUndefined = false;
                mSignalReady = true;

                setAudioData();
            }
            else
            {
                QString msg = "The selected file does not contain any valid signal.";
                QMessageBox msgBox;
                msgBox.setText( msg );
                msgBox.exec();
            }

            updateControls();
        }
        else if( fileName.size() )
        {
            QString msg = "Could not open file \"" + fileName +"\".";
            QMessageBox msgBox;
            msgBox.setText( msg );
            msgBox.exec();
        }
    }
}


//!************************************************************************
//! Handle for changing parameters for Triangle
//! *** TPeriod ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTriangleTPeriod()
{
    bool ok = false;
    double newVal = mMainUi->TriangleTPerEdit->text().toDouble( &ok );

    if( ok
     && newVal >= T_MIN_S
      )
    {
        mSignalTriangle.tPeriod = newVal;

        mSignalTriangle.tRise = 0.5 * mSignalTriangle.tPeriod;
        mSignalTriangle.tFall = mSignalTriangle.tRise;

        mMainUi->TriangleTRiseEdit->setText( QString::number( mSignalTriangle.tRise ) );
        mMainUi->TriangleTFallEdit->setText( QString::number( mSignalTriangle.tFall ) );
    }
    else
    {
        QString msg = "T must be >=" + QString::number( T_MIN_S );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TriangleTPerEdit->setText( QString::number( mSignalTriangle.tPeriod ) );
        mMainUi->TriangleTPerEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Triangle
//!  *** TRise ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTriangleTRise()
{
    bool ok = false;
    double newVal = mMainUi->TriangleTRiseEdit->text().toDouble( &ok );

    if( ok
     && newVal < mSignalTriangle.tPeriod
     && newVal > 0
      )
    {
        mSignalTriangle.tRise = newVal;

        mSignalTriangle.tFall = mSignalTriangle.tPeriod - mSignalTriangle.tRise;
        mMainUi->TriangleTFallEdit->setText( QString::number( mSignalTriangle.tFall ) );
    }
    else
    {
        QString msg = "t_rise must be >0 and <" + QString::number( mSignalTriangle.tPeriod );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TriangleTRiseEdit->setText( QString::number( mSignalTriangle.tRise ) );
        mMainUi->TriangleTRiseEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Triangle
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTriangleTDelay()
{
    bool ok = false;
    double newVal = mMainUi->TriangleTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalTriangle.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TriangleTDelayEdit->setText( QString::number( mSignalTriangle.tDelay ) );
        mMainUi->TriangleTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Triangle
//! *** YMax ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTriangleYMax()
{
    bool ok = false;
    double newVal = mMainUi->TriangleYMaxEdit->text().toDouble( &ok );

    if( ok
     && newVal <= 1
     && newVal > mSignalTriangle.yMin
      )
    {
        mSignalTriangle.yMax = newVal;
    }
    else
    {
        QString msg = "max must be <=1 and >" + QString::number( mSignalTriangle.yMin );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TriangleYMaxEdit->setText( QString::number( mSignalTriangle.yMax ) );
        mMainUi->TriangleYMaxEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Triangle
//! *** YMin ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTriangleYMin()
{
    bool ok = false;
    double newVal = mMainUi->TriangleYMinEdit->text().toDouble( &ok );

    if( ok
     && newVal >= -1
     && newVal < mSignalTriangle.yMax
      )
    {
        mSignalTriangle.yMin = newVal;
    }
    else
    {
        QString msg = "min must be >=-1 and <" + QString::number( mSignalTriangle.yMax );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TriangleYMinEdit->setText( QString::number( mSignalTriangle.yMin ) );
        mMainUi->TriangleYMinEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for Rectangle
//! *** TPeriod ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRectangleTPeriod()
{
    bool ok = false;
    double newVal = mMainUi->RectangleTPerEdit->text().toDouble( &ok );

    if( ok
     && newVal >= T_MIN_S
      )
    {
        mSignalRectangle.tPeriod = newVal;
    }
    else
    {
        QString msg = "T must be >=" + QString::number( T_MIN_S );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RectangleTPerEdit->setText( QString::number( mSignalRectangle.tPeriod ) );
        mMainUi->RectangleTPerEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Rectangle
//! *** FillFactor ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRectangleFillFactor()
{
    bool ok = false;
    double newVal = mMainUi->RectangleFillFactorEdit->text().toDouble( &ok );

    if( ok
     && newVal <= 1
     && newVal >= 0
      )
    {
        mSignalRectangle.fillFactor = newVal;
    }
    else
    {
        QString msg = "fill factor must be <=1 and >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RectangleFillFactorEdit->setText( QString::number( mSignalRectangle.fillFactor ) );
        mMainUi->RectangleFillFactorEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Rectangle
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRectangleTDelay()
{
    bool ok = false;
    double newVal = mMainUi->RectangleTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalRectangle.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RectangleTDelayEdit->setText( QString::number( mSignalRectangle.tDelay ) );
        mMainUi->RectangleTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Rectangle
//! *** YMax ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRectangleYMax()
{
    bool ok = false;
    double newVal = mMainUi->RectangleYMaxEdit->text().toDouble( &ok );

    if( ok
     && newVal <= 1
     && newVal > mSignalRectangle.yMin
      )
    {
        mSignalRectangle.yMax = newVal;
    }
    else
    {
        QString msg = "max must be <=1 and >" + QString::number( mSignalRectangle.yMin );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RectangleYMaxEdit->setText( QString::number( mSignalRectangle.yMax ) );
        mMainUi->RectangleYMaxEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Rectangle
//! *** YMin ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRectangleYMin()
{
    bool ok = false;
    double newVal = mMainUi->RectangleYMinEdit->text().toDouble( &ok );

    if( ok
     && newVal >= -1
     && newVal < mSignalRectangle.yMax
      )
    {
        mSignalRectangle.yMin = newVal;
    }
    else
    {
        QString msg = "min must be >=-1 and <" + QString::number( mSignalRectangle.yMax );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RectangleYMinEdit->setText( QString::number( mSignalRectangle.yMin ) );
        mMainUi->RectangleYMinEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** TPeriod ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseTPeriod()
{
    bool ok = false;
    double newVal = mMainUi->PulseTPerEdit->text().toDouble( &ok );

    if( ok
     && newVal >= T_MIN_S
      )
    {
        mSignalPulse.tPeriod = newVal;

        mSignalPulse.tRise = 0.125 * mSignalPulse.tPeriod;
        mSignalPulse.tFall = 0.125 * mSignalPulse.tPeriod;
        mSignalPulse.tWidth = 0.25 * mSignalPulse.tPeriod;

        mMainUi->PulseTRiseEdit->setText( QString::number( mSignalPulse.tRise ) );
        mMainUi->PulseTFallEdit->setText( QString::number( mSignalPulse.tFall ) );
        mMainUi->PulseTWidthEdit->setText( QString::number( mSignalPulse.tWidth ) );
    }
    else
    {
        QString msg = "T must be >=" + QString::number( T_MIN_S );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseTPerEdit->setText( QString::number( mSignalPulse.tPeriod ) );
        mMainUi->PulseTPerEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** TRise ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseTRise()
{
    bool ok = false;
    double newVal = mMainUi->PulseTRiseEdit->text().toDouble( &ok );
    double maxVal = mSignalPulse.tPeriod - mSignalPulse.tFall - mSignalPulse.tWidth;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalPulse.tRise = newVal;
    }
    else
    {
        QString msg = "t_rise must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseTRiseEdit->setText( QString::number( mSignalPulse.tRise ) );
        mMainUi->PulseTRiseEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** TWidth ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseTWidth()
{
    bool ok = false;
    double newVal = mMainUi->PulseTWidthEdit->text().toDouble( &ok );
    double maxVal = mSignalPulse.tPeriod - mSignalPulse.tRise - mSignalPulse.tFall;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalPulse.tWidth = newVal;
    }
    else
    {
        QString msg = "t_width must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseTWidthEdit->setText( QString::number( mSignalPulse.tWidth ) );
        mMainUi->PulseTWidthEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** TFall ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseTFall()
{
    bool ok = false;
    double newVal = mMainUi->PulseTFallEdit->text().toDouble( &ok );
    double maxVal = mSignalPulse.tPeriod - mSignalPulse.tRise - mSignalPulse.tWidth;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalPulse.tFall = newVal;
    }
    else
    {
        QString msg = "t_fall must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseTFallEdit->setText( QString::number( mSignalPulse.tFall ) );
        mMainUi->PulseTFallEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseTDelay()
{
    bool ok = false;
    double newVal = mMainUi->PulseTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalPulse.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseTDelayEdit->setText( QString::number( mSignalPulse.tDelay ) );
        mMainUi->PulseTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** YMax ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseYMax()
{
    bool ok = false;
    double newVal = mMainUi->PulseYMaxEdit->text().toDouble( &ok );

    if( ok
     && newVal <= 1
     && newVal > mSignalPulse.yMin
      )
    {
        mSignalPulse.yMax = newVal;
    }
    else
    {
        QString msg = "max must be <=1 and >" + QString::number( mSignalPulse.yMin );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseYMaxEdit->setText( QString::number( mSignalPulse.yMax ) );
        mMainUi->PulseYMaxEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Pulse
//! *** YMin ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedPulseYMin()
{
    bool ok = false;
    double newVal = mMainUi->PulseYMinEdit->text().toDouble( &ok );

    if( ok
     && newVal >= -1
     && newVal < mSignalPulse.yMax
      )
    {
        mSignalPulse.yMin = newVal;
    }
    else
    {
        QString msg = "min must be >=-1 and <" + QString::number( mSignalPulse.yMax );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->PulseYMinEdit->setText( QString::number( mSignalPulse.yMin ) );
        mMainUi->PulseYMinEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallTDelay()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalRiseFall.tDelay = newVal;

        if( mSignalRiseFall.tDelayRise < mSignalRiseFall.tDelay )
        {
            mSignalRiseFall.tDelayRise = mSignalRiseFall.tDelay;
            mMainUi->RiseFallTDelayRiseEdit->setText( QString::number( mSignalRiseFall.tDelayRise ) );
        }

        if( mSignalRiseFall.tDelayFall <= mSignalRiseFall.tDelayRise )
        {
            mSignalRiseFall.tDelayFall = 1 + mSignalRiseFall.tDelayRise;
            mMainUi->RiseFallTDelayFallEdit->setText( QString::number( mSignalRiseFall.tDelayFall ) );
        }
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallTDelayEdit->setText( QString::number( mSignalRiseFall.tDelay ) );
        mMainUi->RiseFallTDelayEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** TDelayRise ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallTDelayRise()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallTDelayRiseEdit->text().toDouble( &ok );

    if( ok
     && newVal >= mSignalRiseFall.tDelay
      )
    {
        mSignalRiseFall.tDelayRise = newVal;

        if( mSignalRiseFall.tDelayFall <= mSignalRiseFall.tDelayRise )
        {
            mSignalRiseFall.tDelayFall = 1 + mSignalRiseFall.tDelayRise;
            mMainUi->RiseFallTDelayFallEdit->setText( QString::number( mSignalRiseFall.tDelayFall ) );
        }
    }
    else
    {
        QString msg = "t_delay_rise must be >=" + QString::number( mSignalRiseFall.tDelay );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallTDelayRiseEdit->setText( QString::number( mSignalRiseFall.tDelayRise ) );
        mMainUi->RiseFallTDelayRiseEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** TRampRise ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallTRampRise()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallTRampRiseEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
      )
    {
        mSignalRiseFall.tRampRise = newVal;
    }
    else
    {
        QString msg = "t_ramp_rise must be >0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallTRampRiseEdit->setText( QString::number( mSignalRiseFall.tRampRise ) );
        mMainUi->RiseFallTRampRiseEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** TDelayFall ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallTDelayFall()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallTDelayFallEdit->text().toDouble( &ok );

    if( ok
     && newVal > mSignalRiseFall.tDelayRise
      )
    {
        mSignalRiseFall.tDelayFall = newVal;
    }
    else
    {
        QString msg = "t_delay_fall must be >" + QString::number( mSignalRiseFall.tDelayRise );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallTDelayFallEdit->setText( QString::number( mSignalRiseFall.tDelayFall ) );
        mMainUi->RiseFallTDelayFallEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** TRampFall ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallTRampFall()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallTRampFallEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
      )
    {
        mSignalRiseFall.tRampFall = newVal;
    }
    else
    {
        QString msg = "t_ramp_fall must be >0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallTRampFallEdit->setText( QString::number( mSignalRiseFall.tRampFall ) );
        mMainUi->RiseFallTRampFallEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** YMax ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallYMax()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallYMaxEdit->text().toDouble( &ok );

    if( ok
     && newVal <= 1
     && newVal > mSignalRiseFall.yMin
      )
    {
        mSignalRiseFall.yMax = newVal;
    }
    else
    {
        QString msg = "max must be <=1 and >" + QString::number( mSignalRiseFall.yMin );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallYMaxEdit->setText( QString::number( mSignalRiseFall.yMax ) );
        mMainUi->RiseFallYMaxEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for RiseFall
//! *** YMin ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedRiseFallYMin()
{
    bool ok = false;
    double newVal = mMainUi->RiseFallYMinEdit->text().toDouble( &ok );

    if( ok
     && newVal >= -1
     && newVal < mSignalRiseFall.yMax
      )
    {
        mSignalRiseFall.yMin = newVal;
    }
    else
    {
        QString msg = "min must be >=-1 and <" + QString::number( mSignalRiseFall.yMax );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->RiseFallYMinEdit->setText( QString::number( mSignalRiseFall.yMin ) );
        mMainUi->RiseFallYMinEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampFreq()
{
    bool ok = false;
    double newVal = mMainUi->SinDampFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalSinDamp.freqHz = newVal;
    }
    else
    {
        QString msg = "f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampFreqEdit->setText( QString::number( mSignalSinDamp.freqHz ) );
        mMainUi->SinDampFreqEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** Phi ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampPhi()
{
    bool ok = false;
    double newValDeg = mMainUi->SinDampPhiEdit->text().toDouble( &ok );

    if( ok
     && newValDeg >= 0
     && newValDeg < 360
      )
    {
        mSignalSinDamp.phiRad = newValDeg * M_PI / 180.0;
    }
    else
    {
        QString msg = PHI_SMALL + " must be >=0 and <360";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampPhiEdit->setText( QString::number( mSignalSinDamp.phiRad * 180 / M_PI ) );
        mMainUi->SinDampPhiEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampTDelay()
{
    bool ok = false;
    double newVal = mMainUi->SinDampTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalSinDamp.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampTDelayEdit->setText( QString::number( mSignalSinDamp.tDelay ) );
        mMainUi->SinDampTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->SinDampAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalSinDamp.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampAmplitEdit->setText( QString::number( mSignalSinDamp.amplit ) );
        mMainUi->SinDampAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampOffset()
{
    bool ok = false;
    double newVal = mMainUi->SinDampOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalSinDamp.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampOffsetEdit->setText( QString::number( mSignalSinDamp.offset ) );
        mMainUi->SinDampOffsetEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDamp
//! *** Damping ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampDamping()
{
    bool ok = false;
    double newVal = mMainUi->SinDampDampingEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalSinDamp.damping = newVal;
    }
    else
    {
        QString msg = "damping must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampDampingEdit->setText( QString::number( mSignalSinDamp.damping ) );
        mMainUi->SinDampDampingEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseFreq()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalSinRise.freqHz = newVal;
    }
    else
    {
        QString msg = "f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseFreqEdit->setText( QString::number( mSignalSinRise.freqHz ) );
        mMainUi->SinRiseFreqEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** Phi ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRisePhi()
{
    bool ok = false;
    double newValDeg = mMainUi->SinRisePhiEdit->text().toDouble( &ok );

    if( ok
     && newValDeg >= 0
     && newValDeg < 360
      )
    {
        mSignalSinRise.phiRad = newValDeg * M_PI / 180.0;
    }
    else
    {
        QString msg = PHI_SMALL + " must be >=0 and <360";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRisePhiEdit->setText( QString::number( mSignalSinRise.phiRad * 180 / M_PI ) );
        mMainUi->SinRisePhiEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** TEnd ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseTEnd()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseTEndEdit->text().toDouble( &ok );

    if( ok
     && newVal > mSignalSinRise.tDelay
      )
    {
        mSignalSinRise.tEnd = newVal;
    }
    else
    {
        QString msg = "t_end must be >" + QString::number( mSignalSinRise.tDelay );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseTEndEdit->setText( QString::number( mSignalSinRise.tEnd ) );
        mMainUi->SinRiseTEndEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseTDelay()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
     && newVal < mSignalSinRise.tEnd
      )
    {
        mSignalSinRise.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0 and <" + QString::number( mSignalSinRise.tEnd );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseTDelayEdit->setText( QString::number( mSignalSinRise.tDelay ) );
        mMainUi->SinRiseTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalSinRise.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseAmplitEdit->setText( QString::number( mSignalSinRise.amplit ) );
        mMainUi->SinRiseAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseOffset()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalSinRise.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseOffsetEdit->setText( QString::number( mSignalSinRise.offset ) );
        mMainUi->SinRiseOffsetEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinRise
//! *** Damping ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinRiseDamping()
{
    bool ok = false;
    double newVal = mMainUi->SinRiseDampingEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalSinRise.damping = newVal;
    }
    else
    {
        QString msg = "damping must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinRiseDampingEdit->setText( QString::number( mSignalSinRise.damping ) );
        mMainUi->SinRiseDampingEdit->setFocus();
    }
}



//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinFreq()
{
    bool ok = false;
    double newVal = mMainUi->WavSinFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalWavSin.freqHz = newVal;
    }
    else
    {
        QString msg = "f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinFreqEdit->setText( QString::number( mSignalWavSin.freqHz ) );
        mMainUi->WavSinFreqEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** Phi ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinPhi()
{
    bool ok = false;
    double newValDeg = mMainUi->WavSinPhiEdit->text().toDouble( &ok );

    if( ok
     && newValDeg >= 0
     && newValDeg < 360
      )
    {
        mSignalWavSin.phiRad = newValDeg * M_PI / 180.0;
    }
    else
    {
        QString msg = PHI_SMALL + " must be >=0 and <360";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinPhiEdit->setText( QString::number( mSignalWavSin.phiRad * 180 / M_PI ) );
        mMainUi->WavSinPhiEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinTDelay()
{
    bool ok = false;
    double newVal = mMainUi->WavSinTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalWavSin.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinTDelayEdit->setText( QString::number( mSignalWavSin.tDelay ) );
        mMainUi->WavSinTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->WavSinAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalWavSin.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinAmplitEdit->setText( QString::number( mSignalWavSin.amplit ) );
        mMainUi->WavSinAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinOffset()
{
    bool ok = false;
    double newVal = mMainUi->WavSinOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalWavSin.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinOffsetEdit->setText( QString::number( mSignalWavSin.offset ) );
        mMainUi->WavSinOffsetEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for WavSin
//! *** NOrder ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedWavSinNOrder
    (
    int aIndex      //!< index
    )
{
    if( aIndex >= 3
     && 1 == aIndex % 2
      )
    {
        mSignalWavSin.index = aIndex;
    }
    else
    {
        QString msg = "N must be >=3 and odd";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->WavSinNOrderSpin->setValue( mSignalWavSin.index );
        mMainUi->WavSinNOrderSpin->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Carrier Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinCarrierFreq()
{
    bool ok = false;
    double newVal = mMainUi->AmSinCarrierFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalAmSin.carrierFreqHz = newVal;
    }
    else
    {
        QString msg = "carrier f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinCarrierFreqEdit->setText( QString::number( mSignalAmSin.carrierFreqHz ) );
        mMainUi->AmSinCarrierFreqEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Carrier Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinCarrierAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->AmSinCarrierAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalAmSin.carrierAmplitude = newVal;
    }
    else
    {
        QString msg = "carrier amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinCarrierAmplitEdit->setText( QString::number( mSignalAmSin.carrierAmplitude ) );
        mMainUi->AmSinCarrierAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Carrier Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinCarrierOffset()
{
    bool ok = false;
    double newVal = mMainUi->AmSinCarrierOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalAmSin.carrierOffset = newVal;
    }
    else
    {
        QString msg = "carrier offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinCarrierOffsetEdit->setText( QString::number( mSignalAmSin.carrierOffset ) );
        mMainUi->AmSinCarrierOffsetEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Carrier TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinCarrierTDelay()
{
    bool ok = false;
    double newVal = mMainUi->AmSinCarrierTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalAmSin.carrierTDelay = newVal;
    }
    else
    {
        QString msg = "carrier t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinCarrierTDelayEdit->setText( QString::number( mSignalAmSin.carrierTDelay ) );
        mMainUi->AmSinCarrierTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Modulation Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinModulationFreq()
{
    bool ok = false;
    double newVal = mMainUi->AmSinModFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalAmSin.modulationFreqHz = newVal;
    }
    else
    {
        QString msg = "modulation f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinModFreqEdit->setText( QString::number( mSignalAmSin.modulationFreqHz ) );
        mMainUi->AmSinModFreqEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Modulation Phi ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinModulationPhi()
{
    bool ok = false;
    double newValDeg = mMainUi->AmSinModPhiEdit->text().toDouble( &ok );

    if( ok
     && newValDeg >= 0
     && newValDeg < 360
      )
    {
        mSignalAmSin.modulationPhiRad = newValDeg * M_PI / 180.0;
    }
    else
    {
        QString msg = "modulation " + PHI_SMALL + " must be >=0 and <360";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinModPhiEdit->setText( QString::number( mSignalAmSin.modulationPhiRad * 180 / M_PI ) );
        mMainUi->AmSinModPhiEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for AmSin
//! *** Modulation Index ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedAmSinModulationIndex()
{
    bool ok = false;
    double newVal = mMainUi->AmSinModModEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalAmSin.modulationIndex = newVal;
    }
    else
    {
        QString msg = "modulation index must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->AmSinModModEdit->setText( QString::number( mSignalAmSin.modulationIndex ) );
        mMainUi->AmSinModModEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinFreq()
{
    bool ok = false;
    double newVal = mMainUi->SinDampSinFreqSinEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalSinDampSin.freqSinHz = newVal;
    }
    else
    {
        QString msg = "f_sin must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinFreqSinEdit->setText( QString::number( mSignalSinDampSin.freqSinHz ) );
        mMainUi->SinDampSinFreqSinEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** TPeriodEnv ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinTPeriodEnv()
{
    bool ok = false;
    double newVal = mMainUi->SinDampSinTEnvEdit->text().toDouble( &ok );

    if( ok
     && newVal >= T_MIN_S
      )
    {
        mSignalSinDampSin.tPeriodEnv = newVal;
    }
    else
    {
        QString msg = "t_env must be >=" + QString::number( T_MIN_S );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinTEnvEdit->setText( QString::number( mSignalSinDampSin.tPeriodEnv ) );
        mMainUi->SinDampSinTEnvEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinTDelay()
{
    bool ok = false;
    double newVal = mMainUi->SinDampSinTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalSinDampSin.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinTDelayEdit->setText( QString::number( mSignalSinDampSin.tDelay ) );
        mMainUi->SinDampSinTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->SinDampSinAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalSinDampSin.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinAmplitEdit->setText( QString::number( mSignalSinDampSin.amplit ) );
        mMainUi->SinDampSinAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinOffset()
{
    bool ok = false;
    double newVal = mMainUi->SinDampSinOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalSinDampSin.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinOffsetEdit->setText( QString::number( mSignalSinDampSin.offset ) );
        mMainUi->SinDampSinOffsetEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for SinDampSin
//! *** DampingType ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedSinDampSinDampingType
    (
    int aIndex      //!< index
    )
{
    if( aIndex >= -3
     && aIndex <= 3
      )
    {
        mSignalSinDampSin.dampingType = aIndex;
    }
    else
    {
        QString msg = "N must be >=-3 and <=3";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->SinDampSinDampingTypeSpin->setValue( mSignalSinDampSin.dampingType );
        mMainUi->SinDampSinDampingTypeSpin->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TPeriod ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTPeriod()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTPerEdit->text().toDouble( &ok );

    if( ok
     && newVal >= T_MIN_S
      )
    {
        mSignalTrapDampSin.tPeriod = newVal;

        mSignalTrapDampSin.tRise = 0.125 * mSignalTrapDampSin.tPeriod;
        mSignalTrapDampSin.tFall = 0.125 * mSignalTrapDampSin.tPeriod;
        mSignalTrapDampSin.tWidth = 0.25 * mSignalTrapDampSin.tPeriod;

        mMainUi->TrapDampSinTRiseEdit->setText( QString::number( mSignalTrapDampSin.tRise ) );
        mMainUi->TrapDampSinTFallEdit->setText( QString::number( mSignalTrapDampSin.tFall ) );
        mMainUi->TrapDampSinTWidthEdit->setText( QString::number( mSignalTrapDampSin.tWidth ) );
    }
    else
    {
        QString msg = "T must be >=" + QString::number( T_MIN_S );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTPerEdit->setText( QString::number( mSignalTrapDampSin.tPeriod ) );
        mMainUi->TrapDampSinTPerEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TRise ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTRise()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTRiseEdit->text().toDouble( &ok );
    double maxVal = mSignalTrapDampSin.tPeriod - mSignalTrapDampSin.tFall - mSignalTrapDampSin.tWidth;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalTrapDampSin.tRise = newVal;
    }
    else
    {
        QString msg = "t_rise must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTRiseEdit->setText( QString::number( mSignalTrapDampSin.tRise ) );
        mMainUi->TrapDampSinTRiseEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TWidth ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTWidth()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTWidthEdit->text().toDouble( &ok );
    double maxVal = mSignalTrapDampSin.tPeriod - mSignalTrapDampSin.tRise - mSignalTrapDampSin.tFall;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalTrapDampSin.tWidth = newVal;
    }
    else
    {
        QString msg = "t_width must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTWidthEdit->setText( QString::number( mSignalTrapDampSin.tWidth ) );
        mMainUi->TrapDampSinTWidthEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TFall ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTFall()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTFallEdit->text().toDouble( &ok );
    double maxVal = mSignalTrapDampSin.tPeriod - mSignalTrapDampSin.tRise - mSignalTrapDampSin.tWidth;

    if( ok
     && newVal > 0
     && newVal < maxVal
      )
    {
        mSignalTrapDampSin.tFall = newVal;
    }
    else
    {
        QString msg = "t_fall must be >0 and <" + QString::number( maxVal );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTFallEdit->setText( QString::number( mSignalTrapDampSin.tFall ) );
        mMainUi->TrapDampSinTFallEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTDelay()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalTrapDampSin.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTDelayEdit->setText( QString::number( mSignalTrapDampSin.tDelay ) );
        mMainUi->TrapDampSinTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** TCross ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinTCross()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinTCrossEdit->text().toDouble( &ok );

    if( ok
     && newVal > mSignalTrapDampSin.tDelay
      )
    {
        mSignalTrapDampSin.tCross = newVal;
    }
    else
    {
        QString msg = "t_cross must be >" + QString::number( mSignalTrapDampSin.tDelay );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinTCrossEdit->setText( QString::number( mSignalTrapDampSin.tCross ) );
        mMainUi->TrapDampSinTCrossEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** Freq ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinFreq()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinFreqEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= FREQ_MAX_HZ
      )
    {
        mSignalTrapDampSin.freqHz = newVal;
    }
    else
    {
        QString msg = "f must be >0 and <=" + QString::number( FREQ_MAX_HZ );
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinFreqEdit->setText( QString::number( mSignalTrapDampSin.freqHz ) );
        mMainUi->TrapDampSinFreqEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalTrapDampSin.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinAmplitEdit->setText( QString::number( mSignalTrapDampSin.amplit ) );
        mMainUi->TrapDampSinAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for TrapDampSin
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedTrapDampSinOffset()
{
    bool ok = false;
    double newVal = mMainUi->TrapDampSinOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalTrapDampSin.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->TrapDampSinOffsetEdit->setText( QString::number( mSignalTrapDampSin.offset ) );
        mMainUi->TrapDampSinOffsetEdit->setFocus();
    }
}


//!************************************************************************
//! Handle for changing parameters for Noise
//! *** Type ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedNoiseType
    (
    int aIndex      //!< index
    )
{
    mSignalNoise.noiseType = static_cast<SignalItem::NoiseType>( aIndex );
}

//!************************************************************************
//! Handle for changing parameters for Noise
//! *** Gamma ***
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::handleSignalChangedNoiseGamma
    (
    double  aValue      //!< value
    )
{
    if( NoisePwrSpectrum::GAMMA_MIN <= aValue
     && aValue <= NoisePwrSpectrum::GAMMA_MAX
      )
    {
        mSignalNoise.gamma = aValue;
    }
}

//!************************************************************************
//! Handle for changing parameters for Noise
//! *** TDelay ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedNoiseTDelay()
{
    bool ok = false;
    double newVal = mMainUi->NoiseTDelayEdit->text().toDouble( &ok );

    if( ok
     && newVal >= 0
      )
    {
        mSignalNoise.tDelay = newVal;
    }
    else
    {
        QString msg = "t_delay must be >=0";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->NoiseTDelayEdit->setText( QString::number( mSignalNoise.tDelay ) );
        mMainUi->NoiseTDelayEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Noise
//! *** Amplitude ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedNoiseAmplitude()
{
    bool ok = false;
    double newVal = mMainUi->NoiseAmplitEdit->text().toDouble( &ok );

    if( ok
     && newVal > 0
     && newVal <= 1
      )
    {
        mSignalNoise.amplit = newVal;
    }
    else
    {
        QString msg = "amplitude must be >0 and <=1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->NoiseAmplitEdit->setText( QString::number( mSignalNoise.amplit ) );
        mMainUi->NoiseAmplitEdit->setFocus();
    }
}

//!************************************************************************
//! Handle for changing parameters for Noise
//! *** Offset ***
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalChangedNoiseOffset()
{
    bool ok = false;
    double newVal = mMainUi->NoiseOffsetEdit->text().toDouble( &ok );

    if( ok
     && newVal > -1
     && newVal < 1
      )
    {
        mSignalNoise.offset = newVal;
    }
    else
    {
        QString msg = "offset must be >-1 and <1";
        QMessageBox msgBox;
        msgBox.setText( msg );
        msgBox.exec();

        mMainUi->NoiseOffsetEdit->setText( QString::number( mSignalNoise.offset ) );
        mMainUi->NoiseOffsetEdit->setFocus();
    }
}


//!************************************************************************
//! Update items required when changing the signal type
//!
//! @returns: nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSignalTypeChanged()
{
    QString tabName;
    QWidget* tabWidget = mMainUi->SignalTypesTab->currentWidget();

    if( tabWidget )
    {
        tabName = tabWidget->objectName();
    }

    for( auto const& crtTabSignalMap : mTabSignalsMap )
    {
        if( tabName == crtTabSignalMap.second.c_str() )
        {
            mCurrentSignalType = crtTabSignalMap.first;
        }
    }

    if( mIsSignalEdited )
    {
        mEditedSignal = nullptr;
        mIsSignalEdited = false;
        updateControls();
    }
}


//!************************************************************************
//! Updates required when changing the audio volume
//!
//! @returns: nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleVolumeChanged
    (
    int     aValue      //!< index
    )
{
    qreal linearVolume = QAudio::convertVolume( aValue / qreal( 100 ),
                                                QAudio::LogarithmicVolumeScale,
                                                QAudio::LinearVolumeScale );

    mAudioOutput->setVolume( linearVolume );
    mMainUi->GenerateVolumeLabel->setText( QString::number( aValue ) + "%" );
}


//!************************************************************************
//! Initialize the audio device
//!
//! @returns: true if audio device can be initialized with required parameters
//!************************************************************************
bool SignalGenerator::initializeAudio
    (
    const QAudioDeviceInfo&     aDeviceInfo     //!< audio device
    )
{
    bool status = false;
    QAudioFormat format;
    format.setSampleRate( 44100 );
    format.setChannelCount( 1 );
    format.setSampleSize( 16 );
    format.setCodec( "audio/pcm" );
    format.setByteOrder( QAudioFormat::LittleEndian );
    format.setSampleType( QAudioFormat::SignedInt );

    status = aDeviceInfo.isFormatSupported( format );

    mAudioSrc.reset( new AudioSource( format, mAudioBufferLength ) );
    mAudioOutput.reset( new QAudioOutput( aDeviceInfo, format ) );

    qreal initialVolume = QAudio::convertVolume( mAudioOutput->volume(),
                                                 QAudio::LinearVolumeScale,
                                                 QAudio::LogarithmicVolumeScale );

    int roundedIntVol = qRound( initialVolume * 100 );
    mMainUi->GenerateVolumeSlider->setValue( roundedIntVol );
    mMainUi->GenerateVolumeLabel->setText( QString::number( roundedIntVol ) + "%" );

    return status;
}


//!************************************************************************
//! Set the audio data
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::setAudioData()
{
    if( mAudioSrc )
    {
        mAudioSrc->setData( mSignalsVector );
    }
}


//!************************************************************************
//! Update on audio buffer timer timeout
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::updateAudioBufferTimer()
{
    if( mSignalStarted && !mSignalPaused )
    {
        mAudioBufferCounter++;
    }

    int fill = 100 * ( mAudioBufferCounter % mAudioBufferLength );

    if( 1 != mAudioBufferLength )
    {
        fill /= ( mAudioBufferLength - 1 );
    }

    mMainUi->BufferProgressBar->setValue( fill );
}


//!************************************************************************
//! Update UI controls depending on current status
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::updateControls()
{
    /////////////////////////////
    // SignalItemGroupBox
    /////////////////////////////
    mMainUi->SignalTypesTab->setEnabled( !mSignalUndefined && !mSignalStarted );

    mMainUi->SignalItemActionButton->setEnabled( !mSignalUndefined && !mSignalStarted );
    mMainUi->SignalItemActionButton->setText( mIsSignalEdited ? "Replace current signal item" : "Add to active signal" );

    /////////////////////////////
    // ActiveSignalGroupBox
    /////////////////////////////
    mMainUi->ActiveSignalGroupBox->setEnabled( !mSignalUndefined && !mSignalStarted );

    bool activeSignalBtnCondition = !mSignalUndefined && mSignalsVector.size() && !mIsSignalEdited;
    mMainUi->ActiveSignalEditButton->setEnabled( activeSignalBtnCondition );
    mMainUi->ActiveSignalSaveButton->setEnabled( activeSignalBtnCondition );
    mMainUi->ActiveSignalRemoveButton->setEnabled( activeSignalBtnCondition );

    mMainUi->ActiveSignalList->setEnabled( !mIsSignalEdited );

    /////////////////////////////
    // GenerateGroupBox
    /////////////////////////////
    mMainUi->GenerateGroupBox->setEnabled( mSignalReady && !mIsSignalEdited );

    mMainUi->GeneratePauseButton->setText( mSignalPaused ? "Continue" : "Pause" );

    mMainUi->GenerateDeviceComboBox->setEnabled( !mSignalStarted && !mSignalPaused );
    mMainUi->BufferLengthSpin->setEnabled( !mSignalStarted && !mSignalPaused );

    mMainUi->GenerateStartButton->setEnabled( mSignalReady && !mSignalStarted && !mSignalPaused );
    mMainUi->GeneratePauseButton->setEnabled( mSignalReady && mSignalStarted );
    mMainUi->GenerateStopButton->setEnabled( mSignalReady && mSignalStarted );
}
