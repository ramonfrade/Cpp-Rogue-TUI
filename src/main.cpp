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
    std::string templateName;
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
    std::vector<Creature> enemies;
    bool isRunning = false;
    std::vector<std::string> map;
    std::vector<CreatureTemplate> templates;
    enum { CP_PLAYER = 1, CP_ENEMY = 2 };

    void fatalError(const std::string& message) {
        printw("%s", message.c_str());
        refresh();
        getch();
        endwin();
        exit(1);
    }

    void initNcurses() {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1);
        init_pair(2, COLOR_RED, -1);
    }

public:
    Game() {
        srand(time(NULL));
        initNcurses();

        loadCreatures();

        applyTemplate(player, findTemplate("player"));

        isRunning = true;
        loadMap();
        loadPlayer();
        loadEnemies();

        if (enemies.empty()) {
            spawnEnemy("zombie", 3, 3);
            spawnEnemy("zombie", 5, 5);
        }
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
            fatalError("Unable to open map file!");
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

    CreatureTemplate findTemplate(const std::string& name) {
        for (int i = 0; i < templates.size(); i++) {
            if (templates[i].name == name) {
                return templates[i];
            }
        }
        fatalError("Error: template '" + name + "' not found!");
        return {};
    }

    void applyTemplate(Creature& c, const CreatureTemplate& t) {
        c.templateName = t.name;
        c.maxHp = t.maxHp;
        c.hp = t.maxHp;
        c.attack = t.attack;
        c.glyph = t.glyph;
        c.alive = true;
    }

    void spawnEnemy(const std::string& templateName, int x, int y) {
        Creature enemy;
        applyTemplate(enemy, findTemplate(templateName));
        enemy.x = x;
        enemy.y = y;
        enemies.push_back(enemy);
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
            file.close();
        } else {
            fatalError("Unable to open creatures file!");
        }
        if (templates.empty()) {
            fatalError("Unable to load creature templates!");
        }
    }

    void loadEnemies() {
        std::ifstream file("../enemies.txt");

        if (file.is_open()) {
            int y = 0;
            int x = 0;
            int hp = 0;
            std::string templateName;

            while (file >> y >> x >> hp >> templateName) {
                spawnEnemy(templateName, x, y);
                enemies.back().hp = hp;
            }

            file.close();
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
            for (int i = 0; i < enemies.size(); i++) {
                if (enemies[i].alive) {
                    file << enemies[i].y << ' ' << enemies[i].x << ' ' << enemies[i].hp << ' '
                         << enemies[i].templateName << '\n';
                }
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

    void drawBar(int y, int x, int hp, int maxHp, int barSize, int colorPair) {
        attron(COLOR_PAIR(colorPair));
        int filled = hp * barSize / maxHp;
        mvprintw(y, x, "[");
        for (int i = 0; i < filled; i++) {
            mvprintw(y, x + 1 + i, "#");
        }
        for (int i = filled; i < barSize; i++) {
            mvprintw(y, x + 1 + i, "-");
        }
        mvprintw(y, x + barSize + 1, "]");
        attroff(COLOR_PAIR(colorPair));
    }

    void draw() {
        clear();
        for (int y = 0; y < map.size(); y++) {
            for (int x = 0; x < map[y].size(); x++) {
                mvaddch(y, x, map[y][x]);
            }
        }

        drawBar(12, 3, player.hp, player.maxHp, 10, CP_PLAYER);
        mvprintw(12, 0, "HP ");
        mvprintw(12, 3 + 12, " ATK: %d", player.attack);

        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].alive) {
                attron(COLOR_PAIR(CP_ENEMY));
                mvaddch(enemies[i].y, enemies[i].x, enemies[i].glyph);
                attroff(COLOR_PAIR(CP_ENEMY));

                int dist = abs(player.y - enemies[i].y) + abs(player.x - enemies[i].x);
                if (dist <= 3 && enemies[i].y - 1 >= 0 && enemies[i].x - 3 >= 0) {
                    int barSize = 5;
                    int filled = enemies[i].hp * barSize / enemies[i].maxHp;
                    if (filled < barSize) {  // Only show bar for damaged enemies.
                        drawBar(enemies[i].y - 1, enemies[i].x - 3, enemies[i].hp, enemies[i].maxHp, barSize, CP_ENEMY);
                    }
                }
            }
        }

        mvprintw(15, 0, "WASD/arrows: move  P: save  Q: quit");
        attron(COLOR_PAIR(CP_PLAYER));
        mvaddch(player.y, player.x, player.glyph);
        attroff(COLOR_PAIR(CP_PLAYER));
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

    bool enemyCanMove(const Creature& enemy, int y, int x) {
        return (enemy.alive && isInBounds(y, x) && map[y][x] != '#' && map[y][x] != '|');
    }

    bool isPlayerAt(int y, int x) { return (player.alive && player.y == y && player.x == x); }

    bool isOccupied(int y, int x, int excludeEnemyIndex) {
        if (isPlayerAt(y, x)) return true;

        for (int i = 0; i < enemies.size(); i++) {
            if (i == excludeEnemyIndex) continue;
            if (enemies[i].alive && enemies[i].y == y && enemies[i].x == x) return true;
        }
        return false;
    }

    void updateEnemyAI() {
        for (int i = 0; i < enemies.size(); i++) {
            if (!enemies[i].alive) continue;

            const std::array<int, 4> dy = {-1, 1, 0, 0};
            const std::array<int, 4> dx = {0, 0, -1, 1};

            int bestY = enemies[i].y;
            int bestX = enemies[i].x;
            int bestDist = INT_MAX;
            bool attacked = false;

            for (size_t j = 0; j < dy.size(); j++) {
                int ny = enemies[i].y + dy[j];
                int nx = enemies[i].x + dx[j];

                if (!enemyCanMove(enemies[i], ny, nx)) continue;

                if (isPlayerAt(ny, nx)) {
                    attack(enemies[i], player);
                    attacked = true;
                    break;
                }

                if (isOccupied(ny, nx, i)) continue;

                int dist = abs(player.y - ny) + abs(player.x - nx);

                if (dist < bestDist) {
                    bestDist = dist;
                    bestY = ny;
                    bestX = nx;
                }
            }

            if (attacked) continue;

            enemies[i].y = bestY;
            enemies[i].x = bestX;
        }
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
                return;
            case 'q':
            case 'Q':
                isRunning = false;
                return;
        }

        if (nextY == player.y && nextX == player.x) {
            return;
        }

        if (isInBounds(nextY, nextX) && map[nextY][nextX] != '#') {
            if (map[nextY][nextX] == '|') {
                map[nextY][nextX] = '/';
            } else {
                bool hitEnemy = false;
                for (int i = 0; i < enemies.size(); i++) {
                    if (enemies[i].alive && enemies[i].y == nextY && enemies[i].x == nextX) {
                        attack(player, enemies[i]);
                        hitEnemy = true;
                        break;
                    }
                }
                if (!hitEnemy) {
                    player.x = nextX;
                    player.y = nextY;
                }
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
