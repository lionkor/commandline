#pragma once
#include <string>
namespace lk {
// These functions are used for UTF-8 string manipulation.
// They do not throw exceptions because in normal usage with the console, invalid inputs are not expected.
// However, even if they receive invalid input (from a file buffer or a console bug), they are still safe due to range checks.
// They never return values larger than the size of the string, so if they receive invalid input, the maximum effect would be display artifacts.
namespace utf_8 {

    // returns offset to previous symbol
    size_t get_prev_symbol_offset(const std::string& str, size_t pos) {
        if (!pos)
            return 0;

        // move back as normal
        size_t res = 1;
        while (--pos) {
            // this condition checks if current symbol is codepoint
            // if not then breaks loop
            if ((str[pos] & 0xc0) != 0x80) {
                break;
            }
            ++res;
        }
        return res;
    }

    // returns offset to next symbol
    size_t get_next_symbol_offset(const std::string& str, size_t pos) {
        size_t size = str.size();
        // move front as normal
        size_t res = 1;
        ++pos;

        if (pos < size) {
            // this condition checks if current symbol is codepoint
            // if yes, then it again moves forward to skip all codepoint bytes
            while ((str[pos] & 0xc0) == 0x80) {
                ++res;
                ++pos;
                if (pos == size)
                    break;
            }
        }
        return res;
    }

    // Returns offset to the nth character in the string
    size_t get_char_offset(const std::string& str, size_t chars_count, size_t offset = 0) {
        size_t res = 0;
        while (chars_count) {
            size_t add_off = get_next_symbol_offset(str, offset);
            res += add_off;
            offset += add_off;
            --chars_count;
        }
        return res;
    }

    // When a UTF-8 symbol represents an ASCII symbol, it will be 1 byte with the most significant bit (MSB) unset.
    // When it represents a codepoint beyond ASCII, it will have the MSB set,
    // and the last byte will have the 10xxxxxx bits set to mark it as end of the codepoint.
    // Therefore, this loop will count all ASCII symbols and the last byte of each codepoint.
    size_t length(const char* str, size_t siz = SIZE_MAX) {
        size_t len = 0;

        while (*str && len != siz) {
            len += (*str++ & 0xc0) != 0x80;
        }
        return len;
    }

    size_t length(const std::string& str) {
        return length(str.c_str(), str.size());
    }

} // namespace utf_8
}
