#include <math.h>
#include "raylib.h"
#define WAVE_COUNT 3
#define RAINDROP_COUNT 160
#define RIPPLE_COUNT 200


typedef struct{
    float x, y;
    float speed;
    float length;
} raindrop_t;
typedef struct{
    bool active;
    float x;
    float age, life;
    float strength, spread_speed;
} ripple_t;
static int screen_width, screen_height;
static ripple_t ripples[RIPPLE_COUNT];


float clamp(float value, float min_value, float max_value){
    if(value <= min_value) return min_value;
    if(value >= max_value) return max_value;

    return value;
}


int get_water_y(int x, float time){
    static const struct{
        int count;
        float amplitude;
    } waves[WAVE_COUNT] = {
        {1, 7.5f},
        {2, 3.0f},
        {3, 1.3f}
    };

    float y = screen_height * .6f;
    for(int i=0; i<WAVE_COUNT; ++i){
        y += sinf(x*1.0f/screen_width*PI*2.0f*waves[i].count+time) * waves[i].amplitude;
    }

    for(int i=0; i<RIPPLE_COUNT; ++i){
        if(ripples[i].active){
            float distance = fabsf(ripples[i].x - x);
            float xx = ripples[i].age * ripples[i].spread_speed;
            float timeFade = 1.0f - ripples[i].age/ripples[i].life;

            if(timeFade > 0.0f){
                float spaceFade = expf(-distance * 0.01f);
                spaceFade *= expf(-fabsf(distance-xx) * 0.2f);

                y += sinf((distance-xx)*0.2f) * ripples[i].strength * timeFade * spaceFade;
            }
        }
    }

    return y;
}


void reset_raindrop(raindrop_t *raindrop){
    raindrop->x = GetRandomValue(0, screen_width - 1);
    raindrop->y = 0.0f;
    raindrop->speed = GetRandomValue(500, 700);
    raindrop->length = GetRandomValue(10, 16);
}


void add_ripple(int x, float strength){
    int idx = -1;
    for(int i=0; i<RIPPLE_COUNT; ++i){
        if(!ripples[i].active){
            idx = i;

            break;
        }
    }

    if(idx == -1){
        idx = 0;
        for(int i=1; i<RIPPLE_COUNT; ++i){
            if(ripples[i].age/ripples[i].life > ripples[idx].age/ripples[idx].life){
                idx = i;
            }
        }
    }

    ripples[idx].active = true;
    ripples[idx].x = x;
    ripples[idx].age = 0.0f;
    ripples[idx].life = 1.0f / 3;
    ripples[idx].strength = strength;
    ripples[idx].spread_speed = 100.0f;
}


int main(){
    // (640, 480) (800, 480) (1024, 600)
    InitWindow(1024, 600, "water");

    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    raindrop_t raindrops[RAINDROP_COUNT];
    for(int i=0; i<RAINDROP_COUNT; ++i){
        reset_raindrop(raindrops + i);
    }

    float time = 0.0f;
    while(!WindowShouldClose()){
        float delta_time = GetFrameTime();
        time += delta_time;

        for(int i=0; i<RIPPLE_COUNT; ++i){
            if(ripples[i].active){
                ripples[i].age += delta_time;

                if(ripples[i].age >= ripples[i].life){
                    ripples[i].active = false;
                }
            }
        }

        for(int i=0; i<RAINDROP_COUNT; ++i){
            raindrops[i].y += raindrops[i].speed * delta_time;

            if(raindrops[i].y >= get_water_y(raindrops[i].x, time)){
                add_ripple(raindrops[i].x, clamp(raindrops[i].speed / 75, 8, 16));
                reset_raindrop(raindrops + i);
            }
        }

        BeginDrawing();
        ClearBackground((Color) {8, 24, 48, 255});

        for(int i=0; i<RAINDROP_COUNT; ++i){
            DrawLine(raindrops[i].x, raindrops[i].y, raindrops[i].x, raindrops[i].y - raindrops[i].length, (Color) {180, 220, 255, 170});
        }

        int previousY = get_water_y(0, time);
        for(int x=1; x<screen_width; ++x){
            int y = get_water_y(x, time);

            DrawLine(x - 1, previousY, x, y, (Color) {64, 196, 255, 255});
            DrawLine(x, y, x, screen_height, (Color) {18, 86, 140, 255});

            previousY = y;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}