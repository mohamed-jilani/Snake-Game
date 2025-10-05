#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WINDOW_SIZE 600
#define MAX_OBSTACLES 20

// Prototypes
typedef struct Point Point;
typedef struct Node Node;
typedef struct Snake Snake;
typedef struct GameMap GameMap;
typedef struct Game Game;

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

// Structures
struct Point {
    int x;
    int y;
};

struct Node {
    Point position;
    Node *next;
};

struct Snake {
    Node *head;
    Node *tail;
    int direction;
    int length;
};

struct GameMap {
    char **grid;
    int size;
    Point fruit;
    Point special_fruit;
    int obstacles_count;
    int special_fruit_timer;
};

struct Game {
    Snake snake;
    GameMap map;
    bool running;
    bool in_menu;
    int score;
    int difficulty;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

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

// Implémentation Snake
Snake create_snake(int start_x, int start_y) {
    Snake snake;
    snake.head = NULL;
    snake.tail = NULL;
    snake.direction = 1;
    snake.length = 0;
    
    add_segment(&snake, start_x, start_y);
    add_segment(&snake, start_x + 1, start_y);
    add_segment(&snake, start_x + 2, start_y);
    
    return snake;
}

void add_segment(Snake *snake, int x, int y) {
    Node *new_node = malloc(sizeof(Node));
    new_node->position.x = x;
    new_node->position.y = y;
    new_node->next = NULL;
    
    if (snake->head == NULL) {
        snake->head = new_node;
        snake->tail = new_node;
    } else {
        new_node->next = snake->head;
        snake->head = new_node;
    }
    snake->length++;
}

void remove_tail(Snake *snake) {
    if (snake->head == NULL) return;
    
    if (snake->head == snake->tail) {
        free(snake->head);
        snake->head = NULL;
        snake->tail = NULL;
    } else {
        Node *current = snake->head;
        while (current->next != snake->tail) {
            current = current->next;
        }
        free(snake->tail);
        current->next = NULL;
        snake->tail = current;
    }
    snake->length--;
}

void free_snake(Snake *snake) {
    while (snake->head != NULL) {
        remove_tail(snake);
    }
}

// Implémentation GameMap
GameMap create_map(int size) {
    GameMap map;
    map.size = size;
    map.grid = malloc(size * sizeof(char*));
    map.obstacles_count = 0;
    map.special_fruit_timer = 0;
    map.special_fruit.x = -1;
    map.special_fruit.y = -1;
    
    for (int i = 0; i < size; i++) {
        map.grid[i] = malloc(size * sizeof(char));
        for (int j = 0; j < size; j++) {
            map.grid[i][j] = ' ';
        }
    }
    
    // Murs extérieurs
    for (int i = 0; i < size; i++) {
        map.grid[0][i] = 'W';
        map.grid[size-1][i] = 'W';
        map.grid[i][0] = 'W';
        map.grid[i][size-1] = 'W';
    }
    
    map.fruit.x = -1;
    map.fruit.y = -1;
    
    return map;
}

void free_map(GameMap *map) {
    for (int i = 0; i < map->size; i++) {
        free(map->grid[i]);
    }
    free(map->grid);
}

char get_cell(GameMap *map, int x, int y) {
    if (x < 0 || x >= map->size || y < 0 || y >= map->size) {
        return 'W';
    }
    return map->grid[y][x];
}

void set_cell(GameMap *map, int x, int y, char value) {
    if (x >= 0 && x < map->size && y >= 0 && y < map->size) {
        map->grid[y][x] = value;
    }
}

void spawn_obstacles(GameMap *map, int size, int difficulty) {
    int num_obstacles;
    
    switch (difficulty) {
        case 1: num_obstacles = size / 4; break;  // Facile
        case 2: num_obstacles = size / 2; break;  // Moyen
        case 3: num_obstacles = size; break;      // Difficile
        default: num_obstacles = size / 4; break;
    }
    
    if (num_obstacles > MAX_OBSTACLES) num_obstacles = MAX_OBSTACLES;
    
    map->obstacles_count = 0;
    
    for (int i = 0; i < num_obstacles; i++) {
        int x, y;
        bool valid_position = false;
        int attempts = 0;
        
        while (!valid_position && attempts < 100) {
            x = rand() % (size - 4) + 2;
            y = rand() % (size - 4) + 2;
            
            if (get_cell(map, x, y) == ' ') {
                valid_position = true;
                set_cell(map, x, y, 'O');
                map->obstacles_count++;
            }
            attempts++;
        }
    }
}

void spawn_fruit(GameMap *map, int size) {
    int x, y;
    bool valid_position = false;
    
    while (!valid_position) {
        x = rand() % (size - 2) + 1;
        y = rand() % (size - 2) + 1;
        
        if (get_cell(map, x, y) == ' ') {
            valid_position = true;
            map->fruit.x = x;
            map->fruit.y = y;
            set_cell(map, x, y, 'F');
        }
    }
    
    // 20% de chance d'apparition d'un fruit spécial
    if (rand() % 5 == 0 && map->special_fruit_timer <= 0) {
        valid_position = false;
        while (!valid_position) {
            x = rand() % (size - 2) + 1;
            y = rand() % (size - 2) + 1;
            
            if (get_cell(map, x, y) == ' ') {
                valid_position = true;
                map->special_fruit.x = x;
                map->special_fruit.y = y;
                map->special_fruit_timer = 100; // Durée d'apparition
                set_cell(map, x, y, 'S');
            }
        }
    }
}

// GameLogic
void init_game(Game *game, int size, int difficulty) {
    game->map = create_map(size);
    game->difficulty = difficulty;
    
    int start_x = size / 2;
    int start_y = size / 2;
    game->snake = create_snake(start_x, start_y);
    game->running = true;
    game->in_menu = false;
    game->score = 0;
    
    spawn_obstacles(&game->map, size, difficulty);
    spawn_fruit(&game->map, size);
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        exit(1);
    }
    
