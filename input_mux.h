#include <sstream>
#include <libasync/async.h>

class InputMux
{
    std::stringstream m_data;
    char              m_buf[64];

    int m_nesting_level;

    async::handle_t m_shared;
    async::handle_t m_private;

public:
    InputMux(async::handle_t h) : m_shared(h)
    {
        m_private = async::connect(28);
    }

    void receive(const char* data, std::size_t n)
    {
        m_data.write(data, n);

        std::string s;
        while(m_data.getline(m_buf, 64))
        {
            s = m_buf;
            if (s == "{") {
                ++m_nesting_level;
            } 
            else if (s == "}") {
                --m_nesting_level;
            } 
            else if (m_nesting_level == 0) {
                async::receive(m_shared, s.c_str(), s.size());
            } 
            else {
                async::receive(m_private, s.c_str(), s.size());
            }
        }
    }

    ~InputMux() {
        async::disconnect(m_private);
    }
};