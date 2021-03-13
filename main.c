
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

int n = 5;
struct entity **entities;

void *update_dwarfs() {
    while(true)
	{	
        clear();
        for (int i = 0; i < n; ++i) {
            struct entity *en = entities[i];
            switch(rand()%4)
            {	case 0:
                    mvaddch(en->y, --en->x, en->c);
                    break;
                case 1:
                    mvaddch(en->y, ++en->x, en->c);
                    break;
                case 3:
                    mvaddch(--en->y, en->x, en->c);
                    break;
                case 2:
                    mvaddch(++en->y, en->x, en->c);
                    break;	
            }
        }
        refresh();
        usleep(500000);
	}
}


int main(int argc, char *argv[])
{	WINDOW *my_win;
	int startx, starty, width, height;
	int ch;

	initscr();			/* Start curses mode 		*/
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
    
    for (int i = 0; i < n; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = startx;
        entities[i]->y = starty;
        entities[i]->c = 'd';
        entities[i]->mover = false;
    }

    pthread_t dwarf;

    pthread_create(&dwarf, NULL, update_dwarfs, NULL);

    while (getch() != KEY_F(1));
		
	endwin();			/* End curses mode		  */
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}