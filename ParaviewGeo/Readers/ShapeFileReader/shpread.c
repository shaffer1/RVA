/**************************************************************************
 * shpread.c  a subset of the code provided in shapelib-1.2.10\shpopen.c
 * extracted 20 Sep 2007 - for MIRARCO, B.Anderson
 **************************************************************************
 * $Id: shpopen.c,v 1.39 2002/08/26 06:46:56 warmerda Exp $
 *
 * Project:  Shapelib
 * Purpose:  Implementation of core Shapefile read/write functions.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 **************************************************************************
 * Copyright (c) 1999, 2001, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **************************************************************************
 * <see shpopen.c for original revision history>
 */

#include "shapefil.h"

#include <math.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

#if UINT_MAX == 65535
typedef long      int32;
#else
typedef int       int32;
#endif

static int  bBigEndian;

/************************************************************************/
/*                              SwapWord()                              */
/*                                                                      */
/*      Swap a 2, 4 or 8 byte word.                                     */
/************************************************************************/

static void SwapWord( int length, void * wordP )
{
    int   i;
    uchar temp;

    for( i=0; i < length/2; i++ )
    {
      temp = ((uchar *) wordP)[i];
      ((uchar *)wordP)[i] = ((uchar *) wordP)[length-i-1];
      ((uchar *) wordP)[length-i-1] = temp;
    }
}

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/

static void * SfRealloc( void * pMem, int nNewSize )
{
    if( pMem == NULL )
        return( (void *) malloc(nNewSize) );
    else
        return( (void *) realloc(pMem,nNewSize) );
}

/************************************************************************/
/*                              SHPOpen()                               */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name. Return the SHPInfo pointer.          */
/************************************************************************/

