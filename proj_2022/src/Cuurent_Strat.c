/* vim: :se ai :se sw=4 :se ts=4 :se sts :se et */

/*H**********************************************************************
 *
 *    This is a skeleton to guide development of Othello engines that can be used
 *    with the Ingenious Framework and a Tournament Engine. 
 *
 *    The communication with the referee is handled by an implementaiton of comms.h,
 *    All communication is performed at rank 0.
 *
 *    Board co-ordinates for moves start at the top left corner of the board i.e.
 *    if your engine wishes to place a piece at the top left corner, 
 *    the "gen_move_master" function must return "00".
 *
 *    The match is played by making alternating calls to each engine's 
 *    "gen_move_master" and "apply_opp_move" functions. 
 *    The progression of a match is as follows:
 *        1. Call gen_move_master for black player
 *        2. Call apply_opp_move for white player, providing the black player's move
 *        3. Call gen move for white player
 *        4. Call apply_opp_move for black player, providing the white player's move
 *        .
 *        .
 *        .
 *        N. A player makes the final move and "game_over" is called for both players
 *    
 *    IMPORTANT NOTE:
 *        Write any (debugging) output you would like to see to a file. 
 *        	- This can be done using file fp, and fprintf()
 *        	- Don't forget to flush the stream
 *        	- Write a method to make this easier
 *        In a multiprocessor version 
 *        	- each process should write debug info to its own file 
 *H***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <mpi.h>
#include <time.h>
#include <assert.h>
#include "comms.h"

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int MAX = 1000;
const int MIN = -1000;
const int MAXDEPTH = 6;

const int OUTER = 3;
const int ALLDIRECTIONS[8] = {-11, -10, -9, -1, 1, 9, 10, 11};
const int BOARDSIZE = 100;

const int LEGALMOVSBUFSIZE = 65;
const char piecenames[4] = {'.', 'b', 'w', '?'};

void run_master(int argc, char *argv[]);
int initialise_master(int argc, char *argv[], int *time_limit, int *my_colour, FILE **fp);
void gen_move_master(char *move, int my_colour, FILE *fp);
void apply_opp_move(char *move, int my_colour, FILE *fp);
void game_over();
void run_worker();
void initialise_board();
void free_board();

void legal_moves(int player, int *moves, FILE *fp);
int legalp(int move, int player, FILE *fp);
int validp(int move);
int would_flip(int move, int dir, int player, FILE *fp);
int opponent(int player, FILE *fp);
int find_bracket_piece(int square, int dir, int player, FILE *fp);
int random_strategy(int my_colour, FILE *fp);
void make_move(int move, int player, FILE *fp);
void make_flips(int move, int dir, int player, FILE *fp);
int get_loc(char *movestring);
void get_move_string(int loc, char *ms);
void print_board(FILE *fp);
char nameof(int piece);
int count(int player, int *board);

int location_strategy(int my_colour, FILE *fp);
int find_highestPos(int *moves);
int max(int num1, int num2);
int min(int num1, int num2);
int minimax_score(int depth, int bMaxMin, int my_colour, FILE *fp, int alpha, int beta);
int minimax_strategy(int my_colour, FILE *fp);
int evaluatePosition(int my_colour, FILE *fp);
int evaluateMobility(int my_colour, FILE *fp);
int evaluateDiscDifference(int my_colour, FILE *fp);
void duplicateBoard(int *board, int *cBoard);
int evaluateStability(int my_colour, FILE *fp);
int evaluateCorners(int my_colour, FILE *fp);
int evaluateGameTime(int my_colour, FILE *fp);

int boardWeighted[8][8] = {{100, -10, 11, 6, 6, 11, -10, 100},
						   {-10, -20, 1, 2, 2, 1, -20, -10},
						   {10, 1, 5, 4, 4, 5, 1, 10},
						   {6, 2, 4, 2, 2, 4, 2, 6},
						   {6, 2, 4, 2, 2, 4, 2, 6},
						   {10, 1, 5, 4, 4, 5, 1, 10},
						   {-10, -20, 1, 2, 2, 1, -20, -10},
						   {100, -10, 11, 6, 6, 11, -10, 100}};

int stabilityWeights2[8][8] = {{4, -3, 3, 2, 2, 3, -3, 4},
							   {-3, -4, -1, -1, -1, -1, -4, -3},
							   {3, -1, 1, 0, 0, 1, -1, 3},
							   {2, -1, 0, 1, 1, 0, -1, 2},
							   {2, -1, 0, 1, 1, 0, -1, 2},
							   {3, -1, 1, 0, 0, 1, -1, 3},
							   {-3, -4, -1, -1, -1, -1, -4, -3},
							   {4, -3, 3, 2, 2, 3, -3, 4}};

int cornersWeights[8][8] = {{10, 1, 5, 3, 3, 5, 1, 10},
							{1, 0, 2, 2, 2, 2, 0, 1},
							{5, 2, 2, 1, 1, 2, 2, 5},
							{3, 2, 2, 2, 2, 2, 2, 3},
							{3, 2, 2, 2, 2, 2, 2, 3},
							{5, 2, 2, 2, 2, 2, 2, 5},
							{1, 0, 2, 2, 2, 2, 0, 1},
							{10, 1, 5, 3, 3, 5, 1, 10}};

int *board;

int main(int argc, char *argv[])
{
	int rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//MPI_Comm_size(MPI_COMM_WORLD, &size);
	initialise_board(); //one for each process

	if (rank == 0)
	{
		run_master(argc, argv);
	}
	else
	{
		run_worker(rank);
	}
	game_over();
}

void run_master(int argc, char *argv[])
{
	char cmd[CMDBUFSIZE];
	char my_move[MOVEBUFSIZE];
	char opponent_move[MOVEBUFSIZE];
	int time_limit;
	int my_colour;
	int running = 0;
	FILE *fp = NULL;

	if (initialise_master(argc, argv, &time_limit, &my_colour, &fp) != FAILURE)
	{
		running = 1;
	}
	if (my_colour == EMPTY)
		my_colour = BLACK;
	// Broadcast my_colour

	while (running == 1)
	{
		/* Receive next command from referee */
		if (comms_get_cmd(cmd, opponent_move) == FAILURE)
		{
			fprintf(fp, "Error getting cmd\n");
			fflush(fp);
			running = 0;
			break;
		}

		/* Received game_over message */
		if (strcmp(cmd, "game_over") == 0)
		{
			running = 0;
			fprintf(fp, "Game over\n");
			fflush(fp);
			break;

			/* Received gen_move message */
		}
		else if (strcmp(cmd, "gen_move") == 0)
		{
			// Broadcast running
			MPI_Bcast(&running, 1, MPI_INT, 0, MPI_COMM_WORLD);

			// Broadcast board
			MPI_Bcast(board, 1, MPI_INT, 0, MPI_COMM_WORLD);

			gen_move_master(my_move, my_colour, fp);
			print_board(fp);

			if (comms_send_move(my_move) == FAILURE)
			{
				running = 0;
				fprintf(fp, "Move send failed\n");
				fflush(fp);
				break;
			}

			/* Received opponent's move (play_move mesage) */
		}
		else if (strcmp(cmd, "play_move") == 0)
		{
			apply_opp_move(opponent_move, my_colour, fp);
			print_board(fp);

			/* Received unknown message */
		}
		else
		{
			fprintf(fp, "Received unknown command from referee\n");
		}
	}
	// Broadcast running
	MPI_Bcast(&running, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

int initialise_master(int argc, char *argv[], int *time_limit, int *my_colour, FILE **fp)
{
	int result = FAILURE;

	if (argc == 5)
	{
		unsigned long ip = inet_addr(argv[1]);
		int port = atoi(argv[2]);
		*time_limit = atoi(argv[3]);

		*fp = fopen(argv[4], "w");
		if (*fp != NULL)
		{
			fprintf(*fp, "Initialise communication and get player colour \n");
			if (comms_init_network(my_colour, ip, port) != FAILURE)
			{
				result = SUCCESS;
			}
			fflush(*fp);
		}
		else
		{
			fprintf(stderr, "File %s could not be opened", argv[4]);
		}
	}
	else
	{
		fprintf(*fp, "Arguments: <ip> <port> <time_limit> <filename> \n");
	}

	return result;
}

void initialise_board()
{
	int i;
	board = (int *)malloc(BOARDSIZE * sizeof(int));
	for (i = 0; i <= 9; i++)
		board[i] = OUTER;
	for (i = 10; i <= 89; i++)
	{
		if (i % 10 >= 1 && i % 10 <= 8)
			board[i] = EMPTY;
		else
			board[i] = OUTER;
	}
	for (i = 90; i <= 99; i++)
		board[i] = OUTER;
	board[44] = WHITE;
	board[45] = BLACK;
	board[54] = BLACK;
	board[55] = WHITE;
}

void free_board()
{
	free(board);
}
void free_this_board(int *this_board)
{
	free(this_board);
}

/**
 *   Rank i (i != 0) executes this code 
 *   ----------------------------------
 *   Called at the start of execution on all ranks except for rank 0.
 *   - run_worker should play minimax from its move(s) 
 *   - results should be send to Rank 0 for final selection of a move 
 */
void run_worker()
{
	int running = 0;
	// Broadcast colour
	int my_colour;
	MPI_Bcast(&my_colour, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Broadcast running
	MPI_Bcast(&running, 1, MPI_INT, 0, MPI_COMM_WORLD);

	while (running == 1)
	{
		// Broadcast board
		MPI_Bcast(board, 100, MPI_INT, 0, MPI_COMM_WORLD);
		// Generate move
		//minimax strategy
		//MPI_gather receiving best value
		// Broadcast running
		MPI_Bcast(&running, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
}

/**
 *  Rank 0 executes this code: 
 *  --------------------------
 *  Called when the next move should be generated 
 *  - gen_move_master should play minimax from its move(s)
 *  - the ranks may communicate during execution 
 *  - final results should be gathered at rank 0 for final selection of a move 
 */
void gen_move_master(char *move, int my_colour, FILE *fp)
{
	int loc;

	/* generate move */
	// loc = location_strategy(my_colour, fp); //random_strategy
	loc = minimax_strategy(my_colour, fp);

	if (loc == -1)
	{
		strncpy(move, "pass\n", MOVEBUFSIZE);
	}
	else
	{
		/* apply move */
		get_move_string(loc, move);
		make_move(loc, my_colour, fp);
	}
}

void apply_opp_move(char *move, int my_colour, FILE *fp)
{
	int loc;
	if (strcmp(move, "pass\n") == 0)
	{
		return;
	}
	loc = get_loc(move);
	make_move(loc, opponent(my_colour, fp), fp);
}

void game_over()
{
	free_board();
	MPI_Finalize();
}

void get_move_string(int loc, char *ms)
{
	int row, col, new_loc;
	new_loc = loc - (9 + 2 * (loc / 10));
	row = new_loc / 8;
	col = new_loc % 8;
	ms[0] = row + '0';
	ms[1] = col + '0';
	ms[2] = '\n';
	ms[3] = 0;
}

int get_loc(char *movestring)
{
	int row, col;
	/* movestring of form "xy", x = row and y = column */
	row = movestring[0] - '0';
	col = movestring[1] - '0';
	return (10 * (row + 1)) + col + 1;
}

void legal_moves(int player, int *moves, FILE *fp)
{
	int move, i;
	moves[0] = 0;
	i = 0;
	for (move = 11; move <= 88; move++)
		if (legalp(move, player, fp))
		{
			i++;
			moves[i] = move;
		}
	moves[0] = i;
}

int legalp(int move, int player, FILE *fp)
{
	int i;
	if (!validp(move))
		return 0;
	if (board[move] == EMPTY)
	{
		i = 0;
		while (i <= 7 && !would_flip(move, ALLDIRECTIONS[i], player, fp))
			i++;
		if (i == 8)
			return 0;
		else
			return 1;
	}
	else
		return 0;
}

int validp(int move)
{
	if ((move >= 11) && (move <= 88) && (move % 10 >= 1) && (move % 10 <= 8))
		return 1;
	else
		return 0;
}

int would_flip(int move, int dir, int player, FILE *fp)
{
	int c;
	c = move + dir;
	if (board[c] == opponent(player, fp))
		return find_bracket_piece(c + dir, dir, player, fp);
	else
		return 0;
}

int find_bracket_piece(int square, int dir, int player, FILE *fp)
{
	while (board[square] == opponent(player, fp))
		square = square + dir;
	if (board[square] == player)
		return square;
	else
		return 0;
}

int opponent(int player, FILE *fp)
{
	if (player == BLACK)
		return WHITE;
	if (player == WHITE)
		return BLACK;
	fprintf(fp, "illegal player\n");
	return EMPTY;
}

int random_strategy(int my_colour, FILE *fp)
{
	int r;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);

	legal_moves(my_colour, moves, fp);
	if (moves[0] == 0)
	{
		return -1;
	}
	srand(time(NULL));
	r = moves[(rand() % moves[0]) + 1];
	free(moves);
	return (r);
}

int location_strategy(int my_colour, FILE *fp)
{
	int r;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);

	legal_moves(my_colour, moves, fp);
	if (moves[0] == 0)
	{
		return -1;
	}
	int best_loc;
	srand(time(NULL));
	best_loc = find_highestPos(moves);
	int cnt, i, j;
	for (i = 1; i <= moves[0]; i++)
	{
		for (j = 0; j < 8; j++)
		{
			cnt = would_flip(moves[i], ALLDIRECTIONS[j], my_colour, fp);
			fprintf(fp, "my_c=%d move=%d would flip=%d in dir %d\n", my_colour, moves[i], cnt, ALLDIRECTIONS[j]);
		}
	}

	r = moves[best_loc];

	free(moves);
	return (r);
}
int find_highestPos(int *moves)
{
	int x, y;
	int max = -21;
	int max_i = 0;
	for (int i = 1; i <= moves[0]; i++)
	{
		x = moves[i] / 10;
		y = moves[i] % 10;
		int val = boardWeighted[x - 1][y - 1];
		if (val > max)
		{
			max = val;
			max_i = i;
		}
	}
	return max_i;
}

int minimax_strategy(int my_colour, FILE *fp)
{
	int i, loc, best_score, best_move = 0, score;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);
	int *original_board = (int *)malloc(BOARDSIZE * sizeof(int));
	duplicateBoard(board, original_board); //copied original state of board

	legal_moves(my_colour, moves, fp); //all possible moves
	if (moves[0] == 0)
	{
		return -1; //no moves
	}
	else
	{
		best_score = MIN;
		for (i = 1; i <= moves[0]; i++)
		{
			duplicateBoard(original_board, board);
			loc = moves[i];
			make_move(loc, my_colour, fp);
			score = minimax_score(1, 1, opponent(my_colour, fp), fp, MIN, MAX);
			if (score > best_score)
			{
				best_score = score;
				best_move = moves[i];
			}
		}
		fprintf(fp, "best score for board =%d at %d\n", best_score, best_move);
		duplicateBoard(original_board, board); //reset board to original_board before move
		free(moves);
		free_this_board(original_board);

		return best_move;
	}
}
int minimax_score(int depth, int bMaxMin, int my_colour, FILE *fp, int alpha, int beta)
{
	int i;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);
	int *original_board = (int *)malloc(BOARDSIZE * sizeof(int));
	duplicateBoard(board, original_board); //copied original state of board
	int best;

	if (depth == MAXDEPTH)
	{
		duplicateBoard(original_board, board); //reset board to original_board before move
		free(moves);
		free_this_board(original_board);
		if (bMaxMin == 0)
		{
			return evaluatePosition(my_colour, fp); //colour for max?
		}
		return evaluatePosition(opponent(my_colour, fp), fp); //colour for max?
	}
	legal_moves(my_colour, moves, fp); //all possible moves
	if (moves[0] == 0)
	{
		return -1; //no moves
	}

	if (bMaxMin == 0)
	{
		best = MIN;
		for (i = 1; i <= moves[0]; i++)
		{
			duplicateBoard(original_board, board);
			make_move(moves[i], my_colour, fp);
			int score = minimax_score(depth + 1, 1, opponent(my_colour, fp), fp, alpha, beta);
			best = max(best, score);
			alpha = max(alpha, best);

			if (beta <= alpha)
			{
				break; //prune
			}
		}
		duplicateBoard(original_board, board); //reset board to original_board before move
		free(moves);
		free_this_board(original_board);
		//return best;
	}
	else
	{
		best = MAX;
		for (i = 1; i <= moves[0]; i++)
		{
			duplicateBoard(original_board, board);
			make_move(moves[i], my_colour, fp);
			int score = minimax_score(depth + 1, 0, opponent(my_colour, fp), fp, alpha, beta);
			best = min(best, score);
			beta = min(beta, best);

			if (beta <= alpha)
			{
				break; //prune
			}
		}
		duplicateBoard(original_board, board); //reset board to original_board before move
		free(moves);
		free_this_board(original_board);
		//return best;
	}
	return best;
}

