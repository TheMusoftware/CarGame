#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define wHeight 40          // height of the road
#define wWidth 100          // width of the road
#define lineX 45            // x coordinate of the middle line
#define lineLEN 10          // distance of the middle line from the beginning and the end
#define EXITY 35            // coordinate showing the end of the road
#define leftKeyArrow 260    // ASCII code of the left arrow key
#define RightKeyArrow 261   // ASCII code of the right arrow key
#define leftKeyA 97         // ASCII code of A
#define RightKeyD 100       // ASCII code of D
#define ESC 27              // // ASCII code of the ESC key
#define ENTER 10            // ASCII code of the ENTER key
#define KEYUP 259           // ASCII code of the up arrow key
#define KEYDOWN 258         // ASCII code of the down arrow key
#define KEYERROR -1         // ASCII code returned if an incorrect key is pressed
#define SAVEKEY 115         // ASCII code of S
#define levelBound 300      // To increase level after 300 points
#define MAXSLEVEL 5         // maximum level
#define ISPEED 500000       // initial value for game moveSpeed
#define DRATESPEED 100000   // to decrease moveSpeed after each new level
#define MINX 5              // minimum x coordinate value when creating cars
#define MINY 10             // the maximum y coordinate value when creating the cars, then we multiply it by -1 and take its inverse
#define MINH 5              // minimum height when creating cars
#define MINW 5              // minimum width when creating cars
#define SPEEDOFCAR 3        // speed of the car driven by the player
#define YOFCAR 34           // y coordinate of the car used by the player
#define XOFCAR 45           // x coordinate of the car used by the player
#define IDSTART 10          // initial value for cars ID
#define IDMAX 20            // maximum value for cars ID
#define COLOROFCAR 3        // color value of the car used by the player
#define POINTX 91           //x coordinate where the point is written
#define POINTY 42           //y coordinate where the point is written
#define MENUX 10            // x coordinate for the starting row of the menus
#define MENUY 5             // y coordinate for the starting row of the menus
#define MENUDIF 2           // difference between menu rows
#define MENUDIFX 20         // difference between menu columns
#define MENSLEEPRATE 200000 // sleep time for menu input
#define GAMESLEEPRATE 250000// sleep time for player arrow keys
#define EnQueueSleep 1      // EnQueue sleep time
#define DeQueueSleepMin 2   // DeQueue minimum sleep time
#define numOfcolors 4       // maximum color value that can be selected for cars
#define maxCarNumber 5      // maximum number of cars in the queue
#define numOfChars 3        // maximum number of patterns that can be selected for cars
#define settingMenuItem 2   // number of options in the setting menu
#define mainMenuItem 6      // number of options in the main menu
/* Mustafa Kazı */
#define instructionsItem 4// number of options in the instructions
#define COLOR_PAIR_GREEN 1
#define COLOR_PAIR_RED 2


using namespace std;
typedef struct Car {//
    int ID;
    int x;
    int y;
    int height;
    int width;
    int speed;
    int clr;
    bool isExist;
    char chr;

} Car;
typedef struct Game {
    int leftKey;
    int rightKey;
    queue<Car> cars;
    bool IsGameRunning;
    bool IsSaveCliked;
    int counter;
    pthread_mutex_t mutexFile;
    Car current;
    int level;
    int moveSpeed;
    int points;
} Game;
Game playingGame;// Global variable used for new game
const char *gameTxt = "game.txt";
const char *CarsTxt = "cars.txt";
const char *pointsTxt = "points.txt";
//Array with options for the Setting menu
const char *settingMenu[50] = {"Play with < and > arrow keys", "Play with A and D keys"};
//Array with options for the Main menu
const char *mainMenu[50] = {"New Game", "Load the last game", "Instructions", "Settings", "Points", "Exit"};
//Array with options for the Instructors menu -> Mustafa Kazı
const char *instructors[50] = {"< or A: moves the car to the left", " > or D: moves the car to the right",
                               "ESC: exits the game without saving", "S: saves and exits the game"};

