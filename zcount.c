//
// zcount.c: count the number of zero-bytes in files or stdin
//
// Copyright (C) 2012 Leonid Chaichenets
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// Authors:
//      Leonid Chaichenets <leonid.chaichenets@googlemail.com>
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <argp.h>

// actual zero-byte counter; counts and returns the number of zero-bytes in
// the *open* filepointer fp; stops counting if it reaches the limit upper
// (if non-zero), does not allow the counter to overrun
unsigned long int countZB(FILE * fp, unsigned long int upper) {

	char c; // character worked on
	long unsigned int zeros = 0; // number of counted zero-bytes

	if (upper == 0) upper = ULONG_MAX;

	while( (c = getc(fp)) != EOF && zeros < upper && zeros < ULONG_MAX )
		if (c == '\0') zeros++;

	return zeros;

};

/* Argument parser setup -- START */
// program version:
const char *argp_program_version = "zcount 1.0";

// contact address:
const char * argp_program_bug_address =
	"Leonid Chaichenets <leonid.chaichenets@googlemail.com>";

// program description:
const char doc[] =
	"zcount -- A program for counting zero bytes in given files.\v"
	"Principal use of this program is to detect corrupt files: Lost data chunks "
	"are usually replaced by zero-bytes (0x00) by the filesystem checkers. Thus, "
	"corrupted files are easily identified by a large number of zero-bytes.\n\n"
	"If no input files are given on the command line, then stdin is used. The "
	"return code of the program is the number of files containing at least "
	"NUMBER2 zero-bytes (or INT_MAX). WARNING: By default no output is produced, " 
	"as the program is intended to be used in a script. Set at least one '-v' "
	"for human readable output.";

// non-option arguments description:
const char args_doc[] = "[FILE1] [FILE2] ...";

// communication between parse_opt() and main():
struct arguments {
	int verbosity;			/* verbosity option */
	unsigned long int upper;	/* upper limit option */
	unsigned long int lower;	/* lower limit option */
	int retcode;			/* return code of main() */	
};

// option arguments we understand:
const struct argp_option options[] = {
	{"verbose", 'v', 0, 0, "Produce verbose output, multiple flags allowed"},
	{"upper", 'u', "NUMBER1", 0,
		"Stop after counting NUMBER1 (long unsigned integer) of zero-bytes "
		"(NUMBER1=0 [default] for no limit)"},
	{"lower", 'l', "NUMBER2", 0, "Consider a file damaged after counting "
		"at least NUMBER2 of zero-bytes (if NUMBER2 > NUMBER1 then NUMBER1 is " 
		"used for both limits, default is NUMBER2=1)"},
	{0}
};

// argument parser logics:
error_t parse_opt (int key, char * arg, struct argp_state * state) {

	struct arguments * arguments = state->input; // communication with main
	char ** tailptr; // used to detect incorrect values of upper or lower limits
	FILE * fp = NULL; // current file pointer
	unsigned long int zeros = 0; // number of zero-bytes counted
	
	switch (key) {

		// initialize paser by setting up default argument values:
		case ARGP_KEY_INIT:
			arguments->verbosity = 0;
			arguments->upper = 0;
			arguments->lower = 1;
			arguments->retcode = 0;
			break;
		
		// parse verbosity flags:
		case 'v':
			if (arguments->verbosity < INT_MAX)
				arguments->verbosity++;
			break;

		// parse upper limit argument:
	    	case 'u':
			// set tailptr to a non-zero value for strtoul
			tailptr = &arg;
			arguments->upper = strtoul(arg, tailptr, 0);
			if (**tailptr != '\0') {
				fprintf(stderr, "'%s' is not a non-negative integer\n", state->argv[state->next-1]);
				return EINVAL;
			};
			break;
			
	    // parse lower limit setting:		
		case 'l':
			// set tailptr to a non-zero value for strtoul
			tailptr = &arg;
			arguments->lower = strtoul(arg, tailptr, 0);
			if (**tailptr != '\0') {
				fprintf(stderr, "'%s' is not a non-negative integer\n", state->argv[state->next-1]);
				return EINVAL;
			};
			break;
			
	    // parse a filename, count zero-bytes in it:
		case ARGP_KEY_ARG:

			if ((fp = fopen(arg,"r"))) {

				zeros = countZB(fp, arguments->upper);
				fclose(fp);

				/* small sanity check: */
				if (arguments->lower > arguments->upper && arguments->upper != 0)
				    arguments->lower = arguments->upper;

				/* update number of suspicious files: */
				if (zeros && arguments->retcode < INT_MAX)
				    arguments->retcode++;

				/* produce output depending on verbosity: */
				if (arguments->verbosity == 1 && zeros >= arguments->lower)
				    fprintf(stderr,"%s: seems corrupted, %lu zero-bytes counted\n", arg, zeros);
				if (arguments->verbosity >= 2) {
				    if(zeros < arguments->lower)
					fprintf(stdout,"%s: %lu zero-bytes counted\n", arg, zeros);
				    else
					fprintf(stderr,"%s: seems corrupted, %lu zero-bytes counted\n", arg, zeros);
				};

			} else // this not an error in argp-context (parsing of arguments continues)!
				fprintf(stderr, "%s: %s\n", arg, strerror(errno));

			break; // case ARGP_KEY_ARG

		// there were no non-option arguments, i.e. no files have been given, count zero-bytes in stdin:
		case ARGP_KEY_NO_ARGS:
			zeros = countZB(stdin, arguments->upper);

			/* small sanity check: */
			if (arguments->lower > arguments->upper && arguments->upper != 0)
			    arguments->lower = arguments->upper;

			/* update number of suspicious files: */
			if (zeros && arguments->retcode < INT_MAX)
			    arguments->retcode++;

			/* produce output depending on verbosity: */
			if (arguments->verbosity == 1 && zeros >= arguments->lower)
			    fprintf(stderr,"data in stdin seems corrupted, %lu zero-bytes counted\n", zeros);
			if (arguments->verbosity >= 2) {
			   if(zeros < arguments->lower)
			       fprintf(stdout,"%lu zero-bytes in stdin counted\n", zeros);
			   else
			       fprintf(stderr,"data in stdin seems corrupted, %lu zero-bytes counted\n", zeros);
			};
			break;

		// parsing has been terminated with an error:
		case ARGP_KEY_ERROR:
			fprintf(stderr, "Argument parsing has been terminated due to an error!\n");
			break;

		// catch all:
		default:
			return ARGP_ERR_UNKNOWN;

	} // switch (key)

	return 0;

} // parse_opt()

// put all ARGP stuff together:
const struct argp argp = {options, parse_opt, args_doc, doc};
/* Argument parser setup -- END*/

int main (int argc, char **argv) {

	// get us a communication channel with the parser:
	struct arguments arguments;

	//parse our arguments, all the work is done there:
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	return arguments.retcode;

};
