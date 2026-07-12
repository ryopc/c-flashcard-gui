#include "raylib.h"
#include "flashcard.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CARD_WIDTH 600
#define CARD_HEIGHT 300
#define BUTTON_WIDTH 180
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 20

// Color palette (Catppuccin Mocha)
const Color COLOR_BG = {30, 30, 46, 255};
const Color COLOR_SURFACE = {49, 50, 68, 255};
const Color COLOR_TEXT = {205, 214, 244, 255};
const Color COLOR_PRIMARY = {137, 180, 250, 255};
const Color COLOR_SUCCESS = {166, 227, 161, 255};
const Color COLOR_DANGER = {243, 139, 168, 255};

typedef enum {
    STATE_STUDY_QUESTION,
    STATE_STUDY_ANSWER,
    STATE_RESULTS
} UIState;

typedef struct {
    Rectangle rect;
    const char *label;
    Color color;
    Color hover_color;
    int is_hovered;
    int is_pressed;
} Button;

Button button_new(float x, float y, float w, float h, const char *label, Color color) {
    Button btn = {
        .rect = {x, y, w, h},
        .label = label,
        .color = color,
        .hover_color = {color.r + 30, color.g + 30, color.b + 30, color.a},
        .is_hovered = 0,
        .is_pressed = 0
    };
    return btn;
}

