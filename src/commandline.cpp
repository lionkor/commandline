#include "commandline.h"

#include "backends/BufferedBackend.h"
#include "backends/InteractiveBackend.h"
#include "impls.h"

#include <memory>
#include <utility>

Commandline::Commandline(const std::string& prompt) {
    if (impl::is_interactive()) {
        m_backend = std::unique_ptr<lk::Backend>(new lk::InteractiveBackend(prompt));
    } else {
        m_backend = std::unique_ptr<lk::Backend>(new lk::BufferedBackend(prompt));
    }
    m_backend->on_command = [this](lk::Backend& be) {
        if (on_command) {
            on_command(*this);
        }
    };
    m_backend->on_write = [this](const std::string& str) {
        if (on_write) {
            on_write(str);
        }
    };
    m_backend->on_autocomplete = [this](lk::Backend& be, std::string str, int n) {
        if (on_autocomplete) {
            return on_autocomplete(*this, std::move(str), n);
        } else {
            return std::vector<std::string> {};
        }
    };
}
