
/***************************************************************************/
/*                                                                         */
/*  Professional Adventure Writing System extractor for ngPAWs Superglus   */
/*  Author: YokiYoki                                                       */
/*  Strongly based on UNPAWS by Jose Luis Cebrian & Carlos Sanchez (CS)    */
/*  Altered by Carlos Sanchez                                              */
/*                                                                         */
/***************************************************************************/
/* -DEF- 15/01/03 *001* Add spChar file "spChar.ac"                        */
/* -DEF- 15/01/03 *002* MODE SCE-PC (Second param become a remark)         */
/***************************************************************************/
/* -CS-  15/07/03 *003* Add PAWPC sysmess fill up for PAWPC compliance	   */
/* -CS-  15/07/03 *004* Add spchar.ac existance check					   */		
/* -CS-  15/07/03 *---* Show PAW compression parameter (-a) removed 	   */	
/* -CS-  15/07/03 *005* Bug in MODE condact solved, remark works fine now  */	
/* -CS-  15/07/03 *---* Turn all code to one only language -> English	   */	
/* -CS-  15/07/03 *---* Added several code remarks						   */	
/* -CS-  15/07/03 *---* Removed some useless code						   */
/***************************************************************************/
/* -CS-  23/04/14 *---* Defaults exporting to ngPAWS/Superglus compatible  */
/* -CS-  23/04/14 *---* Added -C parameter to set classic PAW PC format    */
/* -CS-  23/04/14 *---* spchar.ac renamed to spchar.cfg and made optional  */
/***************************************************************************/


#include "z80load.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

/**********************************************************************************/

#define Version "0.5 Beta"
#define MAXTVOC 6 // Maximum vocabulary type


/* Some  type definitions that make easy to translate from original pascal code */
typedef unsigned char spBYTE;
typedef unsigned short spWORD;
//typedef int bool;
#define true (1==1)
#define false (1==0)


/* On WIN32 and DOS compilers strcasecmp does not exist,  stricmp is used instead */
#if (defined(__MSDOS__))||(defined(WIN32))
#define strcasecmp stricmp
#endif


/* Global Vars */
spBYTE Spectrum[65535];							/* The ZX Spectrum 48 Memory */
FILE *fOut;										/* Output file */
FILE *f;										/* Input file */
long fSize;										/* Size of input file */
spWORD vocPtr, conPtr, proPtr, resPtr;			/* Pointer to the different data in the PAW database */
char inputFileName[256];						
char outputFileName[256];
int objexportmode = 0;
spWORD offCompress, offVoc, offCon, offMsg, offSys,
       offLoc, offObj, offWObj, offLObj, offXObj, offPro; /* Offset on Spectrum's memory of the different data */

int numLoc, numMsg, numSys, numObj, numPro; /* Number of locations, messages, sysmessages, objects and processes */


int i, n, p;									/* General use for variables for function main */
char s[256];									/* General use string variable for function main */

unsigned char spChar[255]; /* 001 */ /* Character code conversion array */

typedef struct _Cond	/* Each condact has a name and a number of expected params */
{
	char condact[21];
	int params;
}
Cond;


/* The PAWS' Condacts */

