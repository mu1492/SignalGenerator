///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2025 Mihai Ursu                                                 //
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
Smc.cpp
This file contains the sources for processing SMC (Strong Motion CD) files.
*/

#include "Smc.h"

#include <cstring>


const std::map<Smc::DataTypeFile, std::string> Smc::DATA_TYPE_FILE_STRINGS =
{
    { Smc::DATA_TYPE_FILE_UNKNOWN,                      "0 UNKNOWN"                 },
    { Smc::DATA_TYPE_FILE_UNCORRECTED_ACCELEROGRAM,     "1 UNCORRECTED ACCELEROGRAM"},
    { Smc::DATA_TYPE_FILE_CORRECTED_ACCELEROGRAM,       "2 CORRECTED ACCELEROGRAM"  },
    { Smc::DATA_TYPE_FILE_VELOCITY,                     "3 VELOCITY"                },
    { Smc::DATA_TYPE_FILE_DISPLACEMENT,                 "4 DISPLACEMENT"            },
    { Smc::DATA_TYPE_FILE_RESPONSE_SPECTRA,             "5 RESPONSE SPECTRA"        },
    { Smc::DATA_TYPE_FILE_FOURIER_AMPLITUDE_SPECTRA,    "6 FOURIER AMPLITUDE SPECTRA OF CORRECTED ACCELERATION" }
};


const std::map<int16_t, std::string> Smc::SENSOR_TYPE_NAMES =
{
    {    2, "Sprengnether SA-3000 3-component fba"  },
    {   30, "Kinemetrics FBA-13 3-component fba"    },
    {   31, "Kinemetrics FBA-11 1-component fba"    },
    {  101, "SMA-1"                                 },
    {  102, "C&GS Standard"                         },
    {  103, "AR-240"                                },
    {  104, "RFT-250"                               },
    {  105, "RFT-350"                               },
    {  106, "MO-2"                                  },
    {  107, "RMT-280"                               },
    {  108, "SMA-2/3"                               },
    {  109, "DSA-1/DSA-3"                           },
    {  110, "DCA-300"                               },
    {  111, "DCA-333"                               },
    {  112, "A-700"                                 },
    {  113, "SSA-1"                                 },
    {  114, "CRA-1"                                 },
    {  115, "MO-2"                                  },
    {  116, "FBA-3"                                 },
    {  117, "SMA-2"                                 },
    {  118, "DCA-310"                               },
    {  119, "FBA-13"                                },
    {  120, "SSA-2"                                 },
    {  121, "SSR-1"                                 },
    {  122, "BIDRA"                                 },
    {  123, "CR-1"                                  },
    {  124, "PDR-1"                                 },
    {  125, "Kinemetrics FBA-23"                    },
    {  126, "Kinemetrics Episensor"                 },
    {  127, "Kinemetrics FBA-4g"                    },
    {  128, "Kinemetrics FBA-2g"                    },
    {  129, "Kinemetrics FBA-1g"                    },
    {  130, "Kinemetrics FBA-0.5g"                  },
    {  131, "Kinemetrics FBA-0.25g"                 },
    {  132, "Kinemetrics FBA-0.1g"                  },
    {  133, "WR1"                                   },
    {  134, "S6000"                                 },
    {  135, "Mark Products L22"                     },
    {  136, "Products L4C"                          },
    {  137, "CMG3"                                  },
    {  138, "CMG3T"                                 },
    {  139, "CMG40T"                                },
    {  140, "CMG5"                                  },
    {  141, "KS-2000"                               },
    {  900, "custom instrument"                     },
    { 1302, "Reftek Model 130-ANSS/02"              }
};

const std::map<Smc::StructureType, std::string> Smc::STRUCTURE_TYPE_NAMES =
{
    { Smc::STRUCTURE_TYPE_NOT_A_STRUCTURE,  "not a structure"   },
    { Smc::STRUCTURE_TYPE_BUILDING,         "building"          },
    { Smc::STRUCTURE_TYPE_BRIDGE,           "bridge"            },
    { Smc::STRUCTURE_TYPE_DAM,              "dam"               },
    { Smc::STRUCTURE_TYPE_OTHER,            "other"             }
};

