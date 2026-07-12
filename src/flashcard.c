#include "flashcard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define CSV_LINE_MAX 512

FlashcardDeck* flashcard_deck_new(void) {
    FlashcardDeck *deck = malloc(sizeof(FlashcardDeck));
    if (!deck) return NULL;
    
    deck->capacity = INITIAL_CAPACITY;
    deck->cards = malloc(sizeof(Flashcard) * deck->capacity);
    if (!deck->cards) {
        free(deck);
        return NULL;
    }
    
    deck->count = 0;
    deck->current_index = 0;
    deck->correct_count = 0;
    deck->shown_answer = 0;
    deck->finished = 0;
    
    return deck;
}

void flashcard_deck_free(FlashcardDeck *deck) {
    if (!deck) return;
    
    for (int i = 0; i < deck->count; i++) {
        free(deck->cards[i].question);
        free(deck->cards[i].answer);
    }
    free(deck->cards);
    free(deck);
}

static char* my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *new_str = malloc(len);
    if (new_str) memcpy(new_str, s, len);
    return new_str;
}

static char* csv_unescape(const char *str) {
    char *unescaped = malloc(strlen(str) + 1);
    if (!unescaped) return NULL;
    
    int out_idx = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\\' && str[i+1]) {
            i++;
            if (str[i] == 'n') {
                unescaped[out_idx++] = '\n';
            } else if (str[i] == 't') {
                unescaped[out_idx++] = '\t';
            } else if (str[i] == 'r') {
                unescaped[out_idx++] = '\r';
            } else if (str[i] == '\\') {
                unescaped[out_idx++] = '\\';
            } else if (str[i] == ',') {
                unescaped[out_idx++] = ',';
            } else {
                unescaped[out_idx++] = str[i];
            }
        } else {
            unescaped[out_idx++] = str[i];
        }
    }
    unescaped[out_idx] = '\0';
    
    return unescaped;
}

static char* csv_escape(const char *str) {
    int escaped_len = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n' || str[i] == '\t' || str[i] == '\r' || 
            str[i] == '\\' || str[i] == ',') {
            escaped_len += 2;
        } else {
            escaped_len++;
        }
    }
    
    char *escaped = malloc(escaped_len + 1);
    if (!escaped) return NULL;
    
    int out_idx = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            escaped[out_idx++] = '\\';
            escaped[out_idx++] = 'n';
        } else if (str[i] == '\t') {
            escaped[out_idx++] = '\\';
            escaped[out_idx++] = 't';
        } else if (str[i] == '\r') {
            escaped[out_idx++] = '\\';
            escaped[out_idx++] = 'r';
        } else if (str[i] == '\\') {
            escaped[out_idx++] = '\\';
            escaped[out_idx++] = '\\';
        } else if (str[i] == ',') {
            escaped[out_idx++] = '\\';
            escaped[out_idx++] = ',';
        } else {
            escaped[out_idx++] = str[i];
        }
    }
    escaped[out_idx] = '\0';
    
    return escaped;
}

int flashcard_load_csv(FlashcardDeck *deck, const char *filename) {
    if (!deck) return -1;
    
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }
    
    char line[CSV_LINE_MAX];
    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';
        
        if (strlen(line) == 0) continue;
        
        // Simple CSV parsing: find comma separator
        char *comma = strchr(line, ',');
        if (!comma) {
            fclose(fp);
            return -1;  // Invalid CSV format
        }
        
        // Split on first comma
        *comma = '\0';
        char *question_raw = line;
        char *answer_raw = comma + 1;
        
        char *question = csv_unescape(question_raw);
        char *answer = csv_unescape(answer_raw);
        
        if (!question || !answer) {
            free(question);
            free(answer);
            fclose(fp);
            return -1;
        }
        
        // Expand capacity if needed
        if (deck->count >= deck->capacity) {
            deck->capacity *= 2;
            Flashcard *new_cards = realloc(deck->cards, sizeof(Flashcard) * deck->capacity);
            if (!new_cards) {
                free(question);
                free(answer);
                fclose(fp);
                return -1;
            }
            deck->cards = new_cards;
        }
        
        deck->cards[deck->count].question = question;
        deck->cards[deck->count].answer = answer;
        deck->count++;
    }
    
    fclose(fp);
    
    if (deck->count == 0) {
        return -1;  // No cards loaded
    }
    
    return 0;
}

