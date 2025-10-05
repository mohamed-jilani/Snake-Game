#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define WINDOW_SIZE 600
#define MAX_OBSTACLES 20

// Structures
typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Node {
    Point position;
    struct Node *next;
} Node;

typedef struct Snake {
    Node *head;
    Node *tail;
    int direction;
    int length;
} Snake;

typedef struct GameMap {
    char **grid;
    int size;
    Point fruit;
    Point special_fruit;
    int obstacles_count;
    int special_fruit_timer;
} GameMap;

typedef struct Game {
    Snake snake;
    GameMap map;
    bool running;
    bool in_menu;
    int score;
    int difficulty;
    int game_speed;
    SDL_Window *window;
    SDL_Renderer *renderer;
} Game;

// Prototypes des fonctions principales
void init_game(Game *game, int size, int difficulty);
void free_game(Game *game);
void handle_events(Game *game);
void update_game(Game *game);
void render_game(Game *game);
void change_direction(Snake *snake, int new_direction);
void move_snake(Game *game);
void spawn_fruit(GameMap *map, int size);
void spawn_obstacles(GameMap *map, int size, int difficulty);
bool check_collision(Game *game);
void show_menu(Game *game);
void render_menu(Game *game, int selected_option);

// Module Snake
Snake create_snake(int start_x, int start_y);
void add_segment(Snake *snake, int x, int y);
void remove_tail(Snake *snake);
void free_snake(Snake *snake);

// Module GameMap
GameMap create_map(int size);
void free_map(GameMap *map);
char get_cell(GameMap *map, int x, int y);
void set_cell(GameMap *map, int x, int y, char value);

#endif