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
Smc.h
This file contains the definitions for processing SMC (Strong Motion CD) files.
*/

#ifndef Smc_h
#define Smc_h

#include <cstdint>
#include <map>
#include <string>
#include <vector>


//************************************************************************
// Class for handling SMC data files
//************************************************************************
class Smc
{
    //************************************************************************
    // constants and types
    //************************************************************************
    public:
        static const uint8_t HEADER_TEXT_LINES_COUNT = 11;
        static const uint8_t HEADER_INT_LINES_COUNT = 6;
        static const uint8_t HEADER_REAL_LINES_COUNT = 10;

        static const uint8_t LAST_TEXT_LINE_NR = HEADER_TEXT_LINES_COUNT;
        static const uint8_t LAST_INT_LINE_NR = LAST_TEXT_LINE_NR + HEADER_INT_LINES_COUNT;
        static const uint8_t LAST_REAL_LINE_NR = LAST_INT_LINE_NR + HEADER_REAL_LINES_COUNT;

        // int header format: 8I10
        static const uint8_t HEADER_INT_VALUES_PER_LINE = 8;
        static const uint8_t HEADER_INT_VALUE_LENGTH = 10;

        // real header format: 5E15.7
        static const uint8_t HEADER_REAL_VALUES_PER_LINE = 5;
        static const uint8_t HEADER_REAL_VALUE_LENGTH = 15;

        // data format: 8E10.4
        static const uint8_t DATA_VALUES_PER_LINE = 8;
        static const uint8_t DATA_VALUE_LENGTH = 10;

        typedef enum : uint8_t
        {
            DATA_TYPE_FILE_UNKNOWN,
            DATA_TYPE_FILE_UNCORRECTED_ACCELEROGRAM,
            DATA_TYPE_FILE_CORRECTED_ACCELEROGRAM,
            DATA_TYPE_FILE_VELOCITY,
            DATA_TYPE_FILE_DISPLACEMENT,
            DATA_TYPE_FILE_RESPONSE_SPECTRA,
            DATA_TYPE_FILE_FOURIER_AMPLITUDE_SPECTRA
        }DataTypeFile;

        static const std::map<DataTypeFile, std::string> DATA_TYPE_FILE_STRINGS;

        static const std::map<int16_t, std::string> SENSOR_TYPE_NAMES;

        typedef enum : uint8_t
        {
            STRUCTURE_TYPE_NOT_A_STRUCTURE,
            STRUCTURE_TYPE_BUILDING,
            STRUCTURE_TYPE_BRIDGE,
            STRUCTURE_TYPE_DAM,
            STRUCTURE_TYPE_OTHER,

            STRUCTURE_TYPE_MAX_KNOWN = STRUCTURE_TYPE_OTHER
        }StructureType;

        static const std::map<StructureType, std::string> STRUCTURE_TYPE_NAMES;

        typedef struct
        {
            int16_t     nrFloorsAboveGrade;
            int16_t     nrStoriesBelowGrade;
            int16_t     floorNrWhereLocated;
        }StructureBuilding;

        typedef enum : uint8_t
        {
            BRIDGE_LOCATION_FREE_FIELD,
            BRIDGE_LOCATION_AT_THE_BASE,
            BRIDGE_LOCATION_ON_AMBUTMENT,
            BRIDGE_LOCATION_ON_DECK_AT_TOP_OF_PIER,
            BRIDGE_LOCATION_ON_DECK_BETWEEN_PIERS
        }BridgeLocation;

        static const std::map<uint8_t, std::string> BRIDGE_LOCATION_NAMES;

        typedef struct
        {
            int16_t         nrSpans;
            BridgeLocation  whereLocated;
        }StructureBridge;

        typedef enum : uint8_t
        {
            DAM_LOCATION_FREE_FIELD,
            DAM_LOCATION_AT_THE_BASE,
            DAM_LOCATION_ON_THE_CREST,
            DAM_LOCATION_ON_THE_AMBUTMENT
        }DamLocation;

        typedef enum : uint8_t
        {
            DAM_CONSTRUCTION_TYPE_REINFORCED_CONCRETE_GRAVITY = 1,
            DAM_CONSTRUCTION_TYPE_REINFORCED_CONCRETE_ARCH,
            DAM_CONSTRUCTION_TYPE_EARTH_FILL,
            DAM_CONSTRUCTION_TYPE_OTHER
        }DamConstructionType;

        typedef struct
        {
            DamLocation         location;
            DamConstructionType constructionType;
        }StructureDam;

