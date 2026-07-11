#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    const char *question;
    const char *answer;
} Flashcard;

static Flashcard *cards = NULL;
static int num_cards = 0;

static int current_index = 0;
static int correct_count = 0;
static gboolean showing_answer = FALSE;
static gboolean is_finished = FALSE;

static GtkWidget *label_progress;
static GtkWidget *label_card;
static GtkWidget *btn_action;
static GtkWidget *box_choices;
static GtkWidget *btn_correct;
static GtkWidget *btn_incorrect;

static void load_cards() {
    FILE *fp = fopen("cards.txt", "r");
    if (!fp) {
        // Create default cards.txt if it doesn't exist
        fp = fopen("cards.txt", "w");
        if (fp) {
            fprintf(fp, "ls -la\n隠しファイルを含むすべてのファイルを詳細情報付きで一覧表示する\n");
            fprintf(fp, "ps aux\n実行中のすべてのプロセスを詳細に表示する\n");
            fprintf(fp, "df -h\nディスクの使用状況を人間が読みやすい形式（GB, MB等）で表示する\n");
            fclose(fp);
        }
        fp = fopen("cards.txt", "r");
    }

    if (!fp) {
        // Fallback in case file operations failed completely
        num_cards = 1;
        cards = malloc(sizeof(Flashcard) * num_cards);
        cards[0].question = g_strdup("エラー");
        cards[0].answer = g_strdup("cards.txt を作成・読み込みできませんでした。");
        return;
    }

    int capacity = 10;
    cards = malloc(sizeof(Flashcard) * capacity);
    num_cards = 0;

    char line1[1024];
    char line2[1024];
    while (fgets(line1, sizeof(line1), fp)) {
        // Strip trailing newline characters
        line1[strcspn(line1, "\r\n")] = '\0';
        if (strlen(line1) == 0) {
            continue; // Skip empty lines
        }

        if (fgets(line2, sizeof(line2), fp)) {
            line2[strcspn(line2, "\r\n")] = '\0';
        } else {
            line2[0] = '\0';
        }

        if (num_cards >= capacity) {
            capacity *= 2;
            cards = realloc(cards, sizeof(Flashcard) * capacity);
        }

        cards[num_cards].question = g_strdup(line1);
        cards[num_cards].answer = g_strdup(line2);
        num_cards++;
    }
    fclose(fp);

    if (num_cards == 0) {
        // Fallback if the file was empty
        num_cards = 1;
        cards = malloc(sizeof(Flashcard) * num_cards);
        cards[0].question = g_strdup("カードがありません");
        cards[0].answer = g_strdup("cards.txt に問題を追加してください。");
    }
}

