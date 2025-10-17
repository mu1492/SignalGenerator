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
SignalItem.h
This file contains the definitions for a signal item.
*/

#ifndef SignalItem_h
#define SignalItem_h

#include <cstdint>
#include <vector>


//************************************************************************
// Class for handling a signal item
//************************************************************************
class SignalItem
{
    //************************************************************************
    // constants and types
    //************************************************************************
    public:
        typedef enum : uint8_t
        {
            SIGNAL_TYPE_INVALID,

            SIGNAL_TYPE_FIRST,
            SIGNAL_TYPE_TRIANGLE = SIGNAL_TYPE_FIRST,
            SIGNAL_TYPE_RECTANGLE,
            SIGNAL_TYPE_PULSE,
            SIGNAL_TYPE_RISEFALL,
            SIGNAL_TYPE_SINDAMP,
            SIGNAL_TYPE_SINRISE,
            SIGNAL_TYPE_WAVSIN,
            SIGNAL_TYPE_AMSIN,
            SIGNAL_TYPE_SINDAMPSIN,
            SIGNAL_TYPE_TRAPDAMPSIN,
            SIGNAL_TYPE_NOISE,
            SIGNAL_TYPE_SMC,

            SIGNAL_TYPE_COUNT
        }SignalType;


        struct SignalTriangle
        {
            SignalType type;

            double  tPeriod;
            double  tRise;
            double  tFall;
            double  tDelay;

            double  yMax;
            double  yMin;

            SignalTriangle()
            {
                type = SIGNAL_TYPE_TRIANGLE;

                tPeriod = 1;
                tRise = 0.5;
                tFall = 0.5;
                tDelay = 0;

                yMax = 1;
                yMin = -1;
            }
        };

        struct SignalRectangle
        {
            SignalType type;

            double  tPeriod;
            double  fillFactor;
            double  tDelay;

            double  yMax;
            double  yMin;

            SignalRectangle()
            {
                type = SIGNAL_TYPE_RECTANGLE;

                tPeriod = 1;
                fillFactor = 0.5;
                tDelay = 0;

                yMax = 1;
                yMin = -1;
            }
        };

        struct SignalPulse
        {
            SignalType type;

            double  tPeriod;
            double  tRise;
            double  tWidth;
            double  tFall;
            double  tDelay;

            double  yMax;
            double  yMin;

            SignalPulse()
            {
                type = SIGNAL_TYPE_PULSE;

                tPeriod = 1;
                tRise = 0.125;
                tWidth = 0.25;
                tFall = 0.125;
                tDelay = 0;

                yMax = 1;
                yMin = -1;
            }
        };

        struct SignalRiseFall
        {
            SignalType type;

            double  tDelay;
            double  tDelayRise;
            double  tRampRise;
            double  tDelayFall;
            double  tRampFall;

            double  yMax;
            double  yMin;

            SignalRiseFall()
            {
                type = SIGNAL_TYPE_RISEFALL;

                tDelay = 0;
                tDelayRise = 2;
                tRampRise = 0.3;
                tDelayFall = 4;
                tRampFall = 0.3;

                yMax = 1;
                yMin = -1;
            }
        };

        struct SignalSinDamp
        {
            SignalType type;

            double  freqHz;
            double  phiRad;
            double  tDelay;

            double  amplit;
            double  offset;
            double  damping;

            SignalSinDamp()
            {
                type = SIGNAL_TYPE_SINDAMP;

                freqHz = 1;
                phiRad = 0;
                tDelay = 0;

                amplit = 1;
                offset = 0;
                damping = 0.5;
            }
        };

        struct SignalSinRise
        {
            SignalType type;

            double  freqHz;
            double  phiRad;
            double  tEnd;
            double  tDelay;

            double  amplit;
            double  offset;
            double  damping;

            SignalSinRise()
            {
                type = SIGNAL_TYPE_SINRISE;

                freqHz = 1;
                phiRad = 0;
                tEnd = 5;
                tDelay = 0;

                amplit = 1;
                offset = 0;
                damping = 0.5;
            }
        };

        struct SignalWavSin
        {
            SignalType type;

            double  freqHz;
            double  phiRad;
            double  tDelay;

            double  amplit;
            double  offset;
            uint8_t index;

            SignalWavSin()
            {
                type = SIGNAL_TYPE_WAVSIN;

                freqHz = 1;
                phiRad = 0;
                tDelay = 0;

                amplit = 1;
                offset = 0;
                index = 19;
            }
        };

        struct SignalAmSin
        {
            SignalType type;

            double  carrierFreqHz;
            double  carrierAmplitude;
            double  carrierOffset;
            double  carrierTDelay;

            double  modulationFreqHz;
            double  modulationPhiRad;
            double  modulationIndex;

            SignalAmSin()
            {
                type = SIGNAL_TYPE_AMSIN;

                carrierFreqHz = 10;
                carrierAmplitude = 1;
                carrierOffset = 0;
                carrierTDelay = 0;

                modulationFreqHz = 2;
                modulationPhiRad = 0;
                modulationIndex = 0.7;
            }
        };

        struct SignalSinDampSin
        {
            SignalType type;

            double  freqSinHz;
            double  tPeriodEnv;
            double  tDelay;

            double  amplit;
            double  offset;
            int8_t  dampingType;

