#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define GRID_SIZE 9
#define SUBGRID_SIZE 3
#define NUM_THREADS 4

int Sol[GRID_SIZE][GRID_SIZE];  // Sudoku solution grid
int Row[GRID_SIZE] = {0};        // Array rows
int Col[GRID_SIZE] = {0};        // Array columns
int Sub[GRID_SIZE] = {0};        // Array sub-grids
int Counter = 0;                 // Counter for valid rows, columns, and sub-grids

pthread_mutex_t mutex;
pthread_cond_t cond ;
int threads_finished = 0;

// read Sudoku from file and initialize Sol array
void initializeSol(const char *filename, int Sol[GRID_SIZE][GRID_SIZE]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read each element from the file and store it in the Sol array
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (fscanf(file, "%d", &Sol[i][j]) != 1) {
                fprintf(stderr, "Error reading from file\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
            
        }
        
    }

    fclose(file);
}

// Method to set a value in the Row array
void setRowValue(int index, int value) {
    pthread_mutex_lock(&mutex);
    Row[index] = value;
    pthread_mutex_unlock(&mutex);
}

// Method to set a value in the Col array
void setColValue(int index, int value) {
    pthread_mutex_lock(&mutex);
    Col[index] = value;
    pthread_mutex_unlock(&mutex);
}

// Method to set a value in the SubGrid array
void setSubValue(int index, int value) {
    pthread_mutex_lock(&mutex);
    Sub[index] = value;
    pthread_mutex_unlock(&mutex);
}

// Method to update the Counter 
void updateCounter(int value) {
    pthread_mutex_lock(&mutex);
    Counter += value;
    pthread_mutex_unlock(&mutex);
}

void validateRowsAndSubgrids(int start_row, int end_row, int threadID) {
    int isValidRow = 1;
    int isValidSubgrid = 1;
    int isThreadValid = 1; // Flag to track thread validity
    char invalidRows[100] = ""; // String to store invalid rows info
    char invalidSubgrids[100] = ""; // String to store invalid subgrids info

    // Check rows
    for (int i = start_row; i < end_row; i++) {
        int rowCheck[GRID_SIZE] = {0};
        for (int j = 0; j < GRID_SIZE; j++) {
            int num = Sol[i][j];
            if (num < 1 || num > 9 || rowCheck[num - 1] != 0) {
                isValidRow = 0;
                break;
            }
            rowCheck[num - 1] = 1;
        }
        if (!isValidRow) {
            sprintf(invalidRows + strlen(invalidRows), "row %d, ", i + 1);
            isThreadValid = 0; // Set thread validity flag to false
        }
        if (isValidRow) {
            setRowValue(i, 1); 
            updateCounter(1); // Increment Counter if row is valid
        }
        isValidRow = 1;
    }
    sleep(1);

    // Check subgrids
    for (int i = start_row; i < end_row; i += SUBGRID_SIZE) {
        for (int j = 0; j < GRID_SIZE; j += SUBGRID_SIZE) {
            int subgridCheck[GRID_SIZE] = {0};
            for (int k = i; k < i + SUBGRID_SIZE; k++) {
                for (int l = j; l < j + SUBGRID_SIZE; l++) {
                    int num = Sol[k][l];
                    if (num < 1 || num > 9 || subgridCheck[num - 1] != 0) {
                        isValidSubgrid = 0;
                        break;
                    }
                    subgridCheck[num - 1] = 1;
                }
            }
            if (!isValidSubgrid) {
                int subgridIndex = (i / SUBGRID_SIZE) * SUBGRID_SIZE + (j / SUBGRID_SIZE);
                sprintf(invalidSubgrids + strlen(invalidSubgrids), "sub-grid %d, ", subgridIndex + 1);
                isThreadValid = 0; // Set thread validity flag to false
            }
            if (isValidSubgrid) {
                int subgridIndex = (i / SUBGRID_SIZE) * SUBGRID_SIZE + (j / SUBGRID_SIZE);
                setSubValue(subgridIndex, 1); 
                updateCounter(1); // Increment Counter if sub-grid is valid
            }
            isValidSubgrid = 1;
        }
    }
    sleep(1);

    // Print thread validation result along with invalid rows and subgrids information
    if (isThreadValid) {
        printf("Thread ID-%d %lu: valid\n", threadID,(unsigned long)pthread_self());
    } else {
        printf("Thread ID-%d %lu: %s%sare invalid\n", threadID,(unsigned long)pthread_self(), invalidRows, invalidSubgrids);
    }
}


