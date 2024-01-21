#include "ClientHandleHelper.hpp"
TCPclient* first_client_holder;
std::atomic_uint64_t TCPsession::id_gen;
bool TCPsession::do_log_connection_errors = false;