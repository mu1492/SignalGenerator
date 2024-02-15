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
NoisePwrSpectrum.cpp
This file contains the sources for the noise power spectral density.
*/

#include "NoisePwrSpectrum.h"

#include <algorithm>
#include <cmath>


//!************************************************************************
//! Constructor
//!************************************************************************
NoisePwrSpectrum::NoisePwrSpectrum
    (
    double aGamma   //!< frequency exponent
    )
{
    mGamma = 0;

    if( GAMMA_MIN <= aGamma
     && aGamma <= GAMMA_MAX
      )
    {
        mGamma = aGamma;
    }

    calculateFilterBlockCoeffs();
    updateFilter();
} 


//!************************************************************************
//! Destructor
//!************************************************************************
NoisePwrSpectrum::~NoisePwrSpectrum()
{
}


//!************************************************************************
//! Calculate the IIR filter coefficients for all blocks
//!
//!        1 - b1*z^(-1)   1 - b2*z^(-1)       1 - bN*z^(-1)
//! H(z) = ------------- * ------------- *...* -------------
//!        1 - a1*z^(-1)   1 - a2*z^(-1)       1 - aN*z^(-1)
//!
//! N = NR_OF_FILTER_BLOCKS
//!
//! References:
//! [1] Corsini, G., Saletti, R. - A 1/f^gamma Power Spectrum Noise Sequence Generator,
//!                                IEEE Trans. Instrum. Meas. 37 (4), 1988, pp. 615-619
//! [2] Saletti, R. - A Comparison Between Two Methods to Generate 1/f^gamma Noise,
//!                   Proceedings of the IEEE 74 (11), 1986, pp. 1595-1596
//!
//! @returns: nothing
//!************************************************************************
void NoisePwrSpectrum::calculateFilterBlockCoeffs()
{
    mABlockCoeffVec.clear();
    mBBlockCoeffVec.clear();

    mABlockCoeffVec.resize( NR_OF_FILTER_BLOCKS );
    mBBlockCoeffVec.resize( NR_OF_FILTER_BLOCKS );

    // pole density == number of poles / decade, see Fig. 2 in [2]
    const double POLE_DENSITY = 1.1;

    // last zero constant = 0.30103
    const double C = -log10( 0.5 );

    for( int i = 1; i <= NR_OF_FILTER_BLOCKS; i++ )
    {
        double e = ( i - NR_OF_FILTER_BLOCKS ) / POLE_DENSITY - 0.5 * mGamma / POLE_DENSITY - C;
        double a = exp( -2.0 * M_PI * pow( 10.0, e ) );
        e +=  0.5 * mGamma / POLE_DENSITY;
        double b = exp( -2.0 * M_PI * pow( 10.0, e ) );

        mABlockCoeffVec[i - 1] = a;
        mBBlockCoeffVec[i - 1] = b;
    }
}


