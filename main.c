/*
TODO
- left click to add line in original space
- right-drag to adjust transformed space
- render current transform in font
*/

#include <stdio.h>
#include <SDL.h>

#include "types.h"

#define MAX_VECTORS 32

int window_width = 1440;
int window_height = 900;
float grid_size = 20;
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

Vector get_vector_from_screen_coords(int x, int y)
{
    Vector result;

    // TODO compress
    int window_half_width = window_width / 2;
    int window_half_height = window_height / 2;

    result.x = (x - window_half_width) / (float)pixels_per_unit;
    result.y = -(y - window_half_height) / (float)pixels_per_unit;
    return result;
}

Vector pixel_vector_to_world(int screen_x, int screen_y)
{
    Vector result;
    result.x = screen_x / (float)pixels_per_unit;
    result.y = -screen_y / (float)pixels_per_unit;
    return result;
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
    // TODO compress

    for (int y = -grid_size; y < grid_size; ++y)
    {
        if (y == 0)
        {
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, r, g, b, 128);
        }

        Vector gridline_start = {-grid_size, y};
        Vector gridline_start_xformed = linear_xform(gridline_start, x_basis, y_basis);

        Vector gridline_end = {grid_size, y};
        Vector gridline_end_xformed = linear_xform(gridline_end, x_basis, y_basis);

        draw_world_line(
            renderer,
            gridline_start_xformed.x,
            gridline_start_xformed.y,
            gridline_end_xformed.x,
            gridline_end_xformed.y
        );
    }

    for (int x = -grid_size; x < grid_size; ++x)
    {
        if (x == 0)
        {
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, r, g, b, 128);
        }

        Vector gridline_start = {x, -grid_size};
        Vector gridline_start_xformed = linear_xform(gridline_start, x_basis, y_basis);

        Vector gridline_end = {x, grid_size};
        Vector gridline_end_xformed = linear_xform(gridline_end, x_basis, y_basis);

        draw_world_line(
            renderer,
            gridline_start_xformed.x,
            gridline_start_xformed.y,
            gridline_end_xformed.x,
            gridline_end_xformed.y
        );
    }
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

    int zoomed_in_this_frame = 0;
    int zoomed_out_this_frame = 0;

    int add_vector_this_frame = 0;
    Vector new_vector_screen_coords = {};

    int is_dragging_x_basis = 0;
    int is_dragging_y_basis = 0;
    Vector mouse_motion_this_frame = {};
    Vector mouse_position = {};

    Vector vectors[MAX_VECTORS] = {};
    int vector_count = 0;

    Vector original_x_basis = {1, 0};
    Vector original_y_basis = {0, 1};

    Vector new_x_basis = original_x_basis;
    Vector new_y_basis = original_y_basis;

    while (running)
    {
        //////////// Collect input ////////////

        add_vector_this_frame = 0;
        mouse_motion_this_frame = (Vector){};
        zoomed_out_this_frame = 0;
        zoomed_in_this_frame = 0;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    if (!event.key.repeat)
                    {
                        if (event.key.keysym.sym == SDLK_RETURN)
                        {
                            add_vector_this_frame = 1;
                        }
                        else if (event.key.keysym.sym == SDLK_x)
                        {
                            is_dragging_x_basis = 1;
                        }
                        else if (event.key.keysym.sym == SDLK_y)
                        {
                            is_dragging_y_basis = 1;
                        }
                    }
                    break;
                case SDL_KEYUP:
                    if (!event.key.repeat)
                    {
                        if (event.key.keysym.sym == SDLK_x)
                        {
                            is_dragging_x_basis = 0;
                        }
                        else if (event.key.keysym.sym == SDLK_y)
                        {
                            is_dragging_y_basis = 0;
                        }
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    if(event.wheel.y > 0) // scroll up
                    {
                         zoomed_in_this_frame = 1;
                    }
                    else if(event.wheel.y < 0) // scroll down
                    {
                         zoomed_out_this_frame = 1;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    mouse_motion_this_frame.x = event.motion.xrel;
                    mouse_motion_this_frame.y = event.motion.yrel;
                    mouse_position.x = event.motion.x;
                    mouse_position.y = event.motion.y;
                    break;
                case SDL_QUIT:
                    running = 0;
                    break;
            }
        }

        //////////// Simulate ////////////

        if (zoomed_in_this_frame)
        {
            pixels_per_unit++;
        }
        else if (zoomed_out_this_frame)
        {
            pixels_per_unit--;
        }

        if (is_dragging_x_basis)
        {
            // TODO that's why i needed to record where the mouse starts, so the movement can be "scaled" correctly.
            // also, yeah, one mouse movement can only define have a transformation, or one of the bases.
            Vector mouse_drag_in_world_units = pixel_vector_to_world(mouse_motion_this_frame.x, mouse_motion_this_frame.y);
            new_x_basis.x += mouse_drag_in_world_units.x;
            new_x_basis.y += mouse_drag_in_world_units.y;
            printf("X basis = [ %f, %f ]\n", new_x_basis.x, new_x_basis.y);
        }

        if (is_dragging_y_basis)
        {
            Vector mouse_drag_in_world_units = pixel_vector_to_world(mouse_motion_this_frame.x, mouse_motion_this_frame.y);
            new_y_basis.x += mouse_drag_in_world_units.x;
            new_y_basis.y += mouse_drag_in_world_units.y;
            printf("Y basis = [ %f, %f ]\n\n", new_y_basis.x, new_y_basis.y);
        }

        if (add_vector_this_frame)
        {
            vectors[vector_count] = get_vector_from_screen_coords(mouse_position.x, mouse_position.y);
            vector_count++;
            vector_count %= MAX_VECTORS + 1;
        }

        /////////////// Render ///////////////

        // Clear to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer, original_x_basis, original_y_basis, 255, 255, 255);

        draw_grid(renderer, new_x_basis, new_y_basis, 0, 255, 255);

        for (int i = 0; i < vector_count; ++i)
        {
            Vector original_vector = vectors[i];
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            draw_world_line(renderer, 0, 0, original_vector.x, original_vector.y);

            Vector xformed_vector = linear_xform(original_vector, new_x_basis, new_y_basis);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            draw_world_line(renderer, 0, 0, xformed_vector.x, xformed_vector.y);
        }

        // Blit
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
