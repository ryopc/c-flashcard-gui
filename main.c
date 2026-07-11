#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    const char *question;
    const char *answer;
} Flashcard;

static Flashcard cards[] = {
    {"ls -la", "隠しファイルを含むすべてのファイルを詳細情報付きで一覧表示する"},
    {"chmod 755 file", "所有者に読込・書込・実行権限、グループと他者に読込・実行権限を付与する"},
    {"ps aux", "実行中のすべてのプロセスを詳細に表示する"},
    {"tar -xzvf file.tar.gz", "gzip形式で圧縮されたtarアーカイブを展開（解凍）する"},
    {"grep -r 'pattern' dir", "指定したディレクトリ内のファイルから文字列を再帰的に検索する"},
    {"find . -name '*.log'", "カレントディレクトリ以下から拡張子が.logのファイルを検索する"},
    {"ssh -i key.pem user@host", "秘密鍵を指定してセキュアシェルでリモートホストに接続する"},
    {"git commit --amend", "直前のコミットメッセージや変更内容を修正・統合する"},
    {"df -h", "ディスクの使用状況を人間が読みやすい形式（GB, MB等）で表示する"},
    {"kill -9 <PID>", "指定したプロセスIDのプロセスを強制終了する"},
    {"ln -s target link", "シンボリックリンク（ショートカット）を作成する"},
    {"mkdir -p path/to/dir", "中間ディレクトリも含めて、必要なら再帰的にディレクトリを作成する"}
};
static const int num_cards = sizeof(cards) / sizeof(cards[0]);

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
    shuffle_cards();

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *card_container;

    gtk_init(&argc, &argv);
    apply_css();

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

    return 0;
}
