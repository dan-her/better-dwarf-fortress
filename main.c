
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
int kgdug = 0; // how many times a dwarf has broken a rock

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

void *update_dwarfs(void *in) {
    while(true) {	
		WINDOW *win = (WINDOW *)in;
        wclear(win);
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
            mvwaddch(win, en->y, en->x, en->c);
        }
	  	box(win, 0, 0);	
        wrefresh(win);
        usleep(250000);
	}
}

void initialize_the_boys() {
    for (int i = 0; i < nboys; ++i) { // dwarves
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%maxx;
        entities[i]->y = rand()%maxy;
        entities[i]->c = 'd';
        entities[i]->mover = true;
    }
    for (int i = nboys; i < n; ++i) { // apples
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%maxx;
        entities[i]->y = rand()%maxy;
        entities[i]->c = 'a';
        entities[i]->mover = false;
    }
}


int main(int argc, char *argv[])
{	
	
	initscr();// begin ncurses
	noecho(); // don't echo inputted chars from any getch() calls
	keypad(stdscr, TRUE); // read keypad input
	int ch; // character for getting
	int dwarfpop = 0; // these check if we've popped up any number of flags
	int applepop = 0;
	int kgpop = 0; // technically unused
	int boyscount = 0;

	printw("Dwarf Game");
	printw("                                                              \n\n\n\n"); // leave a bunch of space for messages
	printw(" a - make an apple\n\n");
	printw(" b - make a new dwarf\n\n");
	printw(" d - current dwarf count\n\n");
	printw(" k - kg of rock moved\n\n");
	refresh();			/* Print it on to the real screen */
	int startx, starty, width, height;

	noecho(); // doesn't print the buttons you press

	height = 3;
	width = 10;
	//printw("Press F1 to exit");
    curs_set(0);
	refresh();
	WINDOW *win = newwin(32, 95, 2, 25); // game window
  	box(win, 0, 0);	
	wrefresh(win);

    entities = malloc(n*sizeof(struct entity *));
    getmaxyx(win, maxy, maxx);
    initialize_the_boys();

    pthread_t dwarf;

    pthread_create(&dwarf, NULL, update_dwarfs, win);

    while ((ch = getch()) != KEY_F(1)) {
		switch(ch) {
			case 'a': // purchase apples, which allow dwarves to duplicate
				move(1, 25);
				printw("A new apple has been created.");
				n++;
				entities = realloc(entities, n*sizeof(struct entity *));
				entities[n-1] =  (struct entity *)malloc(sizeof(struct entity));
 		        entities[n-1]->x = rand()%maxx;
		        entities[n-1]->y = rand()%maxy;
        		entities[n-1]->c = 'a';
        		entities[n-1]->mover = false;
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                             ");
				refresh();
				break;
			case 'b': // purchase new dwarves
				move(1, 25);
				printw("A new dwarf has been created.");
				n++;
				entities = realloc(entities, n*sizeof(struct entity *));
				entities[n-1] =  (struct entity *)malloc(sizeof(struct entity));
 		        entities[n-1]->x = rand()%maxx;
		        entities[n-1]->y = rand()%maxy;
        		entities[n-1]->c = 'd';
        		entities[n-1]->mover = true;
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                             ");
				refresh();
				break;
			case 'd': // count your dwarves
				for (int i = 0; i < n; i++) {
					if (entities[i]->c == 'd') {
						boyscount++;
					}
				}
				move(1, 25);
				printw("There are currently %d dwarves.", boyscount);
				boyscount = 0; // reset boyscount so we don't count more dwarves than there are
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                                ");
				refresh();
				break;
			case 'k': // see how many kg of stone your dwarves have moved in their mining adventures
				move(1, 25);
				printw("The dwarves have broken a total of %dkg of rock.", (kgdug*5));
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                                                  ");
				refresh();
				break;
		}
	};
		
	endwin();
	return 0;
}
