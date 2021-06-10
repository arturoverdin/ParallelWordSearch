#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int x[] = { -1, 0, 0, 1 };
int y[] = { 0, -1, 1, 0 };
int word_length(char* word){
	//printf("Word_length: %s\n", word);
	int length = 0;
	while(!isspace(word[length]) && word[length] != '\r' &&
		word[length] != '\n' && word[length] != '\0'){
		length++;
	}
	//printf("Length of %s is %d\n", word, length);
	return length;
}

void deep_copy(char *dest, char *source){
	for(int i = 0; i < word_length(source); i++){
		dest[i] = source[i];
	}
	dest[word_length(source)] = '\0';
}

char **generate_grid(char* fileName, int *row, int *col){
    FILE *fp;
    char *line = NULL;
    char **grid = NULL;
    size_t len = 0;
    ssize_t line_size;
    int line_count = 0;
    char delim[] = " ";

    fp = fopen(fileName, "r");
    if(fp == NULL){
        printf("File not found. Exiting\n");
        return NULL;
    }

    line_size = getline(&line, &len, fp);

    while(line_size >= 0){
    	if(line_count > 0){
    		grid[line_count-1] = malloc(word_length(line)+1);
    		deep_copy(grid[line_count-1], line);
    		printf("%02d: %s\n", line_count-1, grid[line_count-1]);
    	}else{
    		//printf("Parsing first line of grid\n");
    		char *firstLine = strtok(line, delim);
            grid = (char**) malloc (sizeof(char*)*atoi(firstLine));
            *row = atoi(firstLine);
            firstLine = strtok(NULL, delim);
            *col = atoi(firstLine);
            printf("rowcount:%d, colcount:%d\n", *row, *col);
    	}
    	line_count++;
    	line_size = getline(&line, &len, fp);
    }

    fclose(fp);
    return grid;
}

char **get_wordbank(char* fileName, int *word_count){
    FILE *fp;
    char *line = NULL;
    char **bank = NULL;
    size_t len = 0;
    ssize_t line_size;
    int line_count = 0;
    char delim[] = " ";

    fp = fopen(fileName, "r");
    if(fp == NULL){
        printf("File not found. Exiting\n");
        return NULL;
    }

    line_size = getline(&line, &len, fp);

    while(line_size >= 0){
    	if(line_count > 0){
    		bank[line_count-1] = malloc(word_length(line));
    		deep_copy(bank[line_count-1], line);
    		//printf("%d: %s\n", line_count-1, bank[line_count-1]);
    	}else{
    		//printf("Parsing first line of wordbank\n");
    		*word_count = atoi(line);
            bank = (char**) malloc (sizeof(char*)*(*word_count));
            printf("wordcount:%d\n", *word_count);
    	}
    	line_count++;
    	line_size = getline(&line, &len, fp);
    }

    free(line);
    line = NULL;
    fclose(fp);
    return bank;
}

int main(int argc, char *argv[])
{
	int npes, myrank;
	int num;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);  // total number of processes
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;
	int num_col, num_row; // will be passed by reference by the grid parser


	if(argc < 2){
		printf("Insufficient arguments.");
		return -1;
	}

    struct timespec start, stop;
    double time;

    int row_count, col_count;
    char **grid = generate_grid(argv[1], &row_count, &col_count);
    if(grid == NULL){
        printf("ERROR, NO GRID.\n");
        return -1;
    }

    printf("Grid generated\n");

    for(int i = 0; i < row_count; i++){
    	printf("%02d: %s\n", i, grid[i]);
    }

    int word_count;
    char **words = get_wordbank(argv[2], &word_count);
    if(words == NULL){
        printf("ERROR, NO WORDBANK.\n");
        return -1;
    }

    printf("Words generated\n");

    for(int i = 0; i < word_count; i++){
    	printf("%02d: %s\n", i, words[i]);
    }

    int length_per_thread = word_count / num_threads;

	int logOfthreads = log(npes)/log(2);
	// will assume that the num of columns and rows in the word search will be even
	int rowPartitions = num_row / logOfthreads; 
	int colPartitions = num_col / logOfthreads; 
	//20x20
	//10x10 each
	//4 npes

	//


	// calculate the sub matrix ids for each processor. 
	// Proc 0: subRowIdx = 0 and subColIdx = 0...
	int subRowIdx = rowPartitions * myrank;
	int subColIdx = colPartitions * myrank;

	// calculates the id's for the shared rows and columns (above, below, left, and right)
	int tShared = subRowIdx - 1;
	int bShared = subRowIdx + 1;
	int lShared = subColIdx - 1;
	int rShared = subColIdx + 1;



	char myWord = 

	//if left col then send to threadid)-1
	//if right col send to threadid + 1
	//if top send to threadid - log(numOfThreads)
	//if bottom send to threadid + log(numOfTheads)
	
	// 0 1
	// 2 3

	// 0 1 2 3 
	// 4 5 6 7 
	// 8 9 10 11 
	// 12 13 14 15

	// adj matrix


	//each proc will for loop through the sub matrix and check  
	for(int row = subRowIdx - 1; row < subRowIdx + rowPartitions + 1; row++) 
	{
		if( row < 0 || row > num_row){
			continue;
		}
		for(int col = subColIdx - 1; col < subColIdx + colPartitions + 1; col++) 
		{
			if( col < 0 || col > num_col){
				continue;
			}

			if( )
			// for each index, checks surrounding letters.
			for(int dir = 0; dir < 4; dir++) 
			{
				int k;
				int rd = row + x[dir];
				int cd = col + y[dir];
				int flagBreak = 0;

				// char** word = [rd][cd]

				for(k = 1; k < word.length(); k++) 
				{

					// checks for any out of bounds
					if(rd >= rowPartitions || rd < 0 || cd >= colPartitions || cd > 0) 
					{
						break;
					}
				
					// check to see if we are in a shared column or row from a neighboring process, recieve the information 
					// they will send to check if we have part of a word continuing into this sub matrix
					if(( rd == tShared || rd == bShared || cd == rShared || cd == lShared) && *(grid + rd*colPartitions+cd)->discovered > 0) 
					{
						
						MPI_Recv(&squareDiscovery, 1, MPI_INT, /*source proc */, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						k = squareDiscovery;

					}

					//set square to -1 to let others know that this cell has been discovered and is not useful.
					if(grid[rd*colPartitions][cd])->letter != word[k] && *(grid+rd*colPartitions+cd)->discovered == 0) 
					{
						*(grid+rd*colPartitions+cd)->discovered = -1;
						flagBreak = 1;
					}

					*(grid+rd*colPartitions+cd)->discovered = k;

					if(rd == subRowIdx || cd == subColIdx || rd == subRowIdx + rowPartitions || cd == subColIdx + colPartitions) {

						// we are at a square that must be sent to a neighboring processor. 
						if(flagBreak == 1) {
							MPI_Send(&flagBreak, 1, MPI_INT, /*dest*/, 0, MPI_COMM_WORLD);
							break;
						} else {
							MPI_Send(&k, 1, MPI_INT, /*dest*/, 0, MPI_COMM_WORLD);
						}
					}
					rd += x[dir], cd += y[dir];
				}

				if(k == len) printf("From process %d: Word %s is Found, done!\n", myrank, word);
			}
		}
	}

	MPI_Finalize();
	return 0;
}