static void shuffle_cards() {
    for (int i = num_cards - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Flashcard temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
}

static void update_ui() {
    char progress_text[64];
    char card_text[512];

    if (is_finished) {
        snprintf(progress_text, sizeof(progress_text), "結果発表");
        snprintf(card_text, sizeof(card_text), "全問終了！\n\nスコア: %d / %d 正解", correct_count, num_cards);
        
        gtk_label_set_text(GTK_LABEL(label_progress), progress_text);
        gtk_label_set_text(GTK_LABEL(label_card), card_text);
        
        gtk_widget_show(btn_action);
        gtk_button_set_label(GTK_BUTTON(btn_action), "もう一度挑戦する");
        gtk_widget_hide(box_choices);
    } else {
        snprintf(progress_text, sizeof(progress_text), "問題 %d / %d", current_index + 1, num_cards);
        gtk_label_set_text(GTK_LABEL(label_progress), progress_text);

        if (showing_answer) {
            gtk_label_set_text(GTK_LABEL(label_card), cards[current_index].answer);
            gtk_widget_hide(btn_action);
            gtk_widget_show_all(box_choices);
        } else {
            gtk_label_set_text(GTK_LABEL(label_card), cards[current_index].question);
            gtk_widget_show(btn_action);
            gtk_button_set_label(GTK_BUTTON(btn_action), "答えを表示");
            gtk_widget_hide(box_choices);
        }
    }
}

static void on_btn_action_clicked(GtkWidget *widget, gpointer data) {
    if (is_finished) {
        shuffle_cards();
        current_index = 0;
        correct_count = 0;
        showing_answer = FALSE;
        is_finished = FALSE;
    } else {
        showing_answer = TRUE;
    }
    update_ui();
}

static void on_choice_clicked(GtkWidget *widget, gpointer data) {
    gboolean was_correct = (gboolean)(size_t)data;
    if (was_correct) {
        correct_count++;
    }

    if (current_index + 1 < num_cards) {
        current_index++;
        showing_answer = FALSE;
    } else {
        is_finished = TRUE;
    }
    update_ui();
}

static void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "window {\n"
        "    background-color: #1e1e2e;\n"
        "}\n"
        ".main-box {\n"
        "    padding: 24px;\n"
        "}\n"
        ".progress-label {\n"
        "    color: #a6adc8;\n"
        "    font-size: 14px;\n"
        "    font-weight: bold;\n"
        "    margin-bottom: 10px;\n"
        "}\n"
        ".card-box {\n"
        "    background-color: #313244;\n"
        "    border-radius: 16px;\n"
        "    border: 1px solid #45475a;\n"
        "    padding: 40px 20px;\n"
        "}\n"
        ".card-label {\n"
        "    color: #cdd6f4;\n"
        "    font-size: 22px;\n"
        "    font-weight: bold;\n"
        "}\n"
        ".btn {\n"
        "    font-size: 16px;\n"
        "    font-weight: bold;\n"
        "    padding: 12px 24px;\n"
        "    border-radius: 8px;\n"
        "    border: none;\n"
        "    transition: background-color 0.2s;\n"
        "}\n"
        ".btn-primary {\n"
        "    background-color: #89b4fa;\n"
        "    color: #11111b;\n"
        "}\n"
        ".btn-primary:hover {\n"
        "    background-color: #b4befe;\n"
        "}\n"
        ".btn-success {\n"
        "    background-color: #a6e3a1;\n"
        "    color: #11111b;\n"
        "}\n"
        ".btn-success:hover {\n"
        "    background-color: #94e2d5;\n"
        "}\n"
        ".btn-danger {\n"
        "    background-color: #f38ba8;\n"
        "    color: #11111b;\n"
        "}\n"
        ".btn-danger:hover {\n"
        "    background-color: #f5e0dc;\n"
        "}\n";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    gtk_init(&argc, &argv);
    apply_css();

    load_cards();
    shuffle_cards();

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *card_container;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "暗記カード Pro");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 350);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_style_context_add_class(gtk_widget_get_style_context(vbox), "main-box");
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label_progress = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(label_progress), "progress-label");
    gtk_box_pack_start(GTK_BOX(vbox), label_progress, FALSE, FALSE, 0);

    card_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(card_container), "card-box");
    gtk_box_pack_start(GTK_BOX(vbox), card_container, TRUE, TRUE, 0);

    label_card = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(label_card), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label_card), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(label_card), "card-label");
    gtk_box_pack_start(GTK_BOX(card_container), label_card, TRUE, TRUE, 0);

    btn_action = gtk_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_action), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_action), "btn-primary");
    g_signal_connect(btn_action, "clicked", G_CALLBACK(on_btn_action_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(vbox), btn_action, FALSE, FALSE, 0);

    box_choices = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_set_homogeneous(GTK_BOX(box_choices), TRUE);

    btn_incorrect = gtk_button_new_with_label("まだ（不正解）");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_incorrect), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_incorrect), "btn-danger");
    g_signal_connect(btn_incorrect, "clicked", G_CALLBACK(on_choice_clicked), (gpointer)FALSE);
    gtk_box_pack_start(GTK_BOX(box_choices), btn_incorrect, TRUE, TRUE, 0);

    btn_correct = gtk_button_new_with_label("覚えた（正解）");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_correct), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_correct), "btn-success");
    g_signal_connect(btn_correct, "clicked", G_CALLBACK(on_choice_clicked), (gpointer)TRUE);
    gtk_box_pack_start(GTK_BOX(box_choices), btn_correct, TRUE, TRUE, 0);

    gtk_box_pack_end(GTK_BOX(vbox), box_choices, FALSE, FALSE, 0);

    update_ui();

    gtk_widget_show_all(window);
    gtk_widget_hide(box_choices);

    gtk_main();

    // Clean up allocated memory
    if (cards) {
        for (int i = 0; i < num_cards; i++) {
            g_free((gpointer)cards[i].question);
            g_free((gpointer)cards[i].answer);
        }
        free(cards);
    }

    return 0;
}
