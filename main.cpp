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
void loadColorPair();                        // Assign color pairs
void printTrees();
void savePointFile(int point);
queue<int> getPoints();
void printPoints();
bool isMovementKey(int key);
void moveCar(int key);
void gameOperations(int key);
void handleInput(int key);
Car generateCar(queue<Car> cars);
void *moveEnemyCars(void *args);
void *enqueueCars(void *);
void printCurrentPoints();
void saveGame(Game game);
void loadLastGame();
void saveCarsToFile(Game game);
void readCars();
void writeCarsToWindow(queue<Car> carQueue);

int main() {
    /*  Start - Mustafa Kazı */
    printMainMenu();
    return 0;
}

void initGame() {
    playingGame.cars = queue<Car>();
    playingGame.counter = IDSTART;
    pthread_mutex_init(&playingGame.mutexFile, NULL);
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
    if (playingGame.leftKey != leftKeyA) {
        playingGame.leftKey = leftKeyArrow;
        playingGame.rightKey = RightKeyArrow;
    }
    printWindow();
    drawCar(playingGame.current, 2, 1);// Draw the car the player is driving on the screen

    pthread_t thMoveEnemyCars, thEnqueueCars;
    pthread_create(&thMoveEnemyCars, NULL, moveEnemyCars, NULL);// Start the thread to move enemy cars
    pthread_create(&thEnqueueCars, NULL, enqueueCars, NULL);    // Start the thread to enqueue new cars

    int key;
    while (playingGame.IsGameRunning) {// Continue until the game is over
        printCurrentPoints();
        key = getch();// Get input for the player to press the arrow keys
        if (key != KEYERROR) {
            handleInput(key);
        }
        usleep(GAMESLEEPRATE);// sleep
    }

    pthread_join(thMoveEnemyCars, NULL);
    pthread_join(thEnqueueCars, NULL);

    pthread_exit(NULL);
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

        attron(COLOR_PAIR(c.clr));//enable color pair
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
        attroff(COLOR_PAIR(c.clr));                  // disable color pair
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
                                     NULL);//Wait for the thread to finish, when the newGame function finishes, the thread will also finish.
                        break;

                    case 1:
                        // Load Last Game
                         loadLastGame();
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
                        printPoints();
                        break;
                    case 5:
                        // Exit
                        endwin();
                        exit(1);
                }
                break;
        }
        refresh();
        usleep(MENSLEEPRATE);// Delay for inputs
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
    init_pair(3,COLOR_YELLOW,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);

}

/*Ugur Tansal*/
void savePointFile(int point) {
    FILE *pointFile = fopen(pointsTxt, "a+");
    fwrite(&point, sizeof(int), 1, pointFile);
    fwrite("\n", 2, 1, pointFile);
    fclose(pointFile);
}

/* Mustafa Kazı */
queue<int> getPoints() {
    queue<int> points;

    FILE *pointsFile = fopen(pointsTxt, "r");
    if (pointsFile == nullptr) {
        return points;
    }

    int point = -1;
    while (fscanf(pointsFile, "%d", &point) == 1) {
        points.push(point);
    }

    fclose(pointsFile);
    return points;
}


/*Ugur Tansal*/
void printPoints() {

    queue<int> points = getPoints();
    if (points.empty()) {

        mvprintw(MENUY, MENUX, "No points...");
        refresh();
        return;
    }

    clear();

    attron(COLOR_PAIR(1));

    int x = MENUX, y = MENUY;
    int gameNumber = 1;


    while (!points.empty()) {

        char text[200];
        sprintf(text, "Game %d: %d", gameNumber++, points.front());

        mvprintw(y, x, text);
        y += MENUDIF;


        if (gameNumber % 10 == 1) {
            y = MENUY;
            x += MENUDIFX;
        }

        points.pop();
    }


    attroff(COLOR_PAIR(1));
    refresh();
    sleep(5);
    endwin();
}

/* Mustafa Kazı */
bool isMovementKey(int key) {
    if (playingGame.leftKey == key || playingGame.rightKey == key) return true;
    return false;
}
/* Mustafa Kazı */
void moveCar(int key) {
    if (key == playingGame.leftKey) {
        drawCar(playingGame.current, 1, 1);

        if ((playingGame.current.x - playingGame.current.speed) > 5) {
            playingGame.current.x -= playingGame.current.speed;
        } else {
            playingGame.current.x = 5;
        }

        drawCar(playingGame.current, 2, 1);
    } else if (key == playingGame.rightKey) {
        drawCar(playingGame.current, 1, 1);

        if ((playingGame.current.x + playingGame.current.speed) < 90) {
            playingGame.current.x += playingGame.current.speed;
        } else {
            playingGame.current.x = 90;
        }
        drawCar(playingGame.current, 2, 1);
    }
}

/* Mustafa Kazı */
void gameOperations(int key) {
    if (ESC == key) {
        clear();
        printMainMenu();
        refresh();
    } else if (SAVEKEY == key) {
        saveGame(playingGame);
        clear();
        printMainMenu();
        refresh();
    }
}

/* Mustafa Kazı */
void handleInput(int key) {
    if (isMovementKey(key)) {
        moveCar(key);
    } else {
        gameOperations(key);
    }
}

