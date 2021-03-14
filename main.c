
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void spawn_demon();
void spawn_rocks();
void spawn_dragon();
void spawn_dwarf(int);
void delete_boy(int index);
void find_target();
void move_towards_target(int index, int *dirx, int *diry);

struct entity {
    int x;
    int y;
    char c;
    bool mover;
    int hang_time; // assuming the number of demons > 1
	struct entity *target;
};

int n = 260;
int nboys = 5;
struct entity **entities;
int maxx = 100;
int maxy = 100;
int kigdug = 0; // how many times a dwarf has broken a rock
bool demoned = false;
bool dwarfspawn = false; // flags for checking if we've asked to spawn one of the things the user can spawn
bool applespawn = false;
bool eggspawn = false;
int egg_time = 0;
//int target; // assuming the number of demons = 1

bool is_valid_spot(int x, int y) {
    return x < maxx-1 && x > 0 && y < maxy-1 && y > 0;
}

bool is_taken(int x1, int y1, int j){
    for (int i = 0; i < n; ++i) {
        if (j == i) continue;
		if (entities[i]-> c == '#' && entities[j]->c == 'H') continue;
		if (entities[i]-> c == 'a' && entities[j]->c == 'H') continue;
        if (entities[i]->x == x1 && entities[i]->y == y1) {
			if ((entities[j]->c == 'd' && entities[i]->c == '#') || (entities[j]->c == 'H' && entities[i]->c == 'd') || (entities[j]->c == 'd' && entities[i]->c == 'a') || (entities[j]->c == 'D' && entities[i]->c == 'H')) {
				entities[j]->target = NULL;
				if (entities[j]->c == 'd' && entities[i]->c == 'a') {
					if (entities[i]->c == 'a') {
						spawn_dwarf(i);
					}
				} 
				if ((entities[j]->c == 'd' && entities[i]->c == '#')){
					kigdug++;
				}
				delete_boy(i);
				return false;
			} else { // else is not necessary here but it's nicer 
	            return true; 
			}
        }
    }
    return false;
}

void delete_boy(int index) {
	for (int i = 0; i < n; i++) {
		if (entities[i]->target == entities[index]) {
			entities[i]->target = NULL;
		}
	}
	n--;
	free(entities[index]);
	for (int i = index; i < n; ++i) {
		entities[i] = entities[i+1];
	}
	entities = realloc(entities, sizeof(struct entity *)*n);
}

bool nearby_goodies(int x, int y, int j) {
	int mindex = -1;
	int min = 999999;
	for (int i = 0; i < n; ++i) {
		if (entities[i]->c != '#' && entities[i]->c != 'a') continue;
		int x1 = entities[i]->x - x;
		int y1 = entities[i]->y - y;
		int dist = x1*x1+y1*y1;
		if (dist < 10) {
			if (dist <= min) {
				mindex = i;
				min = dist;
			}
		}
	}
	if (mindex == -1) {
		return false;
	} else {
		entities[j]->target = entities[mindex];
		return true;
	}
}