int flashcard_save_csv(const FlashcardDeck *deck, const char *filename) {
    if (!deck) return -1;
    
    FILE *fp = fopen(filename, "w");
    if (!fp) return -1;
    
    for (int i = 0; i < deck->count; i++) {
        char *q_escaped = csv_escape(deck->cards[i].question);
        char *a_escaped = csv_escape(deck->cards[i].answer);
        
        if (!q_escaped || !a_escaped) {
            free(q_escaped);
            free(a_escaped);
            fclose(fp);
            return -1;
        }
        
        fprintf(fp, "%s,%s\n", q_escaped, a_escaped);
        
        free(q_escaped);
        free(a_escaped);
    }
    
    fclose(fp);
    return 0;
}

void flashcard_shuffle(FlashcardDeck *deck) {
    if (!deck || deck->count <= 1) return;
    
    for (int i = deck->count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Flashcard temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

void flashcard_reset(FlashcardDeck *deck) {
    if (!deck) return;
    
    deck->current_index = 0;
    deck->correct_count = 0;
    deck->shown_answer = 0;
    deck->finished = 0;
}

Flashcard* flashcard_get_current(FlashcardDeck *deck) {
    if (!deck || deck->current_index >= deck->count) return NULL;
    return &deck->cards[deck->current_index];
}

void flashcard_next(FlashcardDeck *deck) {
    if (!deck) return;
    
    deck->shown_answer = 0;
    deck->current_index++;
    
    if (deck->current_index >= deck->count) {
        deck->finished = 1;
    }
}

void flashcard_mark_correct(FlashcardDeck *deck) {
    if (!deck) return;
    deck->correct_count++;
}

int flashcard_add_card(FlashcardDeck *deck, const char *question, const char *answer) {
    if (!deck || !question || !answer) return -1;
    
    // Expand capacity if needed
    if (deck->count >= deck->capacity) {
        int new_capacity = deck->capacity * 2;
        Flashcard *new_cards = realloc(deck->cards, sizeof(Flashcard) * new_capacity);
        if (!new_cards) return -1;
        deck->cards = new_cards;
        deck->capacity = new_capacity;
    }
    
    deck->cards[deck->count].question = my_strdup(question);
    deck->cards[deck->count].answer = my_strdup(answer);
    
    if (!deck->cards[deck->count].question || !deck->cards[deck->count].answer) {
        free(deck->cards[deck->count].question);
        free(deck->cards[deck->count].answer);
        return -1;
    }
    
    deck->count++;
    return 0;
}

int flashcard_remove_card(FlashcardDeck *deck, int index) {
    if (!deck || index < 0 || index >= deck->count) return -1;
    
    free(deck->cards[index].question);
    free(deck->cards[index].answer);
    
    // Shift remaining cards
    for (int i = index; i < deck->count - 1; i++) {
        deck->cards[i] = deck->cards[i + 1];
    }
    
    deck->count--;
    
    // Adjust current_index if needed
    if (deck->current_index > index) {
        deck->current_index--;
    }
    if (deck->current_index >= deck->count) {
        deck->current_index = deck->count - 1;
    }
    
    return 0;
}

int flashcard_update_card(FlashcardDeck *deck, int index, const char *question, const char *answer) {
    if (!deck || index < 0 || index >= deck->count || !question || !answer) return -1;
    
    char *new_q = my_strdup(question);
    char *new_a = my_strdup(answer);
    
    if (!new_q || !new_a) {
        free(new_q);
        free(new_a);
        return -1;
    }
    
    free(deck->cards[index].question);
    free(deck->cards[index].answer);
    deck->cards[index].question = new_q;
    deck->cards[index].answer = new_a;
    
    return 0;
}
