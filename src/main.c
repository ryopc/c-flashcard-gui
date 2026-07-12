#include "raylib.h"
#include "flashcard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 860
#define WINDOW_HEIGHT 680
#define CARD_WIDTH 640
#define CARD_HEIGHT 280
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 52
#define INPUT_BUFFER_SIZE 512
#define MAX_VISIBLE_CARDS 8

// ── Color palette (Catppuccin Mocha enhanced) ──
const Color COLOR_BG        = {30,  30,  46,  255};
const Color COLOR_SURFACE   = {49,  50,  68,  255};
const Color COLOR_OVERLAY   = {69,  71,  90,  255};
const Color COLOR_TEXT      = {205, 214, 244, 255};
const Color COLOR_SUBTEXT   = {147, 153, 178, 255};
const Color COLOR_PRIMARY   = {137, 180, 250, 255};
const Color COLOR_SUCCESS   = {166, 227, 161, 255};
const Color COLOR_DANGER    = {243, 139, 168, 255};
const Color COLOR_WARNING   = {249, 226, 175, 255};
const Color COLOR_ACCENT    = {180, 190, 254, 255};
const Color COLOR_BORDER    = {81,  83,  105, 255};
const Color COLOR_FIELD_BG  = {40,  41,  58,  255};

// ── UI States ──
typedef enum {
    STATE_MENU,
    STATE_STUDY_QUESTION,
    STATE_STUDY_ANSWER,
    STATE_RESULTS,
    STATE_CARD_LIST,
    STATE_EDIT_CARD
} UIState;

// ── Button ──
typedef struct {
    Rectangle rect;
    const char *label;
    const char *jp_label;
    Color color;
    Color hover_color;
    Color text_color;
    int is_hovered;
    int is_pressed;
    int is_disabled;
} Button;

Button button_new(float x, float y, float w, float h,
                  const char *label, const char *jp_label, Color color)
{
    Button btn = {
        .rect = {x, y, w, h},
        .label = label,
        .jp_label = jp_label,
        .color = color,
        .hover_color = {
            (unsigned char)fminf(color.r + 25, 255),
            (unsigned char)fminf(color.g + 25, 255),
            (unsigned char)fminf(color.b + 25, 255),
            color.a
        },
        .text_color = COLOR_BG,
        .is_hovered = 0,
        .is_pressed = 0,
        .is_disabled = 0
    };
    return btn;
}

void button_update(Button *btn) {
    if (btn->is_disabled) return;
    Vector2 mouse = GetMousePosition();
    btn->is_hovered = CheckCollisionPointRec(mouse, btn->rect);
    btn->is_pressed = btn->is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void button_draw(Button *btn, Font font, int use_font) {
    if (btn->is_disabled) {
        DrawRectangleRec(btn->rect, (Color){50, 51, 68, 255});
        DrawRectangleLinesEx(btn->rect, 2, (Color){60, 61, 78, 255});
        return;
    }
    Color draw_color = btn->is_hovered ? btn->hover_color : btn->color;
    DrawRectangleRounded(btn->rect, 0.08f, 8, draw_color);
    // Subtle shadow when hovered
    if (btn->is_hovered) {
        Rectangle shadow = { btn->rect.x + 2, btn->rect.y + 2, btn->rect.width, btn->rect.height };
        DrawRectangleRounded(shadow, 0.08f, 8, (Color){0, 0, 0, 40});
        DrawRectangleRounded(btn->rect, 0.08f, 8, draw_color);
    }

    const char *text = btn->jp_label ? btn->jp_label : btn->label;
    int font_size = 20;
    if (use_font && font.texture.id > 0) {
        Vector2 text_size = MeasureTextEx(font, text, (float)font_size, 1);
        float tx = btn->rect.x + (btn->rect.width  - text_size.x) / 2.0f;
        float ty = btn->rect.y + (btn->rect.height - text_size.y) / 2.0f;
        DrawTextEx(font, text, (Vector2){tx, ty}, (float)font_size, 1, btn->text_color);
    } else {
        int tw = MeasureText(text, font_size);
        int tx = (int)(btn->rect.x + (btn->rect.width  - tw) / 2.0f);
        int ty = (int)(btn->rect.y + (btn->rect.height - font_size) / 2.0f);
        DrawText(text, tx, ty, font_size, btn->text_color);
    }
}

// ── Text Field ──
typedef struct {
    char buffer[INPUT_BUFFER_SIZE];
    int length;
    int is_active;
    int cursor_pos;
    Rectangle rect;
} TextField;

void textfield_init(TextField *tf, float x, float y, float w, float h) {
    tf->buffer[0] = '\0';
    tf->length = 0;
    tf->is_active = 0;
    tf->cursor_pos = 0;
    tf->rect = (Rectangle){x, y, w, h};
}

void textfield_set_text(TextField *tf, const char *text) {
    int len = (int)strlen(text);
    if (len >= INPUT_BUFFER_SIZE) len = INPUT_BUFFER_SIZE - 1;
    memcpy(tf->buffer, text, len);
    tf->buffer[len] = '\0';
    tf->length = len;
    tf->cursor_pos = len;
}

void textfield_update(TextField *tf) {
    // Handle click to activate
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        tf->is_active = CheckCollisionPointRec(mouse, tf->rect);
    }

    if (!tf->is_active) return;

    // Character input
    int key = GetCharPressed();
    while (key > 0) {
        if (tf->length < INPUT_BUFFER_SIZE - 1) {
            // Shift right from cursor position
            for (int i = tf->length; i > tf->cursor_pos; i--) {
                tf->buffer[i] = tf->buffer[i - 1];
            }
            tf->buffer[tf->cursor_pos] = (char)key;
            tf->length++;
            tf->cursor_pos++;
            tf->buffer[tf->length] = '\0';
        }
        key = GetCharPressed();
    }

    // Backspace
    if (IsKeyPressed(KEY_BACKSPACE) && tf->cursor_pos > 0) {
        for (int i = tf->cursor_pos - 1; i < tf->length; i++) {
            tf->buffer[i] = tf->buffer[i + 1];
        }
        tf->length--;
        tf->cursor_pos--;
        tf->buffer[tf->length] = '\0';
    }

    // Delete
    if (IsKeyPressed(KEY_DELETE) && tf->cursor_pos < tf->length) {
        for (int i = tf->cursor_pos; i < tf->length; i++) {
            tf->buffer[i] = tf->buffer[i + 1];
        }
        tf->length--;
        tf->buffer[tf->length] = '\0';
    }

    // Arrow keys
    if (IsKeyPressed(KEY_LEFT) && tf->cursor_pos > 0) tf->cursor_pos--;
    if (IsKeyPressed(KEY_RIGHT) && tf->cursor_pos < tf->length) tf->cursor_pos++;

    // Home / End
    if (IsKeyPressed(KEY_HOME)) tf->cursor_pos = 0;
    if (IsKeyPressed(KEY_END)) tf->cursor_pos = tf->length;
}