Cond condacts[108] = { 
  {"AT", 1},
  {"NOTAT", 1},
  {"ATGT", 1},
  {"ATLT", 1},
  {"PRESENT", 1},
  {"ABSENT", 1},
  {"WORN", 1},
  {"NOTWORN", 1},
  {"CARRIED", 1},
  {"NOTCARR", 1},
  {"CHANCE", 1},
  {"ZERO", 1},
  {"NOTZERO", 1},
  {"EQ", 2},
  {"GT", 2},
  {"LT", 2},
  {"ADJECT1", 1},
  {"ADVERB", 1},
  {"INVEN", 0},
  {"DESC", 0},
  {"QUIT", 0},
  {"END", 0},
  {"DONE", 0},
  {"OK", 0},
  {"ANYKEY", 0},
  {"SAVE", 0},
  {"LOAD", 0},
  {"TURNS", 0},
  {"SCORE", 0},
  {"CLS", 0},
  {"DROPALL", 0},
  {"AUTOG", 0},
  {"AUTOD", 0},
  {"AUTOW", 0},
  {"AUTOR", 0},
  {"PAUSE", 1},
  {"TIMEOUT", 0},
  {"GOTO", 1},
  {"MESSAGE", 1},
  {"REMOVE", 1},
  {"GET", 1},
  {"DROP", 1},
  {"WEAR", 1},
  {"DESTROY", 1},
  {"CREATE", 1},
  {"SWAP", 2},
  {"PLACE", 2},
  {"SET", 1},
  {"CLEAR", 1},
  {"PLUS", 2},
  {"MINUS", 2},
  {"LET", 2},
  {"NEWLINE", 0},
  {"PRINT", 1},
  {"SYSMESS", 1},
  {"ISAT", 2},
  {"COPYOF", 2},
  {"COPYOO", 2},
  {"COPYFO", 2},
  {"COPYFF", 2},
  {"LISTOBJ", 0},
  {"EXTERN", 1},
  {"RAMSAVE", 0},
  {"RAMLOAD", 1},
  {"BEEP", 2},
  {"PAPER", 1},
  {"INK", 1},
  {"BORDER", 1},
  {"PREP", 1},
  {"NOUN2", 1},
  {"ADJECT2", 1},
  {"ADD", 2},
  {"SUB", 2},
  {"PARSE", 0},
  {"LISTAT", 1},
  {"PROCESS", 1},
  {"SAME", 2},
  {"MES", 1},
  {"CHARSET", 1},
  {"NOTEQ", 2},
  {"NOTSAME", 2},
  {"MODE", 2},
  {"LINE", 1},
  {"TIME", 2},
  {"PICTURE", 1},
  {"DOALL", 1},
  {"PROMPT", 1},
  {"GRAPHIC", 1},
  {"ISNOTAT", 2},
  {"WEIGH", 2},
  {"PUTIN", 2},
  {"TAKEOUT", 2},
  {"NEWTEXT", 0},
  {"ABILITY", 2},
  {"WEIGHT", 1},
  {"RANDOM", 1},
  {"INPUT", 1},
  {"SAVEAT", 0},
  {"BACKAT", 0},
  {"PRINTAT", 2},
  {"WHATO", 0},
  {"RESET", 1},
  {"PUTO", 1},
  {"NOTDONE", 0},
  {"AUTOP", 1},
  {"AUTOT", 1},
  {"MOVE", 1},
  {"PROTECT", 0}
};


/* Vocabulary matrix*/

char tVocs[MAXTVOC+1][256] =
  { "verb", "adverb", "noun", "adjective", "preposition", "conjugation",
"pronoun" };


/**********************************************************************************************************/


int LoadSpChar(void) /* 001 */
/* Loads Character Code Conversion Table from file spchar.ac */
{
	FILE *fchar;
	int i;
	char cad[12];


	for (i=0;i<144;i++) spChar[i] = i;
	for (i=143;i<255;i++) spChar[i] = ' '; /* No chars over 144, 144-154 = UDG, >155 = TOKENS*/

	if ((fchar = fopen("spchar.cfg","rt"))==NULL)
	{
		return -1;
	}


	while (!feof(fchar))
	{
		fgets(cad,10,fchar);
		spChar[cad[0]] = cad[2];	/* Easy mode ^_^ */
	}

	fclose (fchar);

	return 1;
}


/**********************************************************************************************************/
char * trim (char *p)
/* returns string without trailing and leading spaces */
{
	while (p[strlen (p) - 1] == ' ')
		p[strlen (p) - 1] = '\0';
	return p;
}

/**********************************************************************************************************/

spWORD dpeek (int c)
/* returns a common Spectrum two-byte value peek(n)+ 256 * peek(n+1) */
{
	return (Spectrum[c] | (Spectrum[c + 1] << 8));
}

/**********************************************************************************************************/

spBYTE peek (int c)
/* Performs and emulated Spectrum Basic 'PEEK' order */
{
	return Spectrum[c];
}

/**********************************************************************************************************/


