#include "commandline.h"

#include "impls.h"

Commandline::Commandline(const std::string& prompt)
    : m_prompt(prompt) {
    impl::init_terminal();
    m_io_thread = std::thread(&Commandline::io_thread_main, this);
}

Commandline::~Commandline() {
    m_shutdown.store(true);
    m_to_write_cond.notify_one();
    m_io_thread.join();
    impl::reset_terminal();
}

void Commandline::set_prompt(const std::string& p) {
    if (impl::is_interactive())
        m_prompt = p;
}

std::string Commandline::prompt() const {
    return m_prompt;
}

void Commandline::add_to_current_buffer(char c) {
    m_current_buffer.insert(m_cursor_pos, 1, c);
    ++m_cursor_pos;
    update_current_buffer_view();
    m_history_temp_buffer = m_current_buffer;
}

void Commandline::update_current_buffer_view() {
    printf("\x1b[2K\x1b[0G%s%s\x1b[%zuG", m_prompt.c_str(), m_current_buffer.c_str(), m_prompt.size() + m_cursor_pos + 1);
    fflush(stdout);
}

void Commandline::go_back() {
    if (m_history.empty()) {
        return;
    }
    go_back_in_history();
    std::lock_guard<std::mutex> guard_history(m_history_mutex);
    if (m_history_index == m_history.size()) {
        m_current_buffer = m_history_temp_buffer;
    } else {
        m_current_buffer = m_history.at(m_history_index);
    }
    m_cursor_pos = (int)m_current_buffer.size();
    update_current_buffer_view();
}

void Commandline::go_forward() {
    if (m_history.empty()) {
        return;
    }
    go_forward_in_history();
    std::lock_guard<std::mutex> guard_history(m_history_mutex);
    if (m_history_index == m_history.size()) {
        m_current_buffer = m_history_temp_buffer;
    } else {
        m_current_buffer = m_history.at(m_history_index);
    }
    m_cursor_pos = (int)m_current_buffer.size();
    update_current_buffer_view();
}

void Commandline::go_left() {
    if (m_cursor_pos > 0 && !m_current_buffer.empty()) {
        --m_cursor_pos;
        update_current_buffer_view();
    }
}

void Commandline::go_right() {
    if (size_t(m_cursor_pos) < m_current_buffer.size()) {
        ++m_cursor_pos;
        update_current_buffer_view();
    }
}

void Commandline::go_to_begin() {
    if (m_cursor_pos > 0 && !m_current_buffer.empty()) {
        m_cursor_pos = 0;
        update_current_buffer_view();
    }
}

void Commandline::go_to_end() {
    m_cursor_pos = (int)m_current_buffer.size();
    update_current_buffer_view();
}

void Commandline::handle_tab(std::unique_lock<std::mutex>& guard, bool forward) {
    forward = impl::is_shift_pressed(forward);

    if (m_autocomplete_suggestions.empty()) { // ensure we don't have suggestions already
        if (on_autocomplete) { // request new ones if we don't
            // we need to unlock the mutex here, because we call back into "userspace",
            // which may want to print, which in turn then wants this mutex.
            guard.unlock();
            m_autocomplete_suggestions = on_autocomplete(*this, m_current_buffer, m_cursor_pos);
            guard.lock();
            m_autocomplete_index = 0;
            m_buffer_before_autocomplete = m_current_buffer;
        }
        if (m_autocomplete_suggestions.empty()) {
            return;
        }
    } else { // we already have suggestions, so tab will loop through them
        if (forward) {
            ++m_autocomplete_index;
        } else {
            --m_autocomplete_index;
        }
        m_autocomplete_index %= m_autocomplete_suggestions.size();
    }

    // display current suggestion
    m_current_buffer = m_autocomplete_suggestions.at(m_autocomplete_index);
    go_to_end();
}

void Commandline::clear_suggestions() {
    m_autocomplete_suggestions.clear();
    m_autocomplete_index = 0;
}

bool Commandline::cancel_autocomplete_suggestion() {
    if (!m_autocomplete_suggestions.empty()) {
        m_current_buffer = m_buffer_before_autocomplete;
        m_buffer_before_autocomplete.clear();
        clear_suggestions();
        go_to_end();
        return true;
    }
    return false;
}

void Commandline::handle_backspace() {
    if (!cancel_autocomplete_suggestion() && !m_current_buffer.empty()) {
        if (--m_cursor_pos < 0) {
            m_cursor_pos = 0;
        }
        m_current_buffer.erase(m_cursor_pos, 1);
        update_current_buffer_view();
    }
}

void Commandline::handle_delete() {
    if (!m_current_buffer.empty() && m_cursor_pos < int(m_current_buffer.size())) {
        m_current_buffer.erase(m_cursor_pos, 1);
        update_current_buffer_view();
    }
}

