#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QGraphicsOpacityEffect>
#include <QThread>
#include <ctime>
#include <sstream>

using std::string;
using std::vector;

string date_time_format(int dt) {
    if (dt < 10)
        return "0";
    else
        return "";
}

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

int stod(const std::string &s) {
    int ret = 0;
    int moltip = 1;
    for (auto it = s.crbegin(); it != s.crend(); ++it, moltip *= 10)
        ret += (*it - '0') * moltip;
    return ret;
}

string cents_to_streur(int cents) {
    int euro = cents / 100;
    cents = cents % 100;
    return to_string(euro) + "," + date_time_format(cents) + to_string(cents) + "€";
}

void MainWindow::init_QLineEdit(QLineEdit **qle, double wsz, double hsz, double wpos, double hpos, bool readonly) {
    (*qle) = new QLineEdit(this);
    (*qle)->resize(width * wsz, height * hsz);
    (*qle)->move(width * wpos, height * hpos);
    (*qle)->setReadOnly(readonly);
    (*qle)->setAlignment(Qt::AlignCenter);
    (*qle)->setAlignment(Qt::AlignHCenter);
    (*qle)->show();
}

void MainWindow::init_QPushButton(QPushButton **qle, double wsz, double hsz, double wpos, double hpos, const char *text) {
    (*qle) = new QPushButton(this);
    (*qle)->resize(width * wsz, height * hsz);
    (*qle)->move(width * wpos, height * hpos);
    (*qle)->setText(tr(text));
    (*qle)->show();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    summary(vector<vector<QLineEdit*>>(7, vector<QLineEdit*>(3, nullptr)))  // numero, data, +quanto
{
    ui->setupUi(this);
    QRect rec = QApplication::desktop()->availableGeometry();
    height = rec.height();
    width = rec.width();
    this->setStyleSheet(" QLineEdit { "
                        " border-radius: 0px; } "

                        " QLineEdit:focus { "
                        " border:0px outset; "
                        " border-radius: 0px; }"
                        
                        " QPushButton { "
                        " border-radius: 0px; }");

    init_QLineEdit(&today, 1. / 3, 1. / 10, 0, 0, true);  // arrivo a 1/20
    init_QPushButton(&refresh, 1. / 5, 1. / 10, 4. / 5, 0, "R");
    colourer(refresh, 255, 255, 0);  // Giallo
    connect(refresh, SIGNAL(released()), this, SLOT(refresh_all()));

    init_summary();  // arrivo a 11 / 20
    
    init_QPushButton(&scrollup, 1. / 5, 1. / 10, 4. / 5, 1. / 5, "^");
    colourer(scrollup, 160, 160, 160);  // Grigio
    connect(scrollup, SIGNAL(released()), this, SLOT(scroll_up()));
    init_QPushButton(&scrolldown, 1. / 5, 1. / 10, 4. / 5, 9. / 20, "v");
    colourer(scrolldown, 160, 160, 160);  // Grigio
    connect(scrolldown, SIGNAL(released()), this, SLOT(scroll_down()));
    
    init_QLineEdit(&current, 3. / 4, 1. / 10, 1. / 8, 23. / 40, true);  // arrivo a 13/20

    init_QLineEdit(&found, 1. / 2, 1. / 10, 1. / 4, 7. / 10, false);  // arrivo a 16/20
    found->setPlaceholderText(QString("Trovati"));
    found->setValidator(new QIntValidator(0, 9999, this));
    found->setInputMethodHints(Qt::ImhPreferNumbers);
    init_QLineEdit(&pen, 1. / 2, 1. / 10, 1. / 4, 17. / 20, false);  // arrivo a 17/20
    pen->setPlaceholderText(QString("Spesi/aggiunti a mano"));
    pen->setValidator(new QIntValidator(0, 9999, this));
    pen->setInputMethodHints(Qt::ImhPreferNumbers);

    init_QPushButton(&found_down, 1. / 5, 1. / 10, 0, 7. / 10, "-");
    colourer(found_down, 255, 100, 100);  // Rosso chiaro
    connect(found_down, SIGNAL(released()), this, SLOT(remove_cents_found()));
    init_QPushButton(&found_up, 1. / 5, 1. / 10, 4. / 5, 7. / 10, "+");
    colourer(found_up, 100, 255, 100);  // Verde chiaro
    connect(found_up, SIGNAL(released()), this, SLOT(add_cents_found()));
    init_QPushButton(&pen_down, 1. / 5, 1. / 10, 0, 17. / 20, "-");
    colourer(pen_down, 255, 100, 100);  // Rosso chiaro
    connect(pen_down, SIGNAL(released()), this, SLOT(remove_cents_pen()));
    init_QPushButton(&pen_up, 1. / 5, 1. / 10, 4. / 5, 17. / 20, "+");
    colourer(pen_up, 100, 255, 100);  // Verde chiaro
    connect(pen_up, SIGNAL(released()), this, SLOT(add_cents_pen()));
    
    refresh_all();

    colourer(this, 100, 178, 255);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete today;
    delete refresh;
    del_summary();
    delete scrollup;
    delete scrolldown;
    delete current;
}

void MainWindow::init_summary() {
    for (unsigned int i = 0; i != summary.size(); ++i) {
        init_QLineEdit(&summary[i][0], 1. / 10, 1. / 20, 1. / 20, (i + 4.) / 20, true);

        init_QLineEdit(&summary[i][1], 1. / 3, 1. / 20, 1. / 6, (i + 4.) / 20, true);

        init_QLineEdit(&summary[i][2], 1. / 5, 1. / 20, 11. / 20, (i + 4.) / 20, true);
    }
}

void MainWindow::adjust_summary() {
    for (unsigned int i = 0; i != summary.size(); ++i) {
        summary[i][0]->move(width / 20., height * (i + 4.) / 20);
        summary[i][1]->move(width / 6., height * (i + 4.) / 20);
        summary[i][2]->move(width * 11 / 20., height * (i + 4.) / 20);
    }
}

void MainWindow::del_summary() {
    for (auto &a : summary)
        for (auto &b : a)
            delete b;
}

void MainWindow::update_today() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    today->setText(tr((date_time_format(tm.tm_mday) + to_string(tm.tm_mday) + "/" +
                       date_time_format(tm.tm_mon + 1) + to_string(tm.tm_mon + 1) + "/" +
                       to_string(tm.tm_year + 1900)).c_str()));
}
template<class T> void colourer (T* thing, int r, int g, int b){
    thing->setStyleSheet(("background:rgb(" + to_string(r) + ", " + to_string(g) + ", " + to_string(b) + ");").c_str());
}