const std::map<uint8_t, std::string> Smc::BRIDGE_LOCATION_NAMES =
{
    { Smc::BRIDGE_LOCATION_FREE_FIELD,              "free field"                        },
    { Smc::BRIDGE_LOCATION_AT_THE_BASE,             "at the base of a pier or ambutment"},
    { Smc::BRIDGE_LOCATION_ON_AMBUTMENT,            "on an ambutment"                   },
    { Smc::BRIDGE_LOCATION_ON_DECK_AT_TOP_OF_PIER,  "on the deck at the top of a pier"  },
    { Smc::BRIDGE_LOCATION_ON_DECK_BETWEEN_PIERS,   "on the deck between piers"         }
};


//************************************************************************
// Constructor
//************************************************************************
Smc::Smc()
    : mSmcFormatOk( true )
    , mSmcTypeAccelerogram( true )
    // text header
    , mTextDataTypeFile( DATA_TYPE_FILE_UNKNOWN )
    // integer header
    , mNoValueInteger( -32768 )
    , mVerticalOrientation( 0 )
    , mHorizontalOrientation( 0 )
    , mSensorTypeCode( 0 )
    , mHeaderCommentLinesCount( 0 )
    , mDataValuesCount( 0 )
    , mDataLinesCount( 0 )
    , mStructureType( STRUCTURE_TYPE_NOT_A_STRUCTURE )
    , mStationNr( 0 )
    , mFirstRecordedSampleIndex( 0 )
    , mLastRecordedSampleIndex( 0 )
    // real header
    , mNoValueReal( 1.7e+38 )
    , mSamplingRate( 0 )
    , mSeismicMomentNm( 0 )
    , mEpicentralDistanceKm( 0 )
    , mEpicenterToStationAzimuth( 0 )
    , mDigitizationUnitsPerCm( 0 )
    , mSensorCutoffFrequency( 0 )
    , mSensorDampingCoefficient( 0 )
    , mRecorderSensitivityCmG( 0 )
    // data
    , mDataValuesRecordedCount( 0 )
    , mDataLengthSeconds( 0 )
{
    /////////////////////////////////
    // text header
    /////////////////////////////////
    mTextStationCodeStr.clear();
    mTextTimeZone = "GMT";
    mTextEarthquakeYear.clear();
    mTextEarthquakeMonth.clear();
    mTextEarthquakeDay.clear();
    mTextEarthquakeHour.clear();
    mTextEarthquakeMinute.clear();
    mEarthquakeTimeStamp.clear();
    mTextEarthquakeName.clear();
    mTextMomentMagnitude.clear();
    mTextSurfaceWaveMagnitude.clear();
    mTextLocalMagnitude.clear();
    mTextStationName.clear();
    mTextComponentOrientation.clear();
    mTextEpicentralDistanceKm.clear();
    mTextPeakAcceleration.clear();
    mTextSensorTypeStr.clear();
    mTextDataSourceStr.clear();

    /////////////////////////////////
    // integer header
    /////////////////////////////////
    mSensorTypeStr.clear();
    mStructureTypeName.clear();
    memset( &mStructureBuilding, 0, sizeof( mStructureBuilding ) );
    memset( &mStructureBridge, 0, sizeof( mStructureBridge ) );
    memset( &mStructureDam, 0, sizeof( mStructureDam ) );

    /////////////////////////////////
    // real header
    /////////////////////////////////
    memset( &mEpicenter, 0, sizeof( mEpicenter ) );
    memset( &mEarthquakeMagnitude, 0, sizeof( mEarthquakeMagnitude ) );
    memset( &mStation, 0, sizeof( mStation ) );
    memset( &mMaximumFromRecord, 0, sizeof( mMaximumFromRecord ) );
    memset( &mMinimumFromRecord, 0, sizeof( mMinimumFromRecord ) );

    /////////////////////////////////
    // data
    /////////////////////////////////
    mDataVector.clear();
}