void drawCar(Car c, int type, int direction);//prints or remove the given car on the screen
void printWindow();                          //Draws the road on the screen
void *newGame(void *);                       // manages new game
void initGame();                             // Assigns initial values to all control parameters for the new game
void initWindow();                           //Creates a new window and sets I/O settings
void printMainMenu();                        // Print main menu
void *printInstructors(void *);              // Print instructions
void *printSettings(void *);                 // Print settings
void loadColorPair();                       // Assign color pairs
void printTrees();

int main() {
    /*  Start - Mustafa Kazı */
    printMainMenu();
    return 0;
}

void initGame() {
    playingGame.cars = queue<Car>();
    playingGame.counter = IDSTART;
    //playingGame.mutexFile = PTHREAD_MUTEX_INITIALIZER; //assigns the initial value for the mutex
    playingGame.level = 1;
    playingGame.moveSpeed = ISPEED;
    playingGame.points = 0;
    playingGame.IsSaveCliked = false;
    playingGame.IsGameRunning = true;
    playingGame.current.ID = IDSTART - 1;
    playingGame.current.height = MINH;
    playingGame.current.width = MINW;
    playingGame.current.speed = SPEEDOFCAR;
    playingGame.current.x = XOFCAR;
    playingGame.current.y = YOFCAR;
    playingGame.current.clr = COLOROFCAR;
    playingGame.current.chr = '*';
}

void *newGame(void *) {
    printWindow();
    drawCar(playingGame.current, 2, 1);// Draw the car the player is driving on the screen
    int key;
    while (playingGame.IsGameRunning) {//continue until the game is over
        key = getch();                 //Get input for the player to press the arrow keys
        if (key != KEYERROR) {
            if (key == playingGame.leftKey) {                      // If the left  key is pressed
                drawCar(playingGame.current, 1, 1);                // removes player's car from screen
                playingGame.current.x -= playingGame.current.speed;// update position
                drawCar(playingGame.current, 2, 1);                // draw player's car with new position
            }
        }
        usleep(GAMESLEEPRATE);// sleep
    }
}

void initWindow() {
    initscr();            // initialize the ncurses window
    start_color();        // enable color manipulation
    keypad(stdscr, true); // enable the keypad for the screen
    nodelay(stdscr, true);// set the getch() function to non-blocking mode
    curs_set(0);          // hide the cursor
    cbreak();             // disable line buffering
    noecho();             // don't echo characters entered by the user
    clear();              // clear the screen
    sleep(1);
}

void printWindow() {
    for (int i = 1; i < wHeight - 1; ++i) {
        //mvprintw: Used to print text on the window, paramters order: y , x , string
        mvprintw(i, 2, "*");//left side of the road
        mvprintw(i, 0, "*");
        mvprintw(i, wWidth - 1, "*");// right side of the road
        mvprintw(i, wWidth - 3, "*");
    }
    for (int i = lineLEN; i < wHeight - lineLEN; ++i) {//line in the middle of the road
        mvprintw(i, lineX, "#");
    }

    printTrees();

}

/*Ugur Tansal*/
void printTrees() {
    for (int i = 5; i < wHeight - 10; i += 10) {
        attron(COLOR_PAIR(COLOR_PAIR_GREEN));
        mvprintw(i, wWidth + 6, "*");
        mvprintw(i + 1, wWidth + 5, "*");
        mvprintw(i + 1, wWidth + 7, "*");
        mvprintw(i + 2, wWidth + 4, "*");
        mvprintw(i + 2, wWidth + 6, "*");
        mvprintw(i + 2, wWidth + 8, "*");
        attroff(COLOR_PAIR(COLOR_PAIR_GREEN));
        attron(COLOR_PAIR(COLOR_PAIR_RED));
        mvprintw(i + 3, wWidth + 6, "#");
        mvprintw(i + 4, wWidth + 6, "#");
        attroff(COLOR_PAIR(COLOR_PAIR_RED));
    }
}