/**
 * Find maximum between two numbers.
 */
int max(int num1, int num2)
{
	return (num1 > num2) ? num1 : num2;
}

/**
 * Find minimum between two numbers.
 */
int min(int num1, int num2)
{
	return (num1 > num2) ? num2 : num1;
}

int evaluatePosition(int my_colour, FILE *fp)
{
	int mobilityScore = evaluateMobility(my_colour, fp);
	int discDifference = evaluateDiscDifference(my_colour, fp);
	int stabilityScore = evaluateStability(my_colour, fp);
	int cornerEdgeScore = evaluateCorners(my_colour, fp);
	int gameTime = evaluateGameTime(my_colour, fp);

	if (gameTime == 0)
	{
		return  ((stabilityScore) + (mobilityScore * 5)+ (discDifference) + (cornerEdgeScore))/4;
	}
	else if (gameTime == 1)
	{
		return  ((stabilityScore * 2) + (mobilityScore * 3) + (discDifference) + (cornerEdgeScore*2))/4;
	}
	else
	{
		return  ((stabilityScore*3) + (mobilityScore * 2) + (discDifference*5) + (cornerEdgeScore*3))/4;
	}
	//return 2 * mobilityScore + discDifference;
	// return (stabilityScore * 0.3) + (mobilityScore * 0.3) + (discDifference * 0.05) + (cornerEdgeScore * 0.35);
}

