#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10,SEED_NUMBER=3};


struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
} control_buttons;

struct control_buttons default_controls = {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT};

/*
 √олова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движени€
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */

//структура головы
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons controls;
} snake_t;


typedef struct tail_t
{
    int x;
    int y;
} tail_t;



void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t= {0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}


void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT; //вправо ползет
}

//инициализаци€ змейки
void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail;
    head->tsize = size + 1;
    head->controls = default_controls;
}


void go(struct snake_t *head)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(head->y, head->x, " ");
    switch (head->direction)
    {
    case LEFT:
        head->x = (head->x == 0) ? max_x - 1 : head->x - 1;

        mvprintw(head->y, head->x, "%c", ch);
        break;
    case RIGHT:
        head->x = (head->x == max_x - 1) ? 0 : head->x + 1;

        mvprintw(head->y, head->x, "%c", ch);
        break;
    case UP:
        head->y = (head->y == 0) ? max_y - 1 : head->y - 1;

        mvprintw(head->y, head->x, "%c", ch);
        break;
    case DOWN:
        head->y = (head->y == max_y - 1) ? 0 : head->y + 1;

        mvprintw(head->y, head->x, "%c", ch);
        break;
    default:
        break;
    }
    refresh();
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    if (key == snake->controls.down)
        snake->direction = DOWN;
    else if (key == snake->controls.up)
        snake->direction = UP;
    else if (key == snake->controls.right)
        snake->direction = RIGHT;
    else if (key == snake->controls.left)
        snake->direction = LEFT;
}


void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}
//Colision_Functioun
int check_collision(snake_t* snake)
{
    for (size_t i = 1; i < snake->tsize; i++)
    {
        if (snake->x == snake->tail[i].x && snake->y == snake->tail[i].y)
        {
            return 1;
        }
    }
    return 0;
}


struct food
{
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} food[MAX_FOOD_SIZE];

void initFood(struct food f[], size_t size)
{
    struct food init = {0,0,0,0,0};
    for(size_t i=0; i<size; i++)
    {
        f[i] = init;
    }
}


void putFoodSeed(struct food *fp)
{
    int max_x=0, max_y=0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 1;
    fp->put_time = time(NULL);
    fp->point = '$';
    fp->enable = 1;
    spoint[0] = fp->point;
    mvprintw(fp->y, fp->x, "%s", spoint);
}


void putFood(struct food f[], size_t number_seeds)
{
    for( size_t i = 0; i<number_seeds; i++)
    {
        putFoodSeed(&f[i]);
    }
}

void refreshFood(struct food f[], int nfood)
{
    for(size_t i=0; i<nfood; i++)
    {
        if(f[i].put_time)
        {
            if(!f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS)
            {
                putFoodSeed(&f[i]);
            }
        }
    }
}



int main()
{
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake, START_TAIL_SIZE, 10, 10);
    initscr();
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(FALSE);
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    timeout(0);
    initFood(food, MAX_FOOD_SIZE);
    putFood(food, SEED_NUMBER);
    int key_pressed=0 ;
    int max_x = 0, max_y = 0;
    while( key_pressed != STOP_GAME )
    {
        key_pressed = getch();
        go(snake);
        goTail(snake);
        if (check_collision(snake))
        {
            getmaxyx(stdscr, max_y, max_x);
            mvprintw(max_y / 2, max_x / 2, "Game Over!");
            refresh();
            sleep(3);

            break;
        }
        timeout(100);
        refreshFood(food, SEED_NUMBER);
        changeDirection(snake, key_pressed);
    }
    free(snake->tail);
    free(snake);
    endwin();
    return 0;
}