SHPInfo* SHPOpen( const char * pszLayer, const char * pszAccess )
{
    char    *pszFullname, *pszBasename;
    SHPHandle   psSHP;

    uchar   *pabyBuf;
    int     i;
    double    dValue;

/* -------------------------------------------------------------------- */
/*      Ensure the access string is one of the legal ones.  We          */
/*      ensure the result string indicates binary to avoid common       */
/*      problems on Windows.                                            */
/* -------------------------------------------------------------------- */
    if( strcmp(pszAccess,"rb+") == 0 || strcmp(pszAccess,"r+b") == 0
        || strcmp(pszAccess,"r+") == 0 )
        pszAccess = "r+b";
    else
        pszAccess = "rb";

/* -------------------------------------------------------------------- */
/*  Establish the byte order on this machine.     */
/* -------------------------------------------------------------------- */
    i = 1;
    if( *((uchar *) &i) == 1 )
        bBigEndian = 0; // false;
    else
        bBigEndian = 1; // true;

/* -------------------------------------------------------------------- */
/*  Initialize the info structure.          */
/* -------------------------------------------------------------------- */
    psSHP = (SHPHandle) calloc(sizeof(SHPInfo),1);

    psSHP->bUpdated = 0;  // false;

/* -------------------------------------------------------------------- */
/*  Compute the base (layer) name.  If there is any extension */
/*  on the passed in filename we will strip it off.     */
/* -------------------------------------------------------------------- */
    pszBasename = (char *) malloc(strlen(pszLayer)+5);
    strcpy( pszBasename, pszLayer );
    for( i = (int)strlen(pszBasename)-1;
   i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
         && pszBasename[i] != '\\';
   i-- ) {}

    if( pszBasename[i] == '.' )
        pszBasename[i] = '\0';

/* -------------------------------------------------------------------- */
/*  Open the .shp and .shx files.  Note that files pulled from  */
/*  a PC to Unix with upper case filenames won't work!    */
/* -------------------------------------------------------------------- */
    pszFullname = (char *) malloc(strlen(pszBasename) + 5);
    sprintf( pszFullname, "%s.shp", pszBasename );
    psSHP->fpSHP = fopen(pszFullname, pszAccess );
    if( psSHP->fpSHP == NULL )
    {
        sprintf( pszFullname, "%s.SHP", pszBasename );
        psSHP->fpSHP = fopen(pszFullname, pszAccess );
    }

    if( psSHP->fpSHP == NULL )
    {
        free( psSHP );
        free( pszBasename );
        free( pszFullname );
        return( NULL );
    }

    sprintf( pszFullname, "%s.shx", pszBasename );
    psSHP->fpSHX = fopen(pszFullname, pszAccess );
    if( psSHP->fpSHX == NULL )
    {
        sprintf( pszFullname, "%s.SHX", pszBasename );
        psSHP->fpSHX = fopen(pszFullname, pszAccess );
    }

    if( psSHP->fpSHX == NULL )
    {
        fclose( psSHP->fpSHP );
        free( psSHP );
        free( pszBasename );
        free( pszFullname );
        return( NULL );
    }

    free( pszFullname );
    free( pszBasename );

/* -------------------------------------------------------------------- */
/*  Read the file size from the SHP file.       */
/* -------------------------------------------------------------------- */
    pabyBuf = (uchar *) malloc(100);
    fread( pabyBuf, 100, 1, psSHP->fpSHP );

    psSHP->nFileSize = (pabyBuf[24] * 256 * 256 * 256
      + pabyBuf[25] * 256 * 256
      + pabyBuf[26] * 256
      + pabyBuf[27]) * 2;

/* -------------------------------------------------------------------- */
/*  Read SHX file Header info                                           */
/* -------------------------------------------------------------------- */
    fread( pabyBuf, 100, 1, psSHP->fpSHX );

    if( pabyBuf[0] != 0
        || pabyBuf[1] != 0
        || pabyBuf[2] != 0x27
        || (pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d) )
    {
  fclose( psSHP->fpSHP );
  fclose( psSHP->fpSHX );
  free( psSHP );

  return( NULL );
    }

    psSHP->nRecords = pabyBuf[27] + pabyBuf[26] * 256
      + pabyBuf[25] * 256 * 256 + pabyBuf[24] * 256 * 256 * 256;
    psSHP->nRecords = (psSHP->nRecords*2 - 100) / 8;

    psSHP->nShapeType = pabyBuf[32];

    if( psSHP->nRecords < 0 || psSHP->nRecords > 256000000 )
    {
        /* this header appears to be corrupt.  Give up. */
  fclose( psSHP->fpSHP );
  fclose( psSHP->fpSHX );
  free( psSHP );

  return( NULL );
    }

/* -------------------------------------------------------------------- */
/*      Read the bounds.                                                */
/* -------------------------------------------------------------------- */
    if( bBigEndian ) SwapWord( 8, pabyBuf+36 );
    memcpy( &dValue, pabyBuf+36, 8 );
    psSHP->adBoundsMin[0] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+44 );
    memcpy( &dValue, pabyBuf+44, 8 );
    psSHP->adBoundsMin[1] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+52 );
    memcpy( &dValue, pabyBuf+52, 8 );
    psSHP->adBoundsMax[0] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+60 );
    memcpy( &dValue, pabyBuf+60, 8 );
    psSHP->adBoundsMax[1] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+68 );   /* z */
    memcpy( &dValue, pabyBuf+68, 8 );
    psSHP->adBoundsMin[2] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+76 );
    memcpy( &dValue, pabyBuf+76, 8 );
    psSHP->adBoundsMax[2] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+84 );   /* z */
    memcpy( &dValue, pabyBuf+84, 8 );
    psSHP->adBoundsMin[3] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+92 );
    memcpy( &dValue, pabyBuf+92, 8 );
    psSHP->adBoundsMax[3] = dValue;

    free( pabyBuf );

