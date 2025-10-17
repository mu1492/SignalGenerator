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
SignalItem.cpp
This file contains the sources for a signal item.
*/

#include "SignalItem.h"

#include <cstring>


//!************************************************************************
//! Constructor for Triangle signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalTriangle      aSignalData     //!< Triangle signal data
    )
    : mType( SIGNAL_TYPE_TRIANGLE )
{
    cleanDataStructures();
    memcpy( &mSignalDataTriangle, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for Rectangle signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalRectangle     aSignalData     //!< Rectangle signal data
    )
    : mType( SIGNAL_TYPE_RECTANGLE )
{
    cleanDataStructures();
    memcpy( &mSignalDataRectangle, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for Pulse signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalPulse         aSignalData     //!< Pulse signal data
    )
    : mType( SIGNAL_TYPE_PULSE )
{
    cleanDataStructures();
    memcpy( &mSignalDataPulse, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for RiseFall signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalRiseFall      aSignalData     //!< RiseFall signal data
    )
    : mType( SIGNAL_TYPE_RISEFALL )
{
    cleanDataStructures();
    memcpy( &mSignalDataRiseFall, &aSignalData, sizeof( aSignalData ) );
 }

//!************************************************************************
//! Constructor for SinDamp signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalSinDamp       aSignalData     //!< SinDamp signal data
    )
    : mType( SIGNAL_TYPE_SINDAMP )
{
    cleanDataStructures();
    memcpy( &mSignalDataSinDamp, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for SinRise signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalSinRise       aSignalData     //!< SinRise signal data
    )
    : mType( SIGNAL_TYPE_SINRISE )
{
    cleanDataStructures();
    memcpy( &mSignalDataSinRise, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for WavSin signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalWavSin        aSignalData     //!< WavSin signal data
    )
    : mType( SIGNAL_TYPE_WAVSIN )
{
    cleanDataStructures();
    memcpy( &mSignalDataWavSin, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for AmSin signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalAmSin         aSignalData     //!< AmSin signal data
    )
    : mType( SIGNAL_TYPE_AMSIN )
{
    cleanDataStructures();
    memcpy( &mSignalDataAmSin, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for SinDampSin signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalSinDampSin    aSignalData     //!< SinDampSin signal data
    )
    : mType( SIGNAL_TYPE_SINDAMPSIN )
{
    cleanDataStructures();
    memcpy( &mSignalDataSinDampSin, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for TrapDampSin signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalTrapDampSin   aSignalData     //!< TrapDampSin signal data
    )
    : mType( SIGNAL_TYPE_TRAPDAMPSIN )
{
    cleanDataStructures();
    memcpy( &mSignalDataTrapDampSin, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for Noise signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalNoise         aSignalData     //!< Noise signal data
    )
    : mType( SIGNAL_TYPE_NOISE )
{
    cleanDataStructures();
    memcpy( &mSignalDataNoise, &aSignalData, sizeof( aSignalData ) );
}

//!************************************************************************
//! Constructor for SMC signal type
//!************************************************************************
SignalItem::SignalItem
    (
    SignalSmc           aSignalData     //!< SMC signal data
    )
    : mType( SIGNAL_TYPE_SMC )
{
    cleanDataStructures();
    memcpy( &mSignalDataSmc, &aSignalData, sizeof( aSignalData ) );
}


//!************************************************************************
//! Clean all data structures
//!
//! @returns: nothing
//!************************************************************************
void SignalItem::cleanDataStructures()
{
    memset( &mSignalDataTriangle,       0, sizeof( mSignalDataTriangle ) );
    memset( &mSignalDataRectangle,      0, sizeof( mSignalDataRectangle ) );
    memset( &mSignalDataPulse,          0, sizeof( mSignalDataPulse ) );
    memset( &mSignalDataRiseFall,       0, sizeof( mSignalDataRiseFall ) );
    memset( &mSignalDataSinDamp,        0, sizeof( mSignalDataSinDamp ) );
    memset( &mSignalDataSinRise,        0, sizeof( mSignalDataSinRise ) );
    memset( &mSignalDataWavSin,         0, sizeof( mSignalDataWavSin ) );
    memset( &mSignalDataAmSin,          0, sizeof( mSignalDataAmSin ) );
    memset( &mSignalDataSinDampSin,     0, sizeof( mSignalDataSinDampSin ) );
    memset( &mSignalDataTrapDampSin,    0, sizeof( mSignalDataTrapDampSin ) );
    memset( &mSignalDataNoise,          0, sizeof( mSignalDataNoise ) );
    memset( &mSignalDataSmc,            0, sizeof( mSignalDataSmc ) );
}


//!************************************************************************
//! Get Triangle signal data
//!
//! @returns: SignalTriangle data structure
//!************************************************************************
SignalItem::SignalTriangle SignalItem::getSignalDataTriangle() const
{
    return mSignalDataTriangle;
}

//!************************************************************************
//! Get Rectangle signal data
//!
//! @returns: SignalRectangle data structure
//!************************************************************************
SignalItem::SignalRectangle SignalItem::getSignalDataRectangle() const
{
    return mSignalDataRectangle;
}

//!************************************************************************
//! Get Pulse signal data
//!
//! @returns: SignalPulse data structure
//!************************************************************************
SignalItem::SignalPulse SignalItem::getSignalDataPulse() const
{
    return mSignalDataPulse;
}

//!************************************************************************
//! Get RiseFall signal data
//!
//! @returns: SignalRiseFall data structure
//!************************************************************************
SignalItem::SignalRiseFall SignalItem::getSignalDataRiseFall() const
{
    return mSignalDataRiseFall;
}

//!************************************************************************
//! Get SinDamp signal data
//!
//! @returns: SignalSinDamp data structure
//!************************************************************************
SignalItem::SignalSinDamp SignalItem::getSignalDataSinDamp() const
{
    return mSignalDataSinDamp;
}

//!************************************************************************
//! Get SinRise signal data
//!
//! @returns: SignalSinRise data structure
//!************************************************************************
SignalItem::SignalSinRise SignalItem::getSignalDataSinRise() const
{
    return mSignalDataSinRise;
}

//!************************************************************************
//! Get WavSin signal data
//!
//! @returns: SignalWavSin data structure
//!************************************************************************
SignalItem::SignalWavSin SignalItem::getSignalDataWavSin() const
{
    return mSignalDataWavSin;
}

//!************************************************************************
//! Get AmSin signal data
//!
//! @returns: SignalAmSin data structure
//!************************************************************************
SignalItem::SignalAmSin SignalItem::getSignalDataAmSin() const
{
    return mSignalDataAmSin;
}

//!************************************************************************
//! Get SinDampSin signal data
//!
//! @returns: SignalSinDampSin data structure
//!************************************************************************
SignalItem::SignalSinDampSin SignalItem::getSignalDataSinDampSin() const
{
    return mSignalDataSinDampSin;
}

//!************************************************************************
//! Get TrapDampSin signal data
//!
//! @returns: SignalTrapDampSin data structure
//!************************************************************************
SignalItem::SignalTrapDampSin SignalItem::getSignalDataTrapDampSin() const
{
    return mSignalDataTrapDampSin;
}

//!************************************************************************
//! Get Noise signal data
//!
//! @returns: SignalNoise data structure
//!************************************************************************
SignalItem::SignalNoise SignalItem::getSignalDataNoise() const
{
    return mSignalDataNoise;
}

//!************************************************************************
//! Get SMC signal data
//!
//! @returns: SignalNoise data structure
//!************************************************************************
SignalItem::SignalSmc SignalItem::getSignalDataSmc() const
{
    return mSignalDataSmc;
}


//!************************************************************************
//! Get the signal type
//!
//! @returns: SignalType
//!************************************************************************
SignalItem::SignalType SignalItem::getType() const
{
    return mType;
}
