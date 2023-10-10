/*
 * cache.c by Yeadam Kim
 * Date: 4/8/23
 *
 * Simulation of multi-level memory:
 * One CPU register, three levels of fully associative cache (L1, L2, L3), RAM.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#define NEWLINE putchar('\n')
#define FILENAME_BUFLEN 128
#define INPUT_BUFLEN 256

// Function Prototypes
void printSummary();
void printCacheLines();
void ClearBuffer();
void printHelp();
unsigned int updateCache(unsigned int mem_addr);

// Cache Configurations
typedef struct /* Cache Line */
{
	int tag; // -1 if not set (Both age and tag)
	int age; // Least Recently Used (LRU) Replacement Policy
} cache_line;

struct /* L1 Cache */
{
	unsigned int line_size;
	unsigned int lines_per_set; // Redundant but defined for clarity
	unsigned int sets;
	unsigned int latency;
	cache_line cache[4];
} L1 = {256, 4, 1, 1, {{0}}};

struct /* L2 Cache */
{
	unsigned int line_size;
	unsigned int lines_per_set;
	unsigned int sets;
	unsigned int latency;
	cache_line cache[64];
} L2 = {1024, 64, 1, 10, {{0}}};

struct /* L3 Cache */
{
	unsigned int line_size;
	unsigned int lines_per_set;
	unsigned int sets;
	unsigned int latency;
	cache_line cache[256];
} L3 = {4096, 256, 1, 100, {{0}}};

struct /* RAM */
{
	unsigned int latency;
} RAM = {1000};

/*
 * Main Function
 */
