#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#define FILENAME_LENGTH 100
#define FILECONTENT_LENGTH 5000

//a struct to store the matrix information
typedef struct
{
    int rows;
    int cols;
    double elements[FILECONTENT_LENGTH][FILECONTENT_LENGTH];
} Matrix;

//a struct to store the parameters that are sent to the (per element) thread function
typedef struct
{
    int rowNumber;
    int colNumber;
} Params;

//file declarations
FILE *input_file1, *input_file2, *output_file1, *output_file2, *output_file3;

//the matrices are defined on the global scope
Matrix matrix1, matrix2, per_matrix, per_row, per_element;

//read_files: reads in the number of parameters given in the console and the names of the files
void read_files(int argc, char* argv[])
{
    char* filename = (char*)malloc(FILENAME_LENGTH);    //works as a temporary space in memory to hold the name of the file until we assign it to the global file variable
    //./matMultp x y z case
    if(argc == 4){
        input_file1 = fopen(strcat(strcpy(filename, argv[1]), ".txt"), "r+");
        input_file2 = fopen(strcat(strcpy(filename, argv[2]), ".txt"), "r+");
        if(input_file1 == NULL || input_file2 == NULL){
            printf("Error: couldn't read input files\n");
            exit(1);
        }
        output_file1 = fopen(strcat(strcpy(filename, argv[3]), "_per_matrix.txt"), "w+");
        output_file2 = fopen(strcat(strcpy(filename, argv[3]), "_per_row.txt"), "w+");
        output_file3 = fopen(strcat(strcpy(filename, argv[3]), "_per_element.txt"), "w+");
    }else if(argc == 1){    //./matMultp case
        input_file1 = fopen("a.txt", "r+");
        input_file2 = fopen("b.txt", "r+");
        if(input_file1 == NULL || input_file2 == NULL){
            printf("Error: couldn't read input files\n");
            exit(1);
        }
        output_file1 = fopen("c_per_matrix.txt", "w+");
        output_file2 = fopen("c_per_row.txt", "w+");
        output_file3 = fopen("c_per_element.txt", "w+");
    }else{  //error in reading the given arguments
        printf("Error: couldn't read input files\n");
        exit(1);    //quit with error code 1
    }

    //read the first input file
    fseek(input_file1, 0L, SEEK_END);
    long numbytes1 = ftell(input_file1);
    fseek(input_file1, 0L, SEEK_SET);
    char* input_file1_text = (char*)calloc(numbytes1, sizeof(char));
    char* input_file1_text_start = input_file1_text;
    if(input_file1_text == NULL){   //error in reading the first input file
        printf("Error: couldn't read input files\n");
        exit(1);    //quit with error code 1
    }
    fread(input_file1_text, sizeof(char), numbytes1, input_file1);

    //read the second input file
    fseek(input_file2, 0L, SEEK_END);
    long numbytes2 = ftell(input_file2);
    fseek(input_file2, 0L, SEEK_SET);
    char* input_file2_text = (char*)calloc(numbytes2, sizeof(char));
    char* input_file2_text_start = input_file2_text;
    if(input_file2_text == NULL){   //error in reading the second input file
        printf("Error: couldn't read input files\n");
        exit(1);    //quit with error code 1
    }
    fread(input_file2_text, sizeof(char), numbytes2, input_file2);

    //parsing text (matrix 1)
    while(*input_file1_text != '=' && *input_file1_text != '\0') input_file1_text++;
    matrix1.rows = strtol(++input_file1_text, (char**)NULL, 10);
    while(*input_file1_text != '=' && *input_file1_text != '\0') input_file1_text++;
    matrix1.cols = strtol(++input_file1_text, (char**)NULL, 10);
    if(matrix1.rows == 0 || matrix1.cols == 0){     //detect an empty matrix
        printf("Error: invalid dimensions(1).\n");
        exit(2);    //quit with error code 2
    }
    while(*input_file1_text != '\n') input_file1_text++;
    input_file1_text++;
    for(int i = 0; i < matrix1.rows; ++i){
        for(int j = 0; j < matrix1.cols; ++j){
            int temp = 0, neg = 0;
            if(*input_file1_text == '-') neg = 1, input_file1_text++;
            while(*input_file1_text != ' ' && *input_file1_text != '\n' && *input_file1_text != '\0') input_file1_text++, temp++;
            char* temp2 = (char*)malloc(temp);
            input_file1_text -= temp;
            strncpy(temp2, input_file1_text, temp);
            matrix1.elements[i][j] = strtod(temp2, NULL);
            if(neg) matrix1.elements[i][j] *= -1;
            free(temp2);
            while(*input_file1_text != ' ' && *input_file1_text != '\n' && *input_file1_text != '\0') input_file1_text++;
            if(*input_file1_text != '\0'){
                while(*input_file1_text == ' ' || *input_file1_text == '\n' || *input_file1_text == '\0') input_file1_text++;
            }else{
                if(i != matrix1.rows - 1 || j != matrix1.cols - 1){     //reach the end of the file but the matrix is not completed yet
                    printf("Error: The provided Matrix Dimensions (1) does not match the given elements.\n");
                    exit(3);    //quit with error code 3
                }
            }
        }
    }

    //parsing text (matrix 2)
    while(*input_file2_text != '=' && *input_file2_text != '\0') input_file2_text++;
    matrix2.rows = strtol(++input_file2_text, (char**)NULL, 10);
    while(*input_file2_text != '=' && *input_file2_text != '\0') input_file2_text++;
    matrix2.cols = strtol(++input_file2_text, (char**)NULL, 10);
    if(matrix2.rows == 0 || matrix2.cols == 0){     //detect an empty matrix
        printf("Error: invalid dimensions(2).\n");
        exit(4);    //quit with error code 4
    }
    while(*input_file2_text != '\n') input_file2_text++;
    input_file2_text++;
    for(int i = 0; i < matrix2.rows; ++i){
        for(int j = 0; j < matrix2.cols; ++j){
            int temp = 0, neg = 0;
            if(*input_file2_text == '-') neg = 1, input_file2_text++;
            while(*input_file2_text != ' ' && *input_file2_text != '\n' && *input_file2_text != '\0') input_file2_text++, temp++;
            char* temp2 = (char*)malloc(temp);
            input_file2_text -= temp;
            strncpy(temp2, input_file2_text, temp);
            matrix2.elements[i][j] = strtod(temp2, NULL);
            if(neg) matrix2.elements[i][j] *= -1;
            free(temp2);
            while(*input_file2_text != ' ' && *input_file2_text != '\n' && *input_file2_text != '\0') input_file2_text++;
            if(*input_file2_text != '\0'){
                while(*input_file2_text == ' ' || *input_file2_text == '\n' || *input_file2_text == '\0') input_file2_text++;
            }else{
                if(i != matrix2.rows - 1 || j != matrix2.cols - 1){     //reach the end of the file but the matrix is not completed yet
                    printf("Error: The provided Matrix Dimensions (2) does not match the given elements.\n");
                    exit(5);    //quit with error code 5
                }
            }
        }
    }

    //deallocating memory
    free(filename);
    free(input_file1_text_start);
    free(input_file2_text_start);
    fclose(input_file1);
    fclose(input_file2);
}

