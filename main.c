/*
TODO
- display current transform in console
- left click to add line in original space
- right-drag to adjust transformed space
- render current transform in font
*/

#include <stdio.h>
#include <SDL.h>

#include "types.h"

int window_width = 1440;
int window_height = 900;
float plot_size = 20;
int pixels_per_unit = 20;

void draw_world_line(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
{
    int window_half_width = window_width / 2;
    int window_half_height = window_height / 2;

    int x1_pixels = x1 * pixels_per_unit + window_half_width;
    int y1_pixels = -y1 * pixels_per_unit + window_half_height;

    int x2_pixels = x2 * pixels_per_unit + window_half_width;
    int y2_pixels = -y2 * pixels_per_unit + window_half_height;

    SDL_RenderDrawLine(renderer, x1_pixels, y1_pixels, x2_pixels, y2_pixels);
}

Vector linear_xform(Vector input, Vector new_x_basis, Vector new_y_basis)
{
    Vector result;
    result.x = new_x_basis.x * input.x + new_y_basis.x * input.y;
    result.y = new_x_basis.y * input.x + new_y_basis.y * input.y;
    return result;
}

void draw_grid(SDL_Renderer *renderer, Vector x_basis, Vector y_basis, Uint8 r, Uint8 g, Uint8 b)
{
    // printf("===========================================\n");
    // printf("drawing grid\n\n");

    // printf("looping Y.........\n");
    // printf("------------------------------------\n");
    for (int y = -plot_size; y < plot_size; ++y)
    {
        if (y == 0)
        {
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, r, g, b, 128);
        }

        Vector gridline_start = {-plot_size, y};
        Vector gridline_start_xformed = linear_xform(gridline_start, x_basis, y_basis);

        Vector gridline_end = {plot_size, y};
        Vector gridline_end_xformed = linear_xform(gridline_end, x_basis, y_basis);

        draw_world_line(
            renderer,
            gridline_start_xformed.x,
            gridline_start_xformed.y,
            gridline_end_xformed.x,
            gridline_end_xformed.y
        );
    }

    // printf("------------------------------------\n");
    // printf("looping X.........\n");
    // printf("------------------------------------\n");

    for (int x = -plot_size; x < plot_size; ++x)
    {
        if (x == 0)
        {
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, r, g, b, 128);
        }

        Vector gridline_start = {x, -plot_size};
        Vector gridline_start_xformed = linear_xform(gridline_start, x_basis, y_basis);

        Vector gridline_end = {x, plot_size};
        Vector gridline_end_xformed = linear_xform(gridline_end, x_basis, y_basis);

        draw_world_line(
            renderer,
            gridline_start_xformed.x,
            gridline_start_xformed.y,
            gridline_end_xformed.x,
            gridline_end_xformed.y
        );
    }

    // printf("=================================================\n");
}

int main(int argc, char const *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Event event;

    window = SDL_CreateWindow(
        "A fun game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        0
    );
    if (!window) {
        printf("Couldn't create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int running = 1;

    int pressing_up = 0;
    int pressing_down = 0;
    int pressing_left = 0;
    int pressing_right = 0;

    int pressed_up_this_frame = 0;
    int pressed_down_this_frame = 0;
    int pressed_left_this_frame = 0;
    int pressed_right_this_frame = 0;

    Vector v1 = {5, 2};

    Vector original_x_basis = {1, 0};
    Vector original_y_basis = {0, 1};

    Vector new_x_basis = original_x_basis;
    Vector new_y_basis = original_y_basis;

    while (running)
    {
        pressed_up_this_frame = 0;
        pressed_down_this_frame = 0;
        pressed_left_this_frame = 0;
        pressed_right_this_frame = 0;

        while (SDL_PollEvent(&event))
        {
            char *keysym;
            char *state;

            switch (event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_UP:
                            pressing_up = event.key.state == SDL_PRESSED;
                            pressed_up_this_frame = pressing_up;
                            break;
                        case SDLK_DOWN:
                            pressing_down = event.key.state == SDL_PRESSED;
                            pressed_down_this_frame = pressing_down;
                            break;
                        case SDLK_LEFT:
                            pressing_left = event.key.state == SDL_PRESSED;
                            pressed_left_this_frame = pressing_left;
                            break;
                        case SDLK_RIGHT:
                            pressing_right = event.key.state == SDL_PRESSED;
                            pressed_right_this_frame = pressing_right;
                            break;
                    }
                    break;
            }
        }

        // Simulate

        if (pressed_up_this_frame)
        {
            pixels_per_unit++;
        }
        else if (pressed_down_this_frame)
        {
            pixels_per_unit--;
        }

        new_x_basis.x -= 0.001;
        new_x_basis.y -= 0.001;

        // Render

        // Clear to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer, original_x_basis, original_y_basis, 255, 255, 255);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        draw_world_line(renderer, 0, 0, v1.x, v1.y);

        draw_grid(renderer, new_x_basis, new_y_basis, 0, 255, 255);

        Vector v2 = linear_xform(v1, new_x_basis, new_y_basis);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        draw_world_line(renderer, 0, 0, v2.x, v2.y);

        // Blit
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