int evaluateMobility(int my_colour, FILE *fp)
{
	int *playerMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	int *opponentMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));

	legal_moves(my_colour, playerMoves, fp);
	legal_moves(opponent(my_colour, fp), opponentMoves, fp);

	if ((playerMoves[0] + opponentMoves[0])==0){
		return 0;
	}
	return 100 * (playerMoves[0] - opponentMoves[0]) / (playerMoves[0] + opponentMoves[0]);
}

int evaluateStability(int my_colour, FILE *fp)
{
	int playerScore = 0, opponentScore = 0;
	for (int i = 11; i <= 88; i++)
	{
		int x = i / 10;
		int y = i % 10;
		int val = stabilityWeights2[x - 1][y - 1];
		if (board[i] == my_colour)
		{ //black
			playerScore += val;
		}
		else if (board[i] == opponent(my_colour, fp))
		{ //white
			opponentScore += val;
		}
	}
	if ((playerScore + opponentScore) == 0)
	{
		return 0;
	}
	return 100 * (playerScore - opponentScore) / (playerScore + opponentScore);
}

int evaluateCorners(int my_colour, FILE *fp)
{
	int playerScore = 0, opponentScore = 0;
	for (int i = 11; i <= 88; i++)
	{
		int x = i / 10;
		int y = i % 10;
		int val = cornersWeights[x - 1][y - 1];
		if (board[i] == my_colour)
		{ //black
			playerScore += val;
		}
		else if (board[i] == opponent(my_colour, fp))
		{ //white
			opponentScore += val;
		}
	}
	if ((playerScore + opponentScore) == 0)
	{
		return 0;
	}
	return 100 * (playerScore - opponentScore) / (playerScore + opponentScore);
}