/* Uğur Tansal */
Car generateCar(queue<Car> cars) {
    if (cars.size() < maxCarNumber) {

        Car newCar;
        if (cars.empty()) {
            newCar.ID = IDSTART;
        } else if (cars.back().ID < IDMAX) {
            newCar.ID = cars.back().ID++;
        } else {
            newCar.ID = IDSTART;
        }

        srand(time(NULL));
        int chrNum;          //Random number for character
        bool control = false;//If the values uniquely

        do {
            control = false;
            newCar.y = rand() % (MINY + 1) + MINY;
            newCar.y*=(-1);
            newCar.height = rand() % (7 - MINH + 1) + MINH;
            newCar.width = rand() % (7 - MINW + 1) + MINW;
            do {
                //newCar.x = rand() % (wWidth - 2 * MINW) + MINW;
                newCar.x=rand() % ( wWidth-10-MINX+1)+MINX;
            } while ((newCar.x<=lineX && newCar.x+newCar.width>lineX) || newCar.x+newCar.width>=wWidth);
            newCar.speed = newCar.height / 2;

            newCar.clr = rand() % (numOfcolors - 1 + 1) + 1;
            newCar.isExist = false;
            chrNum = rand() % (numOfChars - 1 + 1) + 1;
            switch (chrNum) {
                case 1:
                    newCar.chr = '*';
                    break;

                case 2:
                    newCar.chr = '#';
                    break;

                case 3:
                    newCar.chr = '+';
                    break;
            }

            queue<Car> newQueue;
            Car current;
            while (!cars.empty()) {
                current = cars.front();
                newQueue.push(current);
                cars.pop();
                if (newCar.clr == current.clr && newCar.height == current.height && newCar.width == current.width && newCar.speed == current.speed && newCar.chr == current.chr) {
                    control = true;
                }
            }
            playingGame.cars = newQueue;
        } while (control);

        return newCar;
    }
}

/* Mustafa Kazı */
void *moveEnemyCars(void *args) {
    while (playingGame.IsGameRunning) {
        sleep(EnQueueSleep);

        pthread_mutex_lock(&playingGame.mutexFile);
        queue<Car> tempQueue = playingGame.cars;
        playingGame.cars = queue<Car>();
        pthread_mutex_unlock(&playingGame.mutexFile);

        while (!tempQueue.empty()) {
            Car car = tempQueue.front();
            tempQueue.pop();
            drawCar(car, 1, 0);
            car.y += car.speed;

            if (car.y < EXITY) {
                drawCar(car, 2, 0);
                pthread_mutex_lock(&playingGame.mutexFile);
                playingGame.cars.push(car);
                pthread_mutex_unlock(&playingGame.mutexFile);
            } else {
                playingGame.points += car.height * car.width;
                printCurrentPoints();
            }
        }


    }
    pthread_exit(NULL);
}

/* Mustafa Kazı */
void *enqueueCars(void *) {
    while (playingGame.IsGameRunning) {
        pthread_mutex_lock(&playingGame.mutexFile);
        if (playingGame.cars.size() < maxCarNumber) {
            playingGame.cars.push(generateCar(playingGame.cars));
        }
        pthread_mutex_unlock(&playingGame.mutexFile);

        sleep(EnQueueSleep);
    }
    pthread_exit(NULL);
}

/* Mustafa Kazı */
void printCurrentPoints() {
    mvprintw(POINTY, POINTX, "Points: %d", playingGame.points);
    refresh();
}

/*Uğur Tansal*/
void loadLastGame()
{
    FILE *gameFile=fopen(gameTxt,"rb+");
    fread(&playingGame,sizeof(Game),1,gameFile);
    fclose(gameFile);
    clear();
    refresh();
    readCars();
    pthread_t th1;
    pthread_create(&th1, NULL, newGame, NULL);
    pthread_join(th1,NULL);


}

/*Uğur Tansal*/
void saveCarsToFile(Game game)
{
    FILE *carsFile=fopen(CarsTxt,"wb+");
    queue<Car> newQueue;
    Car current;
    while(!game.cars.empty())
    {
        current=game.cars.front();
        fwrite(&current,sizeof(Car),1,carsFile);
        newQueue.push(current);
        game.cars.pop();
    }
    game.cars=newQueue;
    fclose(carsFile);
}
/*Uğur Tansal*/
void readCars()
{
    queue<Car> newQueue;
    Car current;
    FILE *carsFile=fopen(CarsTxt,"rb+");
    fread(&current,sizeof(Car),1,carsFile);
    while(!feof(carsFile))
    {
      fread(&current,sizeof(Car),1,carsFile);
      newQueue.push(current);
    }

    writeCarsToWindow(newQueue);
}
/*Uğur Tansal*/
void writeCarsToWindow(queue<Car> carQueue)
{
    Car car;
    while(!carQueue.empty())
    {
        car=carQueue.front();
        drawCar(car,2,0);
        playingGame.cars.push(car);
        carQueue.pop();
    }
}

/*Uğur Tansal*/

void saveGame(Game game)
{
    saveCarsToFile(game);
    savePointFile(playingGame.points);
    playingGame.IsSaveCliked=true;
    FILE *gameFile = fopen(gameTxt, "wb+");
    fwrite(&game, sizeof(game), 1, gameFile);
    fclose(gameFile);
}
