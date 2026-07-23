#include <ncurses.h>

#include <array>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

struct CreatureTemplate {
    std::string name;
    int maxHp = 0;
    int attack = 0;
    char glyph;
};

struct Creature {
    int hp = 0;
    int maxHp = 0;
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
    std::vector<CreatureTemplate> templates;

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

        loadCreatures();

        CreatureTemplate t = findTemplate("player");
        player.maxHp = t.maxHp;
        player.hp = t.maxHp;
        player.attack = t.attack;
        player.glyph = t.glyph;
        player.alive = true;

        t = findTemplate("zombie");
        enemy.maxHp = t.maxHp;
        enemy.hp = t.maxHp;
        enemy.attack = t.attack;
        enemy.glyph = t.glyph;
        enemy.alive = true;

        isRunning = true;
        loadMap();
        loadPlayer();
        loadEnemies();
    }
    ~Game() { endwin(); }

    void loadMap() {
        std::ifstream file("../map.txt");

        std::string line;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                map.push_back(line);
            }

            file.close();

        } else {
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
            file >> player.y >> player.x >> player.hp;
            file.close();
        } else {
            player.y = 1;
            player.x = 1;
        }
    }

    CreatureTemplate findTemplate(std::string name) {
        for (int i = 0; i < templates.size(); i++) {
            if (templates[i].name == name) {
                return templates[i];
            }
        }
        return templates[0];
    }

    void loadCreatures() {
        CreatureTemplate example;
        std::string name;
        int maxHp = 0;
        int attack = 0;
        char glyph;
        std::ifstream file("../creatures.txt");
        if (file.is_open()) {
            while (file >> name >> maxHp >> attack >> glyph) {
                example.name = name;
                example.maxHp = maxHp;
                example.attack = attack;
                example.glyph = glyph;
                templates.push_back(example);
            }
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
            file << player.y << '\n' << player.x << '\n' << player.hp << '\n';
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

    bool isInBounds(int y, int x) {
        if (y < 0 || x < 0) {
            return false;
        }
        if (y < map.size() && x < map[y].size()) {
            return true;
        }
        return false;
    }

    void draw() {
        clear();
        for (int y = 0; y < map.size(); y++) {
            for (int x = 0; x < map[y].size(); x++) {
                mvaddch(y, x, map[y][x]);
            }
        }

        int barSize = 10;
        int filled = player.hp * barSize / player.maxHp;
        mvprintw(12, 0, "HP [");
        for (int i = 0; i < filled; i++) {
            mvprintw(12, 4 + i, "#");
        }
        for (int i = filled; i < barSize; i++) {
            mvprintw(12, 4 + i, "-");
        }
        mvprintw(12, 4 + barSize, "] ATK: %d", player.attack);

        if (enemy.alive) {
            mvprintw(13, 0, "Enemy: HP %d", enemy.hp);
        }
        mvprintw(15, 0, "WASD/arrows: move  P: save  Q: quit");

        if (enemy.alive) {
            mvaddch(enemy.y, enemy.x, enemy.glyph);
        }
        mvaddch(player.y, player.x, player.glyph);
        refresh();
    }

    void attack(Creature& attacker, Creature& defender) {
        if (!defender.alive) {
            return;
        }

        defender.hp -= attacker.attack;
        if (defender.hp <= 0) {
            defender.alive = false;
            defender.hp = 0;
        }
    }

    bool enemyCanMove(int y, int x) {
        return (enemy.alive && isInBounds(y, x) && map[y][x] != '#' && map[y][x] != '|');
    }

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

        if (isInBounds(nextY, nextX) && map[nextY][nextX] != '#') {
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
            std::remove("../player.txt");
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