    game->window = SDL_CreateWindow(
        "Snake Game - Menu",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE,
        WINDOW_SIZE,
        SDL_WINDOW_SHOWN
    );
    
    if (!game->window) {
        printf("Erreur fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (!game->renderer) {
        printf("Erreur renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        SDL_Quit();
        exit(1);
    }
}

void free_game(Game *game) {
    free_snake(&game->snake);
    free_map(&game->map);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
}

void change_direction(Snake *snake, int new_direction) {
    if ((snake->direction == 0 && new_direction != 2) ||
        (snake->direction == 2 && new_direction != 0) ||
        (snake->direction == 1 && new_direction != 3) ||
        (snake->direction == 3 && new_direction != 1)) {
        snake->direction = new_direction;
    }
}

void move_snake(Game *game) {
    Point new_head = game->snake.head->position;
    
    switch (game->snake.direction) {
        case 0: new_head.y--; break;
        case 1: new_head.x++; break;
        case 2: new_head.y++; break;
        case 3: new_head.x--; break;
    }
    
    // Téléportation aux bords
    if (new_head.x < 0) new_head.x = game->map.size - 1;
    if (new_head.x >= game->map.size) new_head.x = 0;
    if (new_head.y < 0) new_head.y = game->map.size - 1;
    if (new_head.y >= game->map.size) new_head.y = 0;
    
    add_segment(&game->snake, new_head.x, new_head.y);
    
    char cell = get_cell(&game->map, new_head.x, new_head.y);
    if (cell == 'F') {
        game->score += 10;
        spawn_fruit(&game->map, game->map.size);
    } else if (cell == 'S') {
        game->score += 30;
        game->map.special_fruit.x = -1;
        game->map.special_fruit.y = -1;
        game->map.special_fruit_timer = 0;
        set_cell(&game->map, new_head.x, new_head.y, ' ');
        spawn_fruit(&game->map, game->map.size);
    } else {
        remove_tail(&game->snake);
    }
    
    // Mettre à jour le timer du fruit spécial
    if (game->map.special_fruit_timer > 0) {
        game->map.special_fruit_timer--;
        if (game->map.special_fruit_timer == 0) {
            set_cell(&game->map, game->map.special_fruit.x, game->map.special_fruit.y, ' ');
            game->map.special_fruit.x = -1;
            game->map.special_fruit.y = -1;
        }
    }
}

bool check_collision(Game *game) {
    Point head = game->snake.head->position;
    
    // Collision avec murs ou obstacles
    char cell = get_cell(&game->map, head.x, head.y);
    if (cell == 'W' || cell == 'O') {
        return true;
    }
    
    // Collision avec soi-même
    Node *current = game->snake.head->next;
    while (current != NULL) {
        if (current->position.x == head.x && current->position.y == head.y) {
            return true;
        }
        current = current->next;
    }
    
    return false;
}

void update_game(Game *game) {
    move_snake(game);
    
    if (check_collision(game)) {
        game->running = false;
    }
}

void render_menu(Game *game, int selected_option) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    // Titre
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    
    // Dessiner les options
    char *options[] = {
        "1. Facile (Petite map, peu d'obstacles)",
        "2. Moyen (Map moyenne, obstacles modérés)", 
        "3. Difficile (Grande map, nombreux obstacles)",
        "Appuyez sur ESPACE pour selectionner"
    };
    
    for (int i = 0; i < 4; i++) {
        SDL_Color color = (i == selected_option) ? yellow : white;
        
        // Simulation d'affichage de texte (en pratique, utiliser SDL_ttf)
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, color.a);
        SDL_Rect option_rect = {100, 200 + i * 60, 400, 40};
        if (i < 3) {
            SDL_RenderDrawRect(game->renderer, &option_rect);
        }
        
        // Ici normalement on utiliserait TTF_RenderText, mais pour simplifier:
        printf("%s\n", options[i]);
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
                        // Définir la taille selon la difficulté
                        int size;
                        switch (selected_option) {
                            case 0: size = 15; break; // Facile
                            case 1: size = 20; break; // Moyen
                            case 2: size = 25; break; // Difficile
                            default: size = 20; break;
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
        SDL_Delay(16); // ~60 FPS
    }
}

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

int main(int argc, char *argv[]) {
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
    
    // Afficher le menu au démarrage
    show_menu(&game);
    
    Uint32 last_update = SDL_GetTicks();
    
    while (game.running) {
        Uint32 current_time = SDL_GetTicks();
        
        handle_events(&game);
        
        // CHANGEMENT: 100ms / 150ms (plus lent)
        if (!game.in_menu && current_time - last_update > 150) {
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