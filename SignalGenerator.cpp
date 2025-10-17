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
SignalGenerator.cpp
This file contains the sources for the signal generator.
*/

#include "SignalGenerator.h"
#include "./ui_SignalGenerator.h"

#include <QFileDialog>
#include <QTabBar>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>

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
    , mSignalIsSmc( false )
    , mEditedSignal( nullptr )
    , mIsSignalEdited( false )
    , mDevices( new QMediaDevices( this ) )
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
    createTabSignalsMap();
    connect( mMainUi->SignalTypesTab, &QTabWidget::currentChanged, this, &SignalGenerator::handleSignalTypeChanged );

    QTabBar* tabBar = mMainUi->SignalTypesTab->tabBar();
    tabBar->setStyleSheet( "QTabBar::tab::selected { background-color: rgb(250, 250, 150) }" );

    mCurrentSignalType = SignalItem::SIGNAL_TYPE_TRIANGLE;
    int crtTab = mCurrentSignalType - SignalItem::SIGNAL_TYPE_FIRST;
    mMainUi->SignalTypesTab->setCurrentIndex( crtTab );
    handleSignalTypeChanged();


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

    // SMC
    // nothing to do

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
    const QAudioDevice& defaultDeviceInfo = mDevices->defaultAudioOutput();
    mMainUi->GenerateDeviceComboBox->addItem( defaultDeviceInfo.description(), QVariant::fromValue( defaultDeviceInfo ) );

    for( auto &deviceInfo: mDevices->audioOutputs() )
    {
        if( deviceInfo != defaultDeviceInfo )
        {
             mMainUi->GenerateDeviceComboBox->addItem( deviceInfo.description(), QVariant::fromValue( deviceInfo ) );
        }
    }

    connect( mMainUi->GenerateDeviceComboBox, &QComboBox::currentIndexChanged, this, &SignalGenerator::handleDeviceChanged );
    connect( mDevices, &QMediaDevices::audioOutputsChanged, this, &SignalGenerator::updateAudioDevices );

    mMainUi->BufferLengthSpin->setRange( 2, 300 );
    mMainUi->BufferLengthSpin->setValue( mAudioBufferLength );
    connect( mMainUi->BufferLengthSpin, SIGNAL( valueChanged(double) ), this, SLOT( handleAudioBufferLengthChanged(double) ) );

    mMainUi->BufferProgressBar->setRange( 0, 100 );
    mMainUi->BufferProgressBar->setValue( mAudioBufferProgress );

    connect( mAudioBufferTimer, SIGNAL( timeout() ), this, SLOT( updateAudioBufferTimer() ) );

    if( !initializeAudio( mDevices->defaultAudioOutput() ) )
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

    connect( mMainUi->actionSmcOpen, &QAction::triggered, this, &SignalGenerator::handleSmcOpen );

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
//! Check if an integer value is valid
//!
//! @returns: true if valid
//!************************************************************************
bool SignalGenerator::checkValidInteger
    (
    const int16_t aIntValue         //!< integer value
    ) const
{
    return ( aIntValue != mSmc.mNoValueInteger );
}


