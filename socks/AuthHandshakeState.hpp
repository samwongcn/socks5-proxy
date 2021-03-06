#ifndef SOCKS_SOCKSAUTHHANDSHAKESTATE_HPP
#define SOCKS_SOCKSAUTHHANDSHAKESTATE_HPP

#include <memory>       // std::shared_ptr
#include <vector>       // std::vector
#include <tuple>        // std::tuple
#include <deque>      // std::deque, for buffer

#include <Socket.hpp>   // jelford::Socket
#include "AuthHandshakeState.hpp" // socks::AuthHandshakeState
#include "AcceptCommandState.hpp" // socks::AcceptCommandState

#include <iostream>     // std::cerr

namespace socks
{
    class AuthHandshakeState : public SessionState
    {
        private:
            unsigned short m_auth_method_count=0;

        public:
            AuthHandshakeState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buff)
                : SessionState(socket, buff)
            {
            }

            virtual std::string identify() { return "AuthHandshakeState"; }

            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::cerr << "AuthHandshakeState::handle_incoming_data()" << std::endl;
                std::cerr << "(buffer size: " << m_buffer->size() << ")" << std::endl;

                if (m_buffer->size() < 1 || m_buffer->size() < m_auth_method_count)
                    return m_no_change;

                if (m_auth_method_count < 1)
                {
                    auto auth_method_count = m_buffer->front();
                    m_buffer->pop_front();
                    std::cerr << static_cast<short>(auth_method_count) << " available auth methods" << std::endl;
                    m_auth_method_count = static_cast<short>(auth_method_count);
                    if (m_auth_method_count < 1)
                        std::cerr << "No auth methods -- there's going to be a problem." << std::endl;
                }
            
                if (m_buffer->size() < m_auth_method_count)
                    return m_no_change;


                bool no_auth_found=false;
                for (unsigned short i=0; i<m_auth_method_count; ++i)
                {
                    auto method = m_buffer->front();
                    m_buffer->pop_front();
                    if (method == 0x00)
                        no_auth_found = true;
                }
                
                if (!no_auth_found)
                    std::cerr << "Client isn't going to like no_auth..." << std::endl;

                std::shared_ptr<std::vector<unsigned char>> no_auth_reply(new std::vector<unsigned char>({0x05, 0x00}));

                std::cerr << "Sent no-auth reply" << std::endl;

                std::shared_ptr<socks::AcceptCommandState> command_accept_state(new AcceptCommandState(m_socket, m_buffer));

                decltype(m_no_read) listen_list {std::make_tuple(m_socket, command_accept_state)};
                decltype(m_no_write) write_list {std::make_tuple(m_socket, no_auth_reply)};
                return std::tie(listen_list, write_list, m_no_exceptions);
            }
    };
}

#endif