int main(int argc, char *argv[])
{
	int opt = 0;
	int cacheLines = 0;
	char filename[FILENAME_BUFLEN];
	FILE *fp = NULL;

	// Argument parser
	while ((opt = getopt(argc, argv, "-lf:")) != EOF)
	{
		switch (opt)
		{
		case '-':
			fprintf(stderr, "Error: Double dash detected.\n"
											"Usage: %s [-l] [-f]\n"
											"\t-l: Displays all occupied cache lines (file mode only)\n"
											"\t-f: Input file\n",
							argv[0]);
			exit(EXIT_FAILURE);
			break;

		case 'l':
			cacheLines = 1;
			break;

		case 'f':
			if (argc > optind)
			{
				fprintf(stderr, "Error: Cannot read more than one file\n");
				exit(EXIT_FAILURE);
			}
			strcpy(filename, optarg);
			fp = fopen(filename, "r");
			if (NULL == fp)
			{
				fprintf(stderr, "Error: File cannot be opened\n");
				exit(EXIT_FAILURE);
			}
			break;

		default:
			fprintf(stderr, "Usage: %s [-l] [-f]\n"
											"\t-l: Displays all occupied cache lines (file mode only)\n"
											"\t-f: Input file\n",
							argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	// Prints summary of memory structure
	printSummary();

	// VARIABLES
	auto unsigned int cycles = 0;
	auto uint32_t mem_addr = 0;
	auto int i_result = 0;

	// Configure Cache Tags
	// Set cache tags and age to -1
	for (int i = 0; i < L1.lines_per_set; i++)
	{
		L1.cache[i].tag = -1;
		L1.cache[i].age = -1;
	}
	for (int i = 0; i < L2.lines_per_set; i++)
	{
		L2.cache[i].tag = -1;
		L2.cache[i].age = -1;
	}
	for (int i = 0; i < L3.lines_per_set; i++)
	{
		L3.cache[i].tag = -1;
		L3.cache[i].age = -1;
	}

	/*
	 * FILE MODE
	 */
	auto char mem_buf[9] = {0};

	if (fp)
	{
		// Read file input
		printf("Reading from '%s'...\n\n", filename);
		while (!feof(fp))
		{
			if (fgets(mem_buf, 9, fp))
			{
				// Remove dangling newline
				mem_buf[strcspn(mem_buf, "\n")] = '\0';

				// Convert file input to hex
				i_result = sscanf(mem_buf, "%x", &mem_addr);
				if (0 == i_result || EOF == i_result)
				{
					fprintf(stderr, "Error: Memory address must be in hex\n");
					exit(EXIT_FAILURE);
				}

				// Check caches and add cycles
				cycles += updateCache(mem_addr);

				// Clear Buffer
				memset(mem_buf, 0, sizeof(mem_buf));
			}
		}

		fclose(fp);
		NEWLINE;

		// Print total # of CPU cycles executed
		printf("Total # of CPU Cycles: %u\n", cycles);

		// Display cache lines if -l is toggled
		if (cacheLines)
		{
			printCacheLines();
		}

		exit(EXIT_SUCCESS);
	}

	/*
	 * INTERACTIVE MODE
	 */
	auto char input[INPUT_BUFLEN];

	// Print help text
	printHelp();

	while (1)
	{
		// Get user input
		printf("Load address: 0x");
		i_result = scanf("%8s", input);
		if (0 == i_result || EOF == i_result)
		{
			fprintf(stderr, "Error: scanf failed\n");
			exit(EXIT_FAILURE);
		}

		// Calculate CPU Cycles
		if (0 == strcmp(input, "s"))
		{
			printf("Total # of CPU Cycles: %u\n", cycles);
			NEWLINE;
			continue;
		}

		// Display all cache lines
		if (0 == strcmp(input, "l"))
		{
			printCacheLines();
			continue;
		}

		// Quit
		if (0 == strcmp(input, "q"))
		{
			exit(EXIT_SUCCESS);
		}

		// Convert user input to hex
		i_result = sscanf(input, "%x", &mem_addr);
		if (0 == i_result || EOF == i_result)
		{
			fprintf(stderr, "Error: Memory address must be in hex\n");
			NEWLINE;

			// Clear Buffer
			ClearBuffer();

			continue;
		}

		// Check caches and add cycles
		cycles += updateCache(mem_addr);
		NEWLINE;

		// Clear Buffer
		ClearBuffer();
	}

	exit(EXIT_SUCCESS);
} // END OF MAIN

/*
 * Functions
 */

// Clears stdin buffer
void ClearBuffer()
{
	auto int c;
	while ((c = fgetc(stdin)) != EOF && c != '\n')
	{
		continue;
	}
}

// Prints summary text
void printSummary()
{
	puts("----------------------------------------------");

	printf("%-7s %-10s %-5s %-10s %-8s",
				 "", "Line Size", "Sets", "Lines/Set", "Latency");

	NEWLINE;

	printf("%-7s %-10s %-5s %-10s %-8s",
				 "L1", "256", "1", "4", "1");

	NEWLINE;

	printf("%-7s %-10s %-5s %-10s %-8s",
				 "L2", "1024", "1", "64", "10");

	NEWLINE;

	printf("%-7s %-10s %-5s %-10s %-8s",
				 "L3", "4096", "1", "256", "100");

	NEWLINE;

	printf("%-7s %-10s %-5s %-10s %-8s",
				 "Memory", "N/A", "N/A", "N/A", "1000");

	NEWLINE;

	puts("----------------------------------------------");

	NEWLINE;
}

// Prints all occupied cache lines
void printCacheLines()
{
	NEWLINE;

	puts("-- L1 --");
	for (int i = 0; i < L1.lines_per_set; i++)
	{
		if (L1.cache[i].tag != -1)
		{
			printf("%d: %d\n", i, L1.cache[i].tag);
		}
	}

	NEWLINE;

	puts("-- L2 --");
	for (int i = 0; i < L2.lines_per_set; i++)
	{
		if (L2.cache[i].tag != -1)
		{
			printf("%d: %d\n", i, L2.cache[i].tag);
		}
	}

	NEWLINE;

	puts("-- L3 --");
	for (int i = 0; i < L3.lines_per_set; i++)
	{
		if (L3.cache[i].tag != -1)
		{
			printf("%d: %d\n", i, L3.cache[i].tag);
		}
	}

	NEWLINE;
}

// Prints help text
void printHelp()
{
	puts("CPU Cache Simulation\n"
			 " - CPU only supports a single instruction: load address\n"
			 " - Memory addresses limited to 32 bit\n"
			 " - Enter 's' to show the total number of CPU cycles\n"
			 " - Enter 'l' to display all occupied cache lines\n"
			 " - Enter 'q' to exit");
	NEWLINE;
}

// Updates cache and returns # of cycles
unsigned int updateCache(unsigned int mem_addr)
{
	unsigned int frameNum;
	int oldest_age;
	int oldest_line;
	int cached; // boolean

	////////////////////////////////////////////////////

	// L1 Cache
	cached = 0;
	oldest_age = -1;
	oldest_line = -1;
	frameNum = mem_addr / L1.line_size;
	for (int i = 0; i < L1.lines_per_set; i++)
	{
		// Found
		if (L1.cache[i].tag == frameNum)
		{
			// Increase age for all occupied lines
			for (int i = 0; i < L1.lines_per_set; i++)
			{
				if (L1.cache[i].age > -1)
				{
					++L1.cache[i].age;
				}
			}

			printf("%#x: Retrieved from L1, Frame: %d\n", mem_addr, frameNum);
			return L1.latency;
		}
	}

	// L1: Check for empty/old lines and cache memory
	for (int i = 0; i < L1.lines_per_set; i++)
	{
		// Empty
		if (L1.cache[i].tag == -1)
		{
			L1.cache[i].tag = frameNum;
			L1.cache[i].age = 0;
			cached = 1;
			break;
		}

		// Find oldest age within the set
		if (L1.cache[i].age > oldest_age)
		{
			oldest_age = L1.cache[i].age;
			oldest_line = i;
		}
	}

	// L1: Replace oldest line
	if (!cached && oldest_line > -1)
	{
		L1.cache[oldest_line].tag = frameNum;
		L1.cache[oldest_line].age = 0;
	}

	////////////////////////////////////////////////////

	// L2 Cache
	cached = 0;
	oldest_age = -1;
	oldest_line = -1;
	frameNum = mem_addr / L2.line_size;
	for (int i = 0; i < L2.lines_per_set; i++)
	{
		// Found
		if (L2.cache[i].tag == frameNum)
		{
			// Increase age for all occupied lines
			for (int i = 0; i < L2.lines_per_set; i++)
			{
				if (L2.cache[i].age > -1)
				{
					++L2.cache[i].age;
				}
			}

			printf("%#x: Retrieved from L2, Frame: %d\n", mem_addr, frameNum);
			return L1.latency + L2.latency;
		}
	}

	// L2: Check for empty/old lines and cache memory
	for (int i = 0; i < L2.lines_per_set; i++)
	{
		// Empty
		if (L2.cache[i].tag == -1)
		{
			L2.cache[i].tag = frameNum;
			L2.cache[i].age = 0;
			cached = 1;
			break;
		}

		// Find oldest age within the set
		if (L2.cache[i].age > oldest_age)
		{
			oldest_age = L2.cache[i].age;
			oldest_line = i;
		}
	}

	// L2: Replace oldest line
	if (!cached && oldest_line > -1)
	{
		L2.cache[oldest_line].tag = frameNum;
		L2.cache[oldest_line].age = 0;
	}

	////////////////////////////////////////////////////

	// L3 Cache
	cached = 0;
	oldest_age = -1;
	oldest_line = -1;
	frameNum = mem_addr / L3.line_size;
	for (int i = 0; i < L3.lines_per_set; i++)
	{
		// Found
		if (L3.cache[i].tag == frameNum)
		{
			// Increase age for all occupied lines
			for (int i = 0; i < L3.lines_per_set; i++)
			{
				if (L3.cache[i].age > -1)
				{
					++L3.cache[i].age;
				}
			}

			printf("%#x: Retrieved from L3, Frame: %d\n", mem_addr, frameNum);
			return L1.latency + L2.latency + L3.latency;
		}
	}

	// L3: Check for empty lines and cache memory
	for (int i = 0; i < L3.lines_per_set; i++)
	{
		// Empty
		if (L3.cache[i].tag == -1)
		{
			L3.cache[i].tag = frameNum;
			L3.cache[i].age = 0;
			cached = 1;
			break;
		}

		// Find oldest age within the set
		if (L3.cache[i].age > oldest_age)
		{
			oldest_age = L3.cache[i].age;
			oldest_line = i;
		}
	}

	// L3: Replace oldest line
	if (!cached && oldest_line > -1)
	{
		L3.cache[oldest_line].tag = frameNum;
		L3.cache[oldest_line].age = 0;
	}

	////////////////////////////////////////////////////

	// RAM
	printf("%#x: Retrieved from RAM\n", mem_addr);
	return L1.latency + L2.latency + L3.latency + RAM.latency;

} /* END OF FILE */