/* -------------------------------------------------------------------- */
/*  Read the .shx file to get the offsets to each record in   */
/*  the .shp file.              */
/* -------------------------------------------------------------------- */
		psSHP->nMaxRecords = (psSHP->nRecords>1)?psSHP->nRecords:1;

    psSHP->panRecOffset =
        (int *) malloc(sizeof(int) * psSHP->nMaxRecords );
    psSHP->panRecSize =
        (int *) malloc(sizeof(int) * psSHP->nMaxRecords );

    pabyBuf = (uchar *) malloc(8 * psSHP->nMaxRecords );
    fread( pabyBuf, 8, psSHP->nRecords, psSHP->fpSHX );

    for( i = 0; i < psSHP->nRecords; i++ )
    {
  int32   nOffset, nLength;

  memcpy( &nOffset, pabyBuf + i * 8, 4 );
  if( !bBigEndian ) SwapWord( 4, &nOffset );

  memcpy( &nLength, pabyBuf + i * 8 + 4, 4 );
  if( !bBigEndian ) SwapWord( 4, &nLength );

  psSHP->panRecOffset[i] = nOffset*2;
  psSHP->panRecSize[i] = nLength*2;
    }
    free( pabyBuf );

    return( psSHP );
}

/************************************************************************/
/*                              SHPClose()                              */
/*                        */
/*  Close the .shp and .shx files.          */
/************************************************************************/

void SHPClose(SHPHandle psSHP )
{
/* -------------------------------------------------------------------- */
/*      Free all resources, and close files.                            */
/* -------------------------------------------------------------------- */
    free( psSHP->panRecOffset );
    free( psSHP->panRecSize );

    fclose( psSHP->fpSHX );
    fclose( psSHP->fpSHP );

    if( psSHP->pabyRec != NULL )
    {
        free( psSHP->pabyRec );
    }
    free( psSHP );
}

/************************************************************************/
/*                          SHPReadObject()                             */
/*                                                                      */
/*      Read the vertices, parts, and other non-attribute information   */
/*  for one shape.  hEntity - RecIndex.  Return the SHPObject pointer.  */
/************************************************************************/

