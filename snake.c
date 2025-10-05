#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "snake.h"
#include "game.h"
#include "graphics.h"

int main(int argc, char *argv[]) {
    // Éviter les warnings pour paramètres non utilisés
    (void)argc;
    (void)argv;
    
    srand(time(NULL));
    
    Game game;
    
    // Initialisation minimale pour le menu
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    game.window = SDL_CreateWindow(
        "Snake Game - Menu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE,
        WINDOW_SIZE,
        SDL_WINDOW_SHOWN
    );
    
    if (!game.window) {
        printf("Erreur fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!game.renderer) {
        printf("Erreur renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(game.window);
        SDL_Quit();
        return 1;
    }
    
    game.running = true;
    game.in_menu = true;
    game.game_speed = 150; // Valeur par défaut
    
    // Afficher le menu au démarrage
    show_menu(&game);
    
    Uint32 last_update = SDL_GetTicks();
    
    while (game.running) {
        Uint32 current_time = SDL_GetTicks();
        
        handle_events(&game);
        
        if (!game.in_menu && (int)(current_time - last_update) > game.game_speed) {
            update_game(&game);
            last_update = current_time;
        }
        
        if (!game.in_menu) {
            render_game(&game);
        }
        
        SDL_Delay(10);
    }
    
    printf("Game Over! Score final: %d\n", game.score);
    free_game(&game);
    
    return 0;
}