void drawCar(Car c, int type, int direction) {
    //If the user does not want to exit the game and the game continues
    if (playingGame.IsSaveCliked != true && playingGame.IsGameRunning == true) {
        init_pair(c.ID, c.clr,
                  0);// Creates a color pair: init_pair(short pair ID, short foregroundcolor, short backgroundcolor);
        //0: Black (COLOR_BLACK)
        //1: Red (COLOR_RED)
        //2: Green (COLOR_GREEN)
        //3: Yellow (COLOR_YELLOW)
        //4: Blue (COLOR_BLUE)
        attron(COLOR_PAIR(c.ID));//enable color pair
        char drawnChar;
        if (type == 1)
            drawnChar = ' ';// to remove car
        else
            drawnChar = c.chr;//  to draw char
        //mvhline: used to draw a horizontal line in the window
        //shallow. : mvhline(int y, int x, chtype ch, int n)
        //y: horizontal coordinate
        //x: vertical coordinate
        //ch: character to use
        //n: Length of the line
        mvhline(c.y, c.x, drawnChar, c.width);               // top line of rectangle
        mvhline(c.y + c.height - 1, c.x, drawnChar, c.width);//bottom line of rectangle
        if (direction == 0)                                  // If it is any car on the road
            mvhline(c.y + c.height, c.x, drawnChar, c.width);
        else//player's card
            mvhline(c.y - 1, c.x, drawnChar, c.width);
        //mvvline: used to draw a vertical line in the window
        //shallow. : mvhline(int y, int x, chtype ch, int n)
        //y: horizontal coordinate
        //x: vertical coordinate
        //ch: character to use
        //n: Length of the line
        mvvline(c.y, c.x, drawnChar, c.height);              //left line of rectangle
        mvvline(c.y, c.x + c.width - 1, drawnChar, c.height);//right line of rectangle
        char text[5];
        if (type == 1)
            sprintf(text, "  ");//to remove point
        else
            sprintf(text, "%d", c.height * c.width);// to show car's point in rectangle
        mvprintw(c.y + 1, c.x + 1, text);           // display car's point in rectangle
        attroff(COLOR_PAIR(c.ID));                  // disable color pair
    }
}

/* Mustafa Kazı */
void printMainMenu() {
    initWindow();
    loadColorPair();
    int selected_menu_item = 0;
    while (true) {
        clear();// Clear screen
        // Print menu items
        for (int i = 0; i < mainMenuItem; i++) {
            attron(COLOR_PAIR(COLOR_PAIR_GREEN));

            if (i == selected_menu_item) {
                attron(COLOR_PAIR(COLOR_PAIR_RED));
                mvprintw(MENUY + MENUDIF * i, MENUX - 2, "->");// -2 For '->' symbol
            }
            mvprintw(MENUY + MENUDIF * i, MENUX, "%s\n", mainMenu[i]);
            attroff(COLOR_PAIR(COLOR_PAIR_RED));
        }
        int ch = getch();

        switch (ch) {
            case KEYUP:
                if (selected_menu_item > 0) {
                    selected_menu_item--;
                }
                break;

            case KEYDOWN:
                if (selected_menu_item + 1 < mainMenuItem) {
                    selected_menu_item++;
                }
                break;

            case ENTER:
                switch (selected_menu_item) {
                    pthread_t thMenu;
                    case 0:
                        initGame();
                        initWindow();
                        pthread_t th1;                            //create new thread
                        pthread_create(&th1, NULL, newGame, NULL);// Run newGame function with thread
                        pthread_join(th1,
                                     NULL);                  //Wait for the thread to finish, when the newGame function finishes, the thread will also finish.
                        break;

                    case 1:
                        // Load Last Game

                        break;

                    case 2:
                        // Load Instr
                        pthread_create(&thMenu, NULL, printInstructors, NULL);
                        pthread_join(thMenu, NULL);
                        break;
                    case 3:
                        pthread_create(&thMenu, NULL, printSettings, NULL);
                        pthread_join(thMenu, NULL);
                        break;
                    case 4:
                        // Show points
                        break;
                    case 5:
                        // Exit
                        endwin();
                        exit(1);
                }
                break;
        }
        refresh();
        usleep(MENSLEEPRATE); // Delay for inputs
    }
}

