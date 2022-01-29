#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "map.h"


void rand_BFS(int iterator, int map[250][250], int to_check[500][2], int n);
void save_rand_land(FILE *map_file_ptr, int map[250][250], int x, int y, int has_barrack,  int barrack_r, int side, int soldiers);

char* get_new_file_name(); 

Uint32 get_side_normal_color(int side);
void draw_attack_line(SDL_Renderer* Renderer, Land *selected_land_ptr);




// 0 -> successed, 1 -> get error
int load_map(char file_path[100], int *lands_n, Land lands[20]) {
    FILE *file_ptr = fopen(file_path, "r");
    if (file_ptr == NULL) { // check for error
        fprintf(stderr, "%s does not exists.\n", file_path);
        return 1;
    }
    fscanf(file_ptr, "%d", lands_n); // read number

    for (int i = 0; i < *lands_n; i++) { // read lands data
        fscanf(file_ptr, "%s %d %d %d %d %lf %d %d %hd %d", lands[i].path, &lands[i].x, &lands[i].y,
                                                    &lands[i].width, &lands[i].height, &lands[i].angle,
                                                    &lands[i].has_barrack, &lands[i].side,
                                                    &lands[i].barrack_r, &lands[i].soldiers);
        // set barrack cordinates
        lands[i].barrack_x = lands[i].x + lands[i].width/2;
        lands[i].barrack_y = lands[i].y + lands[i].height/2;

        lands[i].selected = 0;
    }
    fclose(file_ptr);
    return 0;
}

void apply_map(SDL_Renderer* Renderer, int lands_n, Land lands[20], Land* selected_land_ptr) {
    for (int i = 0; i < lands_n; i++) {
        // make surface and check
        SDL_Surface* image = SDL_LoadBMP(lands[i].path);
        if (image == NULL) fprintf(stderr, "%s\n", SDL_GetError());
        // make texture and check
        SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer, image);
        if (texture == NULL) fprintf(stderr, "%s\n", SDL_GetError());

        SDL_Rect source = {.x = lands[i].x, .y = lands[i].y, .w = lands[i].width, .h = lands[i].height};
        // render
        SDL_RenderCopyEx(Renderer, texture, NULL, &source, lands[i].angle, NULL, SDL_FLIP_NONE);

        if (lands[i].has_barrack) {
            Uint32 color = lands[i].selected && lands[i].side == 1 ? 0xffede2b9: get_side_normal_color(lands[i].side);
            filledCircleColor(Renderer, lands[i].barrack_x, lands[i].barrack_y, lands[i].barrack_r, color);
            char number[5];
            sprintf(number, "%d", lands[i].soldiers);
            stringRGBA(Renderer, lands[i].barrack_x - 8, lands[i].barrack_y - 5, number, 0, 0, 0, 255);
        }

        // draw attack line
        draw_attack_line(Renderer, selected_land_ptr);

        // free allocated memory
        SDL_FreeSurface(image);
        SDL_DestroyTexture(texture);

    }
}

