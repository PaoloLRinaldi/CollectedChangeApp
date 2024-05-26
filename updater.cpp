#include "updater.h"
#include <fstream>

using std::string;
using std::ifstream;

Updater::Updater()
    // f("/storage/emulated/0/ziopaperone/data.bin")
    // f("data.bin", true)
{
    // ifstream ftry("data.bin");
#ifdef __ANDROID__
    string filename = "/storage/emulated/0/ziopaperone/data.bin";
#else
    string filename = "/home/fp/Dropbox/ProgrammiCPaolo/ProgrammiCppPaolo/ziopap2_proj/ziopap2/data.bin";
#endif
    ifstream ftry(filename);
    if (!ftry.is_open()) {
        // f = new Bin("data.bin", true);
        f = new Bin(filename, true);
        f->write<unsigned int>(0);
        f->flush();
    } else {
        // f = new Bin("data.bin");
        f = new Bin(filename);
    }
}

unsigned int Updater::ndays() {
    int left = f->size() - sizeof(int);
    if (!left) return 0;

    if (left % daysize)
        throw std::domain_error("File in wrong format");

    return left / daysize;
}

const DateGain Updater::operator[](int index) {
    auto nd = ndays();
    if (index < 0 || static_cast<int>(nd) <= index)
        throw std::out_of_range("Index out of range!");
    return DateGain{f->get_string(10, sizeof(int) + (nd - 1 - index) * daysize),
                    f->get_value<int>()};
}


void Updater::delete_last_day() {
    auto data = f->get_values<unsigned char>(f->size(), 0);
    delete f;
#ifdef __ANDROID__
    string filename = "/storage/emulated/0/ziopaperone/data.bin";
#else
    string filename = "/home/fp/Dropbox/ProgrammiCPaolo/ProgrammiCppPaolo/ziopap2_proj/ziopap2/data.bin";
#endif
    f = new Bin(filename, true);
    f->write_many<unsigned char>(data.begin(), data.end() - daysize);
}
