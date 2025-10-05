#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "snake.h"

// Prototypes pour l'affichage graphique
void handle_events(Game *game);
void render_game(Game *game);

// Prototypes pour le menu
void show_menu(Game *game);
void render_menu(Game *game, int selected_option);

#endif