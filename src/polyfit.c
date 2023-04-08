// Name: polyfit.c
// Description: Simple polynomial fitting functions.
// Author: Henry M. Forson, Melbourne, Florida USA
// Simplified to go in Phobri64 codebase

//------------------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2020 Henry M. Forson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------------

// Define this to print matrix computation results to terminal (probably not very useful)
//#define MATRIX_DEBUGGING

#ifdef MATRIX_DEBUGGING

#include <stdio.h>      // printf()

void reallyShowMatrix(double mat[], int rows, int cols) {
	for( int r = 0; r < rows; r++ ) {
		for( int c = 0; c < cols; c++)
		{
			printf( "   %f", mat[r * cols + c]);
		}
		printf( "\n" );
	}
}

#define showMatrix( x, r, c ) do {\
    printf( "   @%d: " #x " =\n", __LINE__ ); \
    reallyShowMatrix( x, r, c); \
    printf( "\n" ); \
} while( 0 )
#define polyfit_dbg(fmt, args...) printf(fmt, ##args)
#else   // MATRIX_DEBUGGING
#define showMatrix( x, r, c )
#define polyfit_dbg(fmt, args...)
#endif   // MATRIX_DEBUGGING

int polyfit( int pointCount, double *xValues, double *yValues, int coefficientCount, double *coefficientResults )
{
    int rVal = 0;
    int degree = coefficientCount - 1;

    // Check that pointCount >= coefficientCount.
    if(pointCount < coefficientCount)
    {
        return -2;
    }
 
    // Make the A matrix:
    double pMatA [pointCount][coefficientCount];


    for( int r = 0; r < pointCount; r++)
    {
	    double acc = 1;
        for( int c = 0; c < coefficientCount; c++)
        {
            // xValues[r] ^ c
            pMatA[r][degree-c] = acc;
            acc *= xValues[r];
        }
    }

    showMatrix( pMatA, pointCount, coefficientCount );

    double* pMatB = yValues; // 1 col, pointCount rows

    // Make the transpose of matrix A:
    double pMatAT [coefficientCount][pointCount];
    
    for( int r = 0; r < coefficientCount; r++ )
    {
        for( int c = 0; c < pointCount; c++ )
        {
            pMatAT[r][c] = pMatA[c][r];
        }
    }

    showMatrix( pMatAT, coefficientCount, pointCount );

    // Make the product of matrices AT and A:
    double pMatATA[coefficientCount][coefficientCount];
    for( int i = 0; i < coefficientCount; i++)
    {
        for( int j = 0; j < coefficientCount; j++ )
        {
            pMatATA[i][j] = 0.0;
            for( int k = 0; k < pointCount; k++)
            {  
                pMatATA[i][j] += pMatAT[i][k] * pMatA[k][j];
            }
        }
    }

    showMatrix( pMatATA, coefficientCount, coefficientCount );

    // Make the product of matrices AT and b:
    double pMatATB[pointCount];

    for (int i = 0; i < coefficientCount; i++)
    {
        pMatATB[i] = 0.0;
        for (int k = 0; k < pointCount; k++) {
            pMatATB[i] += pMatAT[i][k] * pMatB[k];
        }
    }

    showMatrix( pMatATB, pointCount, 1 );

    // Now we need to solve the system of linear equations,
    // (AT)Ax = (AT)b for "x", the coefficients of the polynomial.

    for( int c = 0; c < coefficientCount; c++ )
    {
        int pr = c;     // pr is the pivot row.
        double prVal = pMatATA[pr][c];
        // If it's zero, we can't solve the equations.
        if( 0.0 == prVal )
        {
            polyfit_dbg( "Unable to solve equations, pr = %d, c = %d.\n", pr, c );
            showMatrix( pMatATA, coefficientCount, coefficientCount);
            rVal = -4;
            break;
        }
        for( int r = 0; r < coefficientCount; r++)
        {
            if( r != pr )
            {
                double targetRowVal = pMatATA[r][c];
                double factor = targetRowVal / prVal;
                for( int c2 = 0; c2 < coefficientCount; c2++ )
                {
                    pMatATA[r][c2] -=  pMatATA[pr][c2] * factor; 
                    polyfit_dbg( "c = %d, pr = %d, r = %d, c2=%d, targetRowVal = %f, prVal = %f, factor = %f.\n",
                             c, pr, r, c2, targetRowVal, prVal, factor );
		            showMatrix( pMatATA, coefficientCount, coefficientCount);                   
                }
                pMatATB[r] -=  pMatATB[pr] * factor;

                showMatrix( pMatATB, pointCount, 1);
            }
        }
    }
    for( int c = 0; c < coefficientCount; c++ )
    {
        int pr = c;
        // now, pr is the pivot row.
        double prVal = pMatATA[pr][c];
        pMatATA[pr][c] /= prVal;
        pMatATB[pr] /= prVal;
    }

    showMatrix( pMatATA, coefficientCount, coefficientCount);

    showMatrix( pMatATB, pointCount, 1);

    for( int i = 0; i < coefficientCount; i++)
    {
        coefficientResults[i] = pMatATB[i];
    }

    return rVal;
}