/* Mustafa Kazı */
void *printInstructors(void *) {
    clear();
    for (int i = 0; i < instructionsItem; i++) {
        attron(COLOR_PAIR(COLOR_PAIR_GREEN));
        mvprintw(MENUY + MENUDIF * i, MENUX, "%s\n", instructors[i]);
    }
    refresh();
    sleep(5);
}

/* Mustafa Kazı */
void *printSettings(void *) {
    clear();
    int selected_item = 0;
    start_color();

    while (true) {
        clear();
        for (int i = 0; i < settingMenuItem; i++) {
            if (i == selected_item) {
                attron(COLOR_PAIR(COLOR_PAIR_RED));
                mvprintw(MENUY + MENUDIF * i, MENUX - 2, "->");// -2 For '->' symbol
            } else {
                attron(COLOR_PAIR(COLOR_PAIR_GREEN));
            }
            mvprintw(MENUY + MENUDIF * i, MENUX, "%s\n", settingMenu[i]);
            attroff(COLOR_PAIR(COLOR_PAIR_GREEN));
            attroff(COLOR_PAIR(COLOR_PAIR_RED));
        }
        refresh();

        int ch = getch();

        switch (ch) {
            case KEY_UP:
                if (selected_item > 0) {
                    selected_item--;
                }
                break;

            case KEY_DOWN:
                if (selected_item + 1 < settingMenuItem) {
                    selected_item++;
                }
                break;

            case ENTER:
                switch (selected_item) {
                    case 0:
                        playingGame.leftKey = leftKeyArrow;
                        playingGame.rightKey = RightKeyArrow;
                        printMainMenu();// Return main menu
                        break;

                    case 1:
                        playingGame.leftKey = leftKeyA;
                        playingGame.rightKey = RightKeyD;
                        printMainMenu();// Return main menu
                        break;
                }
                break;
        }
    }
}

/* Mustafa Kazı */
void loadColorPair() {
    init_pair(COLOR_PAIR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_PAIR_RED, COLOR_RED, COLOR_BLACK);
}

/*Ugur Tansal*/
void savePointFile(long point) {
    FILE *pointFile = fopen(pointsTxt, "a+");
    fwrite(&point, sizeof(point), 1, pointFile);
    fwrite("\n", 2, 1, pointFile);
    fclose(pointFile);
}

/* Mustafa Kazı */
queue<int> *getPoints() {
    queue<int> *points = new queue<int>;
    FILE *pointsFile = fopen(pointsTxt, "r");
    int point = -1;
    while (fread(&point, sizeof(long), 1, pointsFile)) {
        points->push(point);
    }
    return points;
}

/*Ugur Tansal*/
void printPoints() {
    queue<int> *points = getPoints();
    clear();
    start_color();
    int x = 10, y = 5;
    int gameNumber = 1;

    char text[100];
    attron(COLOR_PAIR(COLOR_PAIR_GREEN));
    while (!points->empty()) {

        sprintf(text, "Game %d: %d", gameNumber++, points->front());
        mvprintw(y, x, text);
        y += 2;
        if (y == 15) {
            y = 2;
            x += 5;
        }
    }
    attroff(COLOR_PAIR(1));
    refresh();
    sleep(5);
    clear();
    refresh();
    usleep(3000000);
    attroff(COLOR_PAIR(2));
    clear();
    usleep(1000000);
    endwin();

}