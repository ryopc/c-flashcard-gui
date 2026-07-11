#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct {
    char *question;
    char *answer;
} Flashcard;

static Flashcard *cards = NULL;
static int num_cards = 0;

static int current_index = 0;
static int correct_count = 0;
static gboolean showing_answer = FALSE;
static gboolean is_finished = FALSE;
static gboolean is_fullscreen = FALSE;

static GtkWidget *window;
static GtkWidget *notebook;
static GtkWidget *label_progress;
static GtkWidget *label_card;
static GtkWidget *btn_action;
static GtkWidget *box_choices;
static GtkWidget *btn_correct;
static GtkWidget *btn_incorrect;
static GtkWidget *text_view_editor;

// Forward declarations
static void load_cards();
static void shuffle_cards();
static void update_ui();
static void load_editor_content();

static void toggle_fullscreen() {
    if (is_fullscreen) {
        gtk_window_unfullscreen(GTK_WINDOW(window));
        is_fullscreen = FALSE;
    } else {
        gtk_window_fullscreen(GTK_WINDOW(window));
        is_fullscreen = TRUE;
    }
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_F11) {
        toggle_fullscreen();
        return TRUE;
    }
    return FALSE;
}

static void on_btn_fullscreen_clicked(GtkWidget *widget, gpointer data) {
    toggle_fullscreen();
}

static void free_cards() {
    if (cards) {
        for (int i = 0; i < num_cards; i++) {
            g_free(cards[i].question);
            g_free(cards[i].answer);
        }
        free(cards);
        cards = NULL;
        num_cards = 0;
    }
}

static void load_cards() {
    free_cards();

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

static void load_editor_content() {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_editor));
    char *content = NULL;
    gsize length = 0;

    if (g_file_get_contents("cards.txt", &content, &length, NULL)) {
        gtk_text_buffer_set_text(buffer, content, length);
        g_free(content);
    } else {
        gtk_text_buffer_set_text(buffer, "", 0);
    }
}

static void on_btn_save_clicked(GtkWidget *widget, gpointer data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_editor));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    GError *error = NULL;
    if (g_file_set_contents("cards.txt", text, -1, &error)) {
        load_cards();
        shuffle_cards();
        current_index = 0;
        correct_count = 0;
        showing_answer = FALSE;
        is_finished = FALSE;
        update_ui();

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_INFO,
                                                   GTK_BUTTONS_OK,
                                                   "保存して再読み込みしました！");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK,
                                                   "保存に失敗しました: %s", error->message);
        g_error_free(error);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    g_free(text);
}

static void on_notebook_switch_page(GtkNotebook *nb, GtkWidget *page, guint page_num, gpointer data) {
    if (page_num == 1) {
        load_editor_content();
    }
}

static void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = 
        "window {\n"
        "    background-color: #1e1e2e;\n"
        "}\n"
        "notebook {\n"
        "    background-color: #1e1e2e;\n"
        "    border: none;\n"
        "}\n"
        "notebook header {\n"
        "    background-color: #181825;\n"
        "    padding: 4px;\n"
        "}\n"
        "notebook tab {\n"
        "    background-color: #313244;\n"
        "    color: #cdd6f4;\n"
        "    font-weight: bold;\n"
        "    padding: 8px 16px;\n"
        "    border-radius: 6px;\n"
        "    margin: 2px;\n"
        "}\n"
        "notebook tab:checked {\n"
        "    background-color: #89b4fa;\n"
        "    color: #11111b;\n"
        "}\n"
        ".main-box {\n"
        "    padding: 24px;\n"
        "}\n"
        ".progress-label {\n"
        "    color: #a6adc8;\n"
        "    font-size: 16px;\n"
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
        "    font-size: 28px;\n"
        "    font-weight: bold;\n"
        "}\n"
        "textview text {\n"
        "    background-color: #181825;\n"
        "    color: #cdd6f4;\n"
        "    font-family: monospace;\n"
        "    font-size: 16px;\n"
        "    padding: 12px;\n"
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
        "}\n"
        ".btn-secondary {\n"
        "    background-color: #45475a;\n"
        "    color: #cdd6f4;\n"
        "}\n"
        ".btn-secondary:hover {\n"
        "    background-color: #585b70;\n"
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

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "暗記カード Pro");
    gtk_window_set_default_size(GTK_WINDOW(window), 650, 450);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // Main layout container
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // Top bar with Fullscreen button
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(top_bar), "main-box");
    gtk_container_set_border_width(GTK_CONTAINER(top_bar), 6);
    gtk_box_pack_start(GTK_BOX(main_vbox), top_bar, FALSE, FALSE, 0);

    GtkWidget *title_label = gtk_label_new("暗記カード Pro");
    gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "progress-label");
    gtk_box_pack_start(GTK_BOX(top_bar), title_label, FALSE, FALSE, 0);

    GtkWidget *btn_fullscreen = gtk_button_new_with_label("全画面 (F11)");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_fullscreen), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_fullscreen), "btn-secondary");
    g_signal_connect(btn_fullscreen, "clicked", G_CALLBACK(on_btn_fullscreen_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(top_bar), btn_fullscreen, FALSE, FALSE, 0);

    // Notebook for Tabs
    notebook = gtk_notebook_new();
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), NULL);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);

    // ================= TAB 1: STUDY =================
    GtkWidget *study_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_style_context_add_class(gtk_widget_get_style_context(study_vbox), "main-box");

    label_progress = gtk_label_new("");
    gtk_style_context_add_class(gtk_widget_get_style_context(label_progress), "progress-label");
    gtk_box_pack_start(GTK_BOX(study_vbox), label_progress, FALSE, FALSE, 0);

    GtkWidget *card_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(card_container), "card-box");
    gtk_box_pack_start(GTK_BOX(study_vbox), card_container, TRUE, TRUE, 0);

    label_card = gtk_label_new("");
    gtk_label_set_justify(GTK_LABEL(label_card), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label_card), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(label_card), "card-label");
    gtk_box_pack_start(GTK_BOX(card_container), label_card, TRUE, TRUE, 0);

    btn_action = gtk_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_action), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_action), "btn-primary");
    g_signal_connect(btn_action, "clicked", G_CALLBACK(on_btn_action_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(study_vbox), btn_action, FALSE, FALSE, 0);

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

    gtk_box_pack_end(GTK_BOX(study_vbox), box_choices, FALSE, FALSE, 0);

    GtkWidget *tab_label_study = gtk_label_new("学習");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), study_vbox, tab_label_study);

    // ================= TAB 2: EDITOR =================
    GtkWidget *editor_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_vbox), "main-box");

    GtkWidget *editor_info_label = gtk_label_new("1行目に問題、2行目に答えを交互に入力してください。");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_info_label), "progress-label");
    gtk_box_pack_start(GTK_BOX(editor_vbox), editor_info_label, FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(editor_vbox), scrolled_window, TRUE, TRUE, 0);

    text_view_editor = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view_editor);

    GtkWidget *btn_save = gtk_button_new_with_label("保存して再読み込み");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_save), "btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_save), "btn-success");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_btn_save_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(editor_vbox), btn_save, FALSE, FALSE, 0);

    GtkWidget *tab_label_editor = gtk_label_new("編集");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), editor_vbox, tab_label_editor);

    // Initialize UI
    update_ui();

    gtk_widget_show_all(window);
    gtk_widget_hide(box_choices);

    gtk_main();

    // Clean up allocated memory
    free_cards();

    return 0;
}
