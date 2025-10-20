/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <openssl/evp.h>
#include <openssl/x509.h>

#include <library/list_array.hpp>
#include <src/api/chat.hpp>
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/config.hpp>
#include <src/api/packets/client_bound/login.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>
#include <src/util/mojang/api/hash256.hpp>

#define OPENSSL_CHECK(OPERATION, console_output)    \
    if ((OPERATION) <= 0) {                         \
        api::log::error("OpenSSL", console_output); \
        throw std::runtime_error(console_output);   \
    }

#define OPENSSL_CHECK_SESSION(OPERATION, console_output) \
    if ((OPERATION) <= 0) {                              \
        api::log::error("OpenSSL", console_output);      \
        continue;                                        \
    }

namespace copper_server::build_in_plugins::base {
    //provides and manages chat system
    struct CommunicationCorePlugin : public plugin_auto_register<"base/communication_core", CommunicationCorePlugin> {
        fast_task::task_mutex messages_order;

        struct message_identifier {
            std::variant<int32_t, std::array<uint8_t, 256>> id;
        };

        list_array<message_identifier> latest_messages;

        void add_message(std::array<uint8_t, 256>&& val) {
            latest_messages.emplace_back(std::move(val));
            if (latest_messages.size() > 20)
                latest_messages.pop_front();
        }

        void add_message(int32_t val) {
            latest_messages.emplace_back(std::move(val));
            if (latest_messages.size() > 20)
                latest_messages.pop_front();
        }

        struct EVP_PKEY_str_free {
            void operator()(EVP_PKEY* key) {
                EVP_PKEY_free(key);
            }
        };

        struct EVP_MD_CTX_str_free {
            void operator()(EVP_MD_CTX* ctx) {
                EVP_MD_CTX_free(ctx);
            }
        };

        static bool signature_check(api::packets::server_bound::play::chat_session_update& packet, base_objects::shared_client_data& client) {
            bool res = false;
            api::mojang::get_mojang_certificate_public_keys([&](auto& keys) {
                for (auto& key : keys) {
                    const unsigned char* p = (const unsigned char*)key.data();
                    std::unique_ptr<EVP_PKEY, EVP_PKEY_str_free> pkey{d2i_PUBKEY(NULL, &p, (long)key.size())};
                    if (!pkey) {
                        res = false;
                        continue;
                    }
                    std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_str_free> ctx{EVP_MD_CTX_new()};
                    OPENSSL_CHECK_SESSION(EVP_DigestVerifyInit(ctx.get(), NULL, EVP_sha1(), NULL, pkey.get()), "Failed to use EVP_DigestVerifyInit");
                    OPENSSL_CHECK_SESSION(EVP_DigestVerifyUpdate(ctx.get(), &packet.uuid, sizeof(packet.uuid)), "Failed to use EVP_DigestVerifyUpdate");
                    OPENSSL_CHECK_SESSION(EVP_DigestVerifyUpdate(ctx.get(), &packet.expiries_at, sizeof(packet.expiries_at)), "Failed to use EVP_DigestVerifyUpdate");
                    OPENSSL_CHECK_SESSION(EVP_DigestVerifyUpdate(ctx.get(), packet.public_key.data(), packet.public_key.size()), "Failed to use EVP_DigestVerifyUpdate");
                    int result = EVP_DigestVerifyFinal(ctx.get(), packet.key_signature.data(), packet.key_signature.size());
                    OPENSSL_CHECK_SESSION(result, "Failed to use EVP_DigestVerifyFinal");
                    res = result == 1;
                    if (res)
                        return;
                }
            });
            return res;
        }

