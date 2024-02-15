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
NoisePwrSpectrum.h
This file contains the definitions for the noise power spectral density.
*/

#ifndef NoisePwrSpectrum_h
#define NoisePwrSpectrum_h

#include <vector>
#include <cstdint>


//************************************************************************
// Class for handling the noise power spectral density computations
//************************************************************************
class NoisePwrSpectrum
{
    //************************************************************************
    // constants and types
    //************************************************************************
    public:
        // frequency exponent:   -2 <= gamma <= 2
        //
        // Noise Power Spectral Density - particular cases
        // gamma |  noise  |    slope
        //-----------------------------------
        //   -2  |  violet | +20 dB / decade
        //   -1  |  blue   | +10 dB / decade
        //    0  |  white  |   0 dB / decade
        //    1  |  pink   | -10 dB / decade
        //    2  |  brown  | -20 dB / decade

        static constexpr double GAMMA_MIN = -2;     //!< minimum frequency exponent
        static constexpr double GAMMA_MAX = 2;      //!< maxium frequency exponent

    private:
        static const int NR_OF_FILTER_BLOCKS = 7;    //!< N = number of digital filter blocks

        struct DigitalFilter
        {
            int                     N;      //!< degree
            std::vector<double>     b;      //!< poly filter coeffs (numerator)
            std::vector<double>     a;      //!< poly filter coeffs (denominator)

            DigitalFilter()
            {
                N = NR_OF_FILTER_BLOCKS;
            }
        };


    //************************************************************************
    // functions
    //************************************************************************
    public:
        NoisePwrSpectrum
            (
            double aGamma   //!< frequency exponent
            );

        ~NoisePwrSpectrum();
    
        void filterData
            (
            const std::vector<double>   aInSignal,   //!< input signal
            std::vector<double>&        aOutSignal   //!< output signal
            ) const;

        void setGamma
            (
            const double aGamma         //!< frequency exponent
            );

    private:
        void calculateFilterBlockCoeffs();

        void calculateFilterPolyCoeffs
            (
            const std::vector<double>   aBlockCoeffVec,     //!< block filter coefficients
            std::vector<double>&        aPolyCoeffVec       //!< poly filter coefficients
            );

        void updateFilter();


    //************************************************************************
    // variables
    //************************************************************************
    private:
        double                  mGamma;             //!< frequency exponent

        std::vector<double>     mBBlockCoeffVec;    //!< filter coefficients for all z^(-1) blocks (numerator)
        std::vector<double>     mABlockCoeffVec;    //!< filter coefficients for all z^(-1) blocks (denominator)

        DigitalFilter           mFilter;            //!< filter
};

#endif // NoisePwrSpectrum_h
