This was my third year concurrency project. I achieved an 88% for this individual project.

# Project Spec:
For this project you have to implement an Othello player, using C and MPI to distribute the tree search. A minimax search with alpha-beta pruning should be used to evaluate the available moves from it's current position before making a move.

You will be provided with a serial player (random.c) that can make random moves, which you can use as a starting point, as well as a standalone tournament engine, which you can use to test how your player plays against itself, the random player, or any other players.

You should also design your heuristic that takes into account various different strategies.

## How to execute:

1. in a terminal window (when using default ssh)
make 
. runInOneWindow.sh

OR 
1. in a terminal window (on your local machine)
make
. runall.sh

OR 
1. in a terminal window, 
make 
. runserver.sh 

2. in a second terminal window,
. runlobby.sh

3. also in the second terminal window (or in a separate window),
. runplayer1.sh 

4. and in a third terminal window
. runplayer2.sh

