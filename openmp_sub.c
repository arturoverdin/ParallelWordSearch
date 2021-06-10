#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

int x[] = { -1, 0, 0, 1 };
int y[] = { 0, -1, 1, 0 };
int *checkWords;

// -1 if already found, 0 if cannot be found, 1 if found
int search_grid(char **grid, int row, int col,
               char *word, int R, int C, int word_index)
{
    // If first character of word doesn't
    // match with given starting point in grid.
    if (grid[row][col] != word[0])
        return 0;
 
    int len = strlen(word);
 
    // Search word in all 4 directions
    // starting from (row, col)
    for (int dir = 0; dir < 4; dir++) {
        // Initialize starting point
        // for current direction
        int k, rd = row + y[dir], cd = col + x[dir];

        for (k = 1; k < len; k++) { // Match remaining letters
            if(checkWords[word_index] == 1){
                return -1;
            }
            if (rd >= R || rd < 0 || cd >= C || cd < 0){
                break;
            }
            if(grid[rd][cd] != word[k]){
            	break;
            }
            // Moving in particular direction
            rd += y[dir], cd += x[dir];
        }
 
        // If all character matched, then value of k must
        // be equal to length of word
        if (k == len){
            return 1;
        }
    }
    return 0;
}
 
// Searches given word in a given
// matrix in all 4 directions
// R and C should be a thread's local boundaries
void pattern_search(char **grid, char *word, int R, int C, int rStart, int cStart, int word_index)
{
	//printf("Searching for %s size: %d\n", word, strlen(word));
    for (int row = rStart; row < R; row++){
        for (int col = cStart; col < C; col++){
            int searchResult = search_grid(grid, row, col, word, R, C, word_index);
            if (searchResult == 1){
                printf("pattern found for %s at row: %d, col: %d\n", word, row, col);
                checkWords[word_index] = 1;
                return;
            }else if(searchResult == -1){
                return;
            }
        }
    }
}

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
    		//printf("%02d: %s\n", line_count-1, grid[line_count-1]);
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

// execute program like ./run NUMOFTHREADS WORDGRID.TXT WORDBANK.TXT
int main(int argc, char*argv[]){
	if(argc < 3){
		printf("Insufficient arguments.");
		return -1;
	}

    int num_threads = atoi(argv[1]);
    struct timespec start, stop;
    double time;

    int row_count, col_count;
    char **grid = generate_grid(argv[2], &row_count, &col_count);
    if(grid == NULL){
        printf("ERROR, NO GRID.\n");
        return -1;
    }

    printf("Grid generated\n");

    int word_count;
    char **words = get_wordbank(argv[3], &word_count);
    if(words == NULL){
        printf("ERROR, NO WORDBANK.\n");
        return -1;
    }

    printf("Words generated\n");

    checkWords = malloc(sizeof(int) * word_count);
    for(int i = 0; i < word_count; i++){
        checkWords[i] = 0;
    }

    int logOfThreads = log(num_threads)/log(2);
    // will assume that the num of columns and rows in the word search will be even
    int rowPartitions = row_count / logOfThreads; 
    int colPartitions = col_count / logOfThreads; 

    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) { perror( "clock gettime" );}

    omp_set_num_threads(num_threads);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int subRowIdx = rowPartitions * (thread_id / logOfThreads);
        int subColIdx = colPartitions * (thread_id % logOfThreads);
        /*printf("thread_id:%d, subRowIdx:%d, subColIdx:%d, rowPartitions:%d, colPartitions:%d\n",
            thread_id, subRowIdx, subColIdx, rowPartitions, colPartitions);*/
        for(int i = 0; i < word_count; i++){
            pattern_search(grid, words[i], subRowIdx + rowPartitions, subColIdx + colPartitions, subRowIdx, subColIdx, i);
        }
    }

    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror( "clock gettime" );}
    time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;

    printf("Execution time = %f sec\n", time);

    for(int i = 0; i < row_count; i++){
    	free(grid[i]);
    }
    free(grid);

    for(int i = 0; i < word_count; i++){
    	free(words[i]);
    }
    free(words);
    free(checkWords);
    return 0;
}