//checks if the two matrices can be multiplied
void check_for_dimensions()
{
    if(matrix1.cols != matrix2.rows){
        printf("Error: given matrices cannot be multiplied.\n");
        exit(6);    //quit with error code 6
    }
}

//multiplies the two matrices as a whole
void* multiply_per_matrix()
{
    for(int i = 0; i < per_matrix.rows; ++i){
        for(int j = 0; j < per_matrix.cols; ++j){
            per_matrix.elements[i][j] = 0;
            for(int k = 0; k < matrix1.cols; ++k){
                per_matrix.elements[i][j] += matrix1.elements[i][k] * matrix2.elements[k][j];
            }
        }
    }
    return NULL;
}

//multiplies the two matrices but evaluates each row at a time (thread per row)
void* multiply_per_row(void* param)
{
    int row_number = (int)param;
    for(int j = 0; j < per_row.cols; ++j){
        per_matrix.elements[row_number][j] = 0;
        for(int k = 0; k < matrix1.cols; ++k){
            per_row.elements[row_number][j] += matrix1.elements[row_number][k] * matrix2.elements[k][j];
        }
    }
    return NULL;
}

//multiplies the two matrices but evaluates each element at a time (thread per element)
void* multiply_per_element(void* params)
{
    Matrix *param_matrix = (Matrix*)params;
    int row_number = param_matrix -> rows;
    int col_number = param_matrix -> cols;
    per_matrix.elements[row_number][col_number] = 0;
    for(int k = 0; k < matrix1.cols; ++k){
        per_element.elements[row_number][col_number] += matrix1.elements[row_number][k] * matrix2.elements[k][col_number];
    }
    return NULL;
}

