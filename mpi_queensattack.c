#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include <mpi.h>

#define BLOCKSIZE 8
#define EMPTY_CELL "[ ]"
#define PIECE_CELL "[*]"

#define MPI_TAG_MAX_PIECES 0
#define MPI_BCAST_ROOT 0

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

typedef struct
{
    ULL pieces;
    BYTE *board;
} BOARD;

typedef struct
{
    UL x;
    UL y;
    ULL index;
} COORDINATE;

#ifdef MPI_INCLUDED

#else

#endif

int procid;

int numprocs;

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
void PrintBoardIndex(BYTE board[], ULL blocks, ULL size, ULL side);

/// <summary>
/// Check a board
/// </summary>
/// <param name="board">board address</param>
/// <param name="size">the number of the size of the board</param>
/// <param name="side"></param>
/// <param name="attack"></param>
/// <param name="wraparound"></param>
/// <returns></returns>
BOOL CheckBoard(BYTE board[], ULL size, UL side, BYTE attack, BOOL wraparound);

/// <summary>
/// Get the coordinate of a location
/// </summary>
/// <param name="locationIndex"></param>
/// <param name="side"></param>
/// <returns></returns>
COORDINATE GetCoordinate(ULL locationIndex, UL side);

/// <summary>
/// Get location from a coordinate
/// </summary>
/// <param name="c"></param>
/// <param name="side"></param>
/// <returns></returns>
ULL GetLocationIndex(COORDINATE *c, unsigned int side);

/// <summary>
/// Get the piece on a board
/// </summary>
/// <param name="board"></param>
/// <param name="blocks"></param>
/// <param name="size"></param>
/// <param name="cell"></param>
/// <returns></returns>
BOOL GetPieceOnBoard(BYTE board[], ULL blocks, ULL size, ULL cell);