// Method to validate columns (entire grid)
void validateColumns(int threadID) {
    int isValidColumn = 1;
     int isThreadValid = 1; 
    char invalidColumn[100] = ""; 

    for (int j = 0; j < GRID_SIZE; j++) {
        int colCheck[GRID_SIZE] = {0};
        for (int i = 0; i < GRID_SIZE; i++) {
            int num = Sol[i][j];
            if (num < 1 || num > 9 || colCheck[num - 1] != 0) {
                isValidColumn = 0;
                break;
            }
            colCheck[num - 1] = 1;
        }
        if (!isValidColumn) {
            sprintf(invalidColumn + strlen(invalidColumn), "column %d, ", j + 1);
            isThreadValid = 0; // Set thread validity flag to false
            
        }
        if (isValidColumn) {
            
            setColValue(j, 1); 
            updateCounter(1); // Increment Counter if column is valid
        }
        isValidColumn = 1;
    }
    sleep(1);

    // Print thread validation result along with invalid column
    if (isThreadValid) {
        printf("Thread ID-%d %lu: valid\n", threadID,(unsigned long)pthread_self());
    } else {
        printf("Thread ID-%d %lu: %s is invalid\n", threadID,(unsigned long)pthread_self(), invalidColumn);
    }
    printf("Thread ID-4 is the last thread\n");
}

// Struct to hold thread ID and delay
struct ThreadArgs {
    int threadID;
    int delay;
};


// Method to validate region to thread
void *validateRegion(void *param) {
    struct ThreadArgs *args = (struct ThreadArgs *)param;
    int threadID = args->threadID;
    int delay = args->delay;

    int start_row, end_row;

    switch (threadID) {
        case 1: // Rows 1-3 and subgrids 1-3

            start_row = 0;
            end_row = 3;
            validateRowsAndSubgrids(start_row, end_row, threadID);
            
            break;
        case 2: // Rows 4-6 and subgrids 4-6
            pthread_mutex_lock(&mutex);
                while (threads_finished < 1) {
                    pthread_cond_wait(&cond, &mutex); // Wait until the first 3 threads finish
                }
                pthread_mutex_unlock(&mutex);
            start_row = 3;
            end_row = 6;
            validateRowsAndSubgrids(start_row, end_row, threadID);
            
            break;
        case 3: // Rows 7-9 and subgrids 7-9
            pthread_mutex_lock(&mutex);
            while (threads_finished < 2) {
                pthread_cond_wait(&cond, &mutex); // Wait until the first 3 threads finish
            }
            pthread_mutex_unlock(&mutex);
            start_row = 6;
            end_row = GRID_SIZE;
            validateRowsAndSubgrids(start_row, end_row, threadID);
            
            break;
        case 4: // Validate columns (entire grid)
            pthread_mutex_lock(&mutex);
            while (threads_finished < 3) {
                pthread_cond_wait(&cond, &mutex); // Wait until the first 3 threads finish
            }
            pthread_mutex_unlock(&mutex);
            validateColumns(threadID);
            
           
            break;
    }

    // delay to observe synchronization issues
    sleep(delay);
    // Signal that  thread has finished its work
    pthread_mutex_lock(&mutex);
    threads_finished++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        printf("Usage: %s <solution> <delay>\n", argv[0]);
        return 1;
    }

    // Retrieve command-line arguments
    const char *filename = argv[1];
    int delay = atoi(argv[2]);
    if (delay < 1 || delay > 10) {
        printf("Delay value must be between 1 and 10.\n");
        return 1;
    }

    // Initialize Sol array with Sudoku solution from file
    initializeSol(filename, Sol);

    pthread_t threads[NUM_THREADS];
    struct ThreadArgs threadArgs[NUM_THREADS] = {
        {1, delay},
        {2, delay},
        {3, delay},
        {4, delay}
    }; 

    // Create threads to validate rows, columns, and subgrids
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, validateRegion, &threadArgs[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print total number of valid rows, columns, and sub-grids
    printf("There are toal %d valid rows,columns and sub-grids,", Counter);

    // Check if Sudoku is valid
    if (Counter == 27) {
        printf("and the solution is valid.\n");
    } else {
        printf("and the solution is invalid.\n");
    }

    // Terminate
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