void textfield_draw(TextField *tf, Font font, int use_font, const char *label_jp) {
    // Label
    int label_size = 16;
    Color label_col = tf->is_active ? COLOR_PRIMARY : COLOR_SUBTEXT;
    if (use_font && font.texture.id > 0) {
        DrawTextEx(font, label_jp, (Vector2){tf->rect.x, tf->rect.y - label_size - 6}, (float)label_size, 1, label_col);
    } else {
        DrawText(label_jp, (int)tf->rect.x, (int)tf->rect.y - label_size - 6, label_size, label_col);
    }

    // Background
    DrawRectangleRounded(tf->rect, 0.06f, 6, COLOR_FIELD_BG);
    Color border_col = tf->is_active ? COLOR_PRIMARY : COLOR_BORDER;
    DrawRectangleRoundedLinesEx(tf->rect, 0.06f, 6, 2, border_col);

    // Text
    char display[INPUT_BUFFER_SIZE];
    // Replace newlines with spaces for display (or just truncate)
    strncpy(display, tf->buffer, INPUT_BUFFER_SIZE - 1);
    display[INPUT_BUFFER_SIZE - 1] = '\0';
    for (int i = 0; display[i]; i++) {
        if (display[i] == '\n') display[i] = ' ';
    }

    int fs = 18;
    int text_x = (int)tf->rect.x + 12;
    int text_y = (int)tf->rect.y + (int)(tf->rect.height - fs) / 2;

    if (use_font && font.texture.id > 0) {
        DrawTextEx(font, display, (Vector2){(float)text_x, (float)text_y}, (float)fs, 1, COLOR_TEXT);
        // Cursor
        if (tf->is_active && ((int)(GetTime() * 2.0f) % 2 == 0)) {
            char before[INPUT_BUFFER_SIZE];
            strncpy(before, display, tf->cursor_pos);
            before[tf->cursor_pos] = '\0';
            Vector2 cur_size = MeasureTextEx(font, before, (float)fs, 1);
            float cx = tf->rect.x + 12 + cur_size.x;
            DrawRectangle((int)cx, text_y, 2, (int)cur_size.y, COLOR_PRIMARY);
        }
    } else {
        DrawText(display, text_x, text_y, fs, COLOR_TEXT);
        if (tf->is_active && ((int)(GetTime() * 2.0f) % 2 == 0)) {
            int cx = text_x + MeasureText(display, fs);
            DrawRectangle(cx, text_y, 2, fs, COLOR_PRIMARY);
        }
    }
}

// ── Drawing helpers ──
void draw_text(Font font, int use_font, const char *jp_text, const char *en_text,
               float x, float y, int size, Color color)
{
    const char *text = use_font ? jp_text : en_text;
    if (use_font && font.texture.id > 0) {
        DrawTextEx(font, text, (Vector2){x, y}, (float)size, 1, color);
    } else {
        DrawText(text, (int)x, (int)y, size, color);
    }
}

int measure_text(Font font, int use_font, const char *jp_text, const char *en_text, int size) {
    const char *text = use_font ? jp_text : en_text;
    if (use_font && font.texture.id > 0) {
        Vector2 ms = MeasureTextEx(font, text, (float)size, 1);
        return (int)ms.x;
    }
    return MeasureText(text, size);
}

