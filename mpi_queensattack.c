#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <mpi.h>

#define PRINTEVERYBOARD

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

/// <summary>
/// Add a piece on board sequentially
/// </summary>
/// <param name="board">board address</param>
/// <param name="size">board size</param>
/// <returns>board address</returns>
BYTE *AddPieceToBoard(BYTE board[], ULL blocks);

/// <summary>
/// Get how many pieces on the board
/// </summary>
/// <param name="board">board address</param>
/// <param name="blocks">the number of blocks of the board</param>
/// <param name="size">the number of the size of the board</param>
/// <returns>the number of pieces</returns>
ULL GetPiecesCount(BYTE board[], ULL blocks, ULL size);

void PrintBoard(BYTE board[], ULL blocks, ULL size, UL side);

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

    /**
     * the number of pieces on the board
    */
    ULL pieces = 0;

    int threadId = 0;

    while (pieces < size)
    {
        if (threadId == procid)
        {
            AddPieceToBoard(board, blocks);
            pieces = GetPiecesCount(board, blocks, size);

            // BOOL result = CheckBoard(board, size, side, attack, wraparound);
            // if (result == TRUE)
            // {
            //     boardsCount++;

            //     boards = (BOARD *)realloc(boards, boardsCount * sizeof(BOARD));
            //     boards[boardsCount - 1].pieces = pieces;
            //     boards[boardsCount - 1].board = InitializeBoard(blocks, size);
            //     memcpy(boards[boardsCount - 1].board, board, blocks * sizeof(BYTE));
            // }
#ifdef PRINTEVERYBOARD
            PrintBoard(board, blocks, size, side);
            printf("pieces:%lld\r\n", pieces);
            printf("--------------------------------------------\r\n");
#endif
        }

        if (++threadId >= numprocs)
        {
            threadId = 0;
        }
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

BYTE *AddPieceToBoard(BYTE board[], ULL blocks)
{
    BOOL carry = FALSE;
    unsigned int block = 0;

    if (++board[0] == 0)
    {
        carry = TRUE;
    }
    else
    {
        carry = FALSE;
    }

    for (block = 1; block < blocks; block++)
    {
        if (carry == TRUE)
        {
            if (++board[block] == 0)
            {
                carry = TRUE;
            }
            else
            {
                carry = FALSE;
            }
        }
        else
        {
            break;
        }
    }
    return board;
}

ULL GetPiecesCount(BYTE board[], ULL blocks, ULL size)
{
    unsigned int pieces = 0;
    do
    {
        blocks--;
        unsigned int bit = 0;
        do
        {
            bit = --size % 8;
            if ((board[blocks] & 1 << bit) > 0)
            {
                pieces++;
            }
            else
            {
            }

        } while (bit > 0);
    } while (blocks > 0);
    return pieces;
}

void PrintBoard(BYTE board[], ULL blocks, ULL size, UL side)
{
    /*
	* 假设有如下5*5棋盘
	* 10110
	* 01110
	* 00001
	* 11011
	* 10010
	* 总共25个位置，需要25个bit存储，但每8bit为一字节，所以需要4个字节(32个bit)
	* 高位在左，低位在右，右对齐
	* 76543210 size除以8的余数，左移相应位数，表示当前需要获取的bit
	*  byte[3]  byte[2]	 byte[1]  byte[0]
	* 01234567 01234567 01234567 01234567	//每个字节内表示棋子状态
	* -------1 01100111 00000111 01110010	//棋子
	*
	* (1 << 0) = 00000001
	* (1 << 1) = 00000010
	* (1 << 2) = 00000100
	* (1 << 3) = 00001000
	* (1 << 4) = 00010000
	* (1 << 5) = 00100000
	* (1 << 6) = 01000000
	* (1 << 7) = 10000000
	*/

    do
    {

        //从0开始，所以需要最大数减一
        blocks--;
        BYTE bit = 0;
        do
        {
            //size也是从0开始，计算时需要最大数减一
            bit = --size % 8;
            //通过按位与运算得知该位是否为1
            printf((board[blocks] & 1 << bit) > 0 ? PIECE_CELL : EMPTY_CELL);
            //如果当前bit刚好可以整除边长，说明需要换行
            if (size % side == 0)
                printf("\r\n");
        } while (bit > 0); //当bit大于0时，说明仍需要取下一位，
    } while (blocks > 0);  //当blocks大于0时，说明仍然需要取下一字节
}