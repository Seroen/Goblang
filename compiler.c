#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Base string for terminal output
#define ERROR_BASE "\x1B[91m%s\n\x1B[0m" // Bright Red
#define METADATA_BASE "\x1B[97m%s\x1B[0m" // Bright White

// Max path length I think, this shouldn;t be an issue if its off a bit though
#define FILE_PATH_MAX 4096

// String that indicates that the file is a goblang code file
#define GOBLANG_MAGIC_STRING "Goblang "

// Frequently used variables global scoped for ease of access
// x for x86_64, a will be for ARM
char architecture;
// l for Linux, w will be for Windows
char os;

// input_file_text holds the text of the input file
char input_file_path[FILE_PATH_MAX];
FILE *input_file;
char *input_file_text;

bool debug_mode = false;

#define COMPILER_VERSION 1
int version;

// Index to read input file from
int cursor = 0;


void debug_print(char base_string[], char print_sting[]) {
	if (debug_mode) {
		fprintf(stdout, base_string, print_sting);
	}
}


// Parse command line arguments
void process_arguments(int argc, char* argv[]) {
	int arguments = argc;

	if (arguments < 4) {
		fprintf(stderr, ERROR_BASE, "Invalid number of arguments");
		exit(2);
	}

	// Architecture argument
	if (strcmp(argv[1], "-x") == 0) {
		architecture = 'x';
	} else {
		fprintf(stderr, ERROR_BASE, "Invalid or unsuported target architecture");
		exit(2);
	}

	// OS argument
	if (strcmp(argv[2], "-l") == 0) {
		architecture = 'l';
	} else {
		fprintf(stderr, ERROR_BASE, "Invalid or unsuported target operating system");
		exit(2);
	}

	// Input file argument
	if (strlen(argv[3]) > FILE_PATH_MAX) {
		fprintf(stderr, ERROR_BASE, "Input file path too long");
		exit(2);
	}

	for (int i = 0; i < strlen(argv[3]); i++)
		input_file_path[i] = argv[3][i];
	
	input_file = fopen(input_file_path, "r");
	
	if (input_file == NULL) {
		fprintf(stderr, ERROR_BASE, "Could not open input file");
		exit(2);
	}

	fseek(input_file, 0, SEEK_END);
	long input_file_size = ftell(input_file);
	fseek(input_file, 0, SEEK_SET);
	input_file_text = malloc(input_file_size);

	size_t i = 0;
	int c;
	while ((c = fgetc(input_file)) != EOF) {
		input_file_text[i++] = (char)c;
	}

	fclose(input_file);

	input_file_text[i] = '\n';

	// Debug argument
	if (arguments >= 5) {
		if (strcmp(argv[4], "-d") == 0) {
			debug_mode = true;
		} else {
			fprintf(stderr, ERROR_BASE, "Invalid argument");
			exit(2);
		}
	}
}


// Retrive language version and validate file type
void get_version() {
	// Check for goblang header
	char file_magic_string[8];
	memcpy(&file_magic_string, input_file_text, 8);
	cursor += 8;
	
	if (strcmp(file_magic_string, GOBLANG_MAGIC_STRING) != 0) {
		fprintf(stderr, ERROR_BASE, "File does not begin with goblang header");
		debug_print(ERROR_BASE, file_magic_string);
		exit(2);
	}

	debug_print(METADATA_BASE, "Goblang ");

	// Convert version number to int
	char *version_string;
	int version_string_len = 0;
	char character;
	while (character != ';') {
		character = input_file_text[cursor];
		cursor++;

		version_string_len++;
	}

	version_string = malloc(version_string_len);
	memcpy(version_string, input_file_text + 8, version_string_len);
	version_string[version_string_len - 1] = '\0';

	version = atoi(version_string);
	// Minimum version is 1
	if (version <= 0) {
		fprintf(stderr, ERROR_BASE, "Invalid version in header");
		debug_print(ERROR_BASE, version_string);
		exit(2);
	}

	// Check if compiler up to date
	if (version > COMPILER_VERSION) {
		fprintf(stderr, ERROR_BASE, "Input file was made for a later compiler version");
		debug_print(ERROR_BASE, version_string);
		exit(2);
	}

	debug_print(METADATA_BASE, version_string);
	debug_print(METADATA_BASE, ";");
}


int main(int argc, char* argv[]) {
	process_arguments(argc, argv);

	get_version();

	debug_print("%s", "\n");

	return 0;
}