void create_rand_map(int lands_n, Land lands[], int players) {
    int share = lands_n/(players + 2);
    
    // coordinates of lands
    int x = 200;
    int y = 200;
    
    char *file_name = get_new_file_name();

    // open file to save the rand map
    FILE* map_file_ptr = fopen(file_name, "w");
    
    // free file name ptr
    free(file_name);
    
    // add number of lands to file
    fprintf(map_file_ptr, "%d\n", lands_n);
    
    int lands_i = 0;
    for (int i = 1; i < players + 1; i++) { // i refers to side
        for (int j = 0; j < share; j++) {
            int map[250][250] = {0};
            int to_check[500][2] = {{125, 125}};

            rand_BFS(0, map, to_check, 1);

            memcpy(lands[lands_i].pixels, map, 250 * 250 * sizeof(int));
            
            lands[lands_i].has_barrack = 1;
            lands[lands_i].side = i;
            
            lands[lands_i].barrack_x = x + (rand()%2 ? -1: 1) * rand()%50;
            lands[lands_i].barrack_y = y + (rand()%2 ? -1: 1) * rand()%50;
            lands[lands_i].barrack_r = 25;
            
            lands[lands_i].soldiers = 12; // TODO change later
            lands[lands_i].max_soldiers = 50;
            lands[lands_i].rebirth_rate = 30;
            lands[lands_i].rebirth_timer = 30;
           
             lands[lands_i].selected = 0;
            

            // rand x-y for land
            if (y - x > 400) x += 200;
            else if (x - y > 400) y += 200;
            else if (rand()%2) x += 200;
            else y += 200;

            // save land
            save_rand_land(map_file_ptr, map, lands[lands_i].barrack_x, lands[lands_i].barrack_y, lands[lands_i].has_barrack,
                           lands[lands_i].barrack_r, lands[lands_i].side, lands[lands_i].soldiers);
            lands_i++;
        }
    }

    // lands with no side
    int impartials = lands_n - lands_i;
    for (int i = 0; i < impartials; i++) {
        int map[250][250] = {0};
        int to_check[500][2] = {{125, 125}};       

        rand_BFS(0, map, to_check, 1);

        memcpy(lands[lands_i].pixels, map, 250 * 250 * sizeof(int));
        
        lands[lands_i].has_barrack = 1;
        lands[lands_i].side = i;
        
        lands[lands_i].barrack_x = x + (rand()%2 ? -1: 1) * rand()%50;
        lands[lands_i].barrack_y = y + (rand()%2 ? -1: 1) * rand()%50;
        lands[lands_i].barrack_r = 25;
        
        lands[lands_i].soldiers = 12; // TODO change later
        lands[lands_i].max_soldiers = 20;
        lands[lands_i].rebirth_rate = 60;
        lands[lands_i].rebirth_timer = 60;

        lands[lands_i].selected = 0;

        // rand x-y for land
        if (y - x > 400) x += 200;
        else if (x - y > 400) y += 200;
        else if (rand()%2) x += 200;
        else y += 200;
        
        // save land
        save_rand_land(map_file_ptr, map, lands[lands_i].barrack_x, lands[lands_i].barrack_y, lands[lands_i].has_barrack,
                       lands[lands_i].barrack_r, lands[lands_i].side, lands[lands_i].soldiers);

        lands_i++;
    }
}

void rand_BFS(int iterator, int map[250][250], int to_check[500][2], int n) {
    if (iterator == 50) return;
    int ss[500][2];
    int l = 0;
    for (int i = 0; i < n; i++) {
        if (to_check[i][0] + 1  < 250 && map[to_check[i][0] + 1][to_check[i][1]] == 0 && (rand()%2 || iterator < rand()%10 + 20)) {
            map[to_check[i][0] + 1][to_check[i][1]] = 1;
            ss[l][0] = to_check[i][0] + 1;
            ss[l][1] = to_check[i][1];
            l++;
        }
        if (to_check[i][0] - 1 >= 0 && map[to_check[i][0] - 1][to_check[i][1]] == 0 && (rand()%2 || iterator < rand()%10 + 20)) {
            map[to_check[i][0] - 1][to_check[i][1]] = 1;
            ss[l][0] = to_check[i][0] - 1;
            ss[l][1] = to_check[i][1];
            l++;
        }
        if (to_check[i][1] + 1 < 250 && map[to_check[i][0]][to_check[i][1] + 1] == 0 && (rand()%2 || iterator < rand()%10 + 20)) {
            map[to_check[i][0]][to_check[i][1] + 1] = 1;
            ss[l][0] = to_check[i][0];
            ss[l][1] = to_check[i][1] + 1;
            l++;
        }
        if (to_check[i][1] - 1 >= 0 && map[to_check[i][0]][to_check[i][1] - 1] == 0 && (rand()%2 || iterator < rand()%10 + 20)) {
            map[to_check[i][0]][to_check[i][1] - 1] = 1;
            ss[l][0] = to_check[i][0];
            ss[l][1] = to_check[i][1] - 1;
            l++;
        }
    }
    rand_BFS(iterator + 1, map, ss, l);
}

void save_rand_land(FILE *map_file_ptr, int map[250][250], int x, int y, int has_barrack,  int barrack_r, int side, int soldiers) {
    fprintf(map_file_ptr, "%d %d %d %d %d %d\n", x, y, has_barrack, barrack_r, side, soldiers);
    for (int i = 0; i < 250; i++) {
        for (int j = 0; j < 250; j++) {
            fprintf(map_file_ptr, "%d ", map[i][j]);
        }
        fprintf(map_file_ptr, "\n");
    }
}