spBYTE peekNeg (int c)
/* Performs and emulated Spectrum Basic 'PEEK' order and negates the result*/
{
	return Spectrum[c] ^ 255;
}

/**********************************************************************************************************/

char * justify (char *s, unsigned n)
/* Add spaces to the string 's' until the length of the string is n or more, then returns the string */
{
	char res[256];			

	strcpy (res, s);
	while (strlen (res) < n) 
		strcat (res, " ");
	return strdup (res);
}



/**********************************************************************************************************/

char * upSt (char *s)
/* Changes a string to uppercase and returns it */
{
	unsigned i;
	char res[256];			
	
	strcpy (res, s);
	for (i = 0; i < strlen (res); i++)
		res[i] = toupper (res[i]);
	return strdup (res);
}


/**********************************************************************************************************/

char * intToStr (long l)
/* returns a string containing value of a long integer number */
{
	char res[256];			
	sprintf (res, "%d", l);
	return strdup (res);
}

/**********************************************************************************************************/

void error (int n)
/* Shows error mesage and exits with errorlevel=n */
{
	if (n != 0)
		printf ("Error: ");
	
	switch (n)
    {
    case 1:
		printf ("File not found.\n");
		break;
    case 2:
		printf ("Must be a .SP, .SNA or .Z80 file.\n");
		break;
    case 3:
		printf ("It doesn't seem to be a PAW game.\n");
		break;
    case 4:
		printf ("Parameter missing.\n");
		break;
    case 5:
		printf ("The .Z80 format version of this file is not valid. Try using spconv to convert to .SNA or .SP\n");
		break;
    case 6:
		printf ("The .Z80 file is not a 48K snapshot.\n");
		break;
    }
	exit (n);
}

/**********************************************************************************************************/

char * compression (int c)
/* Returns a PAW compressed token whose index is 'c' as a uncompressed string */
{
	spWORD offs;
	char s[256];
	s[0] = '\0';
	offs = offCompress;
	c -= 164;
	while (c != 0)
    {
		while ((Spectrum[offs] & 128) == 0)
			offs++;
		offs++;
		c--;
    }
	
	while ((Spectrum[offs] & 128) == 0)
    {
		s[strlen (s) + 1] = 0;
		s[strlen (s)] = Spectrum[offs];
		offs++;
    }
	s[strlen (s) + 1] = 0;
	s[strlen (s)] = Spectrum[offs] & 127;
	return strdup (s);
};

/**********************************************************************************************************/

char * voctype (int c)
/* Returns 'name' for a concrete vocabulary type */
{
	if ((c >= 0) && (c <= MAXTVOC))
		return tVocs[c];
	else
		return "reserved";
}


/**********************************************************************************************************/

char * VocabularyWord (int num, int thetype)
/* returns word in the vocabulary with that number and type */
{
	spWORD vocPtr;
	char pal[256];
	
	vocPtr = offVoc;
	strcpy (pal, "     ");
	while (peek (vocPtr) != 0)
    {
		if ((peek (vocPtr + 5) == num) && (peek (vocPtr + 6) == thetype))
		{
			pal[0] = peekNeg (vocPtr);
			pal[1] = peekNeg (vocPtr + 1);
			pal[2] = peekNeg (vocPtr + 2);
			pal[3] = peekNeg (vocPtr + 3);
			pal[4] = peekNeg (vocPtr + 4);
			return strdup (pal);
		}
		
		vocPtr += 7;
    }
	return "";
}

/**********************************************************************************************************/




void decode_token (char *s)
{
	spBYTE i;
	
	for (i = 0; i < strlen (s); i++)
    {
		if (s[i] == 7)
			fprintf (fOut, "\n\n");
		else if (((unsigned char) s[i] > 31) && ((unsigned char) s[i] != '^')
			&& ((unsigned char) s[i] != '"')
			&& ((unsigned char) s[i] < 164))
		{
			fprintf (fOut, "%c", spChar[s[i]]); /* 001 */
		}
    }
}

/**********************************************************************************************************/

void additionalSysmess() /*003*/
/* Fills up the system message table up to message 60 to be compliant with PAW for PC */
{
	int i;
	if (numSys < 65) for (i=numSys; i<65; i++) fprintf(fOut, "/%d\n\n", i);
}