void MainWindow::print_summary_from(int n) {
    update_today();
    if (!upd.ndays()) {
        summary[0][0]->setText(tr(to_string(0).c_str()));
        summary[0][1]->setText(today->text());
        summary[0][2]->setText(cents_to_streur(0).c_str());
        colourer(summary[0][0], 255, 255, 255);
        colourer(summary[0][1], 255, 255, 255);
        colourer(summary[0][2], 255, 255, 255);
        return;
    }
    static const vector<int> cols{160, 176, 192, 208, 224, 208, 192, 176};
    bool still_nthng = today->text().toStdString() != upd[0].date;
    int add = still_nthng ? 1 : 0;
    for (unsigned int i = 0; i < qMin(static_cast<unsigned int>(summary.size()), upd.ndays() + add); ++i) {
        if (still_nthng && !(i + n)) {
            summary[i][0]->setText(tr(to_string(0).c_str()));
            summary[i][1]->setText(today->text());
            summary[i][2]->setText(cents_to_streur(0).c_str());
        } else {
            summary[i][0]->setText(tr(to_string(i + n).c_str()));
            summary[i][1]->setText(tr(upd[i + n - add].date.c_str()));
            summary[i][2]->setText(cents_to_streur(upd[i + n - add].gain).c_str());
        }
        int min = ::stod(summary[0][0]->text().toStdString());
        static int csz = cols.size();
        int cind = i + min;
        colourer(summary[i][0], cols[cind % csz], cols[cind % csz], cols[cind % csz]);
        colourer(summary[i][1], cols[cind % csz], cols[cind % csz], cols[cind % csz]);
        colourer(summary[i][2], cols[cind % csz], cols[cind % csz], cols[cind % csz]);
    }
}

void MainWindow::refresh_all() {
    update_today();
    print_summary_from(0);
    current->setText(QString(("Totale nella pennetta: " + cents_to_streur(upd.get_pen())).c_str()));
    found->setText(tr(""));
    pen->setText(tr(""));
    adjust_summary();
}

void MainWindow::add_cents_found() {
    update_today();
    string strfound = found->text().toStdString();
    if (strfound == string("")) return;
    string strdate = today->text().toStdString();
    int dfound = ::stod(strfound);
    if (!dfound) {
        refresh_all();
        return;
    }
    if (dfound + upd.get_pen() > 9999) {
        QMessageBox::about(this, tr("TE PIACEREBBE CHICCO"), tr("Non credo che la pennetta\ncontenga più di 99,99€"));
        refresh_all();
        return;
    }
    if (!upd.ndays() || strdate != upd[0].date) {
        upd.new_day(strdate);
    }
    upd.update_found_today(dfound);
    upd.update_pen(dfound);
    refresh_all();
}