//!************************************************************************
//! Calculate the filter coefficients in polynomial form for N=7
//!
//!        B0 + B1*z^(-1) + ... + B7*z^(-1)
//! H(z) = --------------------------------
//!        A0 + A1*z^(-1) + ... + A7*z^(-1)
//!
//! @returns: nothing
//!************************************************************************
void NoisePwrSpectrum::calculateFilterPolyCoeffs
    (
    const std::vector<double>   aBlockCoeffVec,     //!< filter coefficients / block
    std::vector<double>&        aPolyCoeffVec       //!< filter coefficients / poly
    )
{
    std::vector<double> b( aBlockCoeffVec.size() + 1 );

    for( size_t i = 0; i < aBlockCoeffVec.size(); i++ )
    {
        b[i + 1] = aBlockCoeffVec[i];
    }

    //********
    // 0
    //********
    aPolyCoeffVec[0] = 1;

    //********
    // 1
    //********
    double s12 = b[1] + b[2];
    double s13 = s12 + b[3];
    double s14 = s13 + b[4];
    double s15 = s14 + b[5];
    double s16 = s15 + b[6];
    double s17 = s16 + b[7];

    aPolyCoeffVec[1] = -s17;

    //********
    // 2
    //********
    double b1b2 = b[1] * b[2];
    double b1b3 = b[1] * b[3];
    double b1b4 = b[1] * b[4];
    double b1b5 = b[1] * b[5];
    double b1b6 = b[1] * b[6];
    double b1b7 = b[1] * b[7];

    double b2b3 = b[2] * b[3];
    double b2b4 = b[2] * b[4];
    double b2b5 = b[2] * b[5];
    double b2b6 = b[2] * b[6];
    double b2b7 = b[2] * b[7];

    double b3b4 = b[3] * b[4];
    double b3b5 = b[3] * b[5];
    double b3b6 = b[3] * b[6];
    double b3b7 = b[3] * b[7];

    double b4b5 = b[4] * b[5];
    double b4b6 = b[4] * b[6];
    double b4b7 = b[4] * b[7];

    double b5b6 = b[5] * b[6];
    double b5b7 = b[5] * b[7];

    double b6b7 = b[6] * b[7];

    aPolyCoeffVec[2] = b1b2 + b1b3 + b1b4 + b1b5 + b1b6 + b1b7
                            + b2b3 + b2b4 + b2b5 + b2b6 + b2b7
                                   + b3b4 + b3b5 + b3b6 + b3b7
                                          + b4b5 + b4b6 + b4b7
                                                 + b5b6 + b5b7
                                                        + b6b7;
    //********
    // 3
    //********
    double s67 = b[6] + b[7];
    double s57 = b[5] + s67;
    double s47 = b[4] + s57;
    double s37 = b[3] + s47;

    double tmp = s37 * b1b2
               + s47 * b[3] * s12
               + s57 * b[4] * s13
               + s67 * b[5] * s14
               + b6b7 * s15;

    aPolyCoeffVec[3] = -tmp;

    //********
    // 4
    //********
    double sTmp5 = b6b7;
    tmp = b1b5 * sTmp5;
    double sTmp4 = b[5] * s67 + sTmp5;
    tmp += b1b4 * sTmp4;
    double sTmp3 = b[4] * s57 + sTmp4;
    tmp += b1b3 * sTmp3;
    double sTmp2 = b[3] * s47 + sTmp3;
    tmp += b1b2 * sTmp2;

    tmp += b2b5 * sTmp5;
    tmp += b2b4 * sTmp4;
    tmp += b2b3 * sTmp3;

    tmp += b3b5 * sTmp5;
    tmp += b3b4 * sTmp4;

    tmp += b4b5 * sTmp5;

    aPolyCoeffVec[4] = tmp;

    //********
    // 5
    //********
    double sTmpx3 = b4b5 * s67 + b4b6 * b[7]
                               + b5b6 * b[7];

    tmp = b1b2 * ( b3b4 * s57 + b3b5 * s67 + b3b6 * b[7] + sTmpx3 )
        + b1b3 * sTmpx3
        + b1b4 * b5b6 * b[7]
        + b2b3 * sTmpx3
        + b4b5 * b6b7 * ( b[2] + b[3] );

    aPolyCoeffVec[5] = -tmp;

    //********
    // 6
    //********
    tmp = b1b2 * b3b4 * b5b6
        + b1b2 * b3b4 * b5b7
        + b1b2 * b3b4 * b6b7
        + b1b2 * b3b5 * b6b7
        + b1b2 * b4b5 * b6b7
        + b1b3 * b4b5 * b6b7
        + b2b3 * b4b5 * b6b7;

    aPolyCoeffVec[6] = tmp;

    //********
    // 7
    //********
    tmp = b1b2 * b3b4 * b5b6 * b[7];

    aPolyCoeffVec[7] = -tmp;
}


//!************************************************************************
//! Filter the provided signal
//!
//! @returns: nothing
//!************************************************************************
void NoisePwrSpectrum::filterData
    (
    const std::vector<double>   aInSignal,       //!< input signal
    std::vector<double>&        aOutSignal       //!< output signal
    ) const
{
    size_t nrPoints = aInSignal.size();
    
    if( nrPoints )
    {
        std::vector<double> coeffVec( 1 + mFilter.N );
        
        for( size_t i = 0; i < nrPoints; i++ )
        {
            if( 0 == i % 512 )
            {
                std::fill( coeffVec.begin(), coeffVec.end(), 0 );
            }

            int j = 0;
            
            for( j = mFilter.N; j >= 1; j-- )
            {
                coeffVec[j] = coeffVec[j - 1];
            }

            coeffVec[0] = aInSignal[i];

            for( j = 1; j <= mFilter.N; j++ )
            {
                coeffVec[0] -= mFilter.a[j] * coeffVec[j];
            }

            aOutSignal[i] = 0;
    
            for( j = 0; j <= mFilter.N; j++ )
            {
                aOutSignal[i] += mFilter.b[j] * coeffVec[j];
            }
        }
    }
}


//!************************************************************************
//! Set the frequency exponent
//!
//! @returns: nothing
//!************************************************************************
void NoisePwrSpectrum::setGamma
    (
    const double aGamma         //!< frequency exponent
    )
{
    if( GAMMA_MIN <= aGamma
     && aGamma <= GAMMA_MAX
     )
    {
        mGamma = aGamma;

        calculateFilterBlockCoeffs();
        updateFilter();
    }
}


//!************************************************************************
//! Update the filter parameters
//!
//! @returns: nothing
//!************************************************************************
void NoisePwrSpectrum::updateFilter()
{
    std::vector<double> B( NR_OF_FILTER_BLOCKS + 1 );
    std::vector<double> A( NR_OF_FILTER_BLOCKS + 1 );

    calculateFilterPolyCoeffs( mBBlockCoeffVec, B );
    calculateFilterPolyCoeffs( mABlockCoeffVec, A );

    double nCoeff = 1;

    // N=7, h=1.1, c=0.30103, f=[20Hz..22.05kHz]
    if( mGamma < 0 )
    {
        nCoeff = 1 + 0.39 * pow( fabs( mGamma ), 1.35845 );
    }
    else if( mGamma > 0 )
    {
        nCoeff = 1 + 19 * pow( mGamma, 4.39232 );
    }

    for( size_t i = 0; i < B.size(); i++ )
    {
        B[i] /= nCoeff;
    }

    mFilter.b = B;
    mFilter.a = A;   
}
