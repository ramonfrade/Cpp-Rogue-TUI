#include <ncurses.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

struct Creature {
    int hp = 0;
    int attack = 0;
    int x = 0;
    int y = 0;
    char glyph;
    bool alive = true;
};

class Game {
   private:
    Creature player;
    Creature enemy;
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
        loadEnemies();
        player.hp = 10;
        player.attack = 2;
        player.glyph = '@';
        player.alive = true;

        enemy.hp = 5;
        enemy.attack = 1;
        enemy.glyph = 'Z';
        enemy.alive = true;
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
            file >> player.y >> player.x;
            file.close();
        } else {
            player.y = 1;
            player.x = 1;
        }
    }

    void loadEnemies() {
        std::ifstream file("../enemies.txt");

        if (file.is_open()) {
            file >> enemy.y >> enemy.x;

            file.close();
        } else {
            enemy.y = map.size() - 2;
            enemy.x = map[0].size() - 2;
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
            file << player.y << '\n' << player.x << '\n';
            file.close();
        }
    }

    void saveEnemies() {
        std::ofstream file("../enemies.txt");
        if (file.is_open()) {
            if (enemy.alive) {
                file << enemy.y << ' ' << enemy.x << '\n';
            }
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
        if (enemy.alive) {
            mvaddch(enemy.y, enemy.x, enemy.glyph);  // positioning the enemy
        }
        mvaddch(player.y, player.x, player.glyph);  // positioning the player
        refresh();
    }

    void handleInput() {
        int key = getch();
        int nextY = player.y;
        int nextX = player.x;

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
                saveEnemies();
                break;
            case 'q':
            case 'Q':
                isRunning = false;
                break;
        }

        int direction = rand() % 4;
        int nextEnemyY = enemy.y;
        int nextEnemyX = enemy.x;

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
                player.x = nextX;
                player.y = nextY;
            }
        }

        if (enemy.alive && map[nextEnemyY][nextEnemyX] != '#' &&
            map[nextEnemyY][nextEnemyX] != '|') {
            enemy.x = nextEnemyX;
            enemy.y = nextEnemyY;
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