SHPObject* SHPReadObject( SHPHandle psSHP, int hEntity )
{
    SHPObject   *psShape;

/* -------------------------------------------------------------------- */
/*      Validate the record/entity number.                              */
/* -------------------------------------------------------------------- */
    if( hEntity < 0 || hEntity >= psSHP->nRecords )
        return( NULL );

/* -------------------------------------------------------------------- */
/*      Ensure our record buffer is large enough.                       */
/* -------------------------------------------------------------------- */
    if( psSHP->panRecSize[hEntity]+8 > psSHP->nBufSize )
    {
  psSHP->nBufSize = psSHP->panRecSize[hEntity]+8;
  psSHP->pabyRec = (uchar *) SfRealloc(psSHP->pabyRec,psSHP->nBufSize);
    }

/* -------------------------------------------------------------------- */
/*      Read the record.                                                */
/* -------------------------------------------------------------------- */
    fseek( psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0 );
    fread( psSHP->pabyRec, psSHP->panRecSize[hEntity]+8, 1, psSHP->fpSHP );

/* -------------------------------------------------------------------- */
/*  Allocate and minimally initialize the object.     */
/* -------------------------------------------------------------------- */
    psShape = (SHPObject *) calloc(1,sizeof(SHPObject));
    psShape->nShapeId = hEntity;

    memcpy( &psShape->nSHPType, psSHP->pabyRec + 8, 4 );
    if( bBigEndian ) SwapWord( 4, &(psShape->nSHPType) );

/* ==================================================================== */
/*  Extract vertices for a Polygon or Arc.        */
/* ==================================================================== */
    if( psShape->nSHPType == SHPT_POLYGON || psShape->nSHPType == SHPT_ARC
        || psShape->nSHPType == SHPT_POLYGONZ
        || psShape->nSHPType == SHPT_POLYGONM
        || psShape->nSHPType == SHPT_ARCZ
        || psShape->nSHPType == SHPT_ARCM
        || psShape->nSHPType == SHPT_MULTIPATCH )
    {
  int32   nPoints, nParts;
  int       i, nOffset;

/* -------------------------------------------------------------------- */
/*  Get the X/Y bounds.           */
/* -------------------------------------------------------------------- */
        memcpy( &(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8 );
        memcpy( &(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8 );
        memcpy( &(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8 );
        memcpy( &(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8 );

  if( bBigEndian ) SwapWord( 8, &(psShape->dfXMin) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfYMin) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfXMax) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfYMax) );

/* -------------------------------------------------------------------- */
/*      Extract part/point count, and build vertex and part arrays      */
/*      to proper size.                                                 */
/* -------------------------------------------------------------------- */
  memcpy( &nPoints, psSHP->pabyRec + 40 + 8, 4 );
  memcpy( &nParts, psSHP->pabyRec + 36 + 8, 4 );

  if( bBigEndian ) SwapWord( 4, &nPoints );
  if( bBigEndian ) SwapWord( 4, &nParts );

  psShape->nVertices = nPoints;
        psShape->padfX = (double *) calloc(nPoints,sizeof(double));
        psShape->padfY = (double *) calloc(nPoints,sizeof(double));
        psShape->padfZ = (double *) calloc(nPoints,sizeof(double));
        psShape->padfM = (double *) calloc(nPoints,sizeof(double));

  psShape->nParts = nParts;
        psShape->panPartStart = (int *) calloc(nParts,sizeof(int));
        psShape->panPartType = (int *) calloc(nParts,sizeof(int));

        for( i = 0; i < nParts; i++ )
            psShape->panPartType[i] = SHPP_RING;

/* -------------------------------------------------------------------- */
/*      Copy out the part array from the record.                        */
/* -------------------------------------------------------------------- */
  memcpy( psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts );
  for( i = 0; i < nParts; i++ )
  {
      if( bBigEndian ) SwapWord( 4, psShape->panPartStart+i );
  }

  nOffset = 44 + 8 + 4*nParts;

/* -------------------------------------------------------------------- */
/*      If this is a multipatch, we will also have parts types.         */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_MULTIPATCH )
        {
            memcpy( psShape->panPartType, psSHP->pabyRec + nOffset, 4*nParts );
            for( i = 0; i < nParts; i++ )
            {
                if( bBigEndian ) SwapWord( 4, psShape->panPartType+i );
            }

            nOffset += 4*nParts;
        }

/* -------------------------------------------------------------------- */
/*      Copy out the vertices from the record.                          */
/* -------------------------------------------------------------------- */
  for( i = 0; i < nPoints; i++ )
  {
      memcpy(psShape->padfX + i,
       psSHP->pabyRec + nOffset + i * 16,
       8 );

      memcpy(psShape->padfY + i,
       psSHP->pabyRec + nOffset + i * 16 + 8,
       8 );

      if( bBigEndian ) SwapWord( 8, psShape->padfX + i );
      if( bBigEndian ) SwapWord( 8, psShape->padfY + i );
  }

        nOffset += 16*nPoints;

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_POLYGONZ
            || psShape->nSHPType == SHPT_ARCZ
            || psShape->nSHPType == SHPT_MULTIPATCH )
        {
            memcpy( &(psShape->dfZMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMax) );

            for( i = 0; i < nPoints; i++ )
            {
                memcpy( psShape->padfZ + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
            }

            nOffset += 16 + 8*nPoints;
        }

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints )
        {
            memcpy( &(psShape->dfMMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMax) );

            for( i = 0; i < nPoints; i++ )
            {
                memcpy( psShape->padfM + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfM + i );
            }
        }

    }