void *update_dwarfs(void *in) {
    while(true) {
		WINDOW *win = (WINDOW *)in;
        wclear(win);
		if (dwarfspawn) {
			n++;
			entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
    		entities[n-1] = (struct entity *)malloc(sizeof(struct entity));	
			entities[n-1]->x = rand()%(maxx-1)+1;
    		entities[n-1]->y = rand()%(maxy-1)+1;
    		entities[n-1]->c = 'd';
    		entities[n-1]->mover = true;
			entities[n-1]->hang_time = 0;
			entities[n-1]->target = entities[0];
			dwarfspawn = false;
		}
		if (eggspawn) {
			n++;
			entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
    		entities[n-1] = (struct entity *)malloc(sizeof(struct entity));	
			entities[n-1]->x = rand()%(maxx-1)+1;
    		entities[n-1]->y = rand()%(maxy-1)+1;
    		entities[n-1]->c = 'e';
    		entities[n-1]->mover = false;
			entities[n-1]->hang_time = 0;
			eggspawn = false;
		}
		if (applespawn){
			n++;
			entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
    		entities[n-1] = (struct entity *)malloc(sizeof(struct entity));	
			entities[n-1]->x = rand()%(maxx-1)+1;
    		entities[n-1]->y = rand()%(maxy-1)+1;
    		entities[n-1]->c = 'a';
    		entities[n-1]->mover = false;
			entities[n-1]->hang_time = 0;
			applespawn = false;
		}
        for (int i = 0; i < n; ++i) {
            struct entity *en = entities[i];
            if (en->mover) {
                int dirx=0, diry=0;
				if (en->target != NULL) {
					move_towards_target(i, &dirx, &diry);
				} else {
					if (en->c == 'H') {
						find_target(i);
						move_towards_target(i, &dirx, &diry);
					} else if (en->c == 'D') { 
						find_target(i);
						move_towards_target(i, &dirx, &diry);
					} else {
						bool t = nearby_goodies(entities[i]->x, entities[i]->y, i);
						if (t) {
							move_towards_target(i, &dirx, &diry);
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
					}
				}
                if (is_valid_spot(en->x+dirx, en->y+diry) && !is_taken(en->x+dirx, en->y+diry, i)) {
                    en->x += dirx;
                    en->y += diry;
                }
            }
            switch (en->c) {
				case 'H':
					wattron(win, COLOR_PAIR(1));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(1));
					break;
				case 'd':
					wattron(win, COLOR_PAIR(2));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(2));
					break;
				case '#':
					wattron(win, COLOR_PAIR(6));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(6));
					break;
				case 'a':
					wattron(win, COLOR_PAIR(4));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(4));
					break;
				case 'D':
					wattron(win, COLOR_PAIR(3));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(3));
					break;
				case 'e':
					wattron(win, COLOR_PAIR(5));
					mvwaddch(win, en->y, en->x, en->c);
					wattroff(win, COLOR_PAIR(5));
					break;
				default:
					mvwaddch(win, en->y, en->x, en->c);
					break;
			}
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
		if (egg_time == 3) {
			spawn_dragon();
		}
	  	//box(win, 0, 0);	
        wrefresh(win);
        usleep(250000);
	}
}

void move_towards_target(int index, int *dirx, int *diry) {
	struct entity *en = entities[index];
	if (en->target == NULL) return;
	*dirx = en->x >= en->target->x ? (en->x == en->target->x ? 0 : -1) : 1;
	*diry = en->y >= en->target->y ? (en->y == en->target->y ? 0 : -1) : 1;
} 

void find_target(int j) {
	for (int i = 0; i < n; ++i) {
		if (entities[i]->c == 'd' && entities[j]->c == 'H') {
			entities[j]->target = entities[i];
		} else if (entities[i]->c == 'H' && entities[j]->c == 'D'){
			entities[j]->target = entities[i];
		}
	}
}

void spawn_demon() {
	find_target(n-1);
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
		entities[n-1]->x = rand()%(maxx-2)+1;
    	entities[n-1]->y = rand()%(maxy-2)+1;
    	entities[n-1]->c = '#';
    	entities[n-1]->mover = false;
		entities[n-1]->hang_time = 0;
	}
}

void spawn_dragon() {
	n++;
	entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
	entities[n-1] =  (struct entity *)malloc(sizeof(struct entity));
    entities[n-1]->x = rand()%maxx;
    entities[n-1]->y = rand()%maxy;
	entities[n-1]->c = 'D';
	entities[n-1]->mover = true;
	entities[n-1]->hang_time = 0;
	for (int i = 0; i < n; ++i){
		if (entities[i]->c == 'e') {
			delete_boy(i);
			i--;
			egg_time--;
		}
	}
}