void button_update(Button *btn) {
    Vector2 mouse = GetMousePosition();
    btn->is_hovered = CheckCollisionPointRec(mouse, btn->rect);
    btn->is_pressed = btn->is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void button_draw(Button *btn) {
    Color draw_color = btn->is_hovered ? btn->hover_color : btn->color;
    DrawRectangleRec(btn->rect, draw_color);
    DrawRectangleLinesEx(btn->rect, 2, COLOR_TEXT);
    
    int text_width = MeasureText(btn->label, 20);
    float text_x = btn->rect.x + (btn->rect.width - text_width) / 2;
    float text_y = btn->rect.y + (btn->rect.height - 20) / 2;
    DrawText(btn->label, (int)text_x, (int)text_y, 20, COLOR_BG);
}

int main(void) {
    srand((unsigned)time(NULL));
    
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flashcard - Raylib Edition");
    SetTargetFPS(60);
    
    // Load or create deck
    FlashcardDeck *deck = flashcard_deck_new();
    if (!deck || flashcard_load_csv(deck, "data/cards.csv") != 0) {
        // Create default deck if file not found
        if (!deck) {
            deck = flashcard_deck_new();
        } else {
            flashcard_deck_free(deck);
            deck = flashcard_deck_new();
        }
        
        // Add default cards
        deck->cards = realloc(deck->cards, sizeof(Flashcard) * 3);
        
        deck->cards[0].question = malloc(50);
        strcpy(deck->cards[0].question, "ls -la");
        deck->cards[0].answer = malloc(100);
        strcpy(deck->cards[0].answer, "List files with hidden files and details");
        
        deck->cards[1].question = malloc(50);
        strcpy(deck->cards[1].question, "ps aux");
        deck->cards[1].answer = malloc(100);
        strcpy(deck->cards[1].answer, "Display running processes in detail");
        
        deck->cards[2].question = malloc(50);
        strcpy(deck->cards[2].question, "df -h");
        deck->cards[2].answer = malloc(100);
        strcpy(deck->cards[2].answer, "Show disk usage in human-readable format");
        
        deck->count = 3;
    }
    
    flashcard_shuffle(deck);
    UIState state = STATE_STUDY_QUESTION;
    
    // Create buttons
    Button btn_show_answer = button_new(
        (WINDOW_WIDTH - BUTTON_WIDTH) / 2, 450,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Show Answer", COLOR_PRIMARY
    );
    
    Button btn_incorrect = button_new(
        WINDOW_WIDTH / 2 - BUTTON_WIDTH - BUTTON_SPACING / 2, 450,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Incorrect", COLOR_DANGER
    );
    
    Button btn_correct = button_new(
        WINDOW_WIDTH / 2 + BUTTON_SPACING / 2, 450,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Correct", COLOR_SUCCESS
    );
    
    Button btn_retry = button_new(
        (WINDOW_WIDTH - BUTTON_WIDTH) / 2, 450,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Try Again", COLOR_PRIMARY
    );
    
    // Main loop
    while (!WindowShouldClose()) {
        // Update
        button_update(&btn_show_answer);
        button_update(&btn_incorrect);
        button_update(&btn_correct);
        button_update(&btn_retry);
        
        if (state == STATE_STUDY_QUESTION && btn_show_answer.is_pressed) {
            state = STATE_STUDY_ANSWER;
        } else if (state == STATE_STUDY_ANSWER) {
            if (btn_correct.is_pressed) {
                flashcard_mark_correct(deck);
                flashcard_next(deck);
                
                if (deck->finished) {
                    state = STATE_RESULTS;
                } else {
                    state = STATE_STUDY_QUESTION;
                }
            } else if (btn_incorrect.is_pressed) {
                flashcard_next(deck);
                
                if (deck->finished) {
                    state = STATE_RESULTS;
                } else {
                    state = STATE_STUDY_QUESTION;
                }
            }
        } else if (state == STATE_RESULTS && btn_retry.is_pressed) {
            flashcard_shuffle(deck);
            flashcard_reset(deck);
            state = STATE_STUDY_QUESTION;
        }
        
        // Draw
        BeginDrawing();
        ClearBackground(COLOR_BG);
        
        // Progress text
        char progress_text[64];
        if (state == STATE_RESULTS) {
            snprintf(progress_text, sizeof(progress_text), "Results");
        } else {
            snprintf(progress_text, sizeof(progress_text), "Card %d / %d",
                     deck->current_index + 1, deck->count);
        }
        DrawText(progress_text, 20, 20, 24, COLOR_TEXT);
        
        // Card box
        Rectangle card_rect = {
            (WINDOW_WIDTH - CARD_WIDTH) / 2,
            60,
            CARD_WIDTH,
            CARD_HEIGHT
        };
        DrawRectangleRec(card_rect, COLOR_SURFACE);
        DrawRectangleLinesEx(card_rect, 2, COLOR_PRIMARY);
        
        // Card content
        if (state == STATE_STUDY_QUESTION) {
            Flashcard *card = flashcard_get_current(deck);
            if (card) {
                DrawText("Question:", (int)card_rect.x + 20, (int)card_rect.y + 20, 18, COLOR_TEXT);
                DrawText(card->question, (int)card_rect.x + 20, (int)card_rect.y + 60,
                         24, COLOR_PRIMARY);
            }
            button_draw(&btn_show_answer);
        } else if (state == STATE_STUDY_ANSWER) {
            Flashcard *card = flashcard_get_current(deck);
            if (card) {
                DrawText("Answer:", (int)card_rect.x + 20, (int)card_rect.y + 20, 18, COLOR_TEXT);
                DrawText(card->answer, (int)card_rect.x + 20, (int)card_rect.y + 60,
                         24, COLOR_SUCCESS);
            }
            button_draw(&btn_incorrect);
            button_draw(&btn_correct);
        } else if (state == STATE_RESULTS) {
            char result_text[128];
            snprintf(result_text, sizeof(result_text), "Score: %d / %d",
                     deck->correct_count, deck->count);
            int text_width = MeasureText(result_text, 32);
            DrawText(result_text, (WINDOW_WIDTH - text_width) / 2,
                     (WINDOW_HEIGHT - 100) / 2, 32, COLOR_SUCCESS);
            button_draw(&btn_retry);
        }
        
        EndDrawing();
    }
    
    // Cleanup
    flashcard_deck_free(deck);
    CloseWindow();
    
    return 0;
}