BOOL CheckUp(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckDown(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckLeft(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckRight(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckLefttop(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckRighttop(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckLeftbottom(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);
BOOL CheckRightbottom(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound);

ULL GetMaxPiecesFromBoards(BOARD boards[], ULL boardsCount);

void mpi_print(int procid, int numprocs, const char *format, ...);

#define _p(msg) mpi_print(procid, numprocs, msg)
void mpi_print(int procid, int numprocs, const char *format, ...)
{
    printf("[%d/%d]", procid, numprocs);
    va_list valist;
    va_start(valist, format);
    printf(format, valist);
    va_end(valist);
}

int main(int argc, char *argv[])
{

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
     * chessboards satisfied condition
    */
    BOARD *boards = NULL;

    /**
     * the number of pieces on the board
    */
    ULL pieces = 0;

    /**
     * the number of chessboard satisfied condition
    */
    ULL boardsCount = 0;

    /**
     * 
    */
    int threadId = 0;

    while (pieces < size)
    {
        AddPieceToBoard(board, blocks);
        pieces = GetPiecesCount(board, blocks, size);

        if (threadId == procid)
        {
            BOOL result = CheckBoard(board, size, side, attack, wraparound);
            if (result == TRUE)
            {
                boardsCount++;
                boards = (BOARD *)realloc(boards, boardsCount * sizeof(BOARD));
                boards[boardsCount - 1].pieces = pieces;
                boards[boardsCount - 1].board = InitializeBoard(blocks, size);
                memcpy(boards[boardsCount - 1].board, board, blocks * sizeof(BYTE));

#ifdef satisfiedboard
                PrintBoard(boards[boardsCount - 1].board, blocks, size, side);
                printf("pieces:%lld\r\n", boards[boardsCount - 1].pieces);
                printf("--------------------------------------------\r\n");
#endif
            }
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

    _p("All boards exhausted.\r\n");
    ULL maxPieces = GetMaxPiecesFromBoards(boards, boardsCount);
    printf("[%d/%d]Max pieces:%llu\r\n", procid, numprocs, maxPieces);

    if (procid == 0)
    {
        //Waiting for result
        // int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,MPI_Comm comm, MPI_Status * status)
        int i = 0;
        for (i = 1; i < numprocs; i++)
        {
            ULL max = 0;
            MPI_Status status;
            int r = MPI_Recv(&max, 1, MPI_UNSIGNED_LONG_LONG, i, MPI_TAG_MAX_PIECES, MPI_COMM_WORLD, &status);
            if (r != MPI_SUCCESS)
            {
                fprintf(stderr, "MPI_Recv with error: %d.\r\n", r);
            }

            if (max > maxPieces)
            {
                maxPieces = max;
            }
        }
        printf("[%d/%d]Final max pieces:%llu\r\n", procid, numprocs, maxPieces);
    }
    else
    {
        //Sending result to process 0
        // MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)

        int r = MPI_Send(&maxPieces, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_TAG_MAX_PIECES, MPI_COMM_WORLD);
        if (r != MPI_SUCCESS)
        {
            fprintf(stderr, "MPI_Send with error: %d.\r\n", r);
        }
    }

    //int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
    MPI_Bcast(&maxPieces, 1, MPI_UNSIGNED_LONG_LONG, MPI_BCAST_ROOT, MPI_COMM_WORLD);

    //Export result
    
    if(procid == 0){
        ExportResult();
        MPI_Send(procid+1);
    }else{
        if(procid != numprocs-1){
            MPI_Recv(procid-1,);
            ExportResult();
            MPI_Send(procid+1,);
        }else{
            MPI_Recv(procid-1,);
            ExportResult();
        }
    }

    ULL i = 0;
    for (i = 0; i < boardsCount; i++)
    {
        if (boards[i].pieces == maxPieces)
        {
            printf("[%d/%d]%lu, %d:%llu:", procid, numprocs, side, attack, maxPieces);
            if (printlocation == TRUE)
            {
                PrintBoardIndex(boards[i].board, blocks, size, side);
            }
            else
            {
                printf("\r\n");
            }

#ifdef PRINTBOARD
            PrintBoard(boards[i].board, blocks, size, side);
            printf("--------------------------------------------\r\n");
#endif
        }
    }

    //Release board
    ReleaseBoard(board);
    do
    {
        boardsCount--;
        free(boards[boardsCount].board);
    } while (boardsCount > 0);
    free(boards);

    MPI_Finalize();
    //exit with success
    exit(0);
}

ULL GetMaxPiecesFromBoards(BOARD boards[], ULL boardsCount)
{
    ULL maxPieces = 0;
    ULL i = 0;
    for (i = 0; i < boardsCount; i++)
    {
        if (boards[i].pieces > maxPieces)
        {
            maxPieces = boards[i].pieces;
        }
    }

    return maxPieces;
}

// void ExportResult()
// {
//     //Export result
//     ULL maxPieces = 0;
//     ULL i = 0;
//     for (i = 0; i < boardsCount; i++)
//     {
//         if (boards[i].pieces > maxPieces)
//         {
//             maxPieces = boards[i].pieces;
//         }
//     }

//     printf("Max pieces is:%llu.\r\n", maxPieces);

//     for (i = 0; i < boardsCount; i++)
//     {
//         if (boards[i].pieces == maxPieces)
//         {
//             printf("%lu, %d:%llu:", side, attack, maxPieces);
//             if (printlocation == TRUE)
//             {
//                 // PrintBoardIndex(boards[i].board, blocks, size, side);
//             }
//             else
//             {
//                 printf("\r\n");
//             }

// #ifdef PRINTBOARD
//             PrintBoard(boards[i].board, blocks, size, side);
//             printf("--------------------------------------------\r\n");
// #endif
//         }
//     }
// }

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

void PrintBoardIndex(BYTE board[], ULL blocks, ULL size, ULL side)
{
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
            if ((board[blocks] & 1 << bit) > 0)
            {
                printf("%llu,", side * side - (blocks * 8 + bit));
            }

        } while (bit > 0); //当bit大于0时，说明仍需要取下一位，
    } while (blocks > 0);  //当blocks大于0时，说明仍然需要取下一字节
    printf("\r\n");
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

BOOL CheckBoard(BYTE board[], ULL size, UL side, BYTE attack, BOOL wraparound)
{
    //current position
    ULL p = size;
    do
    {
        /*													y
		*	08 07 06					(3,3)	(2,3)	(1,3)
		*	05 04 03					(3,2)	(2,2)	(1,2)
		*	02 01 00				x 	(3,1)	(2,1)	(1,1)
		*
		*															y
		*	15 14 13 12					(4,4)	(3,4)	(2,4)	(1,4)
		*	11 10 09 08					(4,3)	(3,3)	(2,3)	(1,3)
		*	07 06 05 04					(4,2)	(3,2)	(2,2)	(1,2)
		*	03 02 01 00				x	(4,1)	(3,1)	(2,1)	(1,1)
		*
		*																	y
		*	24 23 22 21 20				(5,5)	(4,5)	(3,5)	(2,5)	(1,5)
		*	19 18 17 16 15				(5,4)	(4,4)	(3,4)	(2,4)	(1,4)
		*	14 13 12 11 10				(5,3)	(4,3)	(3,3)	(2,3)	(1,3)
		*	09 08 07 06 05				(5,2)	(4,2)	(3,2)	(2,2)	(1,2)
		*	04 03 02 01 00			x	(5,1)	(4,1)	(3,1)	(2,1)	(1,1)
		*/
        p--;

        if (GetPieceOnBoard(board, blocks, size, p) == FALSE)
        {
            continue;
        }

        COORDINATE cc = GetCoordinate(p, side);

#ifdef DEBUG
        printf("origin cell:%llu(%lu,%lu)\r\n", p, cc.x, cc.y);
#endif
        BYTE k = 0;

        BOOL *pieceAttacked = (BOOL *)malloc(size * sizeof(BOOL));
        memset(pieceAttacked, FALSE, size * sizeof(BOOL));

        k = (int)CheckUp(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckDown(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckLeft(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckRight(board, cc, blocks, size, side, pieceAttacked, wraparound);

        k += (int)CheckLefttop(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckRighttop(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckLeftbottom(board, cc, blocks, size, side, pieceAttacked, wraparound);
        k += (int)CheckRightbottom(board, cc, blocks, size, side, pieceAttacked, wraparound);

        free(pieceAttacked);

        if (k != attack)
        {
#ifdef DEBUG
            printf("Piece %llu(%lu,%lu) attacks %d more than %d, skip. \r\n", cc.index, cc.x, cc.y, k, attack);
#endif
            return FALSE;
        }
#ifdef DEBUG
        printf("-------------------------------------------------------------\r\n");
#endif

    } while (p > 0);

    return TRUE;
}
BOOL CheckRightbottom(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //rightbottom
        if (coordinate.x <= step)
        {
            if (wraparound)
            {
                p.x = coordinate.x + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.x = coordinate.x - step;
        }

        if (coordinate.y <= step)
        {
            if (wraparound)
            {
                p.y = coordinate.y + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.y = coordinate.y - step;
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckRightbottom piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }
#ifdef DEBUG
        printf("CheckRightbottom\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif
        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckLeftbottom(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //leftbottom
        p.x = coordinate.x + step;
        if (p.x > side)
        {
            if (wraparound)
            {
                p.x -= side;
            }
            else
            {
                break;
            }
        }

        if (coordinate.y <= step)
        {
            if (wraparound)
            {
                p.y = coordinate.y + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.y = coordinate.y - step;
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckLeftbottom piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }
#ifdef DEBUG
        printf("CheckLeftbottom\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif
        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckRighttop(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //righttop
        if (coordinate.x <= step)
        {
            if (wraparound)
            {
                p.x = coordinate.x + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.x = coordinate.x - step;
        }

        p.y = coordinate.y + step;
        if (p.y > side)
        {
            if (wraparound)
            {
                p.y -= side;
            }
            else
            {
                break;
            }
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckRighttop piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }

#ifdef DEBUG
        printf("CheckRighttop\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif

        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckLefttop(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //lefttop
        p.x = coordinate.x + step;
        p.y = coordinate.y + step;
        if (p.x > side)
        {
            if (wraparound)
            {
                p.x -= side;
            }
            else
            {
                break;
            }
        }
        if (p.y > side)
        {
            if (wraparound)
            {
                p.y -= side;
            }
            else
            {
                break;
            }
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckLefttop piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }

#ifdef DEBUG
        printf("CheckLefttop\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif

        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckRight(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //right
        if (coordinate.x <= step)
        {
            if (wraparound)
            {
                p.x = coordinate.x + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.x = coordinate.x - step;
        }

        p.y = coordinate.y;

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckRight piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }
#ifdef DEBUG
        printf("CheckRight\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif

        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckLeft(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //left
        p.x = coordinate.x + step;
        if (p.x > side)
        {
            if (wraparound)
            {
                p.x -= side;
            }
            else
            {
                break;
            }
        }
        p.y = coordinate.y;

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckLeft piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }

#ifdef DEBUG
        printf("CheckLeft\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif
        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckDown(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //bottom
        p.x = coordinate.x;

        if (coordinate.y <= step)
        {
            if (wraparound)
            {
                p.y = coordinate.y + side - step;
            }
            else
            {
                break;
            }
        }
        else
        {
            p.y = coordinate.y - step;
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckDown piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif

            break;
        }

#ifdef DEBUG
        printf("CheckDown\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif

        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

BOOL CheckUp(BYTE board[], COORDINATE coordinate, ULL blocks, ULL size, UL side, BOOL pieceAttacked[], BOOL wraparound)
{
    BOOL result = FALSE;
    UL step = 1;
    for (step = 1; step < side; step++)
    {
        COORDINATE p;

        //top
        p.x = coordinate.x;
        p.y = coordinate.y + step;
        if (p.y > side)
        {
            if (wraparound)
            {
                p.y -= side;
            }
            else
            {
                break;
            }
        }

        GetLocationIndex(&p, side);

        if (pieceAttacked[p.index] == TRUE)
        {
#ifdef DEBUG
            printf("CheckUp piece %llu(%lu,%lu) has been attacked.\r\n", p.index, p.x, p.y);
#endif
            break;
        }

#ifdef DEBUG
        printf("CheckUp\t\t\tstep:%lu, %llu(%lu,%lu)\r\n", step, p.index, p.x, p.y);
#endif

        if (GetPieceOnBoard(board, blocks, size, p.index) == TRUE)
        {
            result = TRUE;
            pieceAttacked[p.index] = result;
            break;
        }
    }
    return result;
}

COORDINATE GetCoordinate(ULL locationIndex, UL side)
{
    COORDINATE c;
    c.index = locationIndex + 1;
    int m = c.index % side;
    if (m == 0)
    {
        c.y = (UL)(c.index / side);
        c.x = side;
    }
    else
    {
        c.y = (UL)(c.index / side + 1);
        c.x = m;
    }

    return c;
}

ULL GetLocationIndex(COORDINATE *c, unsigned int side)
{
    c->index = (c->y * side - (side - 1)) + (c->x - 1) - 1;
    return c->index;
}

BOOL GetPieceOnBoard(BYTE board[], ULL blocks, ULL size, ULL cell)
{
    /*
	* locate a piece on board
	*/
    BYTE bit = cell % 8;
    ULL block = cell / 8;
#ifdef DEBUG
    printf("cell:%llu, bit:%d, block:%llu\r\n", cell, bit, block);
#endif
    return (board[block] & 1 << bit) ? TRUE : FALSE;
}