/**********************************************************************************************************/

void showmessages (char *titulo, spWORD offTab, int num)
/* Shows a collection of messages, starting from the given offset  and until it shows 'num' messages */
/* This function is used for messages, system messages, object names and locations */
{
	int i;
	spBYTE c;
	spWORD offs;
	
	i = 0;
	while (i < num)
    {
		offs = dpeek (offTab + 2 * i);
		fprintf (fOut, "/%d\n", i);
		i++;
		while ((Spectrum[offs] != (31 ^ 255)) && (offs < 65520))
		{
			c = peekNeg (offs);
			offs++;
			
			if (c == 7)
				fprintf (fOut, "\n\n");
			if ((c > 31) && (c != '^') && (c != '"') && (c < 164))
			{
				fprintf (fOut, "%c", spChar[c]); /* 001 */
			
			}
			else if ((c >= 164) && (c <= 255))
				decode_token (compression (c));
		}
		offs++;
		fprintf (fOut, "\n");
    }
}

/**********************************************************************************************************/



void syntax ()
/* Shows the correct sintax for using sp2sce */
{
	printf ("\nSYNTAX: sp2sce -i<filename> [-o<filename>] [-c]\n\n");
	printf ("-i must be a .SP, .SNA or .Z80 file\n\n");
	printf ("-o Text file where output is written. If you don't include\n");
	printf ("   this parameter result will be sent to standard output.\n\n");
	printf ("-c Force classic PAW PC export format (defaults to ngPAWS/Superglus).\n\n");
	printf ("Example :    sna2sce -iripper1.z80 -oripper1.sce\n\n");
	exit (0);
}

/**********************************************************************************/

void checkParameters (int argc, char *argv[])
/* Checks sintax and shows correct sintax on faliure */
{
	spBYTE i;
	char s[256];
	char opt[3] = { 0, 0, 0 };
	
	if (argc == 0)
		syntax ();
	
	strcpy (inputFileName, "");
	strcpy (outputFileName, "");
	for (i = 1; i < argc; i++)
    {
		strcpy (s, argv[i]);
		if (!strcasecmp (strncpy (opt, s, 2), "-I"))
			strcpy (inputFileName, s + 2);
		if (!strcasecmp (strncpy (opt, s, 2), "-O"))
			strcpy (outputFileName, s + 2);
		if (!strcasecmp (strncpy (opt, s, 2), "-C"))
			objexportmode = 1;
		opt[0] = 0;
		opt[1] = 0;
		opt[2] = 0;
    }
	if (!strcmp (inputFileName, ""))
		syntax ();
}


/**********************************************************************************************************/
/*                                            *   MAIN    *                                               */
/**********************************************************************************************************/