/* ==================================================================== */
/*  Extract vertices for a MultiPoint.          */
/* ==================================================================== */
    else if( psShape->nSHPType == SHPT_MULTIPOINT
             || psShape->nSHPType == SHPT_MULTIPOINTM
             || psShape->nSHPType == SHPT_MULTIPOINTZ )
    {
  int32   nPoints;
  int       i, nOffset;

  memcpy( &nPoints, psSHP->pabyRec + 44, 4 );
  if( bBigEndian ) SwapWord( 4, &nPoints );

  psShape->nVertices = nPoints;
        psShape->padfX = (double *) calloc(nPoints,sizeof(double));
        psShape->padfY = (double *) calloc(nPoints,sizeof(double));
        psShape->padfZ = (double *) calloc(nPoints,sizeof(double));
        psShape->padfM = (double *) calloc(nPoints,sizeof(double));

  for( i = 0; i < nPoints; i++ )
  {
      memcpy(psShape->padfX+i, psSHP->pabyRec + 48 + 16 * i, 8 );
      memcpy(psShape->padfY+i, psSHP->pabyRec + 48 + 16 * i + 8, 8 );

      if( bBigEndian ) SwapWord( 8, psShape->padfX + i );
      if( bBigEndian ) SwapWord( 8, psShape->padfY + i );
  }

        nOffset = 48 + 16*nPoints;

/* -------------------------------------------------------------------- */
/*  Get the X/Y bounds.           */
/* -------------------------------------------------------------------- */
        memcpy( &(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8 );
        memcpy( &(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8 );
        memcpy( &(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8 );
        memcpy( &(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8 );

  if( bBigEndian ) SwapWord( 8, &(psShape->dfXMin) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfYMin) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfXMax) );
  if( bBigEndian ) SwapWord( 8, &(psShape->dfYMax) );

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_MULTIPOINTZ )
        {
            memcpy( &(psShape->dfZMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMax) );

            for( i = 0; i < nPoints; i++ )
            {
                memcpy( psShape->padfZ + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
            }

            nOffset += 16 + 8*nPoints;
        }

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( psSHP->panRecSize[hEntity]+8 >= nOffset + 16 + 8*nPoints )
        {
            memcpy( &(psShape->dfMMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMax) );

            for( i = 0; i < nPoints; i++ )
            {
                memcpy( psShape->padfM + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfM + i );
            }
        }
    }

/* ==================================================================== */
/*      Extract vertices for a point.                                   */
/* ==================================================================== */
    else if( psShape->nSHPType == SHPT_POINT
             || psShape->nSHPType == SHPT_POINTM
             || psShape->nSHPType == SHPT_POINTZ )
    {
        int nOffset;

  psShape->nVertices = 1;
        psShape->padfX = (double *) calloc(1,sizeof(double));
        psShape->padfY = (double *) calloc(1,sizeof(double));
        psShape->padfZ = (double *) calloc(1,sizeof(double));
        psShape->padfM = (double *) calloc(1,sizeof(double));

  memcpy( psShape->padfX, psSHP->pabyRec + 12, 8 );
  memcpy( psShape->padfY, psSHP->pabyRec + 20, 8 );

  if( bBigEndian ) SwapWord( 8, psShape->padfX );
  if( bBigEndian ) SwapWord( 8, psShape->padfY );

        nOffset = 20 + 8;

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_POINTZ )
        {
            memcpy( psShape->padfZ, psSHP->pabyRec + nOffset, 8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfZ );

            nOffset += 8;
        }

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( psSHP->panRecSize[hEntity]+8 >= nOffset + 8 )
        {
            memcpy( psShape->padfM, psSHP->pabyRec + nOffset, 8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfM );
        }

/* -------------------------------------------------------------------- */
/*      Since no extents are supplied in the record, we will apply      */
/*      them from the single vertex.                                    */
/* -------------------------------------------------------------------- */
        psShape->dfXMin = psShape->dfXMax = psShape->padfX[0];
        psShape->dfYMin = psShape->dfYMax = psShape->padfY[0];
        psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
        psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
    }
    return( psShape );
}

/************************************************************************/
/*                          SHPDestroyObject()                          */
/************************************************************************/

void SHPDestroyObject( SHPObject * psShape )

{
    if( psShape == NULL )
        return;

    if( psShape->padfX != NULL )
        free( psShape->padfX );
    if( psShape->padfY != NULL )
        free( psShape->padfY );
    if( psShape->padfZ != NULL )
        free( psShape->padfZ );
    if( psShape->padfM != NULL )
        free( psShape->padfM );

    if( psShape->panPartStart != NULL )
        free( psShape->panPartStart );
    if( psShape->panPartType != NULL )
        free( psShape->panPartType );

    free( psShape );
}
