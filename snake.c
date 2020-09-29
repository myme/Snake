#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <curses.h>

#define GRID_WIDTH  50
#define GRID_HEIGHT 20


struct point {
    int x;
    int y;
};


enum direction { LEFT, RIGHT, UP, DOWN };


enum grid_val { EMPTY, SNAKE, APPLE, WALL };


// A grid represents the field (map of "used" cells)
struct grid {
    int width;
    int height;
    int *g;
};


// A linked list of points making up the snake
struct snake_point {
    struct point p;
    struct snake_point *next;
};


// A snake is only updated in head and tail,
// and is present in a grid.
struct snake {
    struct snake_point *head;
    struct snake_point *tail;
    struct grid        *grid;
    enum direction     dir;

    // Some stats
    int length;
    int apples_eaten;
};


void grid_set( struct grid *grid, int x, int y, int val )
{
    *( grid->g + (y * grid->width + x) ) = val;
}


int grid_get( const struct grid *grid, int x, int y )
{
    return *( grid->g + (y * grid->width + x) );
}


void grid_clear( struct grid *grid )
{
    int x, y;

    memset( grid->g, EMPTY, grid->width * grid->height );

    // Add a wall around the entire border
    for ( y = 0; y < grid->height; y++ ) {
        for ( x = 0; x < grid->width; x++ ) {
            if (
                x == 0 || x == grid->width  - 1 ||
                y == 0 || y == grid->height - 1
            ) {
                grid_set( grid, x, y, WALL );
            }
        }
    }

    // Add an interesting wall in the middle of the grid
    for ( y = grid->height / 4; y < grid->height - grid->height / 4; y++ )
        grid_set( grid, 24, y, WALL );
}


void grid_clear_cell( struct grid *grid, int x, int y )
{
    grid_set( grid, x, y, EMPTY );
}


void init_grid( struct grid *grid, int width, int height )
{
    grid->g = malloc( sizeof width * width * height );
    assert(grid->g);

    grid->width  = width;
    grid->height = height;

    grid_clear( grid );
}


// Randomly places the apple in an empty cell in the grid
// and returns the coordinates
void place_apple( struct grid *grid )
{
    int x, y;

    do {
        x = rand() % grid->width;
        y = rand() % grid->height;
    } while ( grid_get( grid, x, y ) != EMPTY );

    grid_set( grid, x, y, APPLE );
}


// Adds (pushes) a point to the head of the snake
void push_point( struct snake *snake, struct point p )
{
    struct snake_point *new;

    new       = (struct snake_point *) malloc( sizeof *new );
    new->p    = p;
    new->next = NULL;

    grid_set( snake->grid, p.x, p.y, SNAKE );

    if ( snake->head != NULL )
        snake->head->next = new;

    if ( snake->tail == NULL )
        snake->tail = new;

    snake->head = new;
}

// Removes (shifts) off a point from the tail of the snake
// and places the value in @p (if defined). Returns non-zero
// if a point was actually removed.
int shift_point( struct snake *snake, struct point *p )
{
    struct snake_point *next;

    if ( snake->tail != NULL ) {
        if ( p != NULL )
            *p = snake->tail->p;

        grid_clear_cell( snake->grid, snake->tail->p.x, snake->tail->p.y );

        next = snake->tail->next;
        free( snake->tail );
        snake->tail = next;

        return 1;
    }

    return 0;
}


void init_snake( struct snake *snake, struct point head, struct point tail, struct grid *grid )
{
    int i;

    snake->head = NULL;
    snake->tail = NULL;
    snake->grid = grid;
    snake->dir  = RIGHT;

    snake->length       = 4;
    snake->apples_eaten = 0;

    // Add the snake to the grid
    for ( i = tail.x; i <= head.x; i++ ) {
        push_point( snake, (struct point) { i, head.y } );
    }
}


void move_snake( struct snake *snake, enum direction dir )
{
    int got_apple = 0;
    enum grid_val val;
    struct point p;

    p = snake->head->p;

    // A change of motion can only be orthogonal to current
    // direction of motion.
    switch (dir) {
        case LEFT:
            if ( snake->dir == RIGHT ) return;
            p.x -= 1;
            break;

        case RIGHT:
            if ( snake->dir == LEFT ) return;
            p.x += 1;
            break;

        case UP:
            if ( snake->dir == DOWN ) return;
            p.y -= 1;
            break;

        case DOWN:
            if ( snake->dir == UP ) return;
            p.y += 1;
            break;
    }

    if (
        p.x < 0 || p.x >= snake->grid->width  ||
        p.y < 0 || p.y >= snake->grid->height
    ) {
        return;
    }

    snake->dir = dir;

    switch ( grid_get( snake->grid, p.x, p.y ) ) {
        case APPLE:
            got_apple = 1;
            break;

        case EMPTY:
            break;

        // "Die" when hitting yourself or a wall
        default:
            return;
    }

    // Add new point at snake head
    push_point( snake, p );

    // Remove last point in the snake, unless we got the apple
    // in which case we'd like to place a new one.
    if ( !got_apple ) {
        shift_point( snake, NULL );
    } else {
        place_apple( snake->grid );
        snake->length++;
        snake->apples_eaten++;
    }
}


void dump_grid( const struct grid *grid )
{
    int w = grid->width;
    int h = grid->height;
    int x, y;

    for ( y = 0; y < h; y++ ) {
        for ( x = 0; x < w; x++ ) {
            char c;
            int val = grid_get( grid, x, y );
            switch ( grid_get( grid, x, y ) ) {
                case EMPTY: c = ' '; break;
                case SNAKE: c = 'o'; break;
                case APPLE: c = '@'; break;
                case WALL:  c = '#'; break;
            }
            //printf( "%c", c );
            addch( c );
        }
        //printf( "\n" );
        addch( '\n' );
    }
}


void show_stats( const struct snake *snake )
{
    char str[100];
    static int start = 0;

    if ( !start ) start = time( NULL );

    sprintf( str, "\nLength: %d\nApples eaten: %d\nTime: %d\n\n",
        snake->length, snake->apples_eaten, (int) time( NULL ) - start );

    addstr( str );
}


int main(void)
{
    int done = 0;

    int g_width  = GRID_WIDTH;
    int g_height = GRID_HEIGHT;

    struct point head_start = { 6, 5 };
    struct point tail_start = { 3, 5 };

    struct snake snake, *sp = &snake;
    struct grid  grid,  *gp = &grid;

    srand( time( NULL ) );

    // Initialize grid and snake, and place initial
    // apple on the grid
    init_grid( gp,  g_width, g_height );
    init_snake( sp, head_start, tail_start, gp );
    place_apple( gp );

    // Setup curses
    initscr();
    cbreak();
    noecho();

    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    // Mainloop
    while ( !done ) {
        int c;

        // Tenths of a second
        halfdelay( 2 );

        if ( ( c = getch() ) != ERR ) {
            switch ( c ) {
                case 'q':
                    done = 1;
                    break;
                case KEY_LEFT:
                    move_snake( sp, LEFT );
                    break;
                case KEY_RIGHT:
                    move_snake( sp, RIGHT );
                    break;
                case KEY_UP:
                    move_snake( sp, UP );
                    break;
                case KEY_DOWN:
                    move_snake( sp, DOWN );
                    break;
            }
        } else {
            move_snake( sp, sp->dir );
        }

        clear();

        dump_grid( gp );
        show_stats( sp );
    }

    endwin();
}

