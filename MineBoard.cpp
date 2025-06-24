#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#define FLAG 9
#define COVER 10
#define MINE_VALUE 11
#define SAFE 0

class MineBoard
{
    public:
    MineBoard(int widith, int height, int num_mines_in);
    ~MineBoard();
    int is_mine(int x, int y);
    void uncover_board();
    void flag(int x, int y);
    int sweep(int x, int y);
    int check_win();
    int check_lose();

    int getWidth() { return size_x; }
    int getHeight() { return size_y; }
    int showSquare(int x, int y);

    void reset();

    private:
    int check_bounds(int x, int y);
    int** board;
    int** covered;
    int size_x, size_y;
    int num_mines;
    
};


int MineBoard::check_bounds(int x, int y)
{
    return !(
        x < 0 
        || x >= size_x
        || y < 0 
        || y >= size_y
    );
}

MineBoard::MineBoard(int widith, int height, int num_mines_in)
{
    srand(time(NULL));   // Initialization, should only be called once.
    size_x = widith, size_y = height, num_mines = num_mines_in;

    board = (int**) calloc(size_y, sizeof(int*));
    covered = (int**) calloc(size_y, sizeof(int*));

    for (int i=0; i< size_y; i++) {
        board[i] = (int*) calloc(size_x, sizeof(int));
        covered[i] = (int*) calloc(size_x, sizeof(int));
    }
    reset();
}

MineBoard::~MineBoard() 
{
    // Cross my T's
    for (int i=0; i< size_y; i++) {
        free(board[i]);
        free(covered[i]);
    }
    free(board);
    free(covered);
}

int MineBoard::is_mine(int x, int y) {
    if (x < 0) {return 0;}
    if (y < 0) {return 0;}
    if (x >= size_x) {return 0;}
    if (y >= size_y) {return 0;}
    return board[y][x] == MINE_VALUE;
}

void MineBoard::uncover_board()
{
    int x, y;
    // Display Uncovered game board
    for (x=0;x<size_x;x++){
        for(y=0;y<size_y;y++){
            covered[y][x] = 0;
        }
    }    
}

void MineBoard::flag(int x, int y){
    if (!check_bounds(x,y)) {return;}
    if (!covered[y][x]) {return;}  // Swept an uncovered place
    else if (covered[y][x] == FLAG) {covered[y][x] = COVER;}  // Unflag
    else {covered[y][x] = FLAG;}
}

int MineBoard::sweep(int x, int y){
    if (!check_bounds(x,y)) {return -1;}

    if (!covered[y][x]) {return SAFE;}  // Swept an uncovered place
    if (covered[y][x] == FLAG) {return FLAG;}  // Do not uncover it
    covered[y][x] = 0;

    if (board[y][x] == SAFE) {
        int node,nx,ny,dx,dy,sx,sy;
        int stack_height = 0;
        int* stack = (int*) calloc(100, sizeof(int));
        int* marked = (int*) calloc(100, sizeof(int));

        
        for (sx=0;sx<size_x;sx++){
            for(sy=0;sy<size_y;sy++){
                if (covered[sy][sx] == 0) {marked[sx+size_x*sy] = 1;}
            }
        }

        stack[stack_height++] = x + (y*size_x); marked[x + (y*size_x)] = 1;

        while (stack_height > 0) {
            // take node off stack
            node = stack[--stack_height];
            nx = node % size_x; ny = node / size_x;
            // uncover! (do action)
            covered[ny][nx] = 0;
            // get adjacent
            for (dx=-1;dx <=1;dx++){
                for (dy=-1;dy <=1;dy++){
                    sy = ny+dy; sx = nx+dx;
                    if (sy >= 0 && sx >= 0 && sy < size_y && sx < size_x){
                        // check blank
                        if (board[sy][sx] == SAFE) {
                            // check unsearched
                            if (marked[(size_x*sy) + sx] == 0) {
                                stack[stack_height++] = sx + (sy*size_x); 
                                marked[sx + (sy*size_x)] = 1;
                            }
                        }
                        else if (board[sy][sx] != MINE_VALUE) {
                            marked[sx + (sy*size_x)] = 1;
                            covered[sy][sx] = 0;
                        }
                    }
                }
            }
        }
        free(stack);
        free(marked);

    }
    return board[y][x];
}

int MineBoard::check_win() {
    int x, y, count;
    for (x=0;x<size_x;x++){
        for(y=0;y<size_y;y++){
            if (covered[y][x]) {
                count++;
                if (count > num_mines) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int MineBoard::check_lose() {
    int x, y;
    for (x=0;x<size_x;x++){
        for(y=0;y<size_y;y++){
            if (!covered[y][x]) {
                if (board[y][x] == MINE_VALUE) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int MineBoard::showSquare(int x, int y) 
{
    if (!check_bounds(x,y)) {return COVER;}
    // 0 for empty
    // 1-8 to show the number of adjacent mines
    // 9 flag
    // 10 cover
    // 11 for mine
    if (covered[y][x] == FLAG) { return FLAG; }
    if (covered[y][x] == COVER) { return COVER; }
    return board[y][x];
}

void MineBoard::reset() {
    int i, x, y, dy, dx;
    // Initialize the board & Cover Map

    for (int y=0; y<size_y; y++) {
        for (int x=0; x<size_x;x++) {
            covered[y][x]=COVER;
        }
    }

    // Put the Mines down
    for (i=0; i<num_mines; i++) {
        x = rand() % size_x;
        y = rand() % size_y;
        while (board[y][x] == MINE_VALUE) {
            x = rand() % size_x;
            y = rand() % size_y;
        }
        board[y][x] = MINE_VALUE;
    }

    // Assign the Mine-Adjacency Numbers
    int mine_count;
    for (x=0;x<size_x;x++){
        for (y=0;y<size_y;y++){
            if (is_mine(x, y)) {}
            else {
                mine_count = 0;
                for (dy=-1; dy<=1;dy++) {
                    for (dx=-1; dx<=1;dx++) {
                        if (is_mine(x+dx, y+dy)) {
                            mine_count++;
                        }
                    }
                }
                board[y][x] = mine_count;
            }
        }
    }
}