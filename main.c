#include <gtk/gtk.h>

typedef struct {
    const char *question;
    const char *answer;
} Flashcard;

static Flashcard cards[] = {
    {"Apple", "りんご"},
    {"Book", "本"},
    {"Cat", "猫"}
};
static const int num_cards = sizeof(cards) / sizeof(cards[0]);

static int current_index = 0;
static gboolean showing_answer = FALSE;

static GtkWidget *label;
static GtkWidget *button;

static void update_ui() {
    if (showing_answer) {
        gtk_label_set_text(GTK_LABEL(label), cards[current_index].answer);
        gtk_button_set_label(GTK_BUTTON(button), "次の問題へ");
    } else {
        gtk_label_set_text(GTK_LABEL(label), cards[current_index].question);
        gtk_button_set_label(GTK_BUTTON(button), "答えを表示");
    }
}

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    if (showing_answer) {
        current_index = (current_index + 1) % num_cards;
        showing_answer = FALSE;
    } else {
        showing_answer = TRUE;
    }
    update_ui();
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "暗記カード");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label = gtk_label_new("");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 24");
    gtk_widget_override_font(label, font_desc);
    pango_font_description_free(font_desc);
    
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

    button = gtk_button_new();
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    update_ui();

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
