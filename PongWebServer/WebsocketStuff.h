#pragma once
#include "inc.h"


typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;

class NoTLSServer {
public:
    NoTLSServer() {
        // Initialize Asio Transport
        m_server.clear_access_channels(websocketpp::log::alevel::all);
        m_server.set_access_channels(websocketpp::log::elevel::rerror | websocketpp::log::elevel::fatal);
        
        m_server.init_asio();

        // Register handler callbacks
        m_server.set_open_handler(bind(&NoTLSServer::on_open, this, ::_1));
        m_server.set_close_handler(bind(&NoTLSServer::on_close, this, ::_1));
        m_server.set_message_handler(bind(&NoTLSServer::on_message, this, ::_1, ::_2));
    }

    void setFunctions(void(*onOpenFunction)(connection_hdl hdl), void(*onCloseFunction)(connection_hdl hdl), void(*onMessageFunction)(connection_hdl hdl, server::message_ptr msg)) {
        onOpen = onOpenFunction;
        onClose = onCloseFunction;
        onMessage = onMessageFunction;
    }

    void run(uint16_t port) {
        // listen on specified port
        m_server.listen(port);

        // Start the server accept loop
        m_server.start_accept();

        // Start the ASIO io_service run loop
        try {
            m_server.run();
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void stop() {
        if (!m_server.stopped()) {
            m_server.stop_listening();
            m_server.stop();
        }
    }

    void on_open(connection_hdl hdl) {
        //When the client connects
        onOpen(hdl);
    }

    void on_close(connection_hdl hdl) {
        //When the client disconnects
        onClose(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        //Client has a message
        onMessage(hdl, msg);
    }

    void sendData(connection_hdl hdl, string payload) {
        websocketpp::lib::error_code ec;
        m_server.send(hdl, payload, websocketpp::frame::opcode::text, ec);
        
    }

    server::connection_ptr getConnection(connection_hdl hdl) {
        return m_server.get_con_from_hdl(hdl);
    }
    
private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;

    server m_server;
    void (*onOpen)(connection_hdl hdl);
    void (*onClose)(connection_hdl hdl);
    void (*onMessage)(connection_hdl hdl, server::message_ptr msg);
};

class multithreadedServerNoTLS {

};