void spawn_dwarf(int i) {
	n++;
	entities = (struct entity **)realloc(entities, sizeof(struct entity *)*n);
	entities[n-1] = (struct entity *)malloc(sizeof(struct entity));	
	entities[n-1]->x = entities[i]->x;
	entities[n-1]->y = entities[i]->y;
	entities[n-1]->c = 'd';
	entities[n-1]->mover = true;
	entities[n-1]->hang_time = 0;
	nearby_goodies(entities[n-1]->x, entities[n-1]->y, n-1);
}

void initialize_the_boys() {
    for (int i = 0; i < nboys; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-2)+1;
        entities[i]->y = rand()%(maxy-2)+1;
        entities[i]->c = 'd';
        entities[i]->mover = true;
        entities[i]->hang_time = 0;
		entities[i]->target = NULL;
    }
    for (int i = nboys; i < 10; ++i) {
        entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-2)+1;
        entities[i]->y = rand()%(maxy-2)+1;
        entities[i]->c = 'a';
        entities[i]->mover = false;
        entities[i]->hang_time = 0;
    }
	for (int i = 10; i < n; ++i){ // rocks
		entities[i] = (struct entity *)malloc(sizeof(struct entity));
        entities[i]->x = rand()%(maxx-2)+1;
        entities[i]->y = rand()%(maxy-2)+1;
        entities[i]->c = '#';
        entities[i]->mover = false;
        entities[i]->hang_time = 0;
	}	
}


int main(int argc, char *argv[]) {	
	srand(time(0));
	initscr();// begin ncurses
	noecho(); // don't echo inputted chars from any getch() calls
	keypad(stdscr, TRUE); // read keypad input
	int ch; // character for getting
	int dwarfpop = 0; // these check if we've popped up any number of flags
	int applepop = 0;
	int kgpop = 0; // technically unused
	int boyscount = 0;
	start_color();
	init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
	init_pair(6, COLOR_YELLOW, COLOR_BLACK);

	printw("Dwarf Aquarium");
	printw("\n\n\n\n"); 
	printw(" a - buy apple: $10\n\n");
	printw(" b - buy dwarf: $50\n\n");
	printw(" d - current dwarf count\n\n");
	printw(" e - buy eggshell: $100\n\n");
	printw(" k - kg of rock moved\n\n");
	printw(" w - check your wallet\n\n");
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
				if (kigdug < 10) {
					printw("You don't have enough money for an apple.");
					refresh();
        			usleep(2500000); // give the reader time to see the message.
					move(1, 25);
					printw("                                         ");
					break;
				}
				applespawn = true;
				kigdug -= 10;
				printw("A new apple has been created. (-$10)");
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                                    ");
				refresh();
				break;
			case 'b': // purchase new dwarves
				move(1, 25);
				if (kigdug  < 50) {
					printw("You don't have enough money for a dwarf.");
					refresh();
        			usleep(2500000); // give the reader time to see the message.
					move(1, 25);
					printw("                                        ");
					break;
				}
				dwarfspawn = true;
				printw("A new dwarf has been created. (-$50)");
				kigdug -= 50;
				refresh();
        		usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                                    ");
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
			case 'e':
				move(1, 25);
				if (kigdug < 100) {
					printw("You don't have enough money for an eggshell.");
					refresh();
        			usleep(2500000); // give the reader time to see the message.
					move(1, 25);
					printw("                                            ");
					break;
				}
				eggspawn = true;
				kigdug -= 100;
				egg_time++;
				printw("An eggshell has been created (%d/3). (-$100)", egg_time);
				refresh();
				usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                                            ");
				break;
			case 'w':
				move(1, 25);
				printw("You have $%d to spend.", (kigdug));
				refresh();
				usleep(2500000); // give the reader time to see the message.
				move(1, 25);
				printw("                      ");
				refresh();
				break;
		}
	};
		
	endwin();
	return 0;
}
