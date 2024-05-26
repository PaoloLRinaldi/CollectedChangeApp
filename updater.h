#ifndef UPDATER_H
#define UPDATER_H
#include <vector>
#include <string>
#include "readwritebin.h"

struct DateGain {
    std::string date;
    int gain;
};

class Updater {
public:
    Updater();

    const DateGain operator[](int index);

    int get_pen() { return f->get_value<int>(0); }
    void update_pen(int cents) {
        f->write<int>(get_pen() + cents, 0);
        f->flush();
    }

    void new_day(const std::string &date) {
        f->write_string(date, f->size());
        f->write<int>(0);
        f->flush();
    }

    void update_found_today(int cents) {
        Bin::size_type pos = f->size() - sizeof(int);
        f->write<int>(f->get_value<int>(pos) + cents, pos);
        f->flush();
    }

    void delete_last_day();

    unsigned int ndays();

    ~Updater() { delete f; }
private:
    Bin *f = nullptr;

    static constexpr int daysize = 10 + sizeof(int);

};

#endif // UPDATER_H
