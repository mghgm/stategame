#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "map.h"
#include "soldier.h"
#include "potion.h"
#include "ai.h"
#include "events.h"

const int FPS = 60;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;



int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "%s", SDL_GetError());
        return 0;
    }

    SDL_Window *Game_Window = SDL_CreateWindow("state.io", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer *Renderer = SDL_CreateRenderer(Game_Window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    // set seed
    srand(time(0));
    
    int lands_n = 17;
    Land lands[MAX_LANDS];
    create_rand_map(lands_n, lands, 1);
    //load_rand_map("./data/maps/map3.txt", &lands_n, lands);
    Land* selected_land = NULL;

    Soldier *soldiers = malloc(MAX_SOLDIERS_STEP * sizeof(Soldier));
    int soldiers_n = 0;
    int max_soldiers = MAX_SOLDIERS_STEP;

    Potion potions[MAX_POTIONS];
    int potions_n = 0;
    
    SDL_bool shallExit = SDL_FALSE;
    
    while (shallExit == SDL_FALSE) {
        // renderer color and clear
        SDL_SetRenderDrawColor(Renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(Renderer);
        
        apply_rand_map(Renderer, lands_n, lands, selected_land);
        
        apply_soldiers(Renderer, soldiers_n, soldiers);

        add_potion(&potions_n, potions, lands_n, lands);
        apply_potions(Renderer, potions_n, potions, lands_n, lands, soldiers_n, soldiers);
        remove_expired_potions(&potions_n, potions);
        
        collision_detection(soldiers_n, soldiers, lands_n, lands, potions_n, potions);
        
        remove_zero_power_soldiers(&soldiers_n, soldiers);        
        
        check_bot_attack(lands_n, lands, &soldiers_n, &max_soldiers, &soldiers);
        
        // listen for key events
        event_listener(&shallExit, lands_n, lands, &selected_land, &soldiers_n,
                       &max_soldiers, &soldiers);

        // update window
        SDL_RenderPresent(Renderer);
        // set delay as FPS
        SDL_Delay(1000 / FPS);
    }
    // free allocated memory
    free(soldiers);
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Game_Window);
    return 0;
}
