/**************************************************************
	LZSS.C -- A Data Compression Program
	(tab = 4 spaces)
***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
typedef unsigned long u32;
struct stat finfo;

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

unsigned long int
		fourmbit = 4194304, /* 4mbit rom size */
		textsize = 0,	/* text size counter */
		codesize = 0,	/* code size counter */
		printcount = 0;	/* counter for reporting progress every 1K bytes */
unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */
int		match_position, match_length,  /* of longest match.  These are
			set by the InsertNode() procedure. */
		lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
			parents -- These constitute binary search trees. */
FILE	*infile, *outfile;  /* input & output files */

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

void InsertNode(int r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	int  i, p, cmp;
	unsigned char  *key;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) p = rson[p];
			else {  rson[p] = r;  dad[r] = p;  return;  }
		} else {
			if (lson[p] != NIL) p = lson[p];
			else {  lson[p] = r;  dad[r] = p;  return;  }
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)  break;
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void DeleteNode(int p)  /* deletes node p from tree */
{
	int  q;

	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}





void Encode(void)
// function patched to produce gba bios decompression function processable data
{
	int  i, c, len, r, s, last_match_length, code_buf_ptr;
	unsigned char  code_buf[17], mask;

	// write 32 bit header needed for GBA BIOS function
	// Bit 0-3   Reserved
	// Bit 4-7   Compressed type (must be 1 for LZ77)
	// Bit 8-31  Size of decompressed data

//	u32 gbaheader = 0x10 + (finfo.st_size<<8);
//	u32 gbaheader = finfo.st_size/fourmbit + 0x000001;

	int poop = finfo.st_size/fourmbit;


//	printf("test: %ld\n", finfo.st_size/fourmbit);
	
//	unsigned char* tmp = (unsigned char*)&gbaheader;
	unsigned char tmp[] = {poop, 0x00, 0x00, 0x01};
	
//	printf("gba header: %x\n", gbaheader );
	for(i=0; i<4; i++) putc( tmp[i], outfile );


	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "0" representing that the unit
		is an unencoded letter (1 byte), "1" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = 1;
	mask = 0x80;	// für GBA fangen wir mit MSB an
	s = 0;
	r = N - F; 	// 4078

	for (i = s; i < r; i++) text_buf[i] = 0xff;  /* Clear the buffer with any character that will appear often. */
	for (len = 0; len < F && (c = getc(infile)) != EOF; len++)
		text_buf[r + len] = c;  /* Read F bytes into the last F bytes of
			the buffer */
	if ((textsize = len) == 0) return;  /* text of size zero */
	for (i = 1; i <= F; i++) InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */


	// kompressions schleife

	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */

		// nicht komprimieren
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			// code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else
		// komprimieren
		{
			code_buf[0] |= mask;  // flag "komprimiert" setzen

			// Bit 0-3   Disp MSBs
			// Bit 4-7   Number of bytes to copy (minus 3)
			// Bit 8-15  Disp LSBs

			code_buf[code_buf_ptr++] = (unsigned char)
				(( (r-match_position-1)>>8) & 0x0f) |
				((match_length - (THRESHOLD + 1))<<4);

			code_buf[code_buf_ptr++] = (unsigned char) ((r-match_position-1) & 0xff);
			/* Send position and length pair. Note match_length > THRESHOLD. */
		}

		// mask shift
		if ((mask >>= 1) == 0) {  /* Shift mask right one bit. */
			for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
				putc(code_buf[i], outfile);     /* code together */
			codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = 1;
			mask = 0x80;
		}

		last_match_length = match_length;
		for (i = 0; i < last_match_length &&
				(c = getc(infile)) != EOF; i++) {
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1) text_buf[s + N] = c;  /* If the position is
				near the end of buffer, extend the buffer to make
				string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		if ((textsize += i) > printcount) {
			printf("%12ld\r", textsize);  printcount += 1024;
				/* Reports progress each time the textsize exceeds
				   multiples of 1024. */
		}
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */


	if (code_buf_ptr > 1) {		/* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++) putc(code_buf[i], outfile);
		codesize += code_buf_ptr;
	}

	// pad output with zeros to make it a multiply of 4
	if(codesize%4)
		for(i=0; i<4-(codesize%4); i++)
			putc(0x00, outfile);

	codesize += 4-(codesize%4);

	printf("In : %ld bytes\n", textsize);	/* Encoding is done. */
	printf("Out: %ld bytes\n", codesize);
	printf("Out/In: %.3f\n", (double)codesize / textsize);
}