//!************************************************************************
//! Check if a real value is valid
//!
//! @returns: true if valid
//!************************************************************************
bool SignalGenerator::checkValidReal
    (
    const double aRealValue         //!< real value
    ) const
{
    return ( std::fabs( aRealValue - mSmc.mNoValueReal ) > 1.e-7 );
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
//! Create a SMC signal with data read from a file
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::createSmcSignal()
{
    mAudioBufferLength = mSmc.mDataLengthSeconds;

    if( mAudioSrc )
    {
        mAudioSrc->setBufferLength( mAudioBufferLength );
    }

    SignalItem::SignalSmc sig;
    sig.nrPoints = mSmc.mDataValuesRecordedCount;
    sig.sps = mSmc.mSamplingRate;
    sig.maxAccelMs2 = std::max( std::fabs( mSmc.mMaximumFromRecord.accelerationMs2 ),
                                std::fabs( mSmc.mMinimumFromRecord.accelerationMs2 ) );
    sig.accelDataVec.resize( sig.nrPoints );
    sig.accelDataVec = mSmc.mDataVector;

    mCurrentSignalType = SignalItem::SIGNAL_TYPE_SMC;
    int crtTab = mCurrentSignalType - SignalItem::SIGNAL_TYPE_FIRST;
    mMainUi->SignalTypesTab->setCurrentIndex( crtTab );
    handleSignalTypeChanged();

    SignalItem* smcSignal = new SignalItem( sig );
    mSignalsVector.push_back( smcSignal );
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
    mTabSignalsMap.insert( std::make_pair( SignalItem::SIGNAL_TYPE_SMC,         "SignalTabSmc" ) );
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
//! Update the field values of SMC tab
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::fillValuesSmc()
{
    mMainUi->SmcFilenameValue->setText( QString::fromStdString( mSmcInputFilename ) );

    mMainUi->SmcDataTypeValue->setText( QString::fromStdString( Smc::DATA_TYPE_FILE_STRINGS.at( mSmc.mTextDataTypeFile ) ) );

    // earthquake
    mMainUi->SmcEqNameValue->setText( QString::fromStdString( mSmc.mTextEarthquakeName ) );

    mMainUi->SmcEqDateValue->setText( QString::fromStdString( mSmc.mEarthquakeTimeStamp ) );
    mMainUi->SmcEqTimezoneValue->setText( QString::fromStdString( mSmc.mTextTimeZone ) );

    mMainUi->SmcEqMwValue->setText( QString::fromStdString( mSmc.mTextMomentMagnitude ) );
    mMainUi->SmcEqMsValue->setText( QString::fromStdString( mSmc.mTextSurfaceWaveMagnitude ) );
    mMainUi->SmcEqMlValue->setText( QString::fromStdString( mSmc.mTextLocalMagnitude ) );

    mMainUi->SmcEqLatValue->setText( QString::number( mSmc.mEpicenter.latitude ) );
    mMainUi->SmcEqLonValue->setText( QString::number( mSmc.mEpicenter.longitude ) );
    mMainUi->SmcEqDepthValue->setText( QString::number( mSmc.mEpicenter.depthKm ) );

    // station
    mMainUi->SmcStationNameValue->setText( QString::fromStdString( mSmc.mTextStationName ) );
    mMainUi->SmcStationCodeValue->setText( QString::fromStdString( mSmc.mTextStationCodeStr ) );
    mMainUi->SmcStationComponentValue->setText( QString::fromStdString( mSmc.mTextComponentOrientation ) );
    mMainUi->SmcStationEpicentralDistValue->setText( QString::fromStdString( mSmc.mTextEpicentralDistanceKm ) );

    const double MS2_TO_G = 9.80665;
    double pkAccelG = 0;

    try
    {
        double pkAccelMs2 = std::stod( mSmc.mTextPeakAcceleration );
        pkAccelG = pkAccelMs2 / MS2_TO_G;
    }
    catch( const std::invalid_argument& )
    {
    }

    QString pkAccelFormattedStr = QString::fromStdString( mSmc.mTextPeakAcceleration ) + " [m/s2] = " + QString::number( pkAccelG ) + " [g]";
    mMainUi->SmcStationPkAccelValue->setText( pkAccelFormattedStr );
    mMainUi->SmcStationStructureTypeValue->setText( QString::fromStdString( mSmc.mStructureTypeName ) );

    // instrument
    mMainUi->SmcInstTypeValue->setText( QString::fromStdString( mSmc.mSensorTypeStr ) );
    mMainUi->SmcInstDataSourceValue->setText( QString::fromStdString( mSmc.mTextDataSourceStr ) );
    mMainUi->SmcInstCutoffValue->setText( checkValidReal( mSmc.mSensorCutoffFrequency ) ? QString::number( mSmc.mSensorCutoffFrequency ) : NA_STR );
    mMainUi->SmcInstDampingCoeffValue->setText( checkValidReal( mSmc.mSensorDampingCoefficient ) ? QString::number( mSmc.mSensorDampingCoefficient ) : NA_STR );
    mMainUi->SmcInstVOrientationValue->setText( checkValidInteger( mSmc.mVerticalOrientation ) ? QString::number( mSmc.mVerticalOrientation ) : NA_STR );
    mMainUi->SmcInstHOrientationValue->setText( checkValidInteger( mSmc.mHorizontalOrientation ) ? QString::number( mSmc.mHorizontalOrientation ) : NA_STR );

    // time series
    mMainUi->SmcTimeUsablePointsValue->setText( checkValidInteger( mSmc.mDataValuesRecordedCount ) ? QString::number( mSmc.mDataValuesRecordedCount ) : NA_STR );

    mMainUi->SmcTimeSpsValue->setText( QString::number( mSmc.mSamplingRate ) );
    mMainUi->SmcTimeDurationValue->setText( QString::number( mSmc.mDataLengthSeconds ) );
    mMainUi->BufferLengthSpin->setValue( mAudioBufferLength );

    if( checkValidReal( mSmc.mMaximumFromRecord.accelerationMs2 ) )
    {
        mMainUi->SmcTimeAccelMaxGValue->setText( QString::number( mSmc.mMaximumFromRecord.accelerationMs2 / MS2_TO_G ) );
    }
    else
    {
        mMainUi->SmcTimeAccelMaxGValue->setText( NA_STR );
    }

    if( checkValidReal( mSmc.mMinimumFromRecord.accelerationMs2 ) )
    {
        mMainUi->SmcTimeAccelMinGValue->setText( QString::number( mSmc.mMinimumFromRecord.accelerationMs2 / MS2_TO_G ) );
    }
    else
    {
        mMainUi->SmcTimeAccelMinGValue->setText( NA_STR );
    }
}


//!************************************************************************
//! Handle for changing the audio buffer length (seconds)
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleAudioBufferLengthChanged
    (
    double aValue      //!< value
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

    initializeAudio( mMainUi->GenerateDeviceComboBox->itemData( aIndex ).value<QAudioDevice>() );

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

            case SignalItem::SIGNAL_TYPE_SMC:
                // intentionally do nothing
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

                case SignalItem::SIGNAL_TYPE_SMC:
                    // intentionally do nothing
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

                case SignalItem::SIGNAL_TYPE_SMC:
                    // intentionally do nothing
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

            case SignalItem::SIGNAL_TYPE_SMC:
                // intentionally do nothing
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

                case SignalItem::SIGNAL_TYPE_SMC:
                    // intentionally do nothing
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
    if( QAudio::ActiveState == mAudioOutput->state() )
    {
        mAudioOutput->suspend();
    }

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
        mSignalIsSmc = false;
        mIsSignalEdited = false;

        mCurrentSignalType = SignalItem::SIGNAL_TYPE_TRIANGLE;
        int crtTab = mCurrentSignalType - SignalItem::SIGNAL_TYPE_FIRST;
        mMainUi->SignalTypesTab->setCurrentIndex( crtTab );
        handleSignalTypeChanged();

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
        mSignalIsSmc = false;
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

                        case SignalItem::SIGNAL_TYPE_SMC:
                            // intentionally do nothing
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
                mSignalIsSmc = false;

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
//! Open an accelerogram from a SMC (Strong Motion CD) data file
//!
//! @returns nothing
//!************************************************************************
/* slot */ void SignalGenerator::handleSmcOpen()
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
        mSignalIsSmc = false;
        mIsSignalEdited = false;

        mSignalsVector.clear();

        mAudioOutput->stop();

        if( mAudioSrc )
        {
            mAudioSrc->stop();
        }

        QString selectedFilter;
        QString fileName = QFileDialog::getOpenFileName( this,
                                                         "Open SMC file",
                                                         "",
                                                         "SMC files (*.smc);;All files (*)",
                                                         &selectedFilter,
                                                         QFileDialog::DontUseNativeDialog
                                                        );

        mSmcInputFilename = fileName.toStdString();
        std::ifstream inputFile( mSmcInputFilename );

        if( inputFile.is_open() )
        {
            mSmc = Smc();

            std::vector<int16_t> intHeaderVec;
            intHeaderVec.resize( Smc::HEADER_INT_LINES_COUNT * Smc::HEADER_INT_VALUES_PER_LINE );

            std::vector<double> realHeaderVec;
            realHeaderVec.resize( Smc::HEADER_REAL_LINES_COUNT * Smc::HEADER_REAL_VALUES_PER_LINE );

            std::vector<std::string> substringsVec;

            const std::string STAR = "*";

            std::string currentLine;
            int crtLineNr = 0;

            while( getline( inputFile, currentLine ) && mSmc.mSmcFormatOk )
            {
                crtLineNr++;

                if( crtLineNr <= Smc::LAST_TEXT_LINE_NR )
                {
                    ///////////////////////
                    // text header
                    ///////////////////////

                    switch( crtLineNr )
                    {
                        case 1:
                            {
                                bool typefound = false;
                                size_t i = 0;
                                trim( currentLine );

                                for( i = 0; i < Smc::DATA_TYPE_FILE_STRINGS.size(); i++ )
                                {
                                    if( Smc::DATA_TYPE_FILE_STRINGS.at( static_cast<Smc::DataTypeFile>( i ) ) == currentLine )
                                    {
                                        typefound = true;
                                        break;
                                    }
                                }

                                if( !typefound )
                                {
                                    mSmc.mSmcFormatOk = false;
                                    mSmc.mSmcTypeAccelerogram = false;

                                    QString msg = "Current file has no SMC header.";
                                    QMessageBox msgBox;
                                    msgBox.setText( msg );
                                    msgBox.exec();

                                    break;
                                }
                                else
                                {
                                    mSmc.mTextDataTypeFile = static_cast<Smc::DataTypeFile>( i );

                                    if( Smc::DATA_TYPE_FILE_UNCORRECTED_ACCELEROGRAM != mSmc.mTextDataTypeFile
                                     && Smc::DATA_TYPE_FILE_CORRECTED_ACCELEROGRAM != mSmc.mTextDataTypeFile )
                                    {
                                        mSmc.mSmcTypeAccelerogram = false;
                                    }

                                    if( !mSmc.mSmcTypeAccelerogram )
                                    {
                                        QString msg = "Current file is not an accelerogram in SMC format.";
                                        QMessageBox msgBox;
                                        msgBox.setText( msg );
                                        msgBox.exec();

                                        break;
                                    }
                                }
                            }
                            break;

                        case 3:
                            trim( currentLine );

                            if( STAR != currentLine )
                            {
                                mSmc.mTextStationCodeStr = currentLine;
                            }
                            break;

                        case 4:
                            {
                                std::string tmpStr = currentLine.substr( 0, 3 );

                                for( char c : tmpStr )
                                {
                                    if( ' ' != c )
                                    {
                                        mSmc.mTextTimeZone = tmpStr;
                                        break;
                                    }
                                }

                                mSmc.mTextEarthquakeYear = currentLine.substr( 5, 4 );
                                mSmc.mTextEarthquakeMonth = currentLine.substr( 11, 2 );
                                mSmc.mTextEarthquakeDay = currentLine.substr( 15, 2 );
                                mSmc.mTextEarthquakeHour = currentLine.substr( 21, 2 );
                                mSmc.mTextEarthquakeMinute = currentLine.substr( 23, 2 );

                                mSmc.mEarthquakeTimeStamp = mSmc.mTextEarthquakeYear + "." + mSmc.mTextEarthquakeMonth + "." + mSmc.mTextEarthquakeDay;
                                mSmc.mEarthquakeTimeStamp += " " + mSmc.mTextEarthquakeHour + ":" + mSmc.mTextEarthquakeMinute;

                                tmpStr = currentLine.substr( 26, 53 );
                                trim( tmpStr );
                                mSmc.mTextEarthquakeName = tmpStr;
                            }
                            break;

                        case 5:
                            mSmc.mSmcFormatOk = ( "Moment Mag=" == currentLine.substr( 0, 11 )
                                               && "Ms=" == currentLine.substr( 21, 3 )
                                               && "Ml=" == currentLine.substr( 34, 3 ) );

                            if( mSmc.mSmcFormatOk )
                            {
                                std::string tmpStr = currentLine.substr( 11, 9 );
                                trim( tmpStr );
                                mSmc.mTextMomentMagnitude = tmpStr;

                                tmpStr = currentLine.substr( 24, 9 );
                                trim( tmpStr );
                                mSmc.mTextSurfaceWaveMagnitude = tmpStr;

                                tmpStr = currentLine.substr( 37, 9 );
                                trim( tmpStr );
                                mSmc.mTextLocalMagnitude = tmpStr;
                            }
                            break;

                        case 6:
                            mSmc.mSmcFormatOk = ( ( "station = " == currentLine.substr( 0, 10 ) || "Station = " == currentLine.substr( 0, 10 ) )
                                               && "component=" == currentLine.substr( 41, 10 ) );

                            if( mSmc.mSmcFormatOk )
                            {
                                std::string tmpStr = currentLine.substr( 10, 30 );
                                trim( tmpStr );
                                mSmc.mTextStationName = tmpStr;

                                tmpStr = currentLine.substr( 52, 6 );
                                trim( tmpStr );
                                mSmc.mTextComponentOrientation = tmpStr;
                            }
                            else
                            {
                                mSmc.mSmcFormatOk = ( ( "station = " == currentLine.substr( 0, 10 ) || "Station = " == currentLine.substr( 0, 10 ) )
                                                   && "component=" == currentLine.substr( 36, 10 ) );

                                if( mSmc.mSmcFormatOk )
                                {
                                    std::string tmpStr = currentLine.substr( 10, 25 );
                                    trim( tmpStr );
                                    mSmc.mTextStationName = tmpStr;

                                    tmpStr = currentLine.substr( 47, 6 );
                                    trim( tmpStr );
                                    mSmc.mTextComponentOrientation = tmpStr;
                                }
                            }
                            break;

                        case 7:
                            mSmc.mSmcFormatOk = ( "epicentral dist =" == currentLine.substr( 0, 17 )
                                             && ( "pk acc =" == currentLine.substr( 33, 8 )
                                               || "pk     =" == currentLine.substr( 33, 8 ) )
                                                );

                            if( mSmc.mSmcFormatOk )
                            {
                                std::string tmpStr = currentLine.substr( 17, 9 );
                                trim( tmpStr );
                                mSmc.mTextEpicentralDistanceKm = tmpStr;

                                tmpStr = currentLine.substr( 41, 10 );
                                trim( tmpStr );

                                try
                                {
                                    // if a value is provided, convert cm/s2 -> m/s2
                                    double pkAccel = std::stod( tmpStr );
                                    pkAccel *= 1.e-2;
                                    mSmc.mTextPeakAcceleration = std::to_string( pkAccel );
                                }
                                catch( const std::invalid_argument& )
                                {
                                    mSmc.mTextPeakAcceleration = tmpStr;
                                }
                            }
                            break;

                        case 8:
                            mSmc.mSmcFormatOk = ( "inst type=" == currentLine.substr( 0, 10 )
                                               && "data source =" == currentLine.substr( 21, 13 ) );

                            if( mSmc.mSmcFormatOk )
                            {
                                std::string tmpStr = currentLine.substr( 10, 5 );
                                trim( tmpStr );
                                mSmc.mTextSensorTypeStr = tmpStr;

                                tmpStr = currentLine.substr( 35, 45 );
                                trim( tmpStr );
                                mSmc.mTextDataSourceStr = tmpStr;
                            }
                            break;

                        case 2:
                        case 9:
                        case 10:
                        case 11:
                            trim( currentLine );

                            if( mSmc.mSmcFormatOk )
                            {
                                mSmc.mSmcFormatOk = ( STAR == currentLine );
                            }
                            break;

                        default:
                            mSmc.mSmcFormatOk = false;
                            break;
                    }
                }
                else if( crtLineNr <= Smc::LAST_INT_LINE_NR )
                {
                    ///////////////////////
                    // integer header
                    ///////////////////////

                    if( Smc::LAST_TEXT_LINE_NR + 1 == crtLineNr )
                    {
                        substringsVec.clear();
                    }

                    for( size_t i = 0; i < currentLine.size(); i += Smc::HEADER_INT_VALUE_LENGTH )
                    {
                        substringsVec.push_back( currentLine.substr( i, Smc::HEADER_INT_VALUE_LENGTH ) );
                    }

                    if( Smc::LAST_INT_LINE_NR == crtLineNr )
                    {
                        std::vector<int16_t> tmpIntVec( substringsVec.size() );

                        for( size_t i = 0; i < tmpIntVec.size(); i++ )
                        {
                            tmpIntVec.at( i ) = std::stoi( substringsVec.at( i ) );
                        }

                        mSmc.mNoValueInteger = tmpIntVec.at( Smc::INT_FIELD_UNDEFINED_VALUE );

                        mSmc.mVerticalOrientation = tmpIntVec.at( Smc::INT_FIELD_VERTICAL_ORIENTATION_FROM_UP );
                        mSmc.mHorizontalOrientation = tmpIntVec.at( Smc::INT_FIELD_HORIZONTAL_ORIENTATION_FROM_NORTH_TO_EAST );

                        mSmc.mSensorTypeCode = tmpIntVec.at( Smc::INT_FIELD_SENSOR_TYPE_CODE );

                        if( checkValidInteger( mSmc.mSensorTypeCode ) )
                        {
                            mSmc.mSensorTypeStr = Smc::SENSOR_TYPE_NAMES.at( mSmc.mSensorTypeCode );
                        }
                        else
                        {
                            mSmc.mSensorTypeStr = "undefined";
                        }

                        mSmc.mHeaderCommentLinesCount = tmpIntVec.at( Smc::INT_FIELD_NR_OF_COMMENT_LINES );

                        mSmc.mDataValuesCount = tmpIntVec.at( Smc::INT_FIELD_NR_OF_VALUES );

                        if( checkValidInteger( mSmc.mDataValuesCount ) )
                        {
                            mSmc.mDataValuesRecordedCount = mSmc.mDataValuesCount;
                        }
                        else
                        {
                            mSmc.mSmcFormatOk = false;

                            QString msg = "No valid data length found in SMC file.";
                            QMessageBox msgBox;
                            msgBox.setText( msg );
                            msgBox.exec();

                            break;
                        }

                        mSmc.mDataLinesCount = static_cast<uint16_t>( std::ceil( static_cast<double>( mSmc.mDataValuesCount ) / Smc::DATA_VALUES_PER_LINE ) );

                        if( 0 == mSmc.mDataLinesCount )
                        {
                            mSmc.mSmcFormatOk = false;

                            QString msg = "No data values specified in SMC file.";
                            QMessageBox msgBox;
                            msgBox.setText( msg );
                            msgBox.exec();

                            break;
                        }

                        mSmc.mStructureType = static_cast<Smc::StructureType>( tmpIntVec.at( Smc::INT_FIELD_STRUCTURE_TYPE ) );

                        mSmc.mStructureTypeName = "unknown";

                        if( mSmc.mStructureType <= Smc::STRUCTURE_TYPE_MAX_KNOWN )
                        {
                            mSmc.mStructureTypeName = Smc::STRUCTURE_TYPE_NAMES.at( mSmc.mStructureType );
                        }

                        switch( mSmc.mStructureType )
                        {
                            case Smc::STRUCTURE_TYPE_BUILDING:
                                mSmc.mStructureBuilding.nrFloorsAboveGrade = tmpIntVec.at( Smc::INT_FIELD_TOTAL_NR_OF_FLOORS_ABOVE_GRADE );
                                mSmc.mStructureBuilding.nrStoriesBelowGrade = tmpIntVec.at( Smc::INT_FIELD_TOTAL_NR_OF_STORIES_BELOW_GRADE );
                                mSmc.mStructureBuilding.floorNrWhereLocated = tmpIntVec.at( Smc::INT_FIELD_FLOOR_NR );
                                break;

                            case Smc::STRUCTURE_TYPE_BRIDGE:
                                mSmc.mStructureBridge.nrSpans = tmpIntVec.at( Smc::INT_FIELD_NR_OF_SPANS );
                                mSmc.mStructureBridge.whereLocated =
                                        static_cast<Smc::BridgeLocation>( tmpIntVec.at( static_cast<size_t>( Smc::INT_FIELD_TRANSDUCER_LOCATION_BRIDGES ) ) );
                                break;

                            case Smc::STRUCTURE_TYPE_DAM:
                                mSmc.mStructureDam.location =
                                    static_cast<Smc::DamLocation>( tmpIntVec.at( static_cast<size_t>( Smc::INT_FIELD_TRANSDUCER_LOCATION_DAMS ) ) );
                                mSmc.mStructureDam.constructionType =
                                    static_cast<Smc::DamConstructionType>( tmpIntVec.at( static_cast<size_t>( Smc::INT_FIELD_CONSTRUCTION_TYPE ) ) );
                                break;

                            default:
                                break;
                        }

                        mSmc.mStationNr = tmpIntVec.at( Smc::INT_FIELD_STATION_NR );

                        mSmc.mFirstRecordedSampleIndex = tmpIntVec.at( Smc::INT_FIELD_FIRST_RECORDED_SAMPLE );
                        mSmc.mLastRecordedSampleIndex = tmpIntVec.at( Smc::INT_FIELD_LAST_RECORDED_SAMPLE );

                        if( checkValidInteger( mSmc.mFirstRecordedSampleIndex ) )
                        {
                            if( mSmc.mFirstRecordedSampleIndex >= 1
                             && mSmc.mFirstRecordedSampleIndex <= mSmc.mDataValuesCount )
                            {
                                mSmc.mDataValuesRecordedCount -= ( mSmc.mFirstRecordedSampleIndex - 1 );
                            }
                            else
                            {
                                mSmc.mFirstRecordedSampleIndex = 1;
                            }
                        }
                        else
                        {
                            mSmc.mFirstRecordedSampleIndex = 1;
                        }

                        if( checkValidInteger( mSmc.mLastRecordedSampleIndex ) )
                        {
                            if( mSmc.mLastRecordedSampleIndex <= mSmc.mDataValuesCount
                             && mSmc.mLastRecordedSampleIndex >= 1 )
                            {
                                mSmc.mDataValuesRecordedCount -= ( mSmc.mDataValuesCount - mSmc.mLastRecordedSampleIndex );
                            }
                            else
                            {
                                mSmc.mLastRecordedSampleIndex = mSmc.mDataValuesCount;
                            }
                        }
                        else
                        {
                            mSmc.mLastRecordedSampleIndex = mSmc.mDataValuesCount;
                        }

                        mSmc.mDataVector.resize( mSmc.mDataValuesRecordedCount );
                    }
                }
                else if( crtLineNr <= Smc::LAST_REAL_LINE_NR )
                {
                    ///////////////////////
                    // real header
                    ///////////////////////

                    if( Smc::LAST_INT_LINE_NR + 1 == crtLineNr )
                    {
                        substringsVec.clear();
                    }

                    for( size_t i = 0; i < currentLine.size(); i += Smc::HEADER_REAL_VALUE_LENGTH )
                    {
                        substringsVec.push_back( currentLine.substr( i, Smc::HEADER_REAL_VALUE_LENGTH ) );
                    }

                    if( Smc::LAST_REAL_LINE_NR == crtLineNr )
                    {
                        std::vector<double> tmpRealVec( substringsVec.size() );

                        for( size_t i = 0; i < tmpRealVec.size(); i++ )
                        {
                            tmpRealVec.at( i ) = std::stod( substringsVec.at( i ) );
                        }

                        mSmc.mNoValueReal = tmpRealVec.at( Smc::REAL_FIELD_UNDEFINED_VALUE );

                        mSmc.mSamplingRate = tmpRealVec.at( Smc::REAL_FIELD_SAMPLING_RATE );

                        if( checkValidReal( mSmc.mSamplingRate ) )
                        {
                            if( mSmc.mSamplingRate > 0 )
                            {
                                mSmc.mDataLengthSeconds = mSmc.mDataValuesRecordedCount / mSmc.mSamplingRate;
                            }
                            else
                            {
                                mSmc.mSmcFormatOk = false;

                                QString msg = "Invalid sampling rate value found in SMC file.";
                                QMessageBox msgBox;
                                msgBox.setText( msg );
                                msgBox.exec();

                                break;
                            }
                        }
                        else
                        {
                            mSmc.mSmcFormatOk = false;

                            QString msg = "No data sampling rate found in SMC file.";
                            QMessageBox msgBox;
                            msgBox.setText( msg );
                            msgBox.exec();

                            break;
                        }

                        mSmc.mEpicenter.latitude = tmpRealVec.at( Smc::REAL_FIELD_EARTHQUAKE_LATITUDE );
                        mSmc.mEpicenter.longitude = tmpRealVec.at( Smc::REAL_FIELD_EARTHQUAKE_LONGITUDE );
                        mSmc.mEpicenter.depthKm = tmpRealVec.at( Smc::REAL_FIELD_EARTHQUAKE_DEPTH_KM );

                        mSmc.mEarthquakeMagnitude.momentMagnitude = tmpRealVec.at( Smc::REAL_FIELD_SOURCE_MOMENT_MAGNITUDE );
                        mSmc.mEarthquakeMagnitude.surfaceWaveMagnitude = tmpRealVec.at( Smc::REAL_FIELD_SOURCE_SURFACE_WAVE_MAGNITUDE );
                        mSmc.mEarthquakeMagnitude.localMagnitude = tmpRealVec.at( Smc::REAL_FIELD_SOURCE_LOCAL_MAGNITUDE );
                        mSmc.mEarthquakeMagnitude.other = tmpRealVec.at( Smc::REAL_FIELD_SOURCE_OTHER );

                        mSmc.mSeismicMomentNm = tmpRealVec.at( Smc::REAL_FIELD_SEISMIC_MOMENT_DYNE_CM );

                        if( checkValidReal( mSmc.mSeismicMomentNm ) )
                        {
                            mSmc.mSeismicMomentNm *= 1.e-7; // dyn-cm to Nm
                        }

                        mSmc.mStation.latitude = tmpRealVec.at( Smc::REAL_FIELD_STATION_LATITUDE );
                        mSmc.mStation.longitude = tmpRealVec.at( Smc::REAL_FIELD_STATION_LONGITUDE );
                        mSmc.mStation.elevationMeters = tmpRealVec.at( Smc::REAL_FIELD_STATION_ELEVATION_M );
                        mSmc.mStation.offsetNorthMeters = tmpRealVec.at( Smc::REAL_FIELD_STATION_OFFSET_N_M );
                        mSmc.mStation.offsetEastMeters = tmpRealVec.at( Smc::REAL_FIELD_STATION_OFFSET_E_M );
                        mSmc.mStation.offsetUpMeters = tmpRealVec.at( Smc::REAL_FIELD_STATION_OFFSET_UP_M );

                        mSmc.mEpicentralDistanceKm = tmpRealVec.at( Smc::REAL_FIELD_EPICENTRAL_DISTANCE_KM );
                        mSmc.mEpicenterToStationAzimuth = tmpRealVec.at( Smc::REAL_FIELD_EPICENTER_TO_STATION_AZIMUTH );

                        mSmc.mDigitizationUnitsPerCm = tmpRealVec.at( Smc::REAL_FIELD_DIGITIZATION_UNITS_1_CM );

                        mSmc.mSensorCutoffFrequency = tmpRealVec.at( Smc::REAL_FIELD_SENSOR_CUTOFF_FREQUENCY_HZ );
                        mSmc.mSensorDampingCoefficient = tmpRealVec.at( Smc::REAL_FIELD_SENSOR_DAMPING_COEFFICIENT );

                        mSmc.mRecorderSensitivityCmG = tmpRealVec.at( Smc::REAL_FIELD_RECORDER_SENSITIVITY_CM_G );

                        mSmc.mMaximumFromRecord.time = tmpRealVec.at( Smc::REAL_FIELD_TIME_OF_MAXIMUM_S );
                        mSmc.mMaximumFromRecord.accelerationMs2 = tmpRealVec.at( Smc::REAL_FIELD_VALUE_OF_MAXIMUM_CM_S2 );

                        if( checkValidReal( mSmc.mMaximumFromRecord.accelerationMs2 ) )
                        {
                            mSmc.mMaximumFromRecord.accelerationMs2 *= 1.e-2; // m/s2
                        }

                        mSmc.mMinimumFromRecord.time = tmpRealVec.at( Smc::REAL_FIELD_TIME_OF_MINIMUM_S );
                        mSmc.mMinimumFromRecord.accelerationMs2 = tmpRealVec.at( Smc::REAL_FIELD_VALUE_OF_MINIMUM_CM_S2 );

                        if( checkValidReal( mSmc.mMinimumFromRecord.accelerationMs2 ) )
                        {
                            mSmc.mMinimumFromRecord.accelerationMs2 *= 1.e-2; // m/s2
                        }
                    }
                }
                else if( crtLineNr <= Smc::LAST_REAL_LINE_NR + mSmc.mHeaderCommentLinesCount )
                {
                    ///////////////////////
                    // comments header
                    ///////////////////////

                    if( Smc::LAST_INT_LINE_NR + 1 == crtLineNr )
                    {
                        substringsVec.clear();
                    }

                    substringsVec.push_back( currentLine );

                    if( Smc::LAST_REAL_LINE_NR + mSmc.mHeaderCommentLinesCount == crtLineNr )
                    {
                        // intentionally do nothing
                    }
                }
                else if( crtLineNr <= Smc::LAST_REAL_LINE_NR + mSmc.mHeaderCommentLinesCount + mSmc.mDataLinesCount )
                {
                    ///////////////////////
                    // data
                    ///////////////////////

                    if( Smc::LAST_REAL_LINE_NR + mSmc.mHeaderCommentLinesCount + 1 == crtLineNr )
                    {
                        substringsVec.clear();
                    }

                    for( size_t i = 0; i < currentLine.size(); i += Smc::DATA_VALUE_LENGTH )
                    {
                        substringsVec.push_back( currentLine.substr( i, Smc::DATA_VALUE_LENGTH ) );
                    }

                    if( Smc::LAST_REAL_LINE_NR + mSmc.mHeaderCommentLinesCount + mSmc.mDataLinesCount == crtLineNr )
                    {
                        std::vector<double> dataVec( substringsVec.size() );

                        for( size_t i = 0; i < dataVec.size(); i++ )
                        {
                            dataVec.at( i ) = std::stod( substringsVec.at( i ) );
                        }

                        if( dataVec.size() != mSmc.mDataValuesCount )
                        {
                            mSmc.mSmcFormatOk = false;

                            QString msg = "Expected data length was " + QString::number( mSmc.mDataValuesCount )
                                    + " , it is " + QString::number( dataVec.size() ) + ".";
                            QMessageBox msgBox;
                            msgBox.setText( msg );
                            msgBox.exec();

                            break;
                        }

                        size_t firstRecordedIndex = mSmc.mFirstRecordedSampleIndex - 1;
                        size_t lastRecordedIndex = dataVec.size() - 1 - ( mSmc.mDataValuesCount - mSmc.mLastRecordedSampleIndex );

                        for( size_t i = firstRecordedIndex, j = 0; i <= lastRecordedIndex; i++, j++ )
                        {
                            mSmc.mDataVector.at( j ) = 0.1 * dataVec.at( i ); // m/s2
                        }
                    }
                }
            }

            inputFile.close();

            if( mSmc.mSmcFormatOk )
            {
                mSignalUndefined = false;
                mSignalReady = true;
                mSignalIsSmc = true;

                createSmcSignal();

                setAudioData();
            }
            else
            {
                QString msg = "SMC file format is wrong at line " + QString::number( crtLineNr ) + ".";
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
    const QAudioDevice&     aDeviceInfo     //!< audio device
    )
{
    bool status = false;
    QAudioFormat format = aDeviceInfo.preferredFormat();
    format.setSampleRate( 44100 );
    format.setSampleFormat( QAudioFormat::Int16 );

    status = aDeviceInfo.isFormatSupported( format );

    mAudioSrc.reset( new AudioSource( format, mAudioBufferLength ) );
    mAudioOutput.reset( new QAudioSink( aDeviceInfo, format ) );

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
//! Trim a string at both ends
//!
//! @returns nothing
//!************************************************************************
void SignalGenerator::trim
    (
    std::string&    aString         //!< string to trim
    )
{
    aString.erase( aString.begin(), std::find_if( aString.begin(), aString.end(), []( unsigned char ch )
    {
        return !std::isspace( ch );
    }));

    aString.erase( std::find_if( aString.rbegin(), aString.rend(), []( unsigned char ch )
    {
        return !std::isspace( ch );
    } ).base(), aString.end());
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

    int fill = 100 * ( mAudioBufferCounter % static_cast<int>( mAudioBufferLength ) );

    if( 1 != mAudioBufferLength )
    {
        fill /= ( mAudioBufferLength - 1 );
    }

    mMainUi->BufferProgressBar->setValue( fill );
}


//!************************************************************************
//! Update when audio devices change
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::updateAudioDevices()
{
    mMainUi->GenerateDeviceComboBox->clear();
    const QList<QAudioDevice> devices = mDevices->audioOutputs();

    for( const QAudioDevice &deviceInfo : devices )
    {
        mMainUi->GenerateDeviceComboBox->addItem( deviceInfo.description(), QVariant::fromValue( deviceInfo ) );
    }
}


//!************************************************************************
//! Update UI controls depending on current status
//!
//! @returns: nothing
//!************************************************************************
void SignalGenerator::updateControls()
{    
    const uint8_t SMC_TAB_INDEX = SignalItem::SIGNAL_TYPE_SMC - SignalItem::SIGNAL_TYPE_FIRST;

    if( mSignalIsSmc )
    {
        for( int tabCtr = 0; tabCtr < mMainUi->SignalTypesTab->count(); tabCtr++ )
        {
            if( SMC_TAB_INDEX == tabCtr )
            {
                mMainUi->SignalTypesTab->setTabEnabled( tabCtr, true );
            }
            else
            {
                mMainUi->SignalTypesTab->setTabEnabled( tabCtr, false );
            }
        }

        mMainUi->SignalTypesTab->setEnabled( true );

        if( mMainUi->SignalItemActionButton->isVisible() )
        {
            mMainUi->SignalItemActionButton->hide();
        }

        if( mMainUi->ActiveSignalGroupBox->isVisible() )
        {
            mMainUi->ActiveSignalGroupBox->hide();
        }

        mMainUi->BufferLengthSpin->setEnabled( false );

        fillValuesSmc();
    }
    else
    {
        /////////////////////////////
        // SignalItemGroupBox
        /////////////////////////////
        for( int tabCtr = 0; tabCtr < mMainUi->SignalTypesTab->count(); tabCtr++ )
        {
            if( SMC_TAB_INDEX == tabCtr )
            {
                mMainUi->SignalTypesTab->setTabEnabled( tabCtr, false );
            }
            else
            {
                mMainUi->SignalTypesTab->setTabEnabled( tabCtr, true );
            }
        }

        if( !mMainUi->SignalItemActionButton->isVisible() )
        {
            mMainUi->SignalItemActionButton->show();
        }

        mMainUi->SignalTypesTab->setEnabled( !mSignalUndefined && !mSignalStarted );

        mMainUi->SignalItemActionButton->setEnabled( !mSignalUndefined && !mSignalStarted );
        mMainUi->SignalItemActionButton->setText( mIsSignalEdited ? "Replace current signal item" : "Add to active signal" );

        /////////////////////////////
        // ActiveSignalGroupBox
        /////////////////////////////
        if( !mMainUi->ActiveSignalGroupBox->isVisible() )
        {
            mMainUi->ActiveSignalGroupBox->show();
        }

        mMainUi->ActiveSignalGroupBox->setEnabled( !mSignalUndefined && !mSignalStarted );

        mMainUi->BufferLengthSpin->setEnabled( !mSignalStarted && !mSignalPaused );

        bool activeSignalBtnCondition = !mSignalUndefined && mSignalsVector.size() && !mIsSignalEdited;
        mMainUi->ActiveSignalEditButton->setEnabled( activeSignalBtnCondition );
        mMainUi->ActiveSignalSaveButton->setEnabled( activeSignalBtnCondition );
        mMainUi->ActiveSignalRemoveButton->setEnabled( activeSignalBtnCondition );

        mMainUi->ActiveSignalList->setEnabled( !mIsSignalEdited );
    }

    /////////////////////////////
    // GenerateGroupBox
    /////////////////////////////
    mMainUi->GenerateGroupBox->setEnabled( mSignalReady && !mIsSignalEdited );

    mMainUi->GeneratePauseButton->setText( mSignalPaused ? "Continue" : "Pause" );

    mMainUi->GenerateDeviceComboBox->setEnabled( !mSignalStarted && !mSignalPaused );

    mMainUi->GenerateStartButton->setEnabled( mSignalReady && !mSignalStarted && !mSignalPaused );
    mMainUi->GeneratePauseButton->setEnabled( mSignalReady && mSignalStarted );
    mMainUi->GenerateStopButton->setEnabled( mSignalReady && mSignalStarted );
}
