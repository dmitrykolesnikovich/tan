#ifndef __TAN_READER_READER_H__
#define __TAN_READER_READER_H__
#include <string>
#include <vector>
#include <stdint.h>
namespace tanlang {

struct line_info;
class Reader final {
public:
    Reader() = default;
    ~Reader();

    void open(const std::string& file_name);
    // bool set_encoding(const std::string& encoding);
    std::string next_line() const;
    std::string get_line(unsigned idx) const;
    unsigned get_line_number() const;
    std::string get_filename() const;
    bool eof() const;

private:
    std::string _filename;
    std::vector<line_info*> _lines;
    // line number starts at 1
    // _curr_line store line number of the line read last time
    mutable unsigned _curr_line = 0;
};

}  // namespace tanlang

#endif  // __TAN_READER_READER_H__
