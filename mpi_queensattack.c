#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <mpi.h>

#define BLOCKSIZE 8
#define EMPTY_CELL "[ ]"
#define PIECE_CELL "[*]"

typedef unsigned long long ULL;
typedef unsigned long UL;
typedef unsigned char BYTE;

typedef bool BOOL;

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifdef MPI_INCLUDED

#else

#endif

/*
* 4,294,967,295 is the max value of a side,
*/
/// <summary>
/// the size of a side of the board
/// </summary>
UL side = 0;

/*
* the unsigned long long max value is 18446744073709551615, so this is the limit of a board size.
*/
/// <summary>
/// total size of the board
/// </summary>
ULL size = 0;

/// <summary>
/// the number of blocks to store a board, each block equals a byte
/// </summary>
ULL blocks = 0;

/// <summary>
/// the number of pieces on the board
/// </summary>
ULL pieces = 0;

/// <summary>
/// the numer of pieces that every queen must attack
/// </summary>
BYTE attack = 0;

/// <summary>
/// whether to print the location
/// </summary>
BOOL printlocation = FALSE;

/// <summary>
/// wraparound
/// </summary>
BOOL wraparound = FALSE;

/// <summary>
/// Initialize a new board
/// </summary>
/// <param name="blocks">how many blocks</param>
/// <param name="size">how many cells on a board</param>
/// <returns></returns>
BYTE *InitializeBoard(ULL blocks, ULL size);

void ReleaseBoard(BYTE board[]);

int main(int argc, char *argv[])
{
    int procid, numprocs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    if (argc == 5)
    {

        side = (UL)atol(argv[1]); //the size of a side of the board
        if (side > 4294967295)
        {
            printf("N must be less than 4294967295\r\n");
            exit(1);
        }
        size = side * side;

        attack = atoi(argv[2]); //the numer of pieces that every queen must attack
        if (attack > 9 || attack < 0)
        {
            printf("k must be between 0 and 9.\r\n");
            exit(1);
        }

        if (atoi(argv[3]) == 0) //format
        {
            printlocation = FALSE;
        }
        else
        {
            printlocation = TRUE;
        }

        if (atoi(argv[4]) == 0) //wraparound
        {
            wraparound = FALSE;
        }
        else
        {
            wraparound = TRUE;
        }

        /*
		* calcuate how many bytes are needed that store the whole board, 如果不能整除8，则再加一个字节
		*/
        blocks = size / BLOCKSIZE + (size % BLOCKSIZE != 0 ? 1 : 0);
    }
    else
    {
        printf("Usage: %s N k l w\r\n", argv[0]);
        printf("\t*N is the size of a side of the board\r\n");
        printf("\t*k is the numer of pieces that every queen must attack\r\n");
        printf("\t*l = 0 is short format N, k:number: and l = 1 is long format N, k : number : pc1, pc2, pc3...\r\n");
        printf("\t*w = 0 is no wraparound, w = 1 is wraparound\r\n");
        exit(1);
    }

    printf("Run in process:%d/%d with %llu(%lu*%lu) board in %llu blocks(bytes),every queen must attack %d.\r\n", procid, numprocs, size, side, side, blocks, attack);

    //Initialize a new board
    BYTE *board = InitializeBoard(blocks, size);

    int threadId = 0;

    while (pieces < size)
    {
        if (threadId == procid)
        {
            
        }

        if (++threadId >= numprocs)
        {
            threadId = 0;
        }
        pieces++;
    }

    //Release board
    ReleaseBoard(board);

    MPI_Finalize();
    //exit with success
    exit(0);
}

BYTE *InitializeBoard(ULL blocks, ULL size)
{
    BYTE *board = (BYTE *)calloc(blocks, sizeof(BYTE));

    return board;
}

void ReleaseBoard(BYTE board[])
{
    free(board);
}
