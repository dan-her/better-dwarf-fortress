
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void spawn_demon();
void spawn_rocks();
void delete_boy(int index);
void find_target();

struct entity {
    int x;
    int y;
    char c;
    bool mover;
    int hang_time; // assuming the number of demons > 1
};

int n = 260;
int nboys = 5;
struct entity **entities;
int maxx = 100;
int maxy = 100;
int kigdug = 0; // how many times a dwarf has broken a rock
bool demoned = false;
int target; // assuming the number of demons = 1

bool is_valid_spot(int x, int y) {
    return x < maxx-1 && x > 0 && y < maxy-1 && y > 0;
}

bool is_taken(int x1, int y1, int j){
    for (int i = 0; i < n; ++i) {
        if (j == i) continue;
		if (entities[i]-> c == '#' && entities[j]->c == 'H') continue;
        if (entities[i]->x == x1 && entities[i]->y == y1) {
			if ((entities[j]->c == 'd' && entities[i]->c == '#') || (entities[j]->c == 'H' && entities[i]->c == 'd')) {
				delete_boy(i);
				if (entities[j]->c == 'd') {
					kigdug++;
				} else {
					target = -1;
				}	
				return false;
			} else { // else is not necessary here but it's nicer 
	            return true; 
			}
        }
    }
    return false;
}

void delete_boy(int index) {
	n--;
	free(entities[index]);
	for (int i = index; i < n; ++i) {
		entities[i] = entities[i+1];
	}
	entities = realloc(entities, sizeof(struct entity *)*n);
}

void *update_dwarfs(void *in) {
    while(true) {	
		WINDOW *win = (WINDOW *)in;
        wclear(win);
        for (int i = 0; i < n; ++i) {
            struct entity *en = entities[i];
            if (en->mover) {
                int dirx=0, diry=0;
                if (en->c == 'H') {
					if (target == -1) find_target();
					dirx = en->x >= entities[target]->x ? (en->x == entities[target]->x ? 0 : -1) : 1;
					diry = en->y >= entities[target]->y ? (en->y == entities[target]->y ? 0 : -1) : 1;
				} else {
					switch(rand()%4){
						case 0:
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
				}
                if (is_valid_spot(en->x+dirx, en->y+diry) && !is_taken(en->x+dirx, en->y+diry, i)) {
                    en->x += dirx;
                    en->y += diry;
                }
            }
            mvwaddch(win, en->y, en->x, en->c);
			if (en->hang_time == 1) {
				delete_boy(i);
				i--;
				demoned = false;
			} else {
				en->hang_time = en->hang_time != 0 ? en->hang_time-1 : 0;
			}
        }
        if (rand()%50 == 0 && !demoned) {
            n++;
            entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
            entities[n-1] = (struct entity *)malloc(sizeof(struct entity));
            spawn_demon();
			spawn_rocks();
        }
	  	box(win, 0, 0);	
        wrefresh(win);
        usleep(250000);
	}
}

void find_target() {
	for (int i = 0; i < n; ++i) {
		if (entities[i]->c == 'd') {
			target = i;
		}
	}
}

void spawn_demon() {
	find_target();
    int x=0, y=0;
    switch (rand()%4) {
        case 0:
            x = maxx/2;
            y = maxy/2;
            break;
        case 1:
            x = maxx/2;
            y = maxy/2;
            break;
        case 2:
            y = maxy/2;
            x = maxx/2;
            break;
        case 3:
            y = maxy/2;
            x = maxx/2;
            break;
    }
    entities[n-1]->x = x;
    entities[n-1]->y = y;
    entities[n-1]->c = 'H';
    entities[n-1]->mover = true;
    entities[n-1]->hang_time = rand()%50+50;
	demoned = true;
}

void spawn_rocks() {
	for (int i = 0; i < 10; ++i){
		n++;
    	entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
    	entities[n-1] = (struct entity *)malloc(sizeof(struct entity));	
		entities[n-1]->x = rand()%(maxx-1)+1;
    	entities[n-1]->y = rand()%(maxy-1)+1;
    	entities[n-1]->c = '#';
    	entities[n-1]->mover = false;
		entities[n-1]->hang_time = 0;
	}
}

void initialize_the_boys() {
    for (int i = 0; i < nboys; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-1)+1;
        entities[i]->y = rand()%(maxy-1)+1;
        entities[i]->c = 'd';
        entities[i]->mover = true;
        entities[i]->hang_time = 0;
    }
    for (int i = nboys; i < 10; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-1)+1;
        entities[i]->y = rand()%(maxy-1)+1;
        entities[i]->c = 'a';
        entities[i]->mover = false;
        entities[i]->hang_time = 0;
    }
	for (int i = 10; i < n; ++i){ // rocks
		entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-1)+1;
        entities[i]->y = rand()%(maxy-1)+1;
        entities[i]->c = '#';
        entities[i]->mover = false;
        entities[i]->hang_time = 0;
	}	
}


int main(int argc, char *argv[])
{	
	srand(time(0));
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
				printw("The dwarves have broken a total of %dkg of rock.", (kigdug*5));
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
