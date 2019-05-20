#include <sstream>
#include <libasync/async.h>

class InputMux
{
    std::stringstream m_data;
    char              m_buf[64];

    async::handle_t m_shared_context;
    async::handle_t m_context;

    int m_nesting_level;

public:
    InputMux(async::handle_t shared, async::handle_t context) 
        : m_shared_context(shared), m_context(context), m_nesting_level(0) 
    {}

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

            s.append("\n");

            if (m_nesting_level == 0) {
                async::receive(m_shared_context, s.c_str(), s.size());
            } 
            else {
                async::receive(m_context, s.c_str(), s.size());
            }
        }

        if (!m_data) {
            m_data.clear();
        }
    }
};