void Commandline::handle_escape_sequence(std::unique_lock<std::mutex>& guard) {
    int c2 = impl::getchar_no_echo();
    if (m_key_debug) {
        fprintf(stderr, "c2: 0x%.2x\n", c2);
    }

#if defined(UNIX)
    int c3 = impl::getchar_no_echo();
    if (m_key_debug) {
        fprintf(stderr, "c3: 0x%.2x\n", c3);
    }
    if (c2 == '[' && history_enabled()) {
        if (c3 == 'A') {
            // up / back
            go_back();
        } else if (c3 == 'B') {
            // down / forward
            go_forward();
        } else if (c3 == 'D') {
            // left
            go_left();
        } else if (c3 == 'C') {
            // right
            go_right();
        } else if (c3 == 0x48 || c3 == 0x31) {
            // HOME
            go_to_begin();
        } else if (c3 == 0x46) {
            // END
            go_to_end();
        } else if (c3 == 0x33) {
            // DEL
            int c4 = impl::getchar_no_echo();
            if (c4 == '~') {
                handle_delete();
            }
        } else if (c3 == 0x5a) {
            // SHIFT+TAB
            handle_tab(guard, false);
        }
#elif defined(WINDOWS)
    if (c2 == 'H') {
        // up / back
        go_back();
    } else if (c2 == 'P') {
        // down / forward
        go_forward();
    } else if (c2 == 'K') {
        // left
        go_left();
    } else if (c2 == 'M') {
        // right
        go_right();
    } else if (c2 == 0x47) {
        // HOME
        go_to_begin();
    } else if (c2 == 0x4f) {
        // END
        go_to_end();
    } else if (c2 == 0x53) {
        // DEL
        handle_delete();
#endif
    } else {
        add_to_current_buffer(c2);
#if defined(UNIX)
        add_to_current_buffer(c3);
#endif
    }
}

void Commandline::input_thread_main() {
    while (!m_shutdown.load()) {
        if (!impl::is_interactive()) {
            break;
        }
        int c = 0;
        while (c != '\n' && c != '\r' && !m_shutdown.load()) {
            if (!impl::is_interactive()) {
                break;
            }
            update_current_buffer_view();
            c = impl::getchar_no_echo();
            if (m_key_debug) {
                fprintf(stderr, "c: 0x%.2x\n", c);
            }
            std::unique_lock<std::mutex> guard(m_current_buffer_mutex);
            if (c != '\t') {
            }
            if (c == '\b' || c == 127) { // backspace or other delete sequence
                handle_backspace();
                clear_suggestions();
            } else if (c == '\t') {
                handle_tab(guard, true);
            } else if (isprint(c)) {
                add_to_current_buffer(c);
                clear_suggestions();
            } else if (c == 0x1b) { // escape sequence
#if defined(UNIX)
                handle_escape_sequence(guard);
#else
                cancel_autocomplete_suggestion();
#endif
            } else if (c == 0xe0) { // escape sequence
#if defined(WINDOWS)
                handle_escape_sequence(guard);
#endif
            } else {
                if (m_key_debug) {
                    fprintf(stderr, "unhandled: 0x%.2x\n", c);
                }
            }
        }
        bool shutdown = m_shutdown.load();
        // check so we dont do anything on the last pass before exit
        if (!shutdown) {
            if (history_enabled() && m_current_buffer.size() > 0) {
                add_to_history(m_current_buffer);
            }
            std::lock_guard<std::mutex> guard(m_to_read_mutex);
            m_to_read.push(m_current_buffer);
            m_current_buffer.clear();
            m_cursor_pos = 0;
            update_current_buffer_view();
        }
        if (on_command && !shutdown) {
            on_command(*this);
        }
    }
}

void Commandline::io_thread_main() {
    if (impl::is_interactive()) {
        std::thread input_thread(&Commandline::input_thread_main, this);
        input_thread.detach();
    }
    while (!m_shutdown.load()) {
        std::unique_lock<std::mutex> guard(m_to_write_mutex);
        m_to_write_cond.wait(guard, [&] { return !m_to_write.empty() || m_shutdown.load(); });
        if (!m_to_write.empty()) {
            auto to_write = m_to_write.front();
            m_to_write.pop();
            std::lock_guard<std::mutex> guard2(m_current_buffer_mutex);
            printf("\x1b[2K\x1b[0G%s\n%s%s\x1b[%zuG", to_write.c_str(), m_prompt.c_str(), m_current_buffer.c_str(), m_prompt.size() + m_cursor_pos + 1);
            fflush(stdout);
            if (on_write) {
                on_write(to_write);
            }
        }
    }
    // after all this, we have to output all that remains in the buffer, so we dont "lose" information
    std::unique_lock<std::mutex> guard(m_to_write_mutex);
    while (!m_to_write.empty()) {
        auto to_write = m_to_write.front();
        m_to_write.pop();
        printf("\x1b[2K\x1b[0G%s", to_write.c_str());
        if (on_write) {
            on_write(to_write);
        }
    }
    fflush(stdout);
}

void Commandline::add_to_history(const std::string& str) {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    // if adding one entry would put us over the limit,
    // remove the oldest one before adding one
    if (m_history.size() + 1 > m_history_limit) {
        m_history.erase(m_history.begin());
    }
    m_history.push_back(str);
    m_history_index = m_history.size(); // point to one after last one
    m_history_temp_buffer.clear();
}

void Commandline::go_back_in_history() {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    if (m_history_index == 0) {
        return;
    } else {
        --m_history_index;
    }
}

void Commandline::go_forward_in_history() {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    if (m_history_index == m_history.size()) {
        return;
    } else {
        ++m_history_index;
    }
}

void Commandline::write(const std::string& str) {
    std::lock_guard<std::mutex> guard(m_to_write_mutex);
    m_to_write.push(str);
    m_to_write_cond.notify_one();
}

bool Commandline::has_command() const {
    std::lock_guard<std::mutex> guard(m_to_read_mutex);
    return !m_to_read.empty();
}

std::string Commandline::get_command() {
    std::lock_guard<std::mutex> guard(m_to_read_mutex);
    auto res = m_to_read.front();
    m_to_read.pop();
    return res;
}

void Commandline::set_history_limit(size_t count) {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    m_history_limit = count;
}

size_t Commandline::history_size() const {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    return m_history.size();
}

void Commandline::clear_history() {
    std::lock_guard<std::mutex> guard(m_history_mutex);
    m_history.clear();
}

void Commandline::enable_key_debug() {
    m_key_debug = true;
}

void Commandline::disable_key_debug() {
    m_key_debug = false;
}