void draw_text_centered(Font font, int use_font, const char *jp_text, const char *en_text,
                         float center_x, float y, int size, Color color)
{
    int w = measure_text(font, use_font, jp_text, en_text, size);
    draw_text(font, use_font, jp_text, en_text, center_x - w / 2.0f, y, size, color);
}

// ── Font Loading ──
int try_load_jp_font(Font *font) {
    const char *paths[] = {
        "data/NotoSansCJKjp-Regular.otf",
        "NotoSansCJKjp-Regular.otf",
        "data/NotoSansJP-Regular.ttf",
        "NotoSansJP-Regular.ttf",
        NULL
    };

    // Also check ~/.local/share/flashcard/ (for user-installed fonts)
    const char *home = getenv("HOME");
    static char user_path[256] = {0};
    if (home) {
        snprintf(user_path, sizeof(user_path), "%s/.local/share/flashcard/NotoSansCJKjp-Regular.otf", home);
        // Append to paths - we'll check it separately
    }

    const char *all_chars =
        " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
        "アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホ"
        "マミムメモヤユヨラリルレロワヲンガギグゲゴザジズゼゾダヂヅデド"
        "バビブベボパピプペポャュョァィゥェォッー"
        "あいうえおかきくけこさしすせそたちつてとなにぬねの"
        "はひふへほまみむめもやゆよらりるれろわをん"
        "がぎぐげござじずぜぞだぢづでどばびぶべぼ"
        "ぱぴぷぺぽゃゅょっゃゅょ"
        "一二三四五六七八九十百千万年日月火水木金土曜日"
        "勉強開始一覧追加質問答え戻保存"
        "編集削除正解不正解結果画面終了確認完了"
        "新規作成編集画面勉強中"
        "、。「」！？　・／〜（）"
        "点間違途中経過終了操作"
        "ありませんが存在";

    for (int i = 0; paths[i]; i++) {
        if (FileExists(paths[i])) {
            int codepoint_count;
            int *codepoints = LoadCodepoints(all_chars, &codepoint_count);
            if (codepoints) {
                *font = LoadFontEx(paths[i], 24, codepoints, codepoint_count);
                UnloadCodepoints(codepoints);
                if (font->texture.id > 0) return 1;
            }
        }
    }

    // Check user data directory
    if (user_path[0] && FileExists(user_path)) {
        int codepoint_count;
        int *codepoints = LoadCodepoints(all_chars, &codepoint_count);
        if (codepoints) {
            *font = LoadFontEx(user_path, 24, codepoints, codepoint_count);
            UnloadCodepoints(codepoints);
            if (font->texture.id > 0) return 1;
        }
    }

    return 0;
}

