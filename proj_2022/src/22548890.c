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
 *        	- Don't forget to flush the streamint alpha_sharing_top(int alpha)
 * https://www.geeksforgeeks.org/minimax-algorithm-in-game-theory-set-4-alpha-beta-pruning/?ref=lbp
 * https://www.javatpoint.com/mini-max-algorithm-in-ai
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <mpi.h>
#include <time.h>
#include <assert.h>
#include "comms.h"
#include <stdarg.h>
#include <unistd.h>

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int MAX = 1000000000;
const int MIN = -1000000000;
const int MAXDEPTH = 8;

const int OUTER = 3;
const int ALLDIRECTIONS[8] = {-11, -10, -9, -1, 1, 9, 10, 11};
const int BOARDSIZE = 100;

const int LEGALMOVSBUFSIZE = 65;
const char piecenames[4] = {'.', 'b', 'w', '?'};

void run_master(int argc, char *argv[], FILE *fp);
int initialise_master(int argc, char *argv[], int *time_limit, int *my_colour, FILE **fp);
void gen_move_master(char *move, int my_colour, FILE *fp);
void apply_opp_move(char *move, int my_colour, FILE *fp);
void game_over();
void run_worker(FILE *fp);
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
int evaluateCorner(int my_colour, FILE *fp);
int all_in_one(int my_colour, int d, int c, int s, int m, int e, int w);
void sortMoves(int *moves);
int get_best_loc(int *buff);
int alpha_sharing_top(int alpha, int my_rank);

int send_arrMovesScore[2];
int size;
int rank;

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
int best_val;
int alpha_sharing;

int main(int argc, char *argv[])
{

	FILE *fp = NULL;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	initialise_board(); //one for each process
	// double time = 0.0;
	// clock_t begin = clock();
	if (rank == 0)
	{
		run_master(argc, argv, fp);
	}
	else
	{

		run_worker(fp);
	}
	// clock_t end = clock();
	// time += (double)(end - begin) / CLOCKS_PER_SEC;
	game_over();
}