int evaluateDiscDifference(int my_colour, FILE *fp)
{
	int playerScore = 0, opponentScore = 0;
	for (int i = 11; i <= 88; i++)
	{
		if (board[i] == my_colour)
		{
			playerScore++;
		}
		if (board[i] == opponent(my_colour, fp))
		{
			opponentScore++;
		}
	}

	return 100 * (playerScore - opponentScore) / (playerScore + opponentScore);
}

int evaluateGameTime(int my_colour, FILE *fp)
{
	int playerScore = 0, opponentScore = 0;
	for (int i = 11; i <= 88; i++)
	{
		if (board[i] == my_colour)
		{
			playerScore++;
		}
		if (board[i] == opponent(my_colour, fp))
		{
			opponentScore++;
		}
	}
	int total_discs = ((playerScore + opponentScore)/2)-4;
	if (total_discs < 10)
	{
		return 0; //early stages
	}
	else if ((total_discs >= 10) && (total_discs < 20))
	{
		return 1; //mid stages
	}
	else
	{
		return 2; //final stages
	}
}

void duplicateBoard(int *board, int *cBoard)
{
	for (int i = 0; i < BOARDSIZE; i++)
	{
		cBoard[i] = board[i];
	}
}

void make_move(int move, int player, FILE *fp)
{
	int i;
	board[move] = player;
	for (i = 0; i <= 7; i++)
		make_flips(move, ALLDIRECTIONS[i], player, fp);
}

void make_flips(int move, int dir, int player, FILE *fp)
{
	int bracketer, c;
	bracketer = would_flip(move, dir, player, fp);
	if (bracketer)
	{
		c = move + dir;
		do
		{
			board[c] = player;
			c = c + dir;
		} while (c != bracketer);
	}
}

void print_board(FILE *fp)
{
	int row, col;
	fprintf(fp, "   1 2 3 4 5 6 7 8 [%c=%d %c=%d]\n",
			nameof(BLACK), count(BLACK, board), nameof(WHITE), count(WHITE, board));
	for (row = 1; row <= 8; row++)
	{
		fprintf(fp, "%d  ", row);
		for (col = 1; col <= 8; col++)
			fprintf(fp, "%c ", nameof(board[col + (10 * row)]));
		fprintf(fp, "\n");
	}
	fflush(fp);
}

char nameof(int piece)
{
	assert(0 <= piece && piece < 5);
	return (piecenames[piece]);
}

int count(int player, int *board)
{
	int i, cnt;
	cnt = 0;
	for (i = 1; i <= 88; i++)
		if (board[i] == player)
			cnt++;
	return cnt;
}
