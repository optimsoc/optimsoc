/*$$HEADER*/
/******************************************************************************/
/*                                                                            */
/*                    H E A D E R   I N F O R M A T I O N                     */
/*                                                                            */
/******************************************************************************/
 
// Project Name                   : ORPSoC v2
// File Name                      : bin2vmem.c
// Prepared By                    : jb, jb@orsoc.se
// Project Start                  : 2009-05-13
 
/*$$COPYRIGHT NOTICE*/
/******************************************************************************/
/*                                                                            */
/*                      C O P Y R I G H T   N O T I C E                       */
/*                                                                            */
/******************************************************************************/
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; 
  version 2.1 of the License, a copy of which is available from
  http://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt.
 
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
 
/*$$DESCRIPTION*/
/******************************************************************************/
/*                                                                            */
/*                           D E S C R I P T I O N                            */
/*                                                                            */
/******************************************************************************/
//
// Generates VMEM output to stdout from binary images.
// Use with redirection like: ./bin2vmem app.bin > app.vmem
// To change either the number of bytes per word or word per line, change
// the following defines.
// Currently output is WORD addressed, NOT byte addressed
// eg: @00000000 00000000 00000000 00000000 00000000
//     @00000004 00000000 00000000 00000000 00000000
//     @00000008 00000000 00000000 00000000 00000000
//     @0000000c 00000000 00000000 00000000 00000000
//     etc..
//
// OR
// 
// Output a list of the words, one per line, as Synplify appears to like
// specify this option with the -synfmt switch on the command line after
// the input file
// eg: ./bin2vmem data.bin -synfmt > data.vmem
//
 
#define WORDS_PER_LINE_DEFAULT 4
#define BYTES_PER_WORD_DEFAULT 4
 
#define FILENAME_CMDLINE_INDEX 1
#define FMT_CMDLINE_INDEX 2
 
#define FMT_WITH_ADDR 0
#define FMT_SYN 1
 
#define PAD_FILL 0
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
 