void run_master(int argc, char *argv[], FILE *fp)
{
	char cmd[CMDBUFSIZE];
	char my_move[MOVEBUFSIZE];
	char opponent_move[MOVEBUFSIZE];
	int time_limit;
	int my_colour;
	int running = 0;

	if (initialise_master(argc, argv, &time_limit, &my_colour, &fp) != FAILURE)
	{
		running = 1;
	}
	if (my_colour == EMPTY)
		my_colour = BLACK;
	// Broadcast my_colour
	MPI_Bcast(&my_colour, 1, MPI_INT, 0, MPI_COMM_WORLD);

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
			MPI_Bcast(board, BOARDSIZE, MPI_INT, 0, MPI_COMM_WORLD);
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

/**
 *   Rank i (i != 0) executes this code 
 *   ----------------------------------
 *   Called at the start of execution on all ranks except for rank 0.
 *   - run_worker should play minimax from its move(s) 
 *   - results should be send to Rank 0 for final selection of a move 
 */
void run_worker(FILE *fp)
{
	int running = 0;
	char my_move[MOVEBUFSIZE];
	// int *buff=(int *)malloc(size * 2 * sizeof(int));;
	// Broadcast colour
	int my_colour;
	MPI_Bcast(&my_colour, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Broadcast running
	MPI_Bcast(&running, 1, MPI_INT, 0, MPI_COMM_WORLD);

	while (running == 1)
	{
		// Broadcast board
		MPI_Bcast(board, BOARDSIZE, MPI_INT, 0, MPI_COMM_WORLD);
		// Generate move

		gen_move_master(my_move, my_colour, fp);
		// loc = minimax_strategy(my_colour, fp, &best_val);
		// send_arrMovesScore[0] = loc;
		// send_arrMovesScore[1] = best_val;
		//gen_move_master(my_move, my_colour, fp);
		//minimax strategy

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
	int *buff = (int *)malloc(size * 2 * sizeof(int));
	/* generate move */
	// loc = location_strategy(my_colour, fp); //random_strategy
	best_val = MIN;
	//dlegate legal moves to all proccesses

	loc = minimax_strategy(my_colour, fp);

	send_arrMovesScore[0] = loc;
	send_arrMovesScore[1] = best_val;

	if (rank == 0)
	{
		MPI_Gather(send_arrMovesScore, 2, MPI_INT, buff, 2, MPI_INT, 0, MPI_COMM_WORLD); //gathers (receive) move and score
		loc = get_best_loc(buff);
		// Debug("best loc %d", loc);														 //get best loc
		free(buff);

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
	else
	{
		MPI_Gather(send_arrMovesScore, 2, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD); //Gathers(sends) move and score
	}
}
/**
 * @brief Get the best loc object given in as array of 2
 * 
 * @param buff array of length 2
 * @return int returns best location of move
 */

int get_best_loc(int *buff)
{
	int best_loc = -1;
	int best_value = MIN;
	for (int i = 0; i < size; i++)
	{
		if (buff[i * 2 + 1] > best_value)
		{
			best_value = buff[i * 2 + 1];
			best_loc = buff[i * 2];
		}
	}
	return best_loc;
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
	{
		if (legalp(move, player, fp))
		{
			i++;
			moves[i] = move;
		}
	}
	moves[0] = i;
	// sortMoves(moves);
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

int location_strategy(int my_colour, FILE *fp) //initial strategy
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
int find_highestPos(int *moves) //only for location strategy
{
	int x, y;
	int max = -21;
	int max_i = 0;
	for (int i = 1; i <= moves[0]; i++)
	{
		x = moves[i] / 10;
		y = moves[i] % 10;
		int val = stabilityWeights2[x - 1][y - 1];
		if (val > max)
		{
			max = val;
			max_i = i;
		}
	}
	return max_i;
}
/**
 * @brief sorts moves based off stability to speed up pruning
 * 
 * @param moves given moves array
 */
void sortMoves(int *moves) //based on stability
{
	int *movesValues = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	movesValues[0] = 0;
	int x, y;
	for (int i = 1; i <= moves[0]; i++)
	{
		x = moves[i] / 10;
		y = moves[i] % 10;
		int val = (stabilityWeights2[x - 1][y - 1]);
		movesValues[i] = val;
	} //add move values to array

	for (int i = 1; i <= moves[0]; ++i)
	{
		for (int j = i + 1; j <= moves[0]; ++j)
		{
			if (movesValues[i] < movesValues[j])
			{
				int tempValue = movesValues[i];
				int tempMove = moves[i];
				movesValues[i] = movesValues[j];
				moves[i] = moves[j];
				movesValues[j] = tempValue;
				moves[j] = tempMove;
			}
		}
	}
	free(movesValues);
}
/**
 * @brief moves for each rank that assigned and returned as pointer rank_moves
 * 
 * @param my_colour colour
 * @param rank_moves pointer to each ranks moves
 * @param fp file
 */
void rank_legal_moves(int my_colour, int *rank_moves, FILE *fp)
{
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);
	legal_moves(my_colour, moves, fp);
	int counter = 0;
	if (moves[0] != 0)
	{
		for (int i = rank + 1; i <= moves[0]; i += size)
		{
			counter++;
			rank_moves[counter] = moves[i]; //assigning a move to rank_moves for each rank
		}
		rank_moves[0] = counter; //length of moves for rank_moves
		if (moves[0] < size)
		{ //excess ranks
			for (int r = size - 1; r >= moves[0]; r--)
			{
				if (rank == r)
				{
					rank_moves[0] = -1; //excess ranks dont need moves
				}
			}
		}
	}

	// Debug("rank move %d for rank %d", rank_moves[0], rank);
	free(moves);
}
/**
 * @brief decides best strategy move for player based on the best minimax score
 * 
 * @param my_colour players colour
 * @param fp file
 * @return int best move
 */

int minimax_strategy(int my_colour, FILE *fp)
{
	int i, loc, best_score, best_move = 0, score;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE / size * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);
	int *original_board = (int *)malloc(BOARDSIZE * sizeof(int));
	// duplicateBoard(board, original_board); //copied original state of board
	memcpy(original_board, board, BOARDSIZE * sizeof(int));
	//get moves from get proc legal moves instead of legal moves
	rank_legal_moves(my_colour, moves, fp);
	//legal_moves(my_colour, moves, fp);
	// Debug("move %d for rank %d", moves[0], rank);
	if (moves[0] <= 0)
	{
		return -1; //no moves
	}
	else
	{
		best_score = MIN; //sortMoves(moves);
		for (i = 1; i <= moves[0]; i++)
		{
			// duplicateBoard(original_board, board);
			memcpy(board, original_board, BOARDSIZE * sizeof(int));
			loc = moves[i];
			//Debug("move %d for rank %d loc %d", moves[0], rank, loc);
			make_move(loc, my_colour, fp);
			score = minimax_score(1, 1, opponent(my_colour, fp), fp, MIN, MAX);
			if (score > best_score)
			{
				best_score = score;
				best_move = moves[i];
			}
			// fprintf(fp, "score=%d at %d\n", score, loc);
		}
		// fprintf(fp, "bestie score=%d at %d\n", best_score, best_move);
		//duplicateBoard(original_board, board); //reset board to original_board before move
		memcpy(board, original_board, BOARDSIZE * sizeof(int));
		free(moves);
		free(original_board);
		best_val = best_score;
		return best_move;
	}
}
/**
 * @brief recursively called by minimax strategy, determining future moves for both max and min players 
 * 
 * @param depth starting depth until max depth
 * @param bMaxMin max(you) or min(opp) player m
 * @param my_colour players colour
 * @param fp file
 * @param alpha value for pruning
 * @param beta value for pruning
 * @return int score for that move max depth later
 */
int minimax_score(int depth, int bMaxMin, int my_colour, FILE *fp, int alpha, int beta)
{
	int i;
	int *moves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	memset(moves, 0, LEGALMOVSBUFSIZE);
	int *original_board = (int *)malloc(BOARDSIZE * sizeof(int));
	//duplicateBoard(board, original_board); //copied original state of board
	memcpy(original_board, board, BOARDSIZE * sizeof(int));
	int best;

	if (depth == MAXDEPTH)
	{
		//duplicateBoard(original_board, board); //reset board to original_board before move
		memcpy(board, original_board, BOARDSIZE * sizeof(int));
		free(moves);
		free(original_board);
		// if (bMaxMin == 0)
		// {
		return evaluatePosition(my_colour, fp); //colour for max?
												// }
												// return evaluatePosition(opponent(my_colour, fp), fp); //colour for max?
	}
	legal_moves(my_colour, moves, fp); //all possible moves
	if (moves[0] <= 0)
	{
		return -1; //no moves
	}
	//
	if (bMaxMin == 0)
	{
		best = MIN;
		sortMoves(moves);
		for (i = 1; i <= moves[0]; i++)
		{
			//duplicateBoard(original_board, board);
			memcpy(board, original_board, BOARDSIZE * sizeof(int));
			make_move(moves[i], my_colour, fp);
			int score = minimax_score(depth + 1, 1, opponent(my_colour, fp), fp, alpha, beta);
			best = max(best, score);
			alpha = max(alpha, best);

			alpha_sharing_top(alpha, 0);
			if (beta <= alpha)
			{
				break; //prune
			}
		}
		//duplicateBoard(original_board, board); //reset board to original_board before move
		memcpy(board, original_board, BOARDSIZE * sizeof(int));
		free(moves);
		free(original_board);
		//return best;
	}
	else
	{
		best = MAX;
		sortMoves(moves);
		for (i = 1; i <= moves[0]; i++)
		{
			//duplicateBoard(original_board, board);
			memcpy(board, original_board, BOARDSIZE * sizeof(int));
			make_move(moves[i], my_colour, fp);
			int score = minimax_score(depth + 1, 0, opponent(my_colour, fp), fp, alpha, beta);
			best = min(best, score);
			beta = min(beta, best);

			if (beta <= alpha)
			{
				break; //prune
			}
		}
		//duplicateBoard(original_board, board); //reset board to original_board before move
		memcpy(board, original_board, BOARDSIZE * sizeof(int));
		free(moves);
		free(original_board);
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

/**
 * @brief the evaluation function deciding what makes one move better than another
 * 
 * @param my_colour players colour
 * @param fp file
 * @return int determines best evaluation based on different heuristics and time of the game
 */
int evaluatePosition(int my_colour, FILE *fp)
{
	//int mobilityScore = evaluateMobility(my_colour, fp);
	//int discDifference = evaluateDiscDifference(my_colour, fp);
	//int stabilityScore = evaluateStability(my_colour, fp);
	//int cornerEdgeScore = evaluateCorners(my_colour, fp);
	//int gameTime = evaluateGameTime(my_colour, fp);
	//return all_in_one(my_colour, 50*gameTime, 100*gameTime, 400-(100*gameTime), 150, 100-(10*gameTime), 100-(20*gameTime));
	//return all_in_one(my_colour, 10*gameTime, 300*gameTime, 400-(100*gameTime), 150-(30*gameTime), 100-(10*gameTime), 100-(30*gameTime));//dynamic
	// int opp_colour = opponent(my_colour, fp); //cheap9
	// int my_discs = 0, opp_discs = 0;
	// if (board[11] == my_colour)
	// 	my_discs++;
	// else if (board[11] == opp_colour)
	// 	opp_discs++;
	// if (board[18] == my_colour)
	// 	my_discs++;
	// else if (board[18] == opp_colour)
	// 	opp_discs++;
	// if (board[81] == my_colour)
	// 	my_discs++;
	// else if (board[81] == opp_colour)
	// 	opp_discs++;
	// if (board[88] == my_colour)
	// 	my_discs++;
	// else if (board[88] == opp_colour)
	// 	opp_discs++;

	// int score = my_discs-(2*opp_discs);
	// if (board[11] == EMPTY)
	// {
	// 	if (board[12] == my_colour)
	// 		my_discs++;
	// 	else if (board[12] == opp_colour)
	// 		opp_discs++;
	// 	if (board[22] == my_colour)
	// 		my_discs++;
	// 	else if (board[22] == opp_colour)
	// 		opp_discs++;
	// 	if (board[21] == my_colour)
	// 		my_discs++;
	// 	else if (board[21] == opp_colour)
	// 		opp_discs++;
	// }
	// if (board[18] == EMPTY)
	// {
	// 	if (board[17] == my_colour)
	// 		my_discs++;
	// 	else if (board[17] == opp_colour)
	// 		opp_discs++;
	// 	if (board[27] == my_colour)
	// 		my_discs++;
	// 	else if (board[27] == opp_colour)
	// 		opp_discs++;
	// 	if (board[28] == my_colour)
	// 		my_discs++;
	// 	else if (board[28] == opp_colour)
	// 		opp_discs++;
	// }
	// if (board[81] == EMPTY)
	// {
	// 	if (board[82] == my_colour)
	// 		my_discs++;
	// 	else if (board[82] == opp_colour)
	// 		opp_discs++;
	// 	if (board[72] == my_colour)
	// 		my_discs++;
	// 	else if (board[72] == opp_colour)
	// 		opp_discs++;
	// 	if (board[71] == my_colour)
	// 		my_discs++;
	// 	else if (board[71] == opp_colour)
	// 		opp_discs++;
	// }
	// if (board[88] == EMPTY)
	// {
	// 	if (board[78] == my_colour)
	// 		my_discs++;
	// 	else if (board[78] == opp_colour)
	// 		opp_discs++;
	// 	if (board[77] == my_colour)
	// 		my_discs++;
	// 	else if (board[77] == opp_colour)
	// 		opp_discs++;
	// 	if (board[87] == my_colour)
	// 		my_discs++;
	// 	else if (board[87] == opp_colour)
	// 		opp_discs++;
	// }
	// score +=-my_discs+opp_discs;
	// return score;

	//return all_in_one(my_colour, 10, 800, 400, 80, 80, 10);

	switch (evaluateGameTime(my_colour, fp)) //batman
	{
	case 0: //1/3
		return evaluateMobility(my_colour, fp) + evaluateCorners(my_colour, NULL) + evaluateCorner(my_colour, NULL);
		break;
	case 1: //2/3
		return evaluateCorner(my_colour, NULL);
		break;
	case 2: //3/3
		return all_in_one(my_colour, 10, 800, 400, 80, 80, 10);
		break;
	default:
		return evaluateCorner(my_colour, NULL);
	}

	// switch (gameTime) //beats thanos not ironman balckpanther
	// {
	// case 0: //1/3
	// 	return 2 * evaluateMobility(my_colour, fp) + evaluateCorner(my_colour, NULL);
	// 	break;
	// case 1: //2/3
	// 	return evaluateCorner(my_colour, fp);
	// 	break;
	// case 2: //3/3
	// 	return all_in_one(my_colour) + evaluateDiscDifference(my_colour, NULL);
	// 	break;
	// }
	// switch (gameTime)//thanos2.0
	// {
	// case 0://1/3
	// 	return 2*evaluateMobility(my_colour, fp) + evaluateStability(my_colour, fp);
	// 	break;
	// case 1://2/3
	// 	return evaluateCorners(my_colour, fp)+ evaluateStability(my_colour, fp);;
	// 	break;
	// case 2://3/3
	// 	return all_in_one(my_colour);
	// 	break;
	// }
	// return evaluateCorner(my_colour, NULL);//spider man

	//return mobilityScore + cornerEdgeScore;
	// if (gameTime == 0)
	// {
	// 	return (stabilityScore * 0.56 * 5) + (mobilityScore * 3 * 5) + (discDifference * 0.92) + (cornerEdgeScore * 1.2);
	// }
	// else if (gameTime == 1)
	// {
	// 	return (stabilityScore * 0.56 * 2) + (mobilityScore * 3) + (discDifference * 0.92 * 2) + (cornerEdgeScore * 1.2 * 4);
	// }
	// else
	// {
	// 	return (stabilityScore * 0.56) + (mobilityScore * 3) + (discDifference * 0.92 * 5) + (cornerEdgeScore * 1.2);
	// }
	//   return evaluateDiscDifference(my_colour,fp);
	//  return (stabilityScore * 0.56) + (mobilityScore * 3) + (discDifference * 0.92 * 2) + (cornerEdgeScore * 1.2); //best
	//return  (stabilityScore * 25*(3-gameTime)) + (mobilityScore *5* (3-gameTime)) + (discDifference *25*gameTime) + (cornerEdgeScore *35*gameTime);//thanos
	//  return (stabilityScore * 0.3) + (mobilityScore * 0.3) + (discDifference * 0.05) + (cornerEdgeScore * 0.35);
}
/**
 * @brief how many moves a player has and their opposition
 * 
 * @param my_colour player colour
 * @param fp file
 * @return int score based on their mobility
 */
int evaluateMobility(int my_colour, FILE *fp)
{
	int *playerMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	int *opponentMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));

	legal_moves(my_colour, playerMoves, fp);
	legal_moves(opponent(my_colour, fp), opponentMoves, fp);

	if ((playerMoves[0] + opponentMoves[0]) <= 0)
	{
		return 0;
	}
	return 100 * (playerMoves[0] - opponentMoves[0]) / (playerMoves[0] + opponentMoves[0]);
}
/**
 * @brief more of a static board than stability, but favours stable and semi stable positions
 * 
 * @param my_colour player colour
 * @param fp file
 * @return int returns the score of stability
 */
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
/**
 * @brief evaluates the best position being corners and edges, simlar to stability but more emphasis
 * 
 * @param my_colour colour
 * @param fp file
 * @return int 
 */
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
/**
 * @brief combination of all heuristics based on proportions given
 * 
 * @param my_colour players colour
 * @param d discs ratio
 * @param c corner ratio
 * @param s stabilty ratio
 * @param m mobility ratio
 * @param e edge ratio
 * @param w weight ration
 * @return int 
 */
int all_in_one(int my_colour, int d, int c, int s, int m, int e, int w)
{
	int opp_colour = opponent(my_colour, NULL);
	int my_discs = 0, opp_discs = 0, j, k, my_edge_discs = 0, opp_edge_discs = 0, x, y;
	double discScore = 0, cornersScore = 0, stabilityCorners = 0, mobilityScore = 0, edges = 0, staticWeight = 0;

	int X1[] = {-1, -1, 0, 1, 1, 1, 0, -1};
	int Y1[] = {0, 1, 1, 1, 0, -1, -1, -1};

	// Piece difference, frontier disks and disk squares
	for (int i = 11; i <= 88; i++)
	{
		x = (i / 10) - 1;
		y = (i % 10) - 1;
		if (board[i] == my_colour)
		{
			staticWeight += stabilityWeights2[x][y];
			my_discs++;
		}
		else if (board[i] == opp_colour)
		{
			staticWeight -= stabilityWeights2[x][y]; //weightings
			opp_discs++;
		}
		if (board[i] != EMPTY)
		{
			for (k = 0; k < 8; k++)
			{
				x = i + X1[k];
				y = j + Y1[k];
				if (x >= 0 && x < 8 && y >= 0 && y < 8 && board[i] == EMPTY)
				{
					if (board[i] == my_colour)
						my_edge_discs++; //edges
					else
						opp_edge_discs++;
					break;
				}
			}
		}
	}

	if (my_discs > opp_discs)
		discScore = (100.0 * my_discs) / (my_discs + opp_discs);
	else if (my_discs < opp_discs)
		discScore = -(100.0 * opp_discs) / (my_discs + opp_discs);
	else
		discScore = 0; //discs

	if (my_edge_discs > opp_edge_discs)
		edges = -(100.0 * my_edge_discs) / (my_edge_discs + opp_edge_discs);
	else if (my_edge_discs < opp_edge_discs)
		edges = (100.0 * opp_edge_discs) / (my_edge_discs + opp_edge_discs);
	else
		edges = 0; //edges

	// Corner occupancy
	my_discs = opp_discs = 0;
	if (board[11] == my_colour)
		my_discs++;
	else if (board[11] == opp_colour)
		opp_discs++;
	if (board[18] == my_colour)
		my_discs++;
	else if (board[18] == opp_colour)
		opp_discs++;
	if (board[81] == my_colour)
		my_discs++;
	else if (board[81] == opp_colour)
		opp_discs++;
	if (board[88] == my_colour)
		my_discs++;
	else if (board[88] == opp_colour)
		opp_discs++;
	cornersScore = 25 * (my_discs - opp_discs);

	// Corner closeness
	my_discs = opp_discs = 0;
	if (board[11] == EMPTY)
	{
		if (board[12] == my_colour)
			my_discs++;
		else if (board[12] == opp_colour)
			opp_discs++;
		if (board[22] == my_colour)
			my_discs++;
		else if (board[22] == opp_colour)
			opp_discs++;
		if (board[21] == my_colour)
			my_discs++;
		else if (board[21] == opp_colour)
			opp_discs++;
	}
	if (board[18] == EMPTY)
	{
		if (board[17] == my_colour)
			my_discs++;
		else if (board[17] == opp_colour)
			opp_discs++;
		if (board[27] == my_colour)
			my_discs++;
		else if (board[27] == opp_colour)
			opp_discs++;
		if (board[28] == my_colour)
			my_discs++;
		else if (board[28] == opp_colour)
			opp_discs++;
	}
	if (board[81] == EMPTY)
	{
		if (board[82] == my_colour)
			my_discs++;
		else if (board[82] == opp_colour)
			opp_discs++;
		if (board[72] == my_colour)
			my_discs++;
		else if (board[72] == opp_colour)
			opp_discs++;
		if (board[71] == my_colour)
			my_discs++;
		else if (board[71] == opp_colour)
			opp_discs++;
	}
	if (board[88] == EMPTY)
	{
		if (board[78] == my_colour)
			my_discs++;
		else if (board[78] == opp_colour)
			opp_discs++;
		if (board[77] == my_colour)
			my_discs++;
		else if (board[77] == opp_colour)
			opp_discs++;
		if (board[87] == my_colour)
			my_discs++;
		else if (board[87] == opp_colour)
			opp_discs++;
	}
	stabilityCorners = -12.5 * (my_discs - opp_discs);

	// Mobility

	int *playerMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));
	int *opponentMoves = (int *)malloc(LEGALMOVSBUFSIZE * sizeof(int));

	legal_moves(my_colour, playerMoves, NULL);
	legal_moves(opp_colour, opponentMoves, NULL);

	my_discs = playerMoves[0];
	opp_discs = opponentMoves[0];
	if (my_discs > opp_discs)
		mobilityScore = (100.0 * my_discs) / (my_discs + opp_discs);
	else if (my_discs < opp_discs)
		mobilityScore = -(100.0 * opp_discs) / (my_discs + opp_discs);
	else
		mobilityScore = 0;

	// final weighted score
	int score = (d * discScore) + (c * cornersScore) + (s * stabilityCorners) + (m * mobilityScore) + (e * edges) + (w * staticWeight);
	return score;
}
/**
 * @brief calculates preference to static board and espicially against opp corners
 * 
 * @param my_colour colour
 * @param fp file
 * @return int score
 */
int evaluateCorner(int my_colour, FILE *fp)
{
	int score = 0;
	int x;
	int y;
	int val;
	for (int i = 11; i <= 88; i++)
	{
		if (board[i] != EMPTY)
		{
			x = i / 10;
			y = i % 10;
			val = stabilityWeights2[x - 1][y - 1];
			if (board[i] == my_colour)
				score += val;
			else
				score -= val;

			switch (i)
			{
			case 11:
				if (board[i] != my_colour)
					score -= 10;
				break;
			case 18:
				if (board[i] != my_colour)
					score -= 10;
				break;
			case 81:
				if (board[i] != my_colour)
					score -= 10;
				break;
			case 88:
				if (board[i] != my_colour)
					score -= 10;
				break;

			default:
				break;
			}
		}
	}
	return 100 * score;
}
/**
 * @brief score of discs
 * 
 * @param my_colour 
 * @param fp 
 * @return int disc scores
 */
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
/**
 * @brief moment of the game in thirds
 * 
 * @param my_colour colour
 * @param fp file
 * @return int 0 1 or 2
 */
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
	int total_discs = ((playerScore + opponentScore) / 2) - 4;
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
/**
 * @brief shares the alpha value between the different ranks, to increase pruning
 * 
 * @param alpha given current alpha value
 * @param my_rank current rank
 * @return int return the alpha vlaue or shared alpha depending if its larger
 */
int alpha_sharing_top(int alpha, int my_rank)
{
	MPI_Request request;
	int alpha_recv = 0;

	if (my_rank != 0)
	{
		for (int p = 0; p <= size; p++)
		{
			if (p != my_rank)
			{
				MPI_Isend(&alpha, 1, MPI_INT, p, 5, MPI_COMM_WORLD, &request);
				//fprintf(fp,"p is %d and rank is %d\n", p, rank); //send to all but itself
			}
		}
		MPI_Iprobe(MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &alpha_recv, MPI_STATUS_IGNORE); //check if received alpha

		if (alpha_recv != 0)
		{ //if alpha was sent
			MPI_Recv(&alpha_sharing, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			alpha_recv = 0; //change alpha sharing variable for all ranks
		}
		alpha = max(alpha_sharing, alpha); //find max for pruning
	}
	return alpha;
}

/**
 * @brief board cpy
 * 
 * @param board original
 * @param cBoard copy
 */
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