void MainWindow::remove_cents_found() {
    update_today();
    string strfound = found->text().toStdString();
    if (strfound == string("")) return;
    string strdate = today->text().toStdString();
    int dfound = ::stod(strfound);
    if (!dfound) {
        refresh_all();
        return;
    }
    if (!upd.ndays() || strdate != upd[0].date) {
        QMessageBox::about(this, tr("A DEFICIENTE"), tr("Non hai trovato nulla oggi,\nnon puoi togliere soldi"));
        refresh_all();
        return;
    }
    if (dfound > upd.get_pen()) {
        QMessageBox::about(this, tr("A CEREBROLESO"), tr("Non puoi togliere più soldi\ndi quelli nella pennetta"));
        refresh_all();
        return;
    } else if (dfound > upd[0].gain) {
        QMessageBox::about(this, tr("A RITARDATO"), tr("Non puoi correggere togliendo\npiù soldi di quelli segnati"));
        refresh_all();
        return;
    }
    upd.update_found_today(-dfound);
    upd.update_pen(-dfound);
    if (upd[0].gain == 0) upd.delete_last_day();
    refresh_all();
}

void MainWindow::add_cents_pen() {
    update_today();
    string strpen = pen->text().toStdString();
    if (strpen == string("")) return;
    int dpen = ::stod(strpen);
    if (dpen + upd.get_pen() > 9999) {
        QMessageBox::about(this, tr("MA ALLORA SEI SCEMO IN TESTA"), tr("Non credo che la pennetta\ncontenga più di 99,99€"));
        refresh_all();
        return;
    }
    upd.update_pen(dpen);
    refresh_all();
}

void MainWindow::remove_cents_pen() {
    update_today();
    string strpen = pen->text().toStdString();
    if (strpen == string("")) return;
    int dpen = ::stod(strpen);
    if (dpen > upd.get_pen()) {
        QMessageBox::about(this, tr("SCIOCCHERELLO"), tr("Non puoi togliere più soldi\ndi quelli nella pennetta"));
        refresh_all();
        return;
    }
    upd.update_pen(-dpen);
    refresh_all();
}

void MainWindow::scroll_up() {
    int min = ::stod(summary[0][0]->text().toStdString());
    if (min == 0) return;
    /*
    for (auto &w : summary) {
        for (auto wi : w) {
            animated_move(wi, 200, wi->pos(), QPoint(wi->x(), wi->y() - height / 20.), SLOT(adjust_summary()));
        }
    }
    */
    print_summary_from(min - 1);
    adjust_summary();
}

void MainWindow::scroll_down() {
    if (!upd.ndays()) return;
    int min = ::stod(summary[0][0]->text().toStdString());
    update_today();
    bool still_nthng = today->text().toStdString() != upd[0].date;
    int add = still_nthng ? 1 : 0;
    if (upd.ndays() <= static_cast<unsigned int>(min + summary.size() - add)) return;
    /*
    for (unsigned int i = 0; i != summary.size(); ++i) {
        for (unsigned int j = 0; j != summary[i].size(); ++j) {
            auto wi = summary [i][j];

          if (i == summary.size() - 1 && j == summary[i].size() - 1)
            animated_move(wi, 200, wi->pos(), QPoint(wi->x(), height * (i + 5.) / 20), SLOT(adjust_summary()));
          else
            animated_move(wi, 200, wi->pos(), QPoint(wi->x(), height * (i + 5.) / 20));

            //animated_move(wi, 200, wi->pos(), QPoint(wi->x(), height * (i + 5.) / 20));
        }
    }
    */
    print_summary_from(min + 1);
    adjust_summary();
}


template <typename T>
void MainWindow::fade_in_widget(T* w, int time) {
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    w->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(time);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

template <typename T>
void MainWindow::fade_out_widget(T* w, int time) {
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    w->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(time);
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::InBack);
a->start(QPropertyAnimation::DeleteWhenStopped); 
}

template <typename T>
void MainWindow::animated_move(T* w, int time, const QPoint &p1, const QPoint &p2, const char* slt) {
    QPropertyAnimation *animation = new QPropertyAnimation(w, "pos"); // just change position
    animation->setDuration(time);
    animation->setStartValue(p1);
    animation->setEndValue(p2);
    if (string(slt) != "") connect(animation, SIGNAL(finished()), this, slt);
    // animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}