            SignalSinDampSin()
            {
                type = SIGNAL_TYPE_SINDAMPSIN;

                freqSinHz = 1;
                tPeriodEnv = 5;
                tDelay = 0;

                amplit = 1;
                offset = 0;
                dampingType = 1;
            }
        };

        struct SignalTrapDampSin
        {
            SignalType type;

            double  tPeriod;
            double  tRise;
            double  tWidth;
            double  tFall;
            double  tDelay;
            double  tCross;
            double  freqHz;

            double  amplit;
            double  offset;

            SignalTrapDampSin()
            {
                type = SIGNAL_TYPE_TRAPDAMPSIN;

                tPeriod = 1;
                tRise = 0.15;
                tWidth = 0.5;
                tFall = 0.15;
                tDelay = 0;
                tCross = 15;
                freqHz = 10;

                amplit = 1;
                offset = 0;
            }
        };

        typedef enum
        {
            NOISE_TYPE_DEK,
            NOISE_TYPE_NAG
        }NoiseType;

        struct SignalNoise
        {
            SignalType type;

            NoiseType   noiseType;
            double      gamma;
            double      tDelay;

            double      amplit;
            double      offset;

            SignalNoise()
            {
                type = SIGNAL_TYPE_NOISE;

                noiseType = NOISE_TYPE_DEK;
                gamma = 0;
                tDelay = 0;

                amplit = 0.1;
                offset = 0;
            }
        };

        struct SignalSmc
        {
            SignalType type;

            static constexpr double MAX_SCALE_ACCEL_MS2 = 11.768; // equivalent to 1.2g

            double              maxAccelMs2;    // m/s2
            uint16_t            nrPoints;
            double              sps;
            std::vector<double> accelDataVec;   // m/s2

            SignalSmc()
            {
                type = SIGNAL_TYPE_SMC;

                maxAccelMs2 = 0;
                nrPoints = 0;
                sps = 0;
            }
        };


    //************************************************************************
    // functions
    //************************************************************************
    public:
        SignalItem
            (
            SignalTriangle      aSignalData     //!< Triangle signal data
            );

        SignalItem
            (
            SignalRectangle     aSignalData     //!< Rectangle signal data
            );

        SignalItem
            (
            SignalPulse         aSignalData     //!< Pulse signal data
            );

        SignalItem
            (
            SignalRiseFall      aSignalData     //!< RiseFall signal data
            );

        SignalItem
            (
            SignalSinDamp       aSignalData     //!< SinDamp signal data
            );

        SignalItem
            (
            SignalSinRise       aSignalData     //!< SinRise signal data
            );

        SignalItem
            (
            SignalWavSin        aSignalData     //!< WavSin signal data
            );

        SignalItem
            (
            SignalAmSin         aSignalData     //!< AmSin signal data
            );

        SignalItem
            (
            SignalSinDampSin    aSignalData     //!< SinDampSin signal data
            );

        SignalItem
            (
            SignalTrapDampSin   aSignalData     //!< TrapDampSin signal data
            );

        SignalItem
            (
            SignalNoise         aSignalData     //!< Noise signal data
            );

        SignalItem
            (
            SignalSmc           aSignalData     //!< SMC signal data
            );


        SignalTriangle      getSignalDataTriangle() const;
        SignalRectangle     getSignalDataRectangle() const;
        SignalPulse         getSignalDataPulse() const;
        SignalRiseFall      getSignalDataRiseFall() const;
        SignalSinDamp       getSignalDataSinDamp() const;
        SignalSinRise       getSignalDataSinRise() const;
        SignalWavSin        getSignalDataWavSin() const;
        SignalAmSin         getSignalDataAmSin() const;
        SignalSinDampSin    getSignalDataSinDampSin() const;
        SignalTrapDampSin   getSignalDataTrapDampSin() const;
        SignalNoise         getSignalDataNoise() const;
        SignalSmc           getSignalDataSmc() const;

        SignalType          getType() const;

    private:
        void cleanDataStructures();


    //************************************************************************
    // variables
    //************************************************************************
    private:
        SignalTriangle          mSignalDataTriangle;            //!< data for Triangle signal type
        SignalRectangle         mSignalDataRectangle;           //!< data for Rectangle signal type
        SignalPulse             mSignalDataPulse;               //!< data for Pulse signal type
        SignalRiseFall          mSignalDataRiseFall;            //!< data for RiseFall signal type
        SignalSinDamp           mSignalDataSinDamp;             //!< data for SinDamp signal type
        SignalSinRise           mSignalDataSinRise;             //!< data for SinRise signal type
        SignalWavSin            mSignalDataWavSin;              //!< data for WavSin signal type
        SignalAmSin             mSignalDataAmSin;               //!< data for AmSin signal type
        SignalSinDampSin        mSignalDataSinDampSin;          //!< data for SinDampSin signal type
        SignalTrapDampSin       mSignalDataTrapDampSin;         //!< data for TrapDampSin signal type
        SignalNoise             mSignalDataNoise;               //!< data for Noise signal type       
        SignalSmc               mSignalDataSmc;                 //!< data for SMC signal type

        SignalType              mType = SIGNAL_TYPE_INVALID;    //!< default signal type format
};

#endif // SignalItem_h