int main(int argc, char **argv)
{
 
	FILE  *fd = NULL;
	int c;
	int i = 0;
	int write_size_word=0; // Disabled by default
	unsigned int image_size;
	//int output_fmt = FMT_WITH_ADDR; // 0 - standard 4 per line with address, 1 - synfmt
	int bytes_per_word = BYTES_PER_WORD_DEFAULT, bytes_per_word_tmp;
	int words_per_line = WORDS_PER_LINE_DEFAULT;
	int printing_addr = 1; // Print address by default
	int pad_row_end = 0; // Default is we don't pad
	int pad_addr_end = 0; // Default is we don't pad
	int pad_row_number = 0;
	int pad_addr_number = 0;
 
	// Counters keeping track of what we've printed
	int current_word_addr = 0;
	int word_counter = 0;
	int total_word_counter = 0;
	int byte_counter = 0;
	int total_byte_counter = 0;
	int row_counter  = 1;
 
 
 
	if(argc < 2) {
	  fprintf(stderr,"\n\tInsufficient options.\n");
	  fprintf(stderr,"\tPlease specify a binary file to convert to VMEM\n");
	  fprintf(stderr,"\n\tbin2vmem - creates vmem output to stdout from bin\n");
	  fprintf(stderr,"\n\tBy default the output is word addressed 32-bit words\n");
	  fprintf(stderr,"\tSpecify -synfmt on the command line after the filename\n");
	  fprintf(stderr,"\tto output in the alterative format, which is a simple\n");
	  fprintf(stderr,"\tlist of the data words. The default bytes per word is 4.\n");
 
	  fprintf(stderr,"\n");
	  fprintf(stderr,"\tAdditionally, to specify the bytes per word (per line)\n");
	  fprintf(stderr,"\twhen specifying the -synfmt simple list format, use the\n");
	  fprintf(stderr,"\tswitch -bpw=N after specifying the -synfmt option. Example:\n");
	  fprintf(stderr,"\t\t./bin2vmem prog.bin -synfmt -bpw=2 > prog.vmem\n");
	  fprintf(stderr,"\n");
	  fprintf(stderr,"\n");
	  fprintf(stderr,"Padding options:\n");
	  fprintf(stderr,"\t--pad-to-row <num>\tPad up to num rows with 0\n");
	  fprintf(stderr,"\t--pad-to-addr <addr>\t Pad UP to (not including) that address with 0\n");
	  fprintf(stderr,"\n");
	  exit(1);
	}
 
 
 
	if (argc > 1) // check for the -synfmt switch
	  {
	    for (i = 1; i< argc; i++)
	      {
		if ((strcmp("-synfmt", argv[i]) == 0) || (strcmp("--synfmt", argv[i]) == 0))
		  {
		    words_per_line = 1;
		    printing_addr = 0;
		    //output_fmt = FMT_SYN; // output synthesis friendly format
		  }
		else if ((1 == sscanf(argv[i], "-bpw=%d", &bytes_per_word_tmp)) ||
			 (strcmp("--bpw", argv[i]) == 0))
		  {
		    if (strcmp("--bpw", argv[i]) == 0)
		      if (i+1 < argc)
			{
			  bytes_per_word_tmp = atoi(argv[i+1]);
			  i++;
			}
 
		    if(bytes_per_word_tmp > 0)
		      {
			bytes_per_word = bytes_per_word_tmp;
		      }
 
		  }
		else if ((strcmp("--bpw", argv[i]) == 0))
		  {
		    if (i+1 < argc)
		      {
			bytes_per_word_tmp = atoi(argv[i+1]);
			i++;
		      }
		    if (bytes_per_word_tmp < 4 && bytes_per_word_tmp > 0)
		      {
			bytes_per_word = bytes_per_word_tmp;
		      }
 
 
		  }
		else if (strcmp("--pad-to-row", argv[i]) == 0)
		  {
		    if (i+1 < argc)
		      {
			uint32_t tmp_addr_dec;
			if (!(sscanf(argv[i+1], "%d", &tmp_addr_dec) == 1))
			  {
			    fprintf(stderr, 
				    "Error: Number of rows to pad to not specified\n");
			    exit(1);			    
			  }
			pad_row_number = tmp_addr_dec;
					      }
		    else
		      {
			fprintf(stderr, 
				"Error: Number of rows to pad to specified\n");
			exit(1);
		      }
 
		    if (pad_addr_end)
		      {
			fprintf(stderr, "Error: Can only specify one padding style\n");
			exit(1);
		      }
		    pad_row_end = 1;
		    i++;
		  }
		else if (strcmp("--pad-to-addr", argv[i]) == 0)
		  {
		    // Get address
		    if (i+1 < argc)
		      {
			uint32_t tmp_addr_hex, hex_used = 0, tmp_addr_dec = 0;
			if (sscanf(argv[i+1], "0x%x", &tmp_addr_hex) == 1)
			  hex_used = 1;
			else
			  {
			    fprintf(stderr, 
				    "Error: No address to pad to specified. Use either hex with 0x prefixed, or decimal\n");
			    exit(1);
 
			  }
 
			if (hex_used)
			  pad_addr_number = tmp_addr_hex;
			else 
			  pad_addr_number = tmp_addr_dec;
 
		      }
		    else
		      {
			fprintf(stderr, 
				"Error: No address to pad to specified\n");
			exit(1);
		      }
		    if (pad_addr_end)
		      {
			fprintf(stderr, "Error: Can only specify one padding style\n");
			exit(1);
		      }		      
		    pad_addr_end = 1;
		    i++;
		  }
		else
		  {
		    // File to open
		    fd = fopen( argv[i], "r" );
		    if (fd == NULL) {
		      fprintf(stderr,"failed to open input file: %s\n",argv[i]);
		      exit(1);
		    }
 
		  }
 
	      }
 
	  }
 
	if (fd == NULL) {
	  fprintf(stderr,"failed to open input file: %s\n",argv[i]);
	  exit(1);
	}
 
 
	fseek(fd, 0, SEEK_END);
	image_size = ftell(fd);
	fseek(fd,0,SEEK_SET);
 
	if (write_size_word)
	  {
	    // or1200 startup method of determining size of boot image we're 
	    // copying by reading out the very first word in flash is used. 
	    // Determine the length of this file.
	    fseek(fd, 0, SEEK_END);
	    image_size = ftell(fd);
	    fseek(fd,0,SEEK_SET);
 
	    // Now we should have the size of the file in bytes. Let's ensure it's a word multiple
	    image_size+=3;
	    image_size &= 0xfffffffc;
 
	    // Sanity check on image size
	    if (image_size < 8){ 
	      fprintf(stderr, "Bad binary image. Size too small\n");
	      return 1;
	    }
 
	    // Now write out the image size
	    printf("@%8x", current_word_addr);
	    printf("%8x", image_size);
	    current_word_addr += words_per_line * bytes_per_word;
	  }
 
 
	i=0;
	int starting_new_line  = 1;
	// Now write out the binary data to specified format. Either
	// more complicated, addressed format:
	// VMEM format: @ADDRESSS XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
	// or simple, synplifyfriendly format which is just a list of
	// the words
 
	while ((c = fgetc(fd)) != EOF) 
	  {
	    if (starting_new_line)
	      {
		// New line - print the current addr and then increment it
		if (printing_addr)
		  printf("@%.8x", current_word_addr);
		starting_new_line = 0;
	      }
 
	    if ( (byte_counter == 0) && 
		 ((words_per_line > 1) || (printing_addr)) )
	      printf(" ");
 
	    printf("%.2x", (unsigned int) c); // now print the actual char
 
	    byte_counter++;
 
	      if (byte_counter == bytes_per_word)
		{
		  word_counter++;
		  total_word_counter++;
		  byte_counter=0;
		}
	      if (word_counter == words_per_line)
		{
		  printf("\n");   
		  word_counter = 0;
		  current_word_addr += words_per_line;
		  starting_new_line = 1;
		  row_counter++;
		}
	  }
 
	/* Padding stuff */
	/* Pad until a set number of rows */
	if (pad_row_end)
	  {
	    // First see if we need to pad to the end of the current word 
	    if (byte_counter > 0)
	      {
		for(i=0;i<(bytes_per_word - byte_counter); i++)
		  {
		    printf("%.2x", (unsigned int) PAD_FILL);
		  }		
		byte_counter = 0;
		word_counter++;
	      }
 
	    if (word_counter == words_per_line)
	      {
		printf("\n");   
		word_counter = 0;
		current_word_addr += words_per_line;
		starting_new_line = 1;
		row_counter++;
	      }
 
	    while (row_counter < pad_row_number + 1)
	      {
		if (starting_new_line)
		  {
		    // New line - print the current addr and then increment it
		    if (printing_addr)
		      printf("@%.8x", current_word_addr);
		    starting_new_line = 0;
		  }
 
		if ( (byte_counter == 0) && 
		     ((words_per_line > 1) || (printing_addr)) )
		  printf(" ");
 
		printf("%.2x", (unsigned int) PAD_FILL); // PAD
 
		byte_counter++;
 
		if (byte_counter == bytes_per_word)
		  {
		  word_counter++;
		  total_word_counter++;
		  byte_counter=0;
		}
		if (word_counter == words_per_line)
		  {
		    printf("\n");   
		    word_counter = 0;
		    current_word_addr += words_per_line;
		    starting_new_line = 1;
		    row_counter++;
		  }
	      }
 
	  }
 
	if (pad_addr_end || pad_row_end)
	  {
	    // Figure out where we are
	    total_byte_counter = (current_word_addr * bytes_per_word) + (word_counter * bytes_per_word) + byte_counter;
 
	    // Loop again, generating 0s this time instead
	    while ( (pad_addr_end && (total_byte_counter < pad_addr_number)) ||
		    (pad_row_end && (row_counter < pad_row_number+1)) )
	      {
		if (starting_new_line)
		  {
		    // New line - print the current addr and then increment it
		    if (printing_addr)
		      printf("@%.8x", current_word_addr);
		    starting_new_line = 0;
		  }
 
		if ( (byte_counter == 0) && 
		     ((words_per_line > 1) || (printing_addr)) )
		  printf(" ");
 
		printf("%.2x", (unsigned int) PAD_FILL); // PAD
 
		total_byte_counter++;
		byte_counter++;
 
		if (byte_counter == bytes_per_word)
		  {
		    word_counter++;
		    total_word_counter++;
		    byte_counter=0;
		  }
		if (word_counter == words_per_line)
		  {
		    printf("\n");   
		    word_counter = 0;
		    current_word_addr += words_per_line;
		    starting_new_line = 1;
		    row_counter++;
		  }
	      }
	  }
 
	return 0;
}
