#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

#define DEFAULT_SIZE 20
#define WINDOW_SIZE 600

// Prototypes
typedef struct Point Point;
typedef struct Node Node;
typedef struct Snake Snake;
typedef struct GameMap GameMap;
typedef struct Game Game;

void init_game(Game *game, int size);
void free_game(Game *game);
void handle_events(Game *game);
void update_game(Game *game);
void render_game(Game *game);
void change_direction(Snake *snake, int new_direction);
void move_snake(Game *game);
void spawn_fruit(GameMap *map, int size);
bool check_collision(Game *game);

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
};

struct Game {
    Snake snake;
    GameMap map;
    bool running;
    int score;
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
    snake.direction = 3;
    snake.length = 0;
    
    add_segment(&snake, start_x, start_y);
    add_segment(&snake, start_x - 1, start_y);
    add_segment(&snake, start_x - 2, start_y);
    
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
    
    for (int i = 0; i < size; i++) {
        map.grid[i] = malloc(size * sizeof(char));
        for (int j = 0; j < size; j++) {
            map.grid[i][j] = ' ';
        }
    }
    
    // Ajouter des murs seulement sur les bords EXTÉRIEURS
    for (int i = 0; i < size; i++) {
        map.grid[0][i] = 'W';        // Mur du haut
        map.grid[size-1][i] = 'W';   // Mur du bas
        map.grid[i][0] = 'W';        // Mur de gauche
        map.grid[i][size-1] = 'W';   // Mur de droite
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

// GameLogic
void init_game(Game *game, int size) {
    game->map = create_map(size);
    // Commencer à une position sûre (pas sur un mur)
    int start_x = size / 2;
    int start_y = size / 2;
    game->snake = create_snake(start_x, start_y);
    game->running = true;
    game->score = 0;
    
    spawn_fruit(&game->map, size);
    
    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        exit(1);
    }
    
    game->window = SDL_CreateWindow(
        "Snake Game",
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
    // Empêcher le demi-tour
    if ((snake->direction == 0 && new_direction != 2) ||
        (snake->direction == 2 && new_direction != 0) ||
        (snake->direction == 1 && new_direction != 3) ||
        (snake->direction == 3 && new_direction != 1)) {
        snake->direction = new_direction;
    }
}

void spawn_fruit(GameMap *map, int size) {
    int x, y;
    bool valid_position = false;
    
    while (!valid_position) {
        x = rand() % (size - 2) + 1;  // Éviter les bords
        y = rand() % (size - 2) + 1;  // Éviter les bords
        
        if (get_cell(map, x, y) == ' ') {
            valid_position = true;
            map->fruit.x = x;
            map->fruit.y = y;
            set_cell(map, x, y, 'F');
        }
    }
}

void move_snake(Game *game) {
    Point new_head = game->snake.head->position;
    
    switch (game->snake.direction) {
        case 0: new_head.y--; break; // Haut
        case 1: new_head.x++; break; // Droite
        case 2: new_head.y++; break; // Bas
        case 3: new_head.x--; break; // Gauche
    }
    
    // Gestion du téléportation (serpent qui traverse les bords)
    // Mais seulement si ce n'est pas un mur !
    if (new_head.x < 0) new_head.x = game->map.size - 1;
    if (new_head.x >= game->map.size) new_head.x = 0;
    if (new_head.y < 0) new_head.y = game->map.size - 1;
    if (new_head.y >= game->map.size) new_head.y = 0;
    
    add_segment(&game->snake, new_head.x, new_head.y);
    
    char cell = get_cell(&game->map, new_head.x, new_head.y);
    if (cell == 'F') {
        game->score += 10;
        spawn_fruit(&game->map, game->map.size);
    } else {
        remove_tail(&game->snake);
    }
}

bool check_collision(Game *game) {
    Point head = game->snake.head->position;
    
    // Vérifier collision avec les murs
    if (get_cell(&game->map, head.x, head.y) == 'W') {
        return true;
    }
    
    // Vérifier collision avec soi-même
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
            }
        }
    }
}

void render_game(Game *game) {
    int cell_size = WINDOW_SIZE / game->map.size;
    
    // Fond
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    // Murs
    SDL_SetRenderDrawColor(game->renderer, 100, 100, 100, 255);
    for (int y = 0; y < game->map.size; y++) {
        for (int x = 0; x < game->map.size; x++) {
            if (get_cell(&game->map, x, y) == 'W') {
                SDL_Rect wall = {x * cell_size, y * cell_size, cell_size, cell_size};
                SDL_RenderFillRect(game->renderer, &wall);
            }
        }
    }
    
    // Fruit
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_Rect fruit_rect = {
        game->map.fruit.x * cell_size,
        game->map.fruit.y * cell_size,
        cell_size, cell_size
    };
    SDL_RenderFillRect(game->renderer, &fruit_rect);
    
    // Serpent
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
    int grid_size = DEFAULT_SIZE;
    
    if (argc > 1) {
        grid_size = atoi(argv[1]);
        if (grid_size < 10) grid_size = 10;
        if (grid_size > 50) grid_size = 50;
    }
    
    srand(time(NULL));
    
    Game game;
    init_game(&game, grid_size);
    
    Uint32 last_update = SDL_GetTicks();
    
    while (game.running) {
        Uint32 current_time = SDL_GetTicks();
        
        handle_events(&game);
        
        if (current_time - last_update > 100) {
            update_game(&game);
            last_update = current_time;
        }
        
        render_game(&game);
        SDL_Delay(10);
    }
    
    printf("Game Over! Score final: %d\n", game.score);
    free_game(&game);
    
    return 0;
}