int main (int argc, char *argv[])
{
	struct stat arch;

	

	printf ("SNA2SCE %s\n", Version);

	checkParameters (argc, argv);

	if ((f = fopen (inputFileName, "rb")) == NULL) error (1);
	
	stat (inputFileName, &arch);

	/* Now we are going to load Spectrum Data into memory in different ways depending of the type of snapshot */
	fSize = arch.st_size;
	if (fSize == 49179)
    {
		fseek (f, 7195, 0);
		fread ((spBYTE *) Spectrum + 23552, 41984, 1, f);
    }
	else if (fSize == 49190)
    {
		fseek (f, 7206, 0);
		fread ((spBYTE *) Spectrum + 23552, 41984, 1, f);
    }
	else if ((strstr (inputFileName, ".Z80"))
		|| (strstr (inputFileName, ".z80")))
    {
		switch (loadZ80 (inputFileName, Spectrum))
		{
		case 1:
			error (5);
			break;
		case 2:
			error (6);
		}
    }
	else
		error (2);
	fclose (f);



	/* Check if the game is A PAWed game O' Hacker ;) */
	if (peek (37943) != 16)
		error (3);
	

	/* Sets the output file */
	if (!strcmp (outputFileName, ""))
		fOut = stdout;
	else
		fOut = fopen (outputFileName, "wb");
	


	/* Gets the position of every kind of data in the PAW database from the PAW header */
	offMsg = dpeek (65503U);
	numMsg = peek (37958U);
	offSys = dpeek (65505U);
	numSys = peek (37959U);
	offLoc = dpeek (65501U);
	numLoc = peek (37957U);
	offObj = dpeek (65499U);
	numObj = peek (37956U);
	numPro = peek (37960U);
	offPro = dpeek (65497U);
	offCompress = dpeek (37964U);

	/* tries to load the character code conversion table */

	if (LoadSpChar()==-1) 
	{
		printf("\nWarning: character conversion file spchar.cfg missing.\n");
	}
	
	/* Starting decoding */

	printf ("\nAnalysing %s\n", inputFileName);
	
	/* Control section */
	fprintf (fOut, "/CTL\n_\n");
	
	/* Vocabulary */
	
	printf ("Extracting vocabulary\n");
	
	fprintf (fOut, "\n/VOC\n\n");
	vocPtr = dpeek (65509U);
	offVoc = dpeek (65509U);
	while ((vocPtr < 65509) && (peek (vocPtr) != 0))
    {
		if ((peekNeg (vocPtr) != '_') && (peekNeg (vocPtr) != '*'))
			fprintf (fOut, "\n%c%c%c%c%c %d %s\n", peekNeg (vocPtr),
			peekNeg (vocPtr + 1), peekNeg (vocPtr + 2),
			peekNeg (vocPtr + 3), peekNeg (vocPtr + 4),
			peek (vocPtr + 5), voctype (peek (vocPtr + 6)));
		vocPtr += 7;
    }
	
	/* System messages */	
	printf ("Extracting %d system message(s)\n", numSys);	
	fprintf (fOut, "\n/STX\n\n");
	showmessages ("Message", offSys, numSys);
	additionalSysmess(); /*003*/
	
	/* Messages */	
	printf ("Extracting %d message(s)\n", numMsg);	
	fprintf (fOut, "\n/MTX\n\n");
	showmessages ("Message", offMsg, numMsg);
	
	/* Object names */
	printf ("Extracting %d object(s)\n", numObj);
	fprintf (fOut, "\n/OTX\n\n");
	showmessages ("Object", offObj, numObj);
	
	/* Locations */	
	printf ("Extracting %d location(s)\n", numLoc);
	fprintf (fOut, "\n/LTX\n\n");
	showmessages ("Location", offLoc, numLoc);

  /* Connections */
	printf ("Extracting connections\n");
	fprintf (fOut, "\n/CON\n\n");
	offCon = dpeek (65507U);
	for (i = 0; i < numLoc; i++)
    {
		fprintf (fOut, "\n/%d\n", i);
		conPtr = dpeek (offCon + 2 * i);
		n = 0;
		while (peek (conPtr) != 255)
		{
			strcpy (s, VocabularyWord (peek (conPtr), 0));
			if (!strcmp (s, ""))
				strcpy (s, VocabularyWord (peek (conPtr), 2));
			n++;
			if (s != "")
				fprintf (fOut, "%s %d\n", s, peek (conPtr + 1));
			else
				fprintf (fOut, "%d %d\n", peek (conPtr), peek (conPtr + 1));
			conPtr += 2;
		}
    }
	
	/* Object Words, weigth, etc. */
	printf ("Extracting object spWORDs\n");
	
	fprintf (fOut, "\n/OBJ\n\n");
	offWObj = dpeek (65513U);
	offLObj = dpeek (65511U);
	offXObj = dpeek (65515U);
	for (i = 0; i < numObj; i++)
    {
		fprintf (fOut, "\n/%d ", i);
		switch (peek (offLObj + i))
		{
		case 252:
			fprintf (fOut, "252 ");
			break;
		case 253:
			fprintf (fOut, "WORN ");
			break;
		case 254:
			fprintf (fOut, "CARRIED ");
			break;
		default:
			fprintf (fOut, "%d ", peek (offLObj + i));
			break;
		}
		
		if (objexportmode==1) // Classic
		{
		fprintf (fOut, "%d %s %s ", peek (offXObj + 1) & 63,
			(peek (offXObj + i) & 64) ? "Y" : "_",
			(peek (offXObj + i) & 128) ? "Y" : "_");
		}
		else  // Superglus/ngPAWS
		{
			fprintf (fOut, "%d ", peek (offXObj + 1) & 63);
		}


		strcpy (s, VocabularyWord (peek (offWObj + 2 * i), 2));
		if (!strcmp (s, ""))
			strcpy (s, intToStr (peek (offWObj + 2 * i)));
		if (peek (offWObj + 2 * i) == 255)
			strcpy (s, "_");
		fprintf (fOut, "%s ", s);
		strcpy (s, VocabularyWord (peek (offWObj + 2 * i + 1), 3));
		if (!strcmp (s, ""))
			strcpy (s, intToStr (peek (offWObj + 2 * i + 1)));
		if (peek (offWObj + 2 * i + 1) == 255)
			strcpy (s, "_");
		fprintf (fOut, "%s", s);
		if (objexportmode==0) // Superglus/ngPAWS
		{
			fprintf(fOut, " %s%s%s00000000000000000000000000000 00000000000000000000000000000000", (i==0) ? "1":"0", 
			(peek (offXObj + i) & 128) ? "1" : "0", (peek (offXObj + i) & 64) ? "1" : "0");
		}


		fprintf (fOut, "\n");
    }
	
	/* Processes */
	
	printf ("Extracting response table and processes\n");
	
	for (p = 0; p < numPro; p++)
    {
		fprintf (fOut, "\n/PRO %d\n\n", p);
		proPtr = dpeek (offPro + p * 2);
		while (peek (proPtr) != 0)
		{
			strcpy (s, VocabularyWord (peek (proPtr), 0));
			if (peek (proPtr) == 1)
				strcat (s, "_ ");
			else if (peek (proPtr) == 255)
				strcat (s, "_ ");
			if (!strcmp (s, ""))
			{
				if (peek (proPtr) < 20)
					strcpy (s, VocabularyWord (peek (proPtr), 2));
				if (!strcmp (s, ""))
					strcpy (s, intToStr (peek (proPtr)));
			}
			
			fprintf (fOut, "%s ", s);
			proPtr++;
			strcpy (s, VocabularyWord (peek (proPtr), 2));
			if (peek (proPtr) == 1)
				strcat (s, "_ ");
			if (peek (proPtr) == 255)
				strcat (s, "_ ");
			if (!strcmp (s, ""))
			{
				if (peek (proPtr) < 20)
					strcpy (s, VocabularyWord (peek (proPtr), 2));
				if (!strcmp (s, ""))
					strcpy (s, intToStr (peek (proPtr)));
			}
			fprintf (fOut, "%s\n ", s);			
			proPtr++;
			resPtr = dpeek (proPtr);
			proPtr += 2;
			n = 0;
			
			while (peek (resPtr) != 255)
			{
				if (n != 0)
					fprintf (fOut, " ");
				n++;
				if (peek (resPtr) < 107)
				{
					/* if the condact is MODE the second value has no sense in paguaglus */
					if (!strcmp(condacts[peek (resPtr)].condact,"MODE")) /* 002 */
					{
						fprintf (fOut, "                  %s", condacts[peek (resPtr)].condact); /* 005 */
						fprintf (fOut, " %d", peek (resPtr + 1));
						fprintf (fOut, " ; %d <- ZX Only", peek (resPtr + 2)); 
						resPtr += condacts[peek (resPtr)].params; /* 005 */
					}
					else
					{
						fprintf (fOut, "                  %s", condacts[peek (resPtr)].condact);
						if (condacts[peek (resPtr)].params > 0)
							fprintf (fOut, " %d", peek (resPtr + 1));
						if (condacts[peek (resPtr)].params > 1)
							fprintf (fOut, " %d", peek (resPtr + 2));
						resPtr += condacts[peek (resPtr)].params;
					}
				}
				resPtr++;
				fprintf (fOut, "\n");
			}
			fprintf(fOut, "\n");
		}
    }
	printf ("Done.\n");
	return 0;
}
