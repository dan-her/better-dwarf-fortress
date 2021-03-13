
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

struct entity {
    int x;
    int y;
    char c;
    bool mover;
};

int n = 10;
int nboys = 5;
struct entity **entities;
int maxx = 100;
int maxy = 100;

bool is_valid_spot(int x, int y) {
    return x < maxx && x > -1 && y < maxy && y > -1;
}

bool is_taken(int x1, int y1, int j){
    for (int i = 0; i < n; ++i) {
        if (j == i) continue;
        if (entities[i]->x == x1 && entities[i]->y == y1) {
            return true;
        }
    }
    return false;
}

void *update_dwarfs() {
    while(true) {	
        clear();
        for (int i = 0; i < n; ++i) {
            struct entity *en = entities[i];
            if (en->mover) {
                int dirx=0, diry=0;
                switch(rand()%4)
                {	case 0:
                        dirx = -1;
                        break;
                    case 1:
                        dirx = 1;
                        break;
                    case 3:
                        diry = 1;
                        break;
                    case 2:
                        diry = -1;
                        break;
                }
                if (is_valid_spot(en->x+dirx, en->y+diry) && !is_taken(en->x+dirx, en->y+diry, i)) {
                    en->x += dirx;
                    en->y += diry;
                }
            }
            mvaddch(en->y, en->x, en->c);
        }
        refresh();
        usleep(250000);
	}
}

void initialize_the_boys() {
    for (int i = 0; i < nboys; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%maxx;
        entities[i]->y = rand()%maxy;
        entities[i]->c = 'd';
        entities[i]->mover = true;
    }
    for (int i = nboys; i < n; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%maxx;
        entities[i]->y = rand()%maxy;
        entities[i]->c = 'a';
        entities[i]->mover = false;
    }
}


int main(int argc, char *argv[])
{	WINDOW *my_win;
	int startx, starty, width, height;
    srand(time(0));

	my_win = initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

	height = 3;
	width = 10;
	starty = (LINES - height) / 2;	/* Calculating for a center placement */
	startx = (COLS - width) / 2;	/* of the window		*/
	//printw("Press F1 to exit");
    curs_set(0);
	refresh();

    entities = malloc(n*sizeof(struct entity *));
    getmaxyx(my_win, maxy, maxx);
    initialize_the_boys(startx, starty);

    pthread_t dwarf;

    pthread_create(&dwarf, NULL, update_dwarfs, NULL);

    while (getch() != KEY_F(1));
		
	endwin();			/* End curses mode		  */
	return 0;
}