// 0 -> successed, 1 -> get error
int load_rand_map(char file_path[100], int *lands_n, Land lands[]) {
    FILE *file_ptr = fopen(file_path, "r");
    if (file_ptr == NULL) { // check for error
        fprintf(stderr, "%s does not exists.\n", file_path);
        return 1;
    }
    fscanf(file_ptr, "%d", lands_n); // read number



    for (int i = 0; i < *lands_n; i++) { // read lands data
        fscanf(file_ptr, "%hd %hd %d %hd %d %d", &lands[i].barrack_x, &lands[i].barrack_y,
                                                 &lands[i].has_barrack, &lands[i].barrack_r,
                                                 &lands[i].side, &lands[i].soldiers);
        lands[i].selected = 0;

        for (int m = 0; m < 250; m++) {
            for (int n = 0; n < 250; n++) {
                fscanf(file_ptr, "%d", &lands[i].pixels[m][n]);
            }
        }
    }
    fclose(file_ptr);
    return 0;
}

void apply_rand_map(SDL_Renderer* Renderer, int lands_n, Land lands[], Land* selected_land_ptr) {
    for (int i = 0; i < lands_n; i++) {
        // draw land with 5x5 pixels
        for (int m = 0; m < 250; m++) {
            for (int n = 0; n < 250; n++) {
                if (lands[i].pixels[m][n] == 1) {
                    boxColor(Renderer, lands[i].barrack_x - (125 - m) * 2, lands[i].barrack_y - (125 - n) * 2,
                    lands[i].barrack_x - (125 - m) * 2 + 3, lands[i].barrack_y - (125 - n) * 2 + 3, 0xffcfcdcc);
                }
            }
        }

        if (lands[i].has_barrack) { 
            // draw barrack
            Uint32 color = lands[i].selected && lands[i].side == 1 ? 0xffede2b9: get_side_normal_color(lands[i].side);
            filledCircleColor(Renderer, lands[i].barrack_x, lands[i].barrack_y, lands[i].barrack_r, color);
            
            // add number
            char number[5];
            sprintf(number, "%d", lands[i].soldiers);
            stringRGBA(Renderer, lands[i].barrack_x - 8, lands[i].barrack_y - 5, number, 0, 0, 0, 255);
            
            // rebirth
            lands[i].rebirth_timer -= 1;
            if (lands[i].rebirth_timer == 0) {
                lands[i].rebirth_timer = lands[i].rebirth_rate;
                if (lands[i].soldiers < lands[i].max_soldiers) {
                    lands[i].soldiers += 1;
                }
            }
        }

        // draw attack line
        draw_attack_line(Renderer, selected_land_ptr);
    }
}

char* get_new_file_name() {
    char* file_name = malloc(50);
    *file_name = '\0';
    
    DIR *maps_dir;
    struct dirent *dir;
    maps_dir = opendir("./data/maps");
    
    int file_number = 0;
    if (maps_dir != NULL) {
        while((dir = readdir(maps_dir)) != NULL) {
            file_number++;
        }
        file_number--; // for . and ..
    }
    else fprintf(stderr, "can not find ./data/maps\n");
    
    char temp[50];
    sprintf(temp, "./data/maps/map%d.txt", file_number);

    strcat(file_name, temp);
    
    return file_name;
}


void draw_attack_line(SDL_Renderer* Renderer, Land *selected_land_ptr) {
    if (selected_land_ptr != NULL) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_RenderDrawLine(Renderer, selected_land_ptr->barrack_x, selected_land_ptr->barrack_y, x, y);
    }
}

Uint32 get_side_normal_color(int side) {
    switch (side) {
        case 0: return 0xffcfcdcc;
        case 1: return 0xfffa863e;
        case 2: return 0xff494beb;
        case 3: return 0xff79ed8a;
        case 4: return 0xff66e3e3;
        case 5: return 0xffe366db;
        case 6: return 0xff9866e3;
        case 7: return 0xff739feb;
        default: return 0xffcfcdcc;
    }
}
