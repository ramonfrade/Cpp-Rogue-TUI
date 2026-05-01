#include <ncurses.h>
#include <sys/types.h>

#include <string>
#include <vector>

class Game {
   private:
    int playerY = 0;
    int playerX = 0;
    bool isRunning;
    std::vector<std::string> map = {
        "#######################", "#..........#..........#",
        "#..........#..........#", "#..........#..........#",
        "#..........#..........#", "#..........+..........#",
        "#..........#..........#", "#..........#..........#",
        "#..........#..........#", "#..........#..........#",
        "#######################"};

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
        initNcurses();
        playerY = 1;
        playerX = 1;
        isRunning = true;
    }
    ~Game() { endwin(); }

    void draw() {
        clear();  // clearing player's previous trail
        for (int y = 0; y < map.size(); y++) {
            for (int x = 0; x < map[y].size(); x++) {
                mvaddch(y, x, map[y][x]);
            }
        }
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
            case 'q':
            case 'Q':
                isRunning = false;
                break;
        }

        if (map[nextY][nextX] != '#') {
            if (map[nextY][nextX] == '+') {
                map[nextY][nextX] = '/';
            } else {
                playerX = nextX;
                playerY = nextY;
            }
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
