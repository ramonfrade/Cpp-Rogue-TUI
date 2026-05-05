#include <ncurses.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

class Game {
   private:
    int playerY = 0;
    int playerX = 0;

    int enemyY = 0;
    int enemyX = 0;

    bool isRunning;
    std::vector<std::string> map;

    void initNcurses() {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
    }

   public:
    Game()  // Constructing the game, just starting the map in the terminal
    {
        srand(time(NULL));
        initNcurses();
        loadMap();
        loadPlayer();
        enemyY = map.size() - 2;
        enemyX = map[0].size() - 2;
        isRunning = true;
    }
    ~Game() { endwin(); }

    // Loading the map from a file, so that it can be easily edited and expanded
    void loadMap() {
        std::ifstream file("../map.txt");

        std::string line;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                map.push_back(line);
            }

            file.close();

        } else {
            // If the file cannot be opened, display an error message and exit
            printw("Unable to open map file!");
            refresh();
            getch();
            endwin();
            exit(1);
        }
    }

    // Loading the player's position from a file, so that it can be easily
    // edited and expanded
    void loadPlayer() {
        std::ifstream file("../player.txt");

        if (file.is_open()) {
            file >> playerY >> playerX;
            file.close();
        } else {
            playerY = 1;
            playerX = 1;
        }
    }

    // Saving the map and player position to a file, so that it can be loaded
    // again when the game is restarted
    void saveMap() {
        std::ofstream file("../map.txt");

        if (file.is_open()) {
            for (int i = 0; i < map.size(); i++) {
                file << map[i] << '\n';
            }
            file.close();
        }
    }

    void savePlayer() {
        std::ofstream file("../player.txt");
        if (file.is_open()) {
            file << playerY << '\n' << playerX << '\n';
            file.close();
        }
    }

    void draw() {
        clear();  // clearing player's previous trail
        for (int y = 0; y < map.size(); y++) {
            for (int x = 0; x < map[y].size(); x++) {
                mvaddch(y, x, map[y][x]);
            }
        }
        mvaddch(enemyY, enemyX, 'Z');    // positioning the enemy
        mvaddch(playerY, playerX, '@');  // positioning the player
        refresh();
    }

    void handleInput() {
        int key = getch();
        int nextY = playerY;
        int nextX = playerX;

        switch (key) {
            case 'w':
            case 'W':
            case KEY_UP:
                nextY--;
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                nextY++;
                break;
            case 'a':
            case 'A':
            case KEY_LEFT:
                nextX--;
                break;
            case 'd':
            case 'D':
            case KEY_RIGHT:
                nextX++;
                break;
            case 'p':
            case 'P':
                saveMap();
                savePlayer();
                break;
            case 'q':
            case 'Q':
                isRunning = false;
                break;
        }

        int direction = rand() % 4;
        int nextEnemyY = enemyY;
        int nextEnemyX = enemyX;

        switch (direction) {
            case 0:
                nextEnemyY--;
                break;
            case 1:
                nextEnemyY++;
                break;
            case 2:
                nextEnemyX--;
                break;
            case 3:
                nextEnemyX++;
                break;
        }

        if (map[nextY][nextX] != '#') {
            if (map[nextY][nextX] == '|') {
                map[nextY][nextX] = '/';
            } else {
                playerX = nextX;
                playerY = nextY;
            }
        }

        if (map[nextEnemyY][nextEnemyX] != '#' &&
            map[nextEnemyY][nextEnemyX] != '|') {
            enemyX = nextEnemyX;
            enemyY = nextEnemyY;
        }
    }

    void run() {
        while (isRunning) {
            draw();
            handleInput();
        }
    }
};

int main() {
    Game game;
    game.run();

    return 0;
}
