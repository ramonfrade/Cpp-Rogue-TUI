#include <ncurses.h>

class Game {
   private:
    int playerY = 0;
    int playerX = 0;
    bool isRunning;

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
        getmaxyx(stdscr, playerY, playerX);  // getting size of the screen

        // positioning the player in the middle of the screen
        playerX = playerX / 2;
        playerY = playerY / 2;

        isRunning = true;
    }
    ~Game() { endwin(); }

    void draw() {
        clear();  // clearing player's previous trail

        mvprintw(0, 0, "Use W, A, S, D or Arrows to move. Press 'Q' to quit.");
        mvprintw(26, 0, "Debug: Current Player Position (X: %d, Y: %d)",
                 playerX, playerY);

        mvaddch(playerY, playerX, '@');  // positioning the player
        refresh();
    }

    void handleInput() {
        int key = getch();

        switch (key) {
            case 'w':
            case 'W':
            case KEY_UP:
                --playerY;
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                ++playerY;
                break;
            case 'a':
            case 'A':
            case KEY_LEFT:
                --playerX;
                break;
            case 'd':
            case 'D':
            case KEY_RIGHT:
                ++playerX;
                break;
            case 'q':
            case 'Q':
                isRunning = false;
                break;
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
