#ifndef FLASHCARD_H
#define FLASHCARD_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    char *question;
    char *answer;
} Flashcard;

typedef struct {
    Flashcard *cards;
    int count;
    int capacity;
    int current_index;
    int correct_count;
    int shown_answer;
    int finished;
} FlashcardDeck;

// Initialize a new deck
FlashcardDeck* flashcard_deck_new(void);

// Free a deck
void flashcard_deck_free(FlashcardDeck *deck);

// Load from CSV file (format: question,answer)
int flashcard_load_csv(FlashcardDeck *deck, const char *filename);

// Save to CSV file
int flashcard_save_csv(const FlashcardDeck *deck, const char *filename);

// Shuffle cards (Fisher-Yates)
void flashcard_shuffle(FlashcardDeck *deck);

// Reset deck state
void flashcard_reset(FlashcardDeck *deck);

// Get current card
Flashcard* flashcard_get_current(FlashcardDeck *deck);

// Advance to next card
void flashcard_next(FlashcardDeck *deck);

// Mark current as correct
void flashcard_mark_correct(FlashcardDeck *deck);

#endif
