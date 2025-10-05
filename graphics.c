#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "game.h"
#include "snake.h"

void handle_events(Game *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    change_direction(&game->snake, 0);
                    break;
                case SDLK_RIGHT:
                    change_direction(&game->snake, 1);
                    break;
                case SDLK_DOWN:
                    change_direction(&game->snake, 2);
                    break;
                case SDLK_LEFT:
                    change_direction(&game->snake, 3);
                    break;
                case SDLK_ESCAPE:
                    game->running = false;
                    break;
                case SDLK_m:
                    game->in_menu = true;
                    show_menu(game);
                    break;
            }
        }
    }
}

void render_menu(Game *game, int selected_option) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    
    char *options[] = {
        "FACILE - Petite map, peu d'obstacles",
        "MOYEN - Map moyenne, obstacles modérés", 
        "DIFFICILE - Grande map, nombreux obstacles"
    };
    
    // Afficher les instructions dans la console
    if (selected_option != -1) {
        system("clear");  // Effacer la console (Linux)
        printf("=== SNAKE GAME - MENU ===\n\n");
        printf("Utilisez les fleches ↑↓ pour naviguer\n");
        printf("ESPACE pour selectionner\n\n");
        
        for (int i = 0; i < 3; i++) {
            if (i == selected_option) {
                printf(">>> %s <<<\n", options[i]);
            } else {
                printf("    %s\n", options[i]);
            }
        }
        printf("\n[ESPACE] Commencer | [ECHAP] Quitter\n");
    }
    
    // Dessiner des formes simples pour le menu visuel
    for (int i = 0; i < 3; i++) {
        SDL_Color color = (i == selected_option) ? yellow : white;
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
        
        // Cadre de l'option
        SDL_Rect option_rect = {100, 200 + i * 80, 400, 60};
        SDL_RenderDrawRect(game->renderer, &option_rect);
        
        // Cercle de sélection
        if (i == selected_option) {
            SDL_Rect selector = {70, 215 + i * 80, 20, 20};
            SDL_RenderFillRect(game->renderer, &selector);
        }
        
        // Texte simplifié (juste des barres pour représenter le texte)
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
        for (int j = 0; j < 5; j++) {
            SDL_Rect text_bar = {120, 225 + i * 80 + j * 6, 200 + j * 10, 3};
            SDL_RenderFillRect(game->renderer, &text_bar);
        }
    }
    
    // Instructions en bas
    SDL_SetRenderDrawColor(game->renderer, 100, 100, 255, 255);
    for (int j = 0; j < 3; j++) {
        SDL_Rect instr_bar = {150, 500 + j * 10, 300 - j * 20, 4};
        SDL_RenderFillRect(game->renderer, &instr_bar);
    }
    
    SDL_RenderPresent(game->renderer);
}

void show_menu(Game *game) {
    int selected_option = 0;
    bool menu_running = true;
    
    while (menu_running && game->running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game->running = false;
                menu_running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        selected_option = (selected_option - 1 + 3) % 3;
                        break;
                    case SDLK_DOWN:
                        selected_option = (selected_option + 1) % 3;
                        break;
                    case SDLK_SPACE:
                    case SDLK_RETURN:
                        menu_running = false;
                        // Définir la taille et la vitesse selon la difficulté
                        int size;
                        switch (selected_option) {
                            case 0: 
                                size = 15; 
                                game->game_speed = 200; // Facile = lent
                                break;
                            case 1: 
                                size = 20; 
                                game->game_speed = 150; // Moyen = normal
                                break;
                            case 2: 
                                size = 25; 
                                game->game_speed = 100; // Difficile = rapide
                                break;
                            default: 
                                size = 20; 
                                game->game_speed = 150;
                                break;
                        }
                        init_game(game, size, selected_option + 1);
                        SDL_SetWindowTitle(game->window, "Snake Game - En cours");
                        break;
                    case SDLK_ESCAPE:
                        game->running = false;
                        menu_running = false;
                        break;
                }
            }
        }
        
        render_menu(game, selected_option);
        SDL_Delay(16);
    }
}

void render_game(Game *game) {
    int cell_size = WINDOW_SIZE / game->map.size;
    
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    // Murs (gris)
    SDL_SetRenderDrawColor(game->renderer, 100, 100, 100, 255);
    for (int y = 0; y < game->map.size; y++) {
        for (int x = 0; x < game->map.size; x++) {
            char cell = get_cell(&game->map, x, y);
            if (cell == 'W') {
                SDL_Rect wall = {x * cell_size, y * cell_size, cell_size, cell_size};
                SDL_RenderFillRect(game->renderer, &wall);
            }
        }
    }
    
    // Obstacles (marron)
    SDL_SetRenderDrawColor(game->renderer, 139, 69, 19, 255);
    for (int y = 0; y < game->map.size; y++) {
        for (int x = 0; x < game->map.size; x++) {
            char cell = get_cell(&game->map, x, y);
            if (cell == 'O') {
                SDL_Rect obstacle = {x * cell_size, y * cell_size, cell_size, cell_size};
                SDL_RenderFillRect(game->renderer, &obstacle);
            }
        }
    }
    
    // Fruit normal (rouge)
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_Rect fruit_rect = {
        game->map.fruit.x * cell_size,
        game->map.fruit.y * cell_size,
        cell_size, cell_size
    };
    SDL_RenderFillRect(game->renderer, &fruit_rect);
    
    // Fruit spécial (or) - clignotant si proche de la fin
    if (game->map.special_fruit_timer > 0) {
        if (game->map.special_fruit_timer > 20 || game->map.special_fruit_timer % 4 < 2) {
            SDL_SetRenderDrawColor(game->renderer, 255, 215, 0, 255);
            SDL_Rect special_rect = {
                game->map.special_fruit.x * cell_size,
                game->map.special_fruit.y * cell_size,
                cell_size, cell_size
            };
            SDL_RenderFillRect(game->renderer, &special_rect);
        }
    }
    
    // Serpent (vert)
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
    Node *current = game->snake.head;
    while (current != NULL) {
        SDL_Rect segment = {
            current->position.x * cell_size,
            current->position.y * cell_size,
            cell_size, cell_size
        };
        SDL_RenderFillRect(game->renderer, &segment);
        current = current->next;
    }
    
    SDL_RenderPresent(game->renderer);
}