        typedef enum : uint8_t
        {
            INT_FIELD_UNDEFINED_VALUE,
            INT_FIELD_YEAR,
            INT_FIELD_JULIAN_DAY,
            INT_FIELD_HOUR,
            INT_FIELD_MINUTE,
            INT_FIELD_SECOND,
            INT_FIELD_MILLISECOND,
            INT_FIELD_RECORDER_SERIAL_NR,
            INT_FIELD_CHANNEL_NR_OF_TRACE,
            INT_FIELD_TOTAL_NR_OF_CHANNELS_IN_RECORD,
            INT_FIELD_TOTAL_NR_OF_CHANNELS_RECORDED_AT_STATION,
            INT_FIELD_SENSOR_SERIAL_NR,
            INT_FIELD_VERTICAL_ORIENTATION_FROM_UP,
            INT_FIELD_HORIZONTAL_ORIENTATION_FROM_NORTH_TO_EAST,
            INT_FIELD_SENSOR_TYPE_CODE,
            INT_FIELD_NR_OF_COMMENT_LINES,
            INT_FIELD_NR_OF_VALUES,
            INT_FIELD_PROBLEM_FLAG,
            INT_FIELD_STRUCTURE_TYPE,
            INT_FIELD_STRUCTURE_NR,
            INT_FIELD_TRANSDUCER_NR_OF_THE_RECORDING_SYSTEM,
            INT_FIELD_TOTAL_NR_OF_TRANSDUCER_CHANNELS,
            // ///////////////////////////
            // buildings only
            INT_FIELD_TOTAL_NR_OF_FLOORS_ABOVE_GRADE,
            INT_FIELD_TOTAL_NR_OF_STORIES_BELOW_GRADE,
            INT_FIELD_FLOOR_NR,
            // ///////////////////////////
            // bridges only
            INT_FIELD_NR_OF_SPANS,
            INT_FIELD_TRANSDUCER_LOCATION_BRIDGES,
            // ///////////////////////////
            // dams only
            INT_FIELD_TRANSDUCER_LOCATION_DAMS,
            INT_FIELD_CONSTRUCTION_TYPE,
            // ///////////////////////////
            INT_FIELD_STATION_NR,
            INT_FIELD_FIRST_RECORDED_SAMPLE,
            INT_FIELD_LAST_RECORDED_SAMPLE,
            INT_FIELD_FILE_FLAG
        }IntField;

        typedef struct
        {
            double latitude;
            double longitude;
            double depthKm;
        }Epicenter;

        typedef struct
        {
            double momentMagnitude;
            double surfaceWaveMagnitude;
            double localMagnitude;
            double other;
        }EarthquakeMagnitude;

        typedef struct
        {
            double latitude;
            double longitude;
            double elevationMeters;
            double offsetNorthMeters;
            double offsetEastMeters;
            double offsetUpMeters;
        }Station;

        typedef struct
        {
            double time;
            double accelerationMs2;
        }TimeAccelerationPair;

        typedef enum : uint8_t
        {
            REAL_FIELD_UNDEFINED_VALUE,
            REAL_FIELD_SAMPLING_RATE,

            REAL_FIELD_EARTHQUAKE_LATITUDE,
            REAL_FIELD_EARTHQUAKE_LONGITUDE,
            REAL_FIELD_EARTHQUAKE_DEPTH_KM,

            REAL_FIELD_SOURCE_MOMENT_MAGNITUDE,
            REAL_FIELD_SOURCE_SURFACE_WAVE_MAGNITUDE,
            REAL_FIELD_SOURCE_LOCAL_MAGNITUDE,
            REAL_FIELD_SOURCE_OTHER,

            REAL_FIELD_SEISMIC_MOMENT_DYNE_CM,

            REAL_FIELD_STATION_LATITUDE,
            REAL_FIELD_STATION_LONGITUDE,
            REAL_FIELD_STATION_ELEVATION_M,
            REAL_FIELD_STATION_OFFSET_N_M,
            REAL_FIELD_STATION_OFFSET_E_M,
            REAL_FIELD_STATION_OFFSET_UP_M,

            REAL_FIELD_EPICENTRAL_DISTANCE_KM,
            REAL_FIELD_EPICENTER_TO_STATION_AZIMUTH,

            REAL_FIELD_DIGITIZATION_UNITS_1_CM,

            REAL_FIELD_DIGITAL_ANTI_ALIAS_FILTER_CORNER_HZ,
            REAL_FIELD_DIGITAL_ANTI_ALIAS_FILTER_POLES,

            REAL_FIELD_SENSOR_CUTOFF_FREQUENCY_HZ,
            REAL_FIELD_SENSOR_DAMPING_COEFFICIENT,

            REAL_FIELD_RECORDER_SENSITIVITY_CM_G,

            REAL_FIELD_DIGITAL_AMPLIFIER_GAIN_DB,
            REAL_FIELD_DIGITAL_PREAMP_GAIN_DB,

            REAL_FIELD_UNDEFINED_27,
            REAL_FIELD_UNDEFINED_28,

            REAL_FIELD_TIME_OF_MAXIMUM_S,
            REAL_FIELD_VALUE_OF_MAXIMUM_CM_S2,

            REAL_FIELD_TIME_OF_MINIMUM_S,
            REAL_FIELD_VALUE_OF_MINIMUM_CM_S2
        }RealField;