        static bool signature_check(api::packets::server_bound::play::chat& packet, base_objects::shared_client_data& client) {
            if (!packet.signature)
                return false;
            std::unique_ptr<EVP_PKEY, EVP_PKEY_str_free> key;
            client.packets_state.get_play_data([&key](auto& data) {
                if (data.signature) {
                    const unsigned char* p = data.signature->public_key.data();
                    key.reset(d2i_PUBKEY(NULL, &p, (long)data.signature->public_key.size()));
                }
            });

            if (!key) {
                return false;
            }
            std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_str_free> ctx{EVP_MD_CTX_new()};
            OPENSSL_CHECK(EVP_DigestVerifyInit(ctx.get(), NULL, EVP_sha256(), NULL, key.get()), "Failed to use EVP_DigestVerifyInit");

            static int32_t check = 1;
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &check, sizeof(check)), "Failed to use EVP_DigestVerifyUpdate");
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &client.data->uuid, sizeof(client.data->uuid)), "Failed to use EVP_DigestVerifyUpdate");
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &client.packets_state.local_chat_counter, sizeof(client.packets_state.local_chat_counter)), "Failed to use EVP_DigestVerifyUpdate");
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &packet.salt, sizeof(packet.salt)), "Failed to use EVP_DigestVerifyUpdate");
            packet.timestamp /= 1000; //milli to seconds
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &packet.timestamp, sizeof(packet.timestamp)), "Failed to use EVP_DigestVerifyUpdate");
            int32_t message_len = (int32_t)packet.message.value.size();
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &message_len, sizeof(message_len)), "Failed to use EVP_DigestVerifyUpdate");
            OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), packet.message.value.data(), message_len), "Failed to use EVP_DigestVerifyUpdate");

            client.packets_state.get_play_data([&ctx](auto& data) {
                int32_t signs_len = (int32_t)data.last_seen_messages.size();
                OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &signs_len, sizeof(signs_len)), "Failed to use EVP_DigestVerifyUpdate");
                for (auto& it : data.last_seen_messages)
                    OPENSSL_CHECK(EVP_DigestVerifyUpdate(ctx.get(), &it.signature, sizeof(it.signature)), "Failed to use EVP_DigestVerifyUpdate");
            });

            int result = EVP_DigestVerifyFinal(ctx.get(), packet.signature.value().data(), 256);
            OPENSSL_CHECK(result, "Failed to use EVP_DigestVerifyFinal");
            return result == 1;
        }

        static bool signature_check(api::packets::server_bound::play::chat_command_signed& packet, base_objects::shared_client_data& client) {
            return true; //TODO implement check for commands
        }

        void on_initialization(const plugin_registration_ptr&) override {
            api::configuration::get() ^ "communication_core" ^ "on_chat_disabled_message" |= enbt::compound{{"translation", "chat.disabled.options"}};
            api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature" |= enbt::value();
            api::configuration::get() ^ "communication_core" ^ "on_invalid_new_signature" |= enbt::compound{{"text", "Failed to verify new chat signature"}};
            api::configuration::get() ^ "communication_core" ^ "on_unload_message" |= enbt::compound{{"text", "The server closing."}};
            api::configuration::get() ^ "communication_core" ^ "allow_send_on" ^ "commands_only" |= true;
            api::configuration::get() ^ "communication_core" ^ "allow_send_on" ^ "hidden" |= false;
            _chat_disabled_notification() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_chat_disabled_message");
            _on_invalid_new_signature() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_invalid_new_signature");
            if (!(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature" ^ get_conf).is_none())
                _on_chat_invalid_signature() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature");
        }

        static Chat& _chat_disabled_notification() {
            static Chat ch;
            return ch;
        }

        static Chat& _on_invalid_new_signature() {
            static Chat ch;
            return ch;
        }

        static Chat& _on_chat_invalid_signature() {
            static Chat ch;
            return ch;
        }

        static void chat_disabled_notification(base_objects::shared_client_data& client) {
            client << api::packets::client_bound::play::system_chat{.content = _chat_disabled_notification()};
        }

        void on_load(const plugin_registration_ptr& _) override {
            latest_messages.clear();
            register_event(api::players::calls::on_player_kick, base_objects::events::priority::low, [](const api::players::personal<Chat>& message) {
                switch (message.player->packets_state.state) {
                case base_objects::shared_client_data::packets_state_t::protocol_state::handshake:
                case base_objects::shared_client_data::packets_state_t::protocol_state::initialization:
                case base_objects::shared_client_data::packets_state_t::protocol_state::status:
                    message.player->sendPacket(base_objects::network::response::disconnect());
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::login:
                    *message.player << api::packets::client_bound::login::login_disconnect{.reason = {message.data.ToStr()}};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::configuration:
                    *message.player << api::packets::client_bound::config::disconnect{.reason = message.data};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::play:
                    *message.player << api::packets::client_bound::play::disconnect{.reason = message.data};
                    break;
                }
                return false;
            });

            register_event(api::players::calls::on_player_ban, base_objects::events::priority::low, [](const api::players::personal<Chat>& message) {
                switch (message.player->packets_state.state) {
                case base_objects::shared_client_data::packets_state_t::protocol_state::handshake:
                case base_objects::shared_client_data::packets_state_t::protocol_state::initialization:
                case base_objects::shared_client_data::packets_state_t::protocol_state::status:
                    message.player->sendPacket(base_objects::network::response::disconnect());
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::login:
                    *message.player << api::packets::client_bound::login::login_disconnect{.reason = {message.data.ToStr()}};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::configuration:
                    *message.player << api::packets::client_bound::config::disconnect{.reason = message.data};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::play:
                    *message.player << api::packets::client_bound::play::disconnect{.reason = message.data};
                    break;
                }
                return false;
            });

            api::packets::processor(*this, [](api::packets::server_bound::play::command_suggestion&& packet, base_objects::shared_client_data& client) {
                base_objects::command_context context(client, true);
                auto suggestions = api::command::get_manager().request_suggestions(packet.command_text.value, context);
                auto pos = packet.command_text.value.find_last_of(" /");
                if (pos == std::string::npos)
                    pos = 0;
                client << api::packets::client_bound::play::command_suggestions{
                    .suggestion_transaction_id = packet.suggestion_transaction_id,
                    .start = (int32_t)pos,
                    .length = int32_t(packet.command_text.value.size() - pos),
                    .matches = suggestions
                                   .convert_fn([](auto& it) {
                                       return api::packets::client_bound::play::command_suggestions::match{.set = it};
                                   })
                };
            });
            api::packets::processor(*this, [](api::packets::server_bound::play::chat_command&& packet, base_objects::shared_client_data& client) {
                base_objects::command_context context(client, true);
                try {
                    api::command::get_manager().execute_command(packet.command, context);
                } catch (base_objects::command_exception& ex) {
                    std::string error_message = (std::string)packet.command;
                    std::string error_place(error_message.size() + 4, ' ');
                    error_place[0] = '\n';
                    error_place[error_place.size() - 2] = '\n';
                    error_place[error_place.size() - 1] = '\t';
                    if (ex.pos != -1)
                        error_place[ex.pos] = '^';
                    Chat res = error_message + error_place + ex.what;
                    res.SetColor("red");
                    client << api::packets::client_bound::play::system_chat{
                        .content = std::move(res),
                        .is_overlay = false
                    };
                } catch (const std::exception& ex) {
                    std::string error_message = (std::string)packet.command;
                    Chat res = error_message + "\n Failed to execute command, reason:\n\t" + ex.what();
                    res.SetColor("red");
                    client << api::packets::client_bound::play::system_chat{
                        .content = std::move(res),
                        .is_overlay = false
                    };
                }
            });

            api::packets::processor(*this, [](api::packets::server_bound::play::chat_session_update&& packet, base_objects::shared_client_data& client) {
                using piu = api::packets::client_bound::play::player_info_update;
                if (!signature_check(packet, client)) {
                    client << api::packets::client_bound::play::disconnect{.reason = _on_invalid_new_signature()};
                    return;
                }
                client.packets_state.get_play_data([&packet](base_objects::shared_client_data::packets_state_t::play_data_t& data) {
                    data.signature = std::make_unique<base_objects::shared_client_data::packets_state_t::play_data_t::signature_t>(
                        packet.uuid,
                        packet.expiries_at,
                        packet.public_key,
                        packet.key_signature
                    );
                });
                piu new_data;
                new_data.actions.push(piu::header{client.data->uuid});
                new_data.actions.push(
                    piu::initialize_chat{
                        .chat_session_id = packet.uuid,
                        .pub_key_expiries_timestamp = packet.expiries_at,
                        .public_key = packet.public_key,
                        .public_signature = packet.key_signature
                    }
                );
                api::players::iterate_online([&new_data, &client](base_objects::shared_client_data& oclient) {
                    if (&oclient != &client)
                        oclient << piu{new_data};
                    return false;
                });
            });
            api::packets::processor(*this, [this](api::packets::server_bound::play::chat&& packet, base_objects::shared_client_data& client) {
                switch (client.chat_mode) {
                case base_objects::shared_client_data::ChatMode::COMMANDS_ONLY:
                    if (!(api::configuration::get() ^ "communication_core" ^ "allow_send_on" ^ "commands_only" ^ get_conf))
                        return chat_disabled_notification(client);
                    break;
                case base_objects::shared_client_data::ChatMode::HIDDEN:
                    if (!(api::configuration::get() ^ "communication_core" ^ "allow_send_on" ^ "hidden" ^ get_conf))
                        return chat_disabled_notification(client);
                    break;
                case base_objects::shared_client_data::ChatMode::ENABLED:
                default:
                    break;
                }
                bool allow_chat_reports = !api::configuration::get().server.prevent_chat_reports;
                if (api::configuration::get().mojang.enforce_secure_profile)
                    if (!signature_check(packet, client)) {
                        if (!(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature" ^ get_conf).is_none())
                            client << api::packets::client_bound::play::system_chat{.content = _on_chat_invalid_signature()};
                        return;
                    }
                std::unique_lock lock(messages_order);
                api::packets::client_bound::play::player_chat msg;
                static int32_t glob_index = 0;
                if (glob_index == INT32_MAX)
                    glob_index = 0;
                msg.global_index = glob_index++;
                msg.sender = client.data->uuid;
                if (allow_chat_reports)
                    msg.signature = packet.signature;
                msg.message = std::move(packet.message);
                msg.timestamp = packet.timestamp;
                msg.salt = packet.salt;
                if (allow_chat_reports) {
                    msg.previous_messages.reserve(latest_messages.size());
                    for (auto& it : latest_messages) {
                        std::visit(
                            [&]<class T>(T& item) {
                                if constexpr (std::is_same_v<T, int32_t>)
                                    msg.previous_messages.push_back({.message_id_or_signature = {item + 1}});
                                else
                                    msg.previous_messages.push_back({.message_id_or_signature = {0, item}});
                            },
                            it.id
                        );
                    }
                }
                {
                    std::optional<Chat> tmp;
                    api::chat::custom_content_provider().notify(tmp, msg.message, client);
                    if (tmp)
                        msg.unsigned_content = std::move(tmp.value());
                    api::chat::custom_name_provider().notify(tmp, client);
                    if (tmp)
                        msg.sender_name = std::move(tmp.value());
                    else
                        msg.sender_name = client.name;
                }
                {
                    api::packets::client_bound::play::player_chat::partially_filtered filt;
                    api::chat::chat_filter().notify(msg.message, filt.filtered_characters, client);
                    if (filt.filtered_characters.size()) {
                        bool is_fully_filtered = true;
                        bool is_fully_unfiltered = true;
                        for (auto& it : filt.filtered_characters.data()) {
                            if (is_fully_filtered)
                                is_fully_filtered = it == UINT64_MAX;
                            if (!is_fully_filtered)
                                break;
                        }
                        for (auto& it : filt.filtered_characters.data()) {
                            if (is_fully_unfiltered)
                                is_fully_unfiltered = it == 0;
                            if (!is_fully_unfiltered)
                                break;
                        }
                        if (is_fully_filtered)
                            msg.filter = api::packets::client_bound::play::player_chat::fully_filtered{};
                        else if (is_fully_unfiltered)
                            msg.filter = api::packets::client_bound::play::player_chat::no_filter{};
                        else
                            msg.filter = std::move(filt);
                    } else
                        msg.filter = api::packets::client_bound::play::player_chat::no_filter{};
                }
                msg.type = base_objects::var_int32::chat_type{"minecraft:chat"};

                if (allow_chat_reports) {
                    if (packet.signature)
                        add_message(std::move(*packet.signature));
                    else
                        add_message(msg.global_index);
                }

                api::players::iterate_online([&msg, &client](base_objects::shared_client_data& oclient) {
                    if (oclient.chat_mode == base_objects::shared_client_data::ChatMode::ENABLED) {
                        api::packets::client_bound::play::player_chat personal{msg};
                        if (!oclient.enable_filtering)
                            personal.filter = api::packets::client_bound::play::player_chat::no_filter{};
                        if (!oclient.enable_chat_colors) {
                            if (personal.unsigned_content)
                                personal.unsigned_content->removeColorRecursive();
                            personal.sender_name.removeColorRecursive();
                        }

                        personal.index = oclient.packets_state.local_chat_counter++;
                        oclient << std::move(personal);
                    }
                    return false;
                });
            });

            api::packets::processor(*this, [this](api::packets::server_bound::play::chat_ack&& packet, base_objects::shared_client_data& client) {
                std::unique_lock lock(messages_order);
                client.packets_state.get_play_data([&](auto& data) {
                    int32_t count = std::max<int32_t>(packet.count, 20);
                    for (size_t i = 0; i < count; i++) {
                        std::visit(
                            [&]<class T>(T& it) {
                                if constexpr (std::is_same_v<std::array<uint8_t, 256>, T>)
                                    data.add_seen_signed_message(it);
                            },
                            latest_messages[i].id
                        );
                    }
                });
            });
            api::packets::processor(*this, [](api::packets::server_bound::play::chat_command_signed&& packet, base_objects::shared_client_data& client) {
                if (!signature_check(packet, client)) {
                    if (!(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature" ^ get_conf).is_none())
                        client << api::packets::client_bound::play::system_chat{.content = _on_chat_invalid_signature()};
                    return;
                }

                base_objects::command_context context(client, true);
                try {
                    api::command::get_manager().execute_command(packet.command, context);
                } catch (base_objects::command_exception& ex) {
                    std::string error_message = (std::string)packet.command;
                    std::string error_place(error_message.size() + 4, ' ');
                    error_place[0] = '\n';
                    error_place[error_place.size() - 2] = '\n';
                    error_place[error_place.size() - 1] = '\t';
                    if (ex.pos != -1)
                        error_place[ex.pos] = '^';
                    Chat res = error_message + error_place + ex.what;
                    res.SetColor("red");
                    client << api::packets::client_bound::play::system_chat{
                        .content = std::move(res),
                        .is_overlay = false
                    };
                } catch (const std::exception& ex) {
                    std::string error_message = (std::string)packet.command;
                    Chat res = error_message + "\n Failed to execute command, reason:\n\t" + ex.what();
                    res.SetColor("red");
                    client << api::packets::client_bound::play::system_chat{
                        .content = std::move(res),
                        .is_overlay = false
                    };
                }
            });
            api::log::info("Communication Core", "chat handlers registered.");
        }

        void on_unload(const plugin_registration_ptr& _) override {
            auto msg = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_unload_message");
            api::players::iterate_players([&msg](auto& it) {
                switch (it.packets_state.state) {
                case base_objects::shared_client_data::packets_state_t::protocol_state::handshake:
                case base_objects::shared_client_data::packets_state_t::protocol_state::initialization:
                case base_objects::shared_client_data::packets_state_t::protocol_state::status:
                    it.sendPacket(base_objects::network::response::disconnect());
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::login:
                    it << api::packets::client_bound::login::login_disconnect{.reason = {msg.ToStr()}};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::configuration:
                    it << api::packets::client_bound::config::disconnect{.reason = msg};
                    break;
                case base_objects::shared_client_data::packets_state_t::protocol_state::play:
                    it << api::packets::client_bound::play::disconnect{.reason = msg};
                    break;
                }
                return false;
            });
        }

        void on_commands_load(const plugin_registration_ptr& _, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            using cmd_pred_entity = base_objects::parsers::command::entity;
            browser.add_child("broadcast")
                .add_child({"message", "broadcast message", "Broadcast a message to all players"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                .set_callback("command.broadcast", [](const list_array<predicate>& args, base_objects::command_context& _) {
                    auto msg = Chat::parseToChat(std::get<pred_string>(args[0]).value);
                    api::players::iterate_online([&msg](base_objects::shared_client_data& context) {
                        context << api::packets::client_bound::play::system_chat{.content = msg};
                        return false;
                    });
                    return true;
                });
            browser.add_child("msg")
                .add_child("target", cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                .add_child({"message", "msg target message", "Send private message to specified player"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                .set_callback("command.msg", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(std::get<pred_string>(args[0]).value);
                    if (!target) {
                        context.executor << api::packets::client_bound::play::system_chat{.content = "Player not found"};
                        return false;
                    }
                    Chat message = Chat::parseToChat(std::get<pred_string>(args[1]).value);
                    context.executor << api::packets::client_bound::play::system_chat{.content = {"To " + target->name + ": ", message}};
                    *target << api::packets::client_bound::play::system_chat{.content = {"From " + context.executor.name + ": ", message}};
                    return true;
                });
            browser.add_child("chat")
                .add_child({"message", "chat message", "Send message to chat"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                .set_callback("command.chat", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto msg = Chat{"[" + context.executor.name + "] ", Chat::parseToChat(std::get<pred_string>(args[0]).value)};
                    api::players::iterate_online([&msg](base_objects::shared_client_data& context) {
                        context << api::packets::client_bound::play::system_chat{.content = msg};
                        return false;
                    });
                    return true;
                });
            browser.add_child("whoami")
                .set_callback("command.whoami", [](const list_array<predicate>& _, base_objects::command_context& context) {
                    context.executor << api::packets::client_bound::play::system_chat{.content = "You are " + context.executor.name};
                    return true;
                });
            browser.add_child("tellraw")
                .add_child({"message", "tellraw message", "Broadcast raw message for everyone."}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                .set_callback("command.tellraw", [](const list_array<predicate>& args, base_objects::command_context& _) {
                    auto msg = Chat::fromStr(std::get<pred_string>(args[0]).value);
                    api::players::iterate_online([&msg](base_objects::shared_client_data& context) {
                        context << api::packets::client_bound::play::system_chat{.content = msg};
                        return false;
                    });
                    return true;
                });
            {
                auto title = browser
                                 .add_child("title")
                                 .add_child("target", cmd_pred_entity{.flag = cmd_pred_entity::only_player_entity});
                title.add_child({"clear", "title target clear", "Clear title"})
                    .set_callback("command.title.clear", [](const list_array<predicate>&, base_objects::command_context&) {
                        //TODO
                        //api::players::iterate_online([&context](base_objects::shared_client_data& context) {
                        //    return false;
                        //});
                        return false;
                    });
            }
        }

        void on_config_reload(const plugin_registration_ptr& _) override {
            _chat_disabled_notification() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_chat_disabled_message");
            _on_invalid_new_signature() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_invalid_new_signature");
            if (!(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature" ^ get_conf).is_none())
                _on_chat_invalid_signature() = Chat::fromEnbt(api::configuration::get() ^ "communication_core" ^ "on_chat_invalid_signature");
        }
    };
}