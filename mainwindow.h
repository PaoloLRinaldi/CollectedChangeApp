#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListView>
#include <QTableView>
#include <QPushButton>
#include <QPoint>
#include <vector>
#include "updater.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void add_cents_found();
    void remove_cents_found();
    void add_cents_pen();
    void remove_cents_pen();
    void scroll_up();
    void scroll_down();
    void refresh_all();
    void anim_summ_fin() { adjust_summary(); }

private:
    Ui::MainWindow *ui;
    QLineEdit *today = nullptr;
    QPushButton *refresh = nullptr;
    std::vector<std::vector<QLineEdit*>> summary;
    QPushButton *scrollup = nullptr;
    QPushButton *scrolldown = nullptr;
    QLineEdit *current = nullptr;
    QLineEdit *found = nullptr;
    QLineEdit *pen = nullptr;
    QPushButton *found_down = nullptr;
    QPushButton *found_up = nullptr;
    QPushButton *pen_down = nullptr;
    QPushButton *pen_up = nullptr;
    
    size_t height;
    size_t width;

    Updater upd;

    void init_QLineEdit(QLineEdit **qle, double wsz, double hsz, double wpos, double hpos, bool readonly = true);
    void init_QPushButton(QPushButton **qle, double wsz, double hsz, double wpos, double hpos, const char *text);
    void init_summary();
    void update_summary();
    void del_summary();
    void adjust_summary();
    void print_summary_from(int n);

    template <typename T>
    void fade_out_widget(T* w, int time);
    template <typename T>
    void fade_in_widget(T* w, int time);
    template <typename T>
    void animated_move(T* w, int time, const QPoint &p1, const QPoint &p2, const char* slt = "");
    void update_today();
};

template<class T> void colourer (T* thing, int r, int g, int b);

#endif // MAINWINDOW_H