void Decode(void)	/* Just the reverse of Encode(). */
// also fixed to decode in the way bios funktion works
{
	int  i, j, k, r, c, z;
	unsigned int  flags;
	u32 decomp_size;	// i´m using decomp_size and cur_size to make sure we dont "decompress" the padding data
	u32 cur_size=0;		// we added to the compressed data to keep it in 4 byte boundaries

	// discard gba header info
//	u32 gbaheader;

//	unsigned char* tmp = (unsigned char*)&gbaheader;
//	for(i=0; i<4; i++) tmp[i] = getc(infile);
//	decomp_size = tmp[0] * fourmbit;

	unsigned char tmp[] = {0,0,0,0};
	for(i=0; i<4; i++) tmp[i] = getc(infile);
	decomp_size = tmp[0] * fourmbit;

	printf("Decomp size %ld\n", decomp_size);	
//	decomp_size = gbaheader>>8;
//	printf("gba header: %x, decompressed size: %d\n", gbaheader, decomp_size );

	for (i = 0; i < N - F; i++) text_buf[i] = 0xff;
	r = N - F;  flags = z = 7;
	for ( ; ; ) {
		flags <<= 1;
		z++;
		if (z == 8) {				// read new block flag
			if ((c = getc(infile)) == EOF) break;
			flags = c;
			z = 0;				// reset counter
		}
		if (!(flags&0x80)) {			// flag bit zero => uncompressed
			if ((c = getc(infile)) == EOF) break;
			if(cur_size<decomp_size) putc(c, outfile);  text_buf[r++] = c;  r &= (N - 1); cur_size++;
		} else {
			if ((i = getc(infile)) == EOF) break;
			if ((j = getc(infile)) == EOF) break;
			j = j | ((i<<8)&0xf00);		// match offset
			i = ((i>>4)&0x0f)+THRESHOLD;	// match length
			for (k = 0; k <= i; k++) {
				c = text_buf[(r-j-1) & (N - 1)];
				if(cur_size<decomp_size) putc(c, outfile);  text_buf[r++] = c;  r &= (N - 1); cur_size++;
			}
		}
	}
}


int main(int argc, char *argv[])
{
	printf("N64 VC Compression/Decompression Utility by Jurai\n"
		"based on code by Haruhiko Okumura\nhttp://www.elotrolado.net\n");
	char  *s = argv[1];

	if (argc != 4) {
		printf("'romc e file1 file2' encodes file1 to file2.\n"
			"'romc d file1 file2' decodes file1 to file2.\n");
/*

		printf("'gbalzss e file1 file2' encodes file1 into file2.\n"
			"'gbalzss d file1 file2' decodes file1 into file2.\n");
*/
		return EXIT_FAILURE;
	}

	if ((strcmp(argv[1], argv[2])==0)
	|| (s = argv[1], s[1] || strpbrk(s, "DEde") == NULL)
	|| (s = argv[2], (infile  = fopen(s, "rb")) == NULL)
	|| (s = argv[3], (outfile = fopen(s, "wb")) == NULL)) {
		printf("Error: Could not access file %s\n", s);  return EXIT_FAILURE;
	}

/*

	if ((strcmp(argv[1], argv[2])==0)
	|| (s = argv[1], s[1] || strpbrk(s, "Ee") == NULL)
	|| (s = argv[2], (infile  = fopen(s, "rb")) == NULL)
	|| (s = argv[3], (outfile = fopen(s, "wb")) == NULL)) {
		printf("Error: Could not access file %s\n", s);  return EXIT_FAILURE;
	}
*/
	stat( argv[2], &finfo );	// get filesize for gba header
	if(toupper(*argv[1]) == 'E')
	{
		if(((finfo.st_size % fourmbit) > 0) || (finfo.st_size == 0))
		{
			printf("Error: Input N64 rom must be a multiple of 4mbit (%ld bytes)\n", fourmbit);
			fclose(infile);  fclose(outfile);
				exit(0);
		}

	}
 if (toupper(*argv[1]) == 'E') Encode();  else Decode();

//	if (toupper(*argv[1]) == 'E') Encode();

/*
	infile  = fopen("test.txt.lz", "rb");		// for debugging in gdb
	outfile = fopen("test2", "wb");
	stat( "test.txt", &finfo );
	Encode();
	Decode();
*/
	fclose(infile);  fclose(outfile);
	return EXIT_SUCCESS;
}