// ── Main ──
int main(void) {
    srand((unsigned)time(NULL));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flashcard - Raylib Edition");
    SetWindowMinSize(640, 500);
    SetTargetFPS(60);

    // Try to load Japanese font
    Font jpFont = {0};
    int use_jp = try_load_jp_font(&jpFont);
    if (use_jp) {
        SetWindowTitle("単語帳 - Flashcard");
    }

    // ── Deck ──
    FlashcardDeck *deck = flashcard_deck_new();
    if (!deck || flashcard_load_csv(deck, "data/cards.csv") != 0) {
        // Free and retry
        if (deck) flashcard_deck_free(deck);
        deck = flashcard_deck_new();

        // Add default cards (not saved to disk - user can save via editor)
        flashcard_add_card(deck, "ls -la", "隠しファイルを含む詳細なファイル一覧");
        flashcard_add_card(deck, "ps aux", "実行中のプロセスを詳細表示");
        flashcard_add_card(deck, "df -h", "ディスク使用量を人間が読みやすい形式で表示");
        flashcard_add_card(deck, "man", "コマンドのマニュアルページを表示");
        flashcard_add_card(deck, "grep", "ファイル内のテキストパターンを検索");
        flashcard_add_card(deck, "find", "ファイル名や属性でファイルを検索");
        flashcard_add_card(deck, "tar", "アーカイブの作成と圧縮");
        flashcard_add_card(deck, "gzip", "gzipでファイルを圧縮");
        flashcard_add_card(deck, "unzip", "ZIPアーカイブを展開");
    }

    flashcard_shuffle(deck);
    UIState state = STATE_MENU;

    // ── Editor state ──
    TextField edit_question, edit_answer;
    int edit_index = -1; // -1 = new card
    int show_confirm_delete = 0;
    int confirm_delete_index = -1;

    // ── Card list scroll ──
    int card_list_scroll = 0;

    // ── Buttons ──
    // Menu
    Button btn_study  = button_new(0, 0, 280, 64, "Start Study",  "勉強を始める", COLOR_PRIMARY);
    Button btn_list   = button_new(0, 0, 280, 64, "Card List",     "カード一覧",   COLOR_SUCCESS);
    Button btn_add    = button_new(0, 0, 280, 64, "Add Card",      "カードを追加", COLOR_WARNING);

    // Study
    Button btn_show_answer = button_new(0, 0, 240, 52, "Show Answer", "回答を見る", COLOR_PRIMARY);
    Button btn_incorrect   = button_new(0, 0, 180, 52, "Incorrect",   "不正解",     COLOR_DANGER);
    Button btn_correct     = button_new(0, 0, 180, 52, "Correct",     "正解",       COLOR_SUCCESS);
    Button btn_retry       = button_new(0, 0, 240, 52, "Try Again",   "もう一度",   COLOR_PRIMARY);
    Button btn_menu_from_study = button_new(0, 0, 140, 40, "Menu", "メニュー", COLOR_OVERLAY);

    // Card list
    Button btn_back_from_list = button_new(0, 0, 140, 40, "Back",    "戻る",       COLOR_OVERLAY);
    Button btn_scroll_up      = button_new(0, 0, 120, 36, "▲", "▲", COLOR_OVERLAY);
    Button btn_scroll_down    = button_new(0, 0, 120, 36, "▼", "▼", COLOR_OVERLAY);

    // Editor
    Button btn_editor_save     = button_new(0, 0, 220, 52, "Save",     "保存",       COLOR_SUCCESS);
    Button btn_editor_cancel   = button_new(0, 0, 220, 52, "Cancel",   "キャンセル", COLOR_DANGER);

    // Confirm
    Button btn_confirm_yes = button_new(0, 0, 160, 48, "Delete",  "削除する", COLOR_DANGER);
    Button btn_confirm_no  = button_new(0, 0, 160, 48, "Cancel",  "キャンセル", COLOR_OVERLAY);

    while (!WindowShouldClose()) {
        float w = (float)GetScreenWidth();
        float h = (float)GetScreenHeight();

        // ── Layout calculations ──
        float center_x = w / 2.0f;
        float card_x = (w - CARD_WIDTH) / 2.0f;

        // Menu buttons
        btn_study.rect  = (Rectangle){ center_x - 140, 220, 280, 64 };
        btn_list.rect   = (Rectangle){ center_x - 140, 300, 280, 64 };
        btn_add.rect    = (Rectangle){ center_x - 140, 380, 280, 64 };

        // Study buttons
        btn_menu_from_study.rect  = (Rectangle){ 20, 20, 140, 40 };
        btn_show_answer.rect      = (Rectangle){ center_x - 120, 420, 240, 52 };
        btn_incorrect.rect        = (Rectangle){ center_x - 190, 420, 180, 52 };
        btn_correct.rect          = (Rectangle){ center_x + 10,  420, 180, 52 };
        btn_retry.rect            = (Rectangle){ center_x - 120, 420, 240, 52 };

        // Card list buttons
        btn_back_from_list.rect   = (Rectangle){ 20, 20, 140, 40 };
        btn_scroll_up.rect        = (Rectangle){ w - 140, 70, 120, 36 };
        btn_scroll_down.rect      = (Rectangle){ w - 140, h - 60, 120, 36 };

        // Editor buttons
        btn_editor_save.rect      = (Rectangle){ center_x - 230, h - 80, 220, 52 };
        btn_editor_cancel.rect    = (Rectangle){ center_x + 10,  h - 80, 220, 52 };

        // Confirm buttons
        btn_confirm_yes.rect      = (Rectangle){ center_x - 170, h / 2 + 30, 160, 48 };
        btn_confirm_no.rect       = (Rectangle){ center_x + 10,  h / 2 + 30, 160, 48 };

        // ── Update ──
        button_update(&btn_menu_from_study);
        button_update(&btn_incorrect);
        button_update(&btn_correct);
        button_update(&btn_show_answer);
        button_update(&btn_retry);
        button_update(&btn_back_from_list);
        button_update(&btn_scroll_up);
        button_update(&btn_scroll_down);
        button_update(&btn_editor_save);
        button_update(&btn_editor_cancel);
        button_update(&btn_confirm_yes);
        button_update(&btn_confirm_no);

        // Prevent menu button clicks when in confirm dialog
        if (!show_confirm_delete) {
            button_update(&btn_study);
            button_update(&btn_list);
            button_update(&btn_add);
        }

        if (state == STATE_MENU) {
            if (btn_study.is_pressed) {
                flashcard_shuffle(deck);
                flashcard_reset(deck);
                state = STATE_STUDY_QUESTION;
            } else if (btn_list.is_pressed) {
                card_list_scroll = 0;
                state = STATE_CARD_LIST;
            } else if (btn_add.is_pressed) {
                edit_index = -1;
                textfield_init(&edit_question,  center_x - 280, 140, 560, 48);
                textfield_init(&edit_answer,    center_x - 280, 260, 560, 48);
                textfield_set_text(&edit_question, "");
                textfield_set_text(&edit_answer, "");
                edit_question.is_active = 1;
                state = STATE_EDIT_CARD;
            }
        } else if (state == STATE_STUDY_QUESTION) {
            if (btn_menu_from_study.is_pressed) {
                state = STATE_MENU;
            } else if (btn_show_answer.is_pressed) {
                state = STATE_STUDY_ANSWER;
            }
        } else if (state == STATE_STUDY_ANSWER) {
            if (btn_menu_from_study.is_pressed) {
                state = STATE_MENU;
            }
            if (btn_correct.is_pressed) {
                flashcard_mark_correct(deck);
                flashcard_next(deck);
                state = deck->finished ? STATE_RESULTS : STATE_STUDY_QUESTION;
            } else if (btn_incorrect.is_pressed) {
                flashcard_next(deck);
                state = deck->finished ? STATE_RESULTS : STATE_STUDY_QUESTION;
            }
        } else if (state == STATE_RESULTS) {
            if (btn_menu_from_study.is_pressed) {
                state = STATE_MENU;
            } else if (btn_retry.is_pressed) {
                flashcard_shuffle(deck);
                flashcard_reset(deck);
                state = STATE_STUDY_QUESTION;
            }
        } else if (state == STATE_CARD_LIST) {
            if (btn_back_from_list.is_pressed) {
                state = STATE_MENU;
            }

            // Scroll buttons
            int total_items = deck->count;
            int max_visible = (int)((h - 160) / 56);
            if (max_visible < 1) max_visible = 1;
            if (btn_scroll_up.is_pressed && card_list_scroll > 0) card_list_scroll--;
            if (btn_scroll_down.is_pressed && card_list_scroll + max_visible < total_items) card_list_scroll++;

            // Handle delete confirmation
            if (show_confirm_delete) {
                if (btn_confirm_yes.is_pressed) {
                    flashcard_remove_card(deck, confirm_delete_index);
                    flashcard_save_csv(deck, "data/cards.csv");
                    show_confirm_delete = 0;
                    confirm_delete_index = -1;
                    if (card_list_scroll >= deck->count && card_list_scroll > 0)
                        card_list_scroll--;
                } else if (btn_confirm_no.is_pressed) {
                    show_confirm_delete = 0;
                    confirm_delete_index = -1;
                }
            } else {
                // Check card item interactions
                float list_start_y = 70;
                float item_h = 56;
                float item_x = 30;
                float item_w = w - 170;                for (int i = card_list_scroll; i < deck->count && i < card_list_scroll + max_visible; i++) {
                    float item_y = list_start_y + (i - card_list_scroll) * item_h;

                    // Edit button per card
                    Rectangle edit_btn_rect  = { item_x + item_w - 190, item_y + 8, 85, item_h - 16 };
                    Rectangle delete_btn_rect = { item_x + item_w - 95,  item_y + 8, 85, item_h - 16 };

                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, edit_btn_rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        edit_index = i;
                        textfield_init(&edit_question, center_x - 280, 140, 560, 48);
                        textfield_init(&edit_answer,   center_x - 280, 260, 560, 48);
                        textfield_set_text(&edit_question, deck->cards[i].question);
                        textfield_set_text(&edit_answer, deck->cards[i].answer);
                        edit_question.is_active = 1;
                        state = STATE_EDIT_CARD;
                    }
                    if (CheckCollisionPointRec(mouse, delete_btn_rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        show_confirm_delete = 1;
                        confirm_delete_index = i;
                    }
                }
            }
        } else if (state == STATE_EDIT_CARD) {
            textfield_update(&edit_question);
            textfield_update(&edit_answer);

            // Tab to switch between fields
            if (IsKeyPressed(KEY_TAB)) {
                if (edit_question.is_active) {
                    edit_question.is_active = 0;
                    edit_answer.is_active = 1;
                } else if (edit_answer.is_active) {
                    edit_answer.is_active = 0;
                    edit_question.is_active = 1;
                }
            }

            if (btn_editor_save.is_pressed) {
                if (strlen(edit_question.buffer) > 0 && strlen(edit_answer.buffer) > 0) {
                    if (edit_index >= 0) {
                        flashcard_update_card(deck, edit_index,
                                              edit_question.buffer, edit_answer.buffer);
                    } else {
                        flashcard_add_card(deck, edit_question.buffer, edit_answer.buffer);
                    }
                    flashcard_save_csv(deck, "data/cards.csv");
                    state = STATE_CARD_LIST;
                    card_list_scroll = 0;
                }
            } else if (btn_editor_cancel.is_pressed) {
                state = STATE_CARD_LIST;
            }
        }

        // ── Draw ──
        BeginDrawing();
        ClearBackground(COLOR_BG);

        if (state == STATE_MENU) {
            // Title
            draw_text_centered(jpFont, use_jp,
                "単語帳", "Flashcard",
                center_x, 100, 48, COLOR_PRIMARY);
            draw_text_centered(jpFont, use_jp,
                "効率的に学ぶ", "Study efficiently",
                center_x, 155, 20, COLOR_SUBTEXT);

            // Decorative line
            DrawRectangle((int)center_x - 60, 185, 120, 3, COLOR_PRIMARY);

            // Menu buttons
            btn_study.text_color = COLOR_BG;
            button_draw(&btn_study, jpFont, use_jp);
            btn_list.text_color = COLOR_BG;
            button_draw(&btn_list, jpFont, use_jp);
            btn_add.text_color = COLOR_BG;
            button_draw(&btn_add, jpFont, use_jp);

            // Version / info
            draw_text_centered(jpFont, use_jp,
                "Raylib で作られた軽量フラッシュカードアプリ",
                "A lightweight flashcard app built with Raylib",
                center_x, h - 40, 14, COLOR_SUBTEXT);

        } else if (state == STATE_STUDY_QUESTION || state == STATE_STUDY_ANSWER) {
            // Back to menu
            btn_menu_from_study.text_color = COLOR_TEXT;
            button_draw(&btn_menu_from_study, jpFont, use_jp);

            // Progress bar
            float prog_w = w - 200;
            float prog_x = (w - prog_w) / 2.0f;
            float prog_y = 20;
            DrawRectangleRounded((Rectangle){prog_x, prog_y, prog_w, 8}, 4, 4, COLOR_SURFACE);
            float fill = (deck->count > 0) ? (float)deck->current_index / deck->count : 0;
            DrawRectangleRounded((Rectangle){prog_x, prog_y, prog_w * fill, 8}, 4, 4, COLOR_PRIMARY);

            // Progress text
            char prog_en[64], prog_jp[64];
            if (use_jp) {
                snprintf(prog_jp, sizeof(prog_jp), "カード %d / %d - 正解 %d",
                         deck->current_index + 1, deck->count, deck->correct_count);
            } else {
                snprintf(prog_en, sizeof(prog_en), "Card %d / %d - Correct: %d",
                         deck->current_index + 1, deck->count, deck->correct_count);
            }
            draw_text_centered(jpFont, use_jp, prog_jp, prog_en,
                               center_x, prog_y + 18, 16, COLOR_SUBTEXT);

            // Card box
            Rectangle card_rect = { card_x, 70, CARD_WIDTH, CARD_HEIGHT };
            DrawRectangleRounded(card_rect, 0.08f, 10, COLOR_SURFACE);
            DrawRectangleRoundedLinesEx(card_rect, 0.08f, 10, 2,
                                      state == STATE_STUDY_ANSWER ? COLOR_SUCCESS : COLOR_PRIMARY);

            Flashcard *card = flashcard_get_current(deck);
            if (card) {
                if (state == STATE_STUDY_QUESTION) {
                    draw_text(jpFont, use_jp, "質問", "Question",
                              card_x + 24, 90, 18, COLOR_SUBTEXT);
                    // Draw question with word wrap (simple - just draw)
                    draw_text(jpFont, use_jp, card->question, card->question,
                              card_x + 24, 130, 26, COLOR_TEXT);
                } else {
                    draw_text(jpFont, use_jp, "答え", "Answer",
                              card_x + 24, 90, 18, COLOR_SUBTEXT);
                    draw_text(jpFont, use_jp, card->answer, card->answer,
                              card_x + 24, 130, 26, COLOR_SUCCESS);
                }
            }

            // Buttons
            btn_show_answer.text_color = COLOR_BG;
            btn_incorrect.text_color   = COLOR_BG;
            btn_correct.text_color     = COLOR_BG;

            if (state == STATE_STUDY_QUESTION) {
                button_draw(&btn_show_answer, jpFont, use_jp);
            } else {
                button_draw(&btn_incorrect, jpFont, use_jp);
                button_draw(&btn_correct, jpFont, use_jp);
            }

            // Hint
            if (state == STATE_STUDY_QUESTION) {
                draw_text_centered(jpFont, use_jp,
                    "回答を見るボタンを押して答えを確認",
                    "Press 'Show Answer' to reveal the answer",
                    center_x, h - 30, 14, COLOR_SUBTEXT);
            } else {
                draw_text_centered(jpFont, use_jp,
                    "正解・不正解を選んで次のカードへ",
                    "Mark as correct or incorrect to continue",
                    center_x, h - 30, 14, COLOR_SUBTEXT);
            }

        } else if (state == STATE_RESULTS) {
            // Back to menu
            btn_menu_from_study.text_color = COLOR_TEXT;
            button_draw(&btn_menu_from_study, jpFont, use_jp);

            // Results card
            float res_card_w = 400;
            float res_card_h = 280;
            float res_card_x = (w - res_card_w) / 2;
            float res_card_y = (h - res_card_h) / 2 - 30;
            DrawRectangleRounded((Rectangle){res_card_x, res_card_y, res_card_w, res_card_h}, 0.1f, 10, COLOR_SURFACE);

            draw_text_centered(jpFont, use_jp,
                "学習結果", "Study Results",
                center_x, res_card_y + 30, 32, COLOR_PRIMARY);

            // Score circle
            float score_radius = 50;
            float score_cx = center_x;
            float score_cy = res_card_y + 120;
            DrawCircle((int)score_cx, (int)score_cy, score_radius, COLOR_OVERLAY);
            DrawCircleLines((int)score_cx, (int)score_cy, score_radius, COLOR_PRIMARY);

            char score_en[32], score_jp[32];
            snprintf(score_jp, sizeof(score_jp), "%d/%d", deck->correct_count, deck->count);
            snprintf(score_en, sizeof(score_en), "%d/%d", deck->correct_count, deck->count);
            draw_text_centered(jpFont, use_jp, score_jp, score_en,
                               score_cx, score_cy - 14, 28, (deck->correct_count >= deck->count / 2) ? COLOR_SUCCESS : COLOR_WARNING);

            draw_text_centered(jpFont, use_jp,
                "正解", "Correct",
                score_cx, score_cy + 22, 14, COLOR_SUBTEXT);

            // Percentage
            int pct = (deck->count > 0) ? (deck->correct_count * 100 / deck->count) : 0;
            char pct_en[16], pct_jp[16];
            snprintf(pct_jp, sizeof(pct_jp), "正解率: %d%%", pct);
            snprintf(pct_en, sizeof(pct_en), "Score: %d%%", pct);
            draw_text_centered(jpFont, use_jp, pct_jp, pct_en,
                               center_x, res_card_y + 180, 18, COLOR_TEXT);

            btn_retry.text_color = COLOR_BG;
            button_draw(&btn_retry, jpFont, use_jp);

        } else if (state == STATE_CARD_LIST) {
            // Back
            btn_back_from_list.text_color = COLOR_TEXT;
            button_draw(&btn_back_from_list, jpFont, use_jp);

            draw_text_centered(jpFont, use_jp,
                "カード一覧", "Card List",
                center_x, 25, 28, COLOR_PRIMARY);

            DrawRectangle((int)center_x - 40, 58, 80, 2, COLOR_PRIMARY);

            if (deck->count == 0) {
                draw_text_centered(jpFont, use_jp,
                    "カードがありません。メニューから追加してください。",
                    "No cards. Add some from the menu!",
                    center_x, h / 2, 20, COLOR_SUBTEXT);
            } else {
                // Scroll buttons
                int max_visible = (int)((h - 160) / 56);
                if (max_visible < 1) max_visible = 1;

                if (deck->count > max_visible) {
                    btn_scroll_up.text_color = COLOR_TEXT;
                    btn_scroll_down.text_color = COLOR_TEXT;
                    button_draw(&btn_scroll_up, jpFont, use_jp);
                    button_draw(&btn_scroll_down, jpFont, use_jp);
                }

                float list_x = 30;
                float list_w = w - 200;
                float start_y = 70;
                float item_h = 56;

                for (int i = card_list_scroll; i < deck->count && i < card_list_scroll + max_visible; i++) {
                    float item_y = start_y + (i - card_list_scroll) * item_h;

                    // Card row background
                    Rectangle item_bg = { list_x, item_y, list_w, item_h - 4 };
                    DrawRectangleRounded(item_bg, 0.06f, 6, COLOR_SURFACE);
                    // Index
                    char idx_str[16];
                    snprintf(idx_str, sizeof(idx_str), "%d", i + 1);
                    if (use_jp && jpFont.texture.id > 0) {
                        DrawTextEx(jpFont, idx_str, (Vector2){list_x + 10, item_y + 14}, 18, 1, COLOR_ACCENT);
                    } else {
                        DrawText(idx_str, (int)list_x + 10, (int)item_y + 14, 18, COLOR_ACCENT);
                    }

                    // Question text (truncated)
                    char q_display[INPUT_BUFFER_SIZE];
                    strncpy(q_display, deck->cards[i].question, 30);
                    q_display[30] = '\0';
                    if (use_jp && jpFont.texture.id > 0) {
                        Vector2 qs = MeasureTextEx(jpFont, q_display, 16, 1);
                        DrawTextEx(jpFont, q_display, (Vector2){list_x + 40, item_y + 16}, 16, 1, COLOR_TEXT);
                        // "->" separator
                        Vector2 arrow_pos = { list_x + 44 + qs.x, item_y + 16 };
                        DrawTextEx(jpFont, "→", arrow_pos, 16, 1, COLOR_SUBTEXT);
                    } else {
                        DrawText(q_display, (int)list_x + 40, (int)item_y + 16, 16, COLOR_TEXT);
                    }

                    // Edit button
                    Rectangle edit_r = { list_x + list_w - 190, item_y + 8, 85, item_h - 16 };
                    Vector2 mouse = GetMousePosition();
                    bool edit_hov = CheckCollisionPointRec(mouse, edit_r);
                    Color edit_col = edit_hov ? (Color){100, 120, 200, 255} : (Color){70, 80, 120, 255};
                    DrawRectangleRounded(edit_r, 0.06f, 6, edit_col);
                    const char *edit_label = use_jp ? "編集" : "Edit";
                    if (use_jp && jpFont.texture.id > 0) {
                        Vector2 es = MeasureTextEx(jpFont, edit_label, 15, 1);
                        DrawTextEx(jpFont, edit_label,
                                   (Vector2){edit_r.x + (edit_r.width - es.x) / 2, edit_r.y + (edit_r.height - es.y) / 2},
                                   15, 1, COLOR_TEXT);
                    } else {
                        int ew = MeasureText("Edit", 15);
                        DrawText("Edit", (int)(edit_r.x + (edit_r.width - ew) / 2),
                                 (int)(edit_r.y + (edit_r.height - 15) / 2), 15, COLOR_TEXT);
                    }

                    // Delete button
                    Rectangle del_r = { list_x + list_w - 95, item_y + 8, 85, item_h - 16 };
                    bool del_hov = CheckCollisionPointRec(mouse, del_r);
                    Color del_col = del_hov ? (Color){200, 80, 80, 255} : (Color){140, 60, 60, 255};
                    DrawRectangleRounded(del_r, 0.06f, 6, del_col);
                    const char *del_label = use_jp ? "削除" : "Delete";
                    if (use_jp && jpFont.texture.id > 0) {
                        Vector2 ds = MeasureTextEx(jpFont, del_label, 15, 1);
                        DrawTextEx(jpFont, del_label,
                                   (Vector2){del_r.x + (del_r.width - ds.x) / 2, del_r.y + (del_r.height - ds.y) / 2},
                                   15, 1, COLOR_TEXT);
                    } else {
                        int dw = MeasureText("Delete", 15);
                        DrawText("Delete", (int)(del_r.x + (del_r.width - dw) / 2),
                                 (int)(del_r.y + (del_r.height - 15) / 2), 15, COLOR_TEXT);
                    }
                }
            }

            // Delete confirmation dialog
            if (show_confirm_delete) {
                // Dim background
                DrawRectangle(0, 0, (int)w, (int)h, (Color){0, 0, 0, 120});

                // Dialog box
                float dia_w = 360, dia_h = 160;
                float dia_x = (w - dia_w) / 2, dia_y = (h - dia_h) / 2 - 20;
                DrawRectangleRounded((Rectangle){dia_x, dia_y, dia_w, dia_h}, 0.08f, 8, COLOR_SURFACE);
                DrawRectangleRoundedLinesEx((Rectangle){dia_x, dia_y, dia_w, dia_h}, 0.08f, 8, 2, COLOR_DANGER);

                draw_text_centered(jpFont, use_jp,
                    "このカードを削除しますか？",
                    "Delete this card?",
                    center_x, dia_y + 30, 22, COLOR_TEXT);

                draw_text_centered(jpFont, use_jp,
                    "この操作は元に戻せません",
                    "This cannot be undone",
                    center_x, dia_y + 60, 16, COLOR_SUBTEXT);

                btn_confirm_yes.text_color = COLOR_BG;
                btn_confirm_no.text_color = COLOR_TEXT;
                button_draw(&btn_confirm_yes, jpFont, use_jp);
                button_draw(&btn_confirm_no, jpFont, use_jp);
            }

        } else if (state == STATE_EDIT_CARD) {
            // Editor background
            DrawRectangle(0, 0, (int)w, (int)h, (Color){0, 0, 0, 40});

            // Editor card
            float ed_w = 640, ed_h = 380;
            float ed_x = (w - ed_w) / 2, ed_y = (h - ed_h) / 2 - 20;
            DrawRectangleRounded((Rectangle){ed_x, ed_y, ed_w, ed_h}, 0.08f, 8, COLOR_SURFACE);
            DrawRectangleRoundedLinesEx((Rectangle){ed_x, ed_y, ed_w, ed_h}, 0.08f, 8, 2, COLOR_PRIMARY);

            if (edit_index >= 0) {
                draw_text_centered(jpFont, use_jp,
                    "カードを編集", "Edit Card",
                    center_x, ed_y + 20, 26, COLOR_PRIMARY);
            } else {
                draw_text_centered(jpFont, use_jp,
                    "新しいカード", "New Card",
                    center_x, ed_y + 20, 26, COLOR_SUCCESS);
            }

            // Reposition text fields inside editor card
            edit_question.rect = (Rectangle){ ed_x + 40, ed_y + 70, ed_w - 80, 48 };
            edit_answer.rect   = (Rectangle){ ed_x + 40, ed_y + 170, ed_w - 80, 48 };

            // Reposition buttons inside editor card
            btn_editor_save.rect   = (Rectangle){ center_x - 230, ed_y + 270, 220, 52 };
            btn_editor_cancel.rect = (Rectangle){ center_x + 10,  ed_y + 270, 220, 52 };

            textfield_draw(&edit_question, jpFont, use_jp,
                use_jp ? "質問" : "Question");
            textfield_draw(&edit_answer, jpFont, use_jp,
                use_jp ? "答え" : "Answer");

            btn_editor_save.text_color = COLOR_BG;
            btn_editor_cancel.text_color = COLOR_BG;
            button_draw(&btn_editor_save, jpFont, use_jp);
            button_draw(&btn_editor_cancel, jpFont, use_jp);
        }

        EndDrawing();
    }

    // ── Cleanup ──
    if (use_jp) UnloadFont(jpFont);
    flashcard_deck_free(deck);
    CloseWindow();
    return 0;
}
