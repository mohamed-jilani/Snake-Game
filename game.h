#ifndef GAME_H
#define GAME_H

#include "snake.h"

// Prototypes pour la logique du jeu
void init_game(Game *game, int size, int difficulty);
void free_game(Game *game);
void change_direction(Snake *snake, int new_direction);
void move_snake(Game *game);
bool check_collision(Game *game);
void update_game(Game *game);

// Prototypes pour le module Snake
Snake create_snake(int start_x, int start_y);
void add_segment(Snake *snake, int x, int y);
void remove_tail(Snake *snake);
void free_snake(Snake *snake);

// Prototypes pour le module GameMap
GameMap create_map(int size);
void free_map(GameMap *map);
char get_cell(GameMap *map, int x, int y);
void set_cell(GameMap *map, int x, int y, char value);
void spawn_obstacles(GameMap *map, int size, int difficulty);
void spawn_fruit(GameMap *map, int size);

#endif