//console printing and generating a thread for the first method (thread per matrix)
void step1()
{
    printf("Starting Step 1: A Thread per Matrix..\n");
    fprintf(output_file1, "Method: A thread per matrix\n");
    per_matrix.rows = matrix1.rows;
    per_matrix.cols = matrix2.cols;
    pthread_t matrixThread;
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start time
    pthread_create(&matrixThread, NULL, multiply_per_matrix, NULL); //create one thread only for the whole matrix
    pthread_join(matrixThread, NULL);   //call the join function for that thread
    gettimeofday(&stop, NULL);  //stop time
    for(int i = 0; i < per_matrix.rows; ++i){
        for(int j = 0; j < per_matrix.cols; ++j){
            fprintf(output_file1, "%0.3lf ", per_matrix.elements[i][j]);
        }
        fprintf(output_file1, "\n");
    }
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads: %d\n\n", 1);
    fclose(output_file1);
}

//console printing and generating threads for the second method (thread per row)
void step2()
{
    printf("Starting Step 2: A Thread per Row..\n");
    fprintf(output_file2, "Method: A thread per row\n");
    per_row.rows = matrix1.rows;
    per_row.cols = matrix2.cols;
    pthread_t rowThreads[per_row.rows];
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start time
    for(int i = 0; i < per_row.rows; ++i){  //create a thread for each row to be evaluated
        pthread_create(&rowThreads[i], NULL, multiply_per_row, (void*)i);
    }
    for(int i = 0; i < per_row.rows; ++i){  //call the join function on each of the created threads
        pthread_join(rowThreads[i], NULL);
    }
    gettimeofday(&stop, NULL); //stop time
    for(int i = 0; i < per_row.rows; ++i){
        for(int j = 0; j < per_row.cols; ++j){
            fprintf(output_file2, "%0.3lf ", per_row.elements[i][j]);
        }
        fprintf(output_file2, "\n");
    }
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads: %d\n\n", per_row.rows);
    fclose(output_file2);
}

//console printing and generating threads for the third method (thread per element)
void step3()
{
    printf("Starting Step 3: A Thread per Element..\n");
    fprintf(output_file3, "Method: A thread per element\n");
    per_element.rows = matrix1.rows;
    per_element.cols = matrix2.cols;
    Params params[per_element.rows][per_element.cols];
    pthread_t elementsThreads[per_element.rows][per_element.cols];
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start time
    for(int i = 0; i < per_element.rows; ++i){  //create a thread for each element to be evaluated
        for(int j = 0; j < per_element.cols; ++j){
            params[i][j].rowNumber = i;
            params[i][j].colNumber = j;
            pthread_create(&elementsThreads[i][j], NULL, multiply_per_element, &params[i][j]);
        }
    }
    for(int i = 0; i < per_element.rows; ++i){  //call the join function on each of the created threads
        for(int j = 0; j < per_element.cols; ++j){
            pthread_join(elementsThreads[i][j], NULL);
        }
    }
    gettimeofday(&stop, NULL); //stop time
    for(int i = 0; i < per_element.rows; ++i){
        for(int j = 0; j < per_element.cols; ++j){
            fprintf(output_file3, "%0.3lf ", per_element.elements[i][j]);
        }
        fprintf(output_file3, "\n");
    }
    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Number of Threads: %d\n\n", per_element.rows * per_element.cols);
    fclose(output_file3);
}

//driver code
int main(int argc, char* argv[])
{
    read_files(argc, argv);
    check_for_dimensions();
    step1();
    step2();
    step3();

    return 0;
}