    //************************************************************************
    // functions
    //************************************************************************
    public:
        Smc();


    //************************************************************************
    // variables
    //************************************************************************
    public:
        bool                    mSmcFormatOk;               //!< true if the SMC file format is OK
        bool                    mSmcTypeAccelerogram;       //!< true if the SMC file is an accelerogram

        /////////////////////////////////
        // text header
        /////////////////////////////////
        // line #1
        DataTypeFile            mTextDataTypeFile;          //!< type of data in file

        // line 2
        // "*" in first column

        // line 3
        std::string             mTextStationCodeStr;        //!< station code

        // line 4
        std::string             mTextTimeZone;              //!< time zone, if different than GMT

        std::string             mTextEarthquakeYear;        //!< earthquake YYYY
        std::string             mTextEarthquakeMonth;       //!< earthquake MM
        std::string             mTextEarthquakeDay;         //!< earthquake DD
        std::string             mTextEarthquakeHour;        //!< earthquake HH
        std::string             mTextEarthquakeMinute;      //!< earthquake MM
        std::string             mEarthquakeTimeStamp;       //!< YYYY.MM.DD HH:MM

        std::string             mTextEarthquakeName;        //!< earthquake name

        // line 5
        std::string             mTextMomentMagnitude;       //!< moment-magnitude
        std::string             mTextSurfaceWaveMagnitude;  //!< surface-wave magnitude
        std::string             mTextLocalMagnitude;        //!< local magnitude

        // line 6
        std::string             mTextStationName;           //!< station name
        std::string             mTextComponentOrientation;  //!< orientation

        // line 7
        std::string             mTextEpicentralDistanceKm;  //!< distance between the epicenter and the station

        std::string             mTextPeakAcceleration;      //!< peak accel [m/s2]

        // line 8
        std::string             mTextSensorTypeStr;         //!< sensor type name

        std::string             mTextDataSourceStr;         //!< data source, e.g. "USGS"

        // lines 9-10-11
        // "*" in first column

        /////////////////////////////////
        // integer header
        /////////////////////////////////
        int16_t                 mNoValueInteger;            //!< integer value corresponding to N/A

        int16_t                 mVerticalOrientation;       //!< degrees from up (up=0, down=180) [deg]
        int16_t                 mHorizontalOrientation;     //!< degrees from N to E (N=0, E=90, S=180, W=270) [deg]

        int16_t                 mSensorTypeCode;            //!< sensor type code
        std::string             mSensorTypeStr;             //!< sensor type name

        int16_t                 mHeaderCommentLinesCount;   //!< number of lines with comments

        int16_t                 mDataValuesCount;           //!< number of data values

        uint16_t                mDataLinesCount;            //!< number of lines with data

        StructureType           mStructureType;             //!< structure type
        std::string             mStructureTypeName;         //!< structure type name
        StructureBuilding       mStructureBuilding;         //!< building details
        StructureBridge         mStructureBridge;           //!< bridge details
        StructureDam            mStructureDam;              //!< dam details

        int16_t                 mStationNr;                 //!< official station number

        int16_t                 mFirstRecordedSampleIndex;  //!< index of first recorded sample
        int16_t                 mLastRecordedSampleIndex;   //!< index of last recorded sample

        /////////////////////////////////
        // real header
        /////////////////////////////////
        double                  mNoValueReal;               //!< floating point value corresponding to N/A

        double                  mSamplingRate;              //!< sampling rate [SPS]

        Epicenter               mEpicenter;                 //!< earthquake location

        EarthquakeMagnitude     mEarthquakeMagnitude;       //!< magnitude info
        double                  mSeismicMomentNm;           //!< seismic moment [Nm]

        Station                 mStation;                   //!< seismic station

        double                  mEpicentralDistanceKm;      //!< distance between the epicenter and the station
        double                  mEpicenterToStationAzimuth; //!< degrees to E from N with the epicenter in origin

        double                  mDigitizationUnitsPerCm;    //!< for analog-recorded data [1/cm]

        double                  mSensorCutoffFrequency;     //!< natural frequency [Hz]
        double                  mSensorDampingCoefficient;  //!< fraction of critical damping [-]

        double                  mRecorderSensitivityCmG;    //!< recorder sensitivity [cm/g]

        TimeAccelerationPair    mMaximumFromRecord;         //!< maximum acceleration [m/s2] from entire record
        TimeAccelerationPair    mMinimumFromRecord;         //!< minimum acceleration [m/s2] from entire record

        /////////////////////////////////
        // data
        /////////////////////////////////
        std::vector<double>     mDataVector;                //!< accelerogram values [m/s2]
        int16_t                 mDataValuesRecordedCount;   //!< lenght of recorded data, considering first and last indexes
        double                  mDataLengthSeconds;         //!< data duration [s]
};

#endif // Smc_h
