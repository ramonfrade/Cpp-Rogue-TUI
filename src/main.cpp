#include <ncurses.h>

#include <array>
#include <climits>
#include <cstddef>
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
    Game() {
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

    // Loading functions

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
        mvprintw(11, 1, "Player HP: %d", player.hp);
        mvprintw(12, 1, "Enemy HP: %d", enemy.hp);
        if (enemy.alive) {
            mvaddch(enemy.y, enemy.x, enemy.glyph);  // positioning the enemy
        }
        mvaddch(player.y, player.x, player.glyph);  // positioning the player
        refresh();
    }

    void attack(Creature& attacker, Creature& defender) {
        defender.hp -= attacker.attack;
        if (defender.hp <= 0) {
            defender.alive = false;
        }
    }

    bool enemyCanMove(int y, int x) { return (enemy.alive && map[y][x] != '#' && map[y][x] != '|'); }

    bool isPlayerAt(int y, int x) { return (player.alive && player.y == y && player.x == x); }

    void updateEnemyAI() {
        if (!enemy.alive) return;

        const std::array<int, 4> dy = {-1, 1, 0, 0};
        const std::array<int, 4> dx = {0, 0, -1, 1};

        int bestY = enemy.y;
        int bestX = enemy.x;
        int bestDist = abs(player.y - enemy.y) + abs(player.x - enemy.x);

        for (size_t i = 0; i < dy.size(); i++) {
            int ny = enemy.y + dy[i];
            int nx = enemy.x + dx[i];

            if (!enemyCanMove(ny, nx)) continue;

            if (isPlayerAt(ny, nx)) {
                attack(enemy, player);
                return;
            }

            int dist = abs(player.y - ny) + abs(player.x - nx);
            if (dist < bestDist) {
                bestDist = dist;
                bestY = ny;
                bestX = nx;
            }
        }

        if (bestY == enemy.y && bestX == enemy.x) {
            int minDist = INT_MAX;

            for (size_t i = 0; i < dy.size(); i++) {
                int ny = enemy.y + dy[i];
                int nx = enemy.x + dx[i];

                if (!enemyCanMove(ny, nx)) continue;

                int dist = abs(player.y - ny) + abs(player.x - nx);
                if (dist < minDist) {
                    minDist = dist;
                    bestY = ny;
                    bestX = nx;
                }
            }
        }

        enemy.y = bestY;
        enemy.x = bestX;
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

        if (map[nextY][nextX] != '#') {
            if (map[nextY][nextX] == '|') {
                map[nextY][nextX] = '/';
            } else if (enemy.alive && enemy.y == nextY && enemy.x == nextX) {
                attack(player, enemy);
            } else {
                player.x = nextX;
                player.y = nextY;
            }
        }

        updateEnemyAI();

        if (!player.alive) {
            clear();
            printw("You died!");
            refresh();
            getch();
            isRunning = false;
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
