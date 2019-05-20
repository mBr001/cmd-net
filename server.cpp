#include <iostream>
#include <sstream>
#include <memory>

#include <boost/asio.hpp>
#include <libasync/async.h>

#include "input_mux.h"

class Updatable
{
public:
    virtual void update() = 0;
};

class Session : public std::enable_shared_from_this<Session>
{
    Updatable* m_owner;
    
    boost::asio::ip::tcp::socket m_socket;
    char m_buf[1024];

    async::handle_t m_shared_context;
    async::handle_t m_private_context;

    InputMux *m_mux;

public:
    Session(Updatable* owner,
            boost::asio::ip::tcp::socket sock, 
            async::handle_t shared, 
            int bulk)
        : m_owner(owner), m_socket(std::move(sock)), m_shared_context(shared)

    {
        async::handle_t m_private_context = async::connect(bulk);
        m_mux = new InputMux(m_shared_context, m_private_context);
    }

    ~Session() 
    {
        m_socket.close();
        async::disconnect(m_private_context);
        delete m_mux;
        m_owner->update();
    }

    void start() 
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        m_socket.async_read_some(boost::asio::buffer(m_buf),
            [this, self](const boost::system::error_code& err, std::size_t n)
            {
                if (!err) {
                    m_mux->receive(m_buf, n);
                    do_read();
                }
            });
    }
};

class Server : public Updatable
{
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket   m_socket;

    async::handle_t m_shared_context;
    int m_bulk;
    int m_session_counter;

public:
    Server(boost::asio::io_service& service, short port, int bulk) 
        : 
        m_acceptor(service, 
                     boost::asio::ip::tcp::endpoint(
                         boost::asio::ip::tcp::v4(), port)), 
        m_socket(service),
        m_shared_context(nullptr),
        m_bulk(bulk),
        m_session_counter(0)
    {
        async::init();
        do_accept();
    }

    ~Server()
    {
        m_socket.close();
        async::disconnect(m_shared_context);
        async::close();
    }

    void update() override
    {
        if (--m_session_counter == 0) {
            async::disconnect(m_shared_context);
            m_shared_context = nullptr;
        }
    }

private:
    void do_accept()
    {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& err) 
            {
                if (err) {
                    return;
                }
                
                if (!m_shared_context) {
                    m_shared_context = async::connect(m_bulk);
                }

                std::make_shared<Session>(this,
                                          std::move(m_socket), 
                                          m_shared_context,
                                          m_bulk)->start();
                ++m_session_counter;
                this->do_accept();
            });
    }
};

int main(int argc, char* argv[]) 
{
    if (argc != 3) {
        std::cout << "too few arguments" << std::endl;
        return 1;
    }

    try {
        int port = std::stoi(argv[1]);
        int bulk = std::stoi(argv[2]);

        boost::asio::io_service service;
        Server server(service, port, bulk);
        service.run();
    }
    catch(std::exception& e) {
        std::cout << e.what();
    }

    return 0;
}
