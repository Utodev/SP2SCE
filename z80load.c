/* Z80Load */

/***************************************************************************)
(*                                                                         *)
(*  Professional Adventure Writing System extractor.                       *)
(*  Author: Jose Luis Cebri n PagÅe                                        *)
(*  Pascal version : Carlos Sanchez (csanchez@temple.subred.org)           *)
(*                                                                         *)
(***************************************************************************)
(*                                                                         *)
(*  Z80 files support and other minor changes added by Carlos Sanchez      *)
(*  when translating the code from C to pascal                             *)
(*                                                                         *)
(***************************************************************************/

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

/* interface */


#include "z80load.h"

/* implementation */

typedef unsigned char Byte;
typedef Byte *pByte;
typedef unsigned short Word;
typedef Word *pWord;
Word MyOffset;

Byte
getByte (pByte * p, pWord counter)
{
  Byte result;

  result = **p;
  *p = *p + 1;
  *counter = *counter - 1;
  return result;
}

void
decode (void *src, void *dst, Word offset3, Word blockLen)
{
  pByte p0, pF;
  Byte a, b;

  p0 = src;
  pF = (pByte) dst + MyOffset;	/*offset3; */
  do
    {
      a = getByte (&p0, &blockLen);
      if (a != 237)
	{
	  *pF = a;
	  pF++;
	}
      else
	{
	  b = getByte (&p0, &blockLen);
	  if (b != 237)
	    {
	      *pF = 237;
	      pF++;
	      *pF = b;
	      pF++;
	    }
	  else
	    {
	      a = getByte (&p0, &blockLen);
	      b = getByte (&p0, &blockLen);
	      memset (pF, b, a);
	      pF = pF + a;
	    }
	}
    }
  while (blockLen != 0);
}

int
loadZ80 (char *fileName, void *Z80RAM)
{
  FILE *f;
  void *p;
  Word addHeaderSize;
  Word blockLen;
  Byte page;
  Byte hwMode;
  Word offset4;

  int result;

  result = 0;
  f = fopen (fileName, "rb");
  fseek (f, 30, 0);
  fread (&addHeaderSize, 1, 2, f);
  if ((addHeaderSize != 23) && (addHeaderSize != 54))
    return 1;
  fseek (f, 34, 0);
  fread (&hwMode, 1, 1, f);
  if (((addHeaderSize == 23) && (hwMode > 2))
      || ((addHeaderSize == 54) && (hwMode > 3)))
    return 2;
  fseek (f, 32 + addHeaderSize, 0);
  while (!feof (f))
    {
      fread (&blockLen, 1, 2, f);
      fread (&page, 1, 1, f);
      p = malloc (blockLen);
      fread (p, blockLen, 1, f);
      switch (page)
	{
	case 4:
	  offset4 = 32768;
	  break;
	case 5:
	  offset4 = 49152;
	  break;
	case 6:
	  offset4 = 0;
	case 7:
	  break;
	case 8:
	  offset4 = 16384;
	  break;
	default:
	  return 1;
	}
      if (offset4 != 0)
	{
	  offset4 -= 16384;
	  MyOffset = offset4;
	  decode (p, Z80RAM, offset4, blockLen);
	}
    }
  fclose (f);
  return 0;
}
