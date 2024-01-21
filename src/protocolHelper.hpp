#include "ClientHandleHelper.hpp"
#include "enbt.hpp"
#include <unordered_set>
#include "chunk_core.hpp"
#include "protocolHelperNBT.hpp"
#include "calculations.hpp"
struct ArrayStream {
    uint8_t* arrau;
    size_t mi;
    ArrayStream(uint8_t* arr,size_t max_index) {
        arrau = arr;
        mi = max_index;
        read_only = false;
    }
    ArrayStream(const uint8_t* arr, size_t max_index) {
        arrau = const_cast<uint8_t * >(arr);
        mi = max_index;
        read_only = true;
    }
    size_t r = 0;
    size_t w = 0;

    void write(uint8_t value) {
        if (mi <= w)
            throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try write in: " + std::to_string(w));
        if (read_only)
            throw std::exception("Readonly Mode");
        arrau[w++] = value;
    }
    uint8_t read() {
        if (mi <= r)
            throw std::out_of_range("array max size is: " + std::to_string(mi) + "byt try read in: " + std::to_string(r));
        return arrau[r++];
    }
private:
    bool read_only;
};
class Chat {
    std::vector<Chat> extra;
    char* text = nullptr;
    char* color = nullptr;
    char* insertion = nullptr;
    struct clickEventS {
        char* open_url;
        char* run_command;
        char* suggest_command;
        char* change_page;
        char* copy_to_clipboard;
        ~clickEventS() {
            if (open_url)
                delete open_url;
            if (run_command)
                delete run_command;
            if (suggest_command)
                delete suggest_command;
            if (change_page)
                delete change_page;
            if (copy_to_clipboard)
                delete copy_to_clipboard;


            open_url = nullptr;
            run_command = nullptr;
            suggest_command = nullptr;
            change_page = nullptr;
            copy_to_clipboard = nullptr;
        }
    }*clickEvent = nullptr;
    struct hoverEventS {
        char* show_text;
        char* show_item;
        char* show_entity;
        ~hoverEventS() {
            if (show_text)
                delete show_text;
            if (show_item)
                delete show_item;
            if (show_entity)
                delete show_entity;
        }
    }*hoverEvent = nullptr;
    size_t bold : 1;
    size_t italic : 1;
    size_t underlined : 1;
    size_t strikethrough : 1;
    size_t obfuscated : 1;

    size_t defined_bold : 1;
    size_t defined_italic : 1;
    size_t defined_underlined : 1;
    size_t defined_strikethrough : 1;
    size_t defined_obfuscated : 1;

    size_t text_is_translation : 1;
    void setString(char*& char_ptr, const std::string& string) {
        if (size_t str_len = string.size(); str_len) {
            str_len++;
            if (char_ptr)
                delete[] char_ptr;
            char_ptr = new char[str_len];
            for (size_t i = 0; i < str_len; i++)
                char_ptr[i] = string[i];
        }
        else if (char_ptr) {
            delete[] char_ptr;
            char_ptr = nullptr;
        }
    }
public:
    Chat(const std::string& set_text = "", bool is_translation = false) {
        setString(text, set_text);
        defined_bold = false;
        defined_italic = false;
        defined_underlined = false;
        defined_strikethrough = false;
        defined_obfuscated = false;
        text_is_translation = is_translation;
    }
    Chat(const Chat& copy) {
        operator=(copy);
    }
    Chat(Chat&& copy) noexcept {
        operator=(std::move(copy));
    }
    Chat& operator=(const Chat& copy) {
        if (copy.text)
            setString(text, copy.text);
        if (copy.color)
            setString(color, copy.color);
        if (copy.insertion)
            setString(insertion, copy.insertion);
        defined_bold = copy.defined_bold;
        defined_italic = copy.defined_italic;
        defined_underlined = copy.defined_underlined;
        defined_strikethrough = copy.defined_strikethrough;
        defined_obfuscated = copy.defined_obfuscated;
        bold = copy.bold;
        italic = copy.italic;
        underlined = copy.underlined;
        strikethrough = copy.strikethrough;
        obfuscated = copy.obfuscated;
        text_is_translation = copy.text_is_translation;
    }
    Chat& operator=(Chat&& copy) noexcept {
        text = copy.text;
        copy.text = nullptr;
        color = copy.color;
        copy.color = nullptr;
        insertion = copy.insertion;
        copy.insertion = nullptr;
        clickEvent = copy.clickEvent;
        copy.clickEvent = nullptr;
        hoverEvent = copy.hoverEvent;
        copy.hoverEvent = nullptr;
        defined_bold = copy.defined_bold;
        defined_italic = copy.defined_italic;
        defined_underlined = copy.defined_underlined;
        defined_strikethrough = copy.defined_strikethrough;
        defined_obfuscated = copy.defined_obfuscated;
        bold = copy.bold;
        italic = copy.italic;
        underlined = copy.underlined;
        strikethrough = copy.strikethrough;
        obfuscated = copy.obfuscated;
        text_is_translation = copy.text_is_translation;
        return *this;
    }
    ~Chat() {
        if (text)
            delete text;
        if (color)
            delete color;
        if (insertion)
            delete insertion;
        if (clickEvent)
            delete clickEvent;
        if (hoverEvent)
            delete hoverEvent;
        text = nullptr;
        color = nullptr;
        insertion = nullptr;
        clickEvent = nullptr;
        hoverEvent = nullptr;
    }
    void SetColor(const std::string& set_text = "") {
        setString(color, set_text);
    }
    void SetInsertion(const std::string& set_text = "") {
        setString(color, set_text);
    }
    void SetBold() { defined_bold = false; }
    void SetItalic() { defined_italic = false; }
    void SetUnderlined() { defined_underlined = false; }
    void SetStrikethrough() { defined_strikethrough = false; }
    void SetObfuscated() { defined_obfuscated = false; }
    void SetBold(bool is) { bold = is; defined_bold = true; }
    void SetItalic(bool is) { italic = is; defined_italic = true; }
    void SetUnderlined(bool is) { underlined = is; defined_underlined = true; }
    void SetStrikethrough(bool is) { strikethrough = is; defined_strikethrough = true; }
    void SetObfuscated(bool is) { obfuscated = is; defined_obfuscated = true; }
    bool GetBold() { return defined_bold ? bold : false; }
    bool GetItalic() { return defined_italic ? italic : false; }
    bool GetUnderlined() { return defined_underlined ? underlined : false; }
    bool GetStrikethrough() { return defined_strikethrough ? strikethrough : false; }
    bool GetObfuscated() { return defined_obfuscated ? obfuscated : false; }
    void SetText(const std::string& set_text = "") {
        setString(text, set_text);
        text_is_translation = false;
    }
    void SetTranslation(const std::string& set_text = "") {
        setString(text, set_text);
        text_is_translation = true;
    }

    void AddExtraChat(const Chat& cha) {
        extra.push_back(cha);
    }
    void AddExtraChat(const std::vector<Chat>& cha) {
        for(auto& it : cha)
            extra.push_back(it);
    }
    void SetExtraChat(const std::vector<Chat>& cha) {
        extra = cha;
    }
    const std::vector<Chat>& GetExtraChat(const Chat& cha) {
        return extra;
    }
    Chat& GetExtraChatItem(size_t pos) {
        return extra[pos];
    }
    std::string ToStr() {
        std::string str = "{";
        if (text) {
            if (text_is_translation)
                str += "\"translation\":\"";
            else
                str += "\"text\":\"";
            str += text;
            str += "\"";
        }
        else {
            str += "\"text\":\"\"";
        }
        if (color) {
            str += ',';
            str += "\"color\":\"";
            str += color;
            str += "\"";
        }
        if (insertion) {
            str += ',';
            str += "\"insertion\":\"";
            str += insertion;
            str += "\"";
        }
        if (defined_bold) {
            str += ',';
            str += "\"bold\":\"";
            str += bold ? "true" : "false";
            str += "\"";
        }
        if (defined_italic) {
            str += ',';
            str += "\"italic\":\"";
            str += italic ? "true" : "false";
            str += "\"";
        }
        if (defined_underlined) {
            str += ',';
            str += "\"underlined\":\"";
            str += underlined ? "true" : "false";
            str += "\"";
        }
        if (defined_strikethrough) {
            str += ',';
            str += "\"strikethrough\":\"";
            str += strikethrough ? "true" : "false";
            str += "\"";
        }
        if (defined_obfuscated) {
            str += ',';
            str += "\"obfuscated\":\"";
            str += obfuscated ? "true" : "false";
            str += "\"";
        }
        if (extra.size()) {
            str += ',';
            str += "\"extra\":[";
            bool need_comma = false;
            for (auto& it : extra)
            {
                if (need_comma) str += ',';
                str += it.ToStr();
                need_comma = true;
            }
        }
        str += '}';
        return str;
    }
    void removeColor() {
        setString(color, "");
        for (auto& it : extra)
            it.removeColor();
    }
};
class SlotItem {
    int32_t item_id;
    uint8_t count : 7;
    ENBT nbt;
};

class TCPClientHandle : public TCPclient {
protected:
    static std::unordered_set<baip::address> banned_players;
    TCPclient* next_handler = nullptr;
    template<class T>
    T readValue(ArrayStream& data) {
        uint8_t tmp[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return ENBT::ConvertNativeEndian(std::endian::big, *(T*)tmp);
    }

    std::string ReadString(ArrayStream& data, int32_t max_string_len) {
        std::string res = "";
        int32_t actual_len = ReadVar<int32_t>(data);
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        for (int32_t i = 0; i < actual_len;) {
            char tmp = (char)data.read();
            i += (tmp & 0xc0) != 0x80;
            res += tmp;
        }
        return res;
    }
    void WriteString(std::vector<uint8_t>& data,const std::string& str, int32_t max_string_len = INT32_MAX) {
        int32_t actual_len = str.size();
        if (actual_len > max_string_len)
            throw std::out_of_range("actual string len out of range");
        WriteValue(actual_len,data);
        data.insert(data.end(), str.begin(), str.end());
    }

    template<class Res>
    static Res ReadVar(ArrayStream& data) {
        size_t len = sizeof(Res);
        Res res = ENBT::fromVar<Res>(data.arrau + data.r, len);
        data.r += len;
        ENBT::ConvertNativeEndian(std::endian::little, res);
        return res;
    }
    template<class T>
    static void WriteVar(T val, std::vector<uint8_t>& data) {
        size_t len = sizeof(T);
        uint8_t* tmp = ENBT::toVar(ENBT::ConvertExternEndian(std::endian::little, val), len);
        for (size_t i = 0; i < len; i++)
            data.push_back(tmp[i]);
        delete[] tmp;
    }
    template<class T>
    static void WriteVar(T val, ArrayStream& data) {
        size_t len;
        uint8_t* tmp = ENBT::toVar(ENBT::ConvertExternEndian(std::endian::little, val), len);
        for (size_t i = 0; i < len; i++)
            data.write(tmp[i]);
        delete[] tmp;
    }
    static void WriteUUID(const ENBT::UUID& val, std::vector<uint8_t>& data) {
        ENBT::UUID temp = ENBT::ConvertExternEndian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            data.push_back(tmp[i]);
    }
    static ENBT::UUID ReadUUID(ArrayStream& data) {
        ENBT::UUID temp;
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < 16; i++)
            tmp[i] = data.read();
        return ENBT::ConvertNativeEndian(std::endian::big, temp);
    }
    template<class T>
    static void WriteValue(const T& val, std::vector<uint8_t>& data) {
        T temp = ENBT::ConvertExternEndian(std::endian::big, val);
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < sizeof(T); i++)
            data.push_back(tmp[i]);
    }
    template<class T>
    static T ReadValue(ArrayStream& data) {
        T temp;
        uint8_t* tmp = (uint8_t*)&temp;
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return ENBT::ConvertNativeEndian(std::endian::big, temp);
    }
    virtual std::vector<uint8_t>& PreparePacket(const std::vector<uint8_t>& packet) {
        std::vector<uint8_t> res = packet;
        return PreparePacket(res);
    }
    virtual std::vector<uint8_t>& PreparePacket(std::vector<uint8_t>& packet) {
        PackPacket(packet);
        std::vector<uint8_t> tmp;
        WriteVar((int32_t)packet.size(), tmp);
        packet.insert(packet.begin(), tmp.begin(), tmp.end());
        return packet;
    }
    virtual std::vector<uint8_t>&& UnpackPacket(std::vector<uint8_t>&& packet) {
        return std::move(packet);
    }
    virtual std::vector<uint8_t>& PackPacket(std::vector<uint8_t>& packet) {
        return packet;
    }
    virtual std::vector<std::vector<uint8_t>> PreparePackets(const std::vector<std::vector<uint8_t>>& packets) {
        std::vector < std::vector<uint8_t>> actuall_packets;
        for (auto& packet : packets) {
            if (!packet.size())
                continue;
            ArrayStream data(packet.begin()._Ptr, packet.size());
        unpack_packet:
            int32_t skip_len = ReadVar<int32_t>(data);
            if (skip_len + data.r > packet.size())
                actuall_packets.emplace_back(UnpackPacket(std::vector<uint8_t>(packet.begin() + data.r, packet.end())));
            else
                actuall_packets.emplace_back(UnpackPacket(std::vector<uint8_t>(packet.begin() + data.r, packet.begin() + data.r + skip_len)));
            data.r += skip_len;
            if (data.mi > data.r)
                goto unpack_packet;
        }
        return actuall_packets;
    }

    virtual Response WorkPacket(std::vector<uint8_t> packet) = 0;
    virtual Response WorkPackets(std::vector<std::vector<uint8_t>> binary_packets) {
        std::vector<std::vector<uint8_t>> answer;
        for (auto& it : binary_packets) {
            Response&& answ_it = WorkPacket(it);
            processed_packets++;
            if (next_handler)
                break;

            if (answ_it.do_disconnect)
                return { {},1 };

            if (answ_it.data.size()) {
                for (auto& resp : answ_it.data)
                    answer.push_back(PreparePacket(resp));
            }

            if (answ_it.do_disconnect_after_send)
                return { answer,0,1 };
        }
        return { answer };
    }
    uint32_t protocol_version = 0;
public:
    ~TCPClientHandle() override {

    }
    TCPclient* RedefineHandler() override {
         return next_handler;
    }

    void PrepareData(std::vector<std::vector<uint8_t>>& clientData) override  {
        clientData = PreparePackets(clientData);
    }
    Response WorkClient(const std::vector<std::vector<uint8_t>>& clientData, uint64_t timeout_ms) override {
        return WorkPackets(clientData);
    }
    bool DoDisconnect(baip::address ip) override {
        return false;
    }
};



struct ClientRegisters {
    std::unordered_map<mcCore::Block,uint16_t, mcCore::BlockHash> BlockPallete;
};
namespace mcCore{
    class TCPClientHandlePlay : public TCPClientHandle, public WorldClusterTalker {
    #pragma region UNUSED
        virtual void ClusterUnloaded(block_pos_t cluster_x, block_pos_t cluster_z) {}
        virtual void BlockChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) {}
        virtual void BlockSeted(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) {}
        virtual void BlockRemoved(block_pos_t x, block_pos_t y, block_pos_t z) {}
        virtual void BlockBreaked(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) {}
        virtual void BlocksChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) {}
        virtual void BlockHandleNbtChanged(Block* block, block_pos_t x, block_pos_t y, block_pos_t z) {}
    #pragma endregion


    protected:
        static std::unordered_map<int32_t, ClientRegisters> clients_protocol_data;

        std::atomic_int32_t eid_gen;
        std::chrono::system_clock::time_point last_packet_time;
        bool TimeoutCheck(uint64_t timeout_ms) {
            auto current = std::chrono::system_clock::now();
            if ((current - last_packet_time) > std::chrono::milliseconds(timeout_ms))
                return true;
            return false;
        }
        bool compression_enabled = false;
        int32_t protocol_version = 0;

        //enable compression, returns -1 if disable or maximum compressed data before compress
        virtual int32_t CompressionEnabled() {
            return -1;
        }

        TCPsession* client_sock;
        int64_t keep_alive_key = 0;
        // first call will be client_key equal zero for start keep alive packets sending
        void KeepAliveAnswer(int64_t client_key) {
            keep_alive_enabled = true;
            int64_t old_key = keep_alive_key;
            keep_alive_key = std::chrono::system_clock::now().time_since_epoch().count();
            mc_keep_alive_timeout->async_wait([this](const boost::system::error_code& ex) {
                if (!ex.failed()) {
                    auto tmp = new std::vector<uint8_t>(10);
                    tmp->at(0) = 9;
                    tmp->at(1) = 0x0F;
                    union {
                        int64_t key = 0;
                        uint8_t part[8];
                    } val;
                    val.key = keep_alive_key;
                    tmp->at(2) = val.part[0];
                    tmp->at(3) = val.part[1];
                    tmp->at(4) = val.part[2];
                    tmp->at(5) = val.part[3];
                    tmp->at(6) = val.part[4];
                    tmp->at(7) = val.part[5];
                    tmp->at(8) = val.part[6];
                    tmp->at(9) = val.part[7];
                    bai::async_write(client_sock->sock, bai::buffer(*tmp), [this, tmp](const boost::system::error_code& ex, size_t) {
                        delete tmp;
                        });
                }
                });
            if (old_key == client_key)
                last_packet_time = std::chrono::system_clock::now();
        }

        Response WorkPacket(std::vector<uint8_t> packet) override {
            ArrayStream data(packet.begin()._Ptr, packet.size());

        }

        //                                            timeout      |     packetId    |   result func
        std::list<std::pair<std::chrono::system_clock::time_point, std::pair<uint8_t, std::function<void(const std::vector<uint8_t>& packet)>>>> wait_packets;

        void TransferPackets(const std::vector<std::vector<uint8_t>>& packets) {
            size_t i = 0;
            if (size_t i = packets.size()) {
                while (i) {
                    if (i ^ 1)
                        bai::write(client_sock->sock, bai::buffer(PreparePacket(packets[i++])), [](const boost::system::error_code& ex, size_t) {return true; });
                    else {
                        std::vector<uint8_t>* packet = new std::vector<uint8_t>(PreparePacket(packets[i]));
                        bai::async_write(client_sock->sock, bai::buffer(*packet), [this, packet](const boost::system::error_code& ex, size_t) {
                            delete packet;
                            });
                    }
                }
            }
        }
        void TransferPacket(const std::vector<uint8_t>& actual_packet) {
           std::vector<uint8_t>* packet = new std::vector<uint8_t>(PreparePacket(actual_packet));
           bai::async_write(client_sock->sock, bai::buffer(*packet), [this, packet](const boost::system::error_code& ex, size_t) {
               delete packet;
           });
        }
        uint8_t chunk_change_count = 0; 
        bool keep_alive_enabled = false;

        uint8_t skin_parts_flags;
        std::string locale;
        int8_t render_distance;
        uint8_t main_hand;
        bool text_filter_enabled;
        bool chat_color_enabled;
    public:
        static uint8_t max_chunk_change_count;
        static std::function<std::string(std::string)> text_filter;
        enum class WindowID {
            player,
            g9x1,
            g9x2,
            g9x3,
            g9x4,
            g9x5,
            g9x6,
            g3x3,
            anvil,
            blast_furnace,
            brewing_stand,
            craft_table,
            enchant_table,
            furnace,
            gringstone,
            hopper,
            lectern,
            loom,
            merchant,
            shulker_box,
            smoker,
            cartography,
            stonecuter
        };
        struct Slot {
            int32_t item_id;
            int8_t count;
            ENBT nbt;
            void WriteSlot(std::vector<uint8_t>& packet) const {
                TCPClientHandlePlay::WriteVar(item_id, packet);
                packet.push_back(count);
                std::vector<uint8_t> pnbt = NBT(nbt);
                packet.insert(packet.end(), pnbt.begin(), pnbt.end());
            }
        };
        struct Position {
            int64_t x : 26;
            int64_t z : 26;
            int64_t y : 12;
            void WritePos(std::vector<uint8_t>& packet) const {
                WriteValue(*(uint64_t*)this, packet);
            }
        };
        enum class Gamemode : int8_t {
            none=-1,
            survival,
            creative,
            adventure,
            spectator
        };
#pragma region NativeCommands
    #pragma region Spawning
        virtual std::vector<uint8_t> SpawnEntity(int32_t entity_id, ENBT::UUID uuid, int32_t type, calc::VECTOR pos, calc::ANGLE_RAD rotation, calc::VECTOR velocity) {
            std::vector<uint8_t> packet;
            packet.push_back(0x00);
            WriteVar(entity_id, packet);
            WriteValue(uuid, packet);
            WriteVar(type, packet);
            WriteValue(pos.x, packet);
            WriteValue(pos.y, packet);
            WriteValue(pos.z, packet);
            packet.push_back((uint8_t)rotation.x);
            packet.push_back((uint8_t)rotation.y);
            WriteValue((short)(velocity.x * 8000), packet);
            WriteValue((short)(velocity.y * 8000), packet);
            WriteValue((short)(velocity.z * 8000), packet);
            return packet;
        }
        virtual std::vector<uint8_t> SpawnExpOrb(int32_t entity_id, calc::VECTOR pos,short orb_exp_count) {
            std::vector<uint8_t> packet;
            packet.push_back(0x01);
            WriteVar(entity_id, packet);
            WriteValue(pos.x, packet);
            WriteValue(pos.y, packet);
            WriteValue(pos.z, packet);
            WriteValue(orb_exp_count, packet);
            return packet;
        }
        virtual std::vector<uint8_t> SpawnLivingEntity(int32_t entity_id, ENBT::UUID uuid, int32_t type, calc::VECTOR pos, calc::ANGLE_RAD rotation, double head_pitch_RAD, calc::VECTOR velocity) {
            std::vector<uint8_t> packet;
            packet.push_back(0x02);
            WriteVar(entity_id, packet);
            WriteValue(uuid, packet);
            WriteVar(type, packet);
            WriteValue(pos.x, packet);
            WriteValue(pos.y, packet);
            WriteValue(pos.z, packet);
            packet.push_back((uint8_t)rotation.x);
            packet.push_back((uint8_t)rotation.y);
            packet.push_back((uint8_t)head_pitch_RAD);
            WriteValue((short)(velocity.x * 8000), packet);
            WriteValue((short)(velocity.y * 8000), packet);
            WriteValue((short)(velocity.z * 8000), packet);
            return packet;
        }
        enum class PaintingDirection : uint8_t{
            South,
            West,
            North,
            East
        };
        virtual std::vector<uint8_t> SpawnPainting(int32_t entity_id, ENBT::UUID uuid, int32_t motive, Position center_cord, PaintingDirection dir) {
            std::vector<uint8_t> packet;
            packet.push_back(0x03);
            WriteVar(entity_id, packet);
            WriteValue(uuid, packet);
            WriteVar(motive, packet);
            center_cord.WritePos(packet);
            packet.push_back((uint8_t)dir);
            return packet;
        }
        virtual std::vector<uint8_t> SpawnPlayer(int32_t entity_id, ENBT::UUID uuid, calc::VECTOR pos, calc::ANGLE_RAD rotation) {
            std::vector<uint8_t> packet;
            packet.push_back(0x04);
            WriteVar(entity_id, packet);
            WriteValue(uuid, packet);
            WriteValue(pos.x, packet);
            WriteValue(pos.y, packet);
            WriteValue(pos.z, packet);
            packet.push_back((uint8_t)rotation.x);
            packet.push_back((uint8_t)rotation.y);
            return packet;
        }
    #pragma endregion
        virtual std::vector<uint8_t> PresentParticleVibrationSignal(Position source,std::string destination_codec,std::vector<uint8_t>entity_data,int32_t arrive_ticks) {
            std::vector<uint8_t> packet;
            packet.push_back(0x05);
            source.WritePos(packet);
            WriteString(packet, destination_codec, 131071);
            packet.insert(packet.end(), entity_data.begin(), entity_data.end());
            WriteValue(arrive_ticks, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PresentParticleVibrationSignal(Position source, std::string destination_codec, int32_t entity_id, int32_t arrive_ticks) {
            std::vector<uint8_t> packet;
            packet.push_back(0x05);
            source.WritePos(packet);
            WriteString(packet, destination_codec, 131071);
            WriteValue(entity_id, packet);
            WriteValue(arrive_ticks, packet);
            return packet;
        }
        enum class AnimationID : uint8_t {
            swing_m_arm,
            take_damg,
            leave_bed,
            swing_o_arm,
            crit_effect,
            magic_crit_effect
        };
        virtual std::vector<uint8_t> SetEntityAnimation(int32_t entity_id,AnimationID animation) {
            std::vector<uint8_t> packet;
            packet.push_back(0x06);
            WriteValue(entity_id, packet);
            packet.push_back((uint8_t)animation);
            return packet;
        }

        struct StatisticData {
            int32_t category_id;
            int32_t statistic_id;
            int32_t value;
        };
        virtual std::vector<uint8_t> SetStatistics(std::vector<StatisticData> stat_data) {
            std::vector<uint8_t> packet;
            packet.push_back(0x07);
            WriteVar((int32_t)stat_data.size(), packet);
            for (auto& it : stat_data) {
                WriteVar((int32_t)it.category_id, packet);
                WriteVar((int32_t)it.statistic_id, packet);
                WriteVar((int32_t)it.value, packet);
            }
            TransferPackets({ packet });
        }

        enum class BlockBreakingStatus {
            started,
            canceled,
            finished
        };
        virtual std::vector<uint8_t> AnnouceBlockBreaking(Position pos,int32_t set_state_id, BlockBreakingStatus status, bool succes) {
            std::vector<uint8_t> packet;
            packet.push_back(0x08);
            pos.WritePos(packet);
            WriteVar(set_state_id, packet);
            WriteVar((int32_t)status, packet);
            WriteValue(succes, packet);
            return packet;
        }
        virtual std::vector<uint8_t> SetBlockBreakAnimationStage(int32_t entity_id,Position pos,int8_t stage) {
            std::vector<uint8_t> packet;
            packet.push_back(0x09);
            WriteVar(entity_id, packet);
            pos.WritePos(packet);
            packet.push_back(stage);
            return packet;
        }

        enum class SBED_Action {
            undefined,
            spawner_data,
            set_command_block_execution_data,
            set_beacon_levels,
            head_rot_a_skin,
            decl_conduit,
            banner_base_color,
            data_structure_tile,
            gateway_destination,
            sign_text,
            undefined,
            decl_bed,
            jigsaw_data,
            campfire_items,
            beehive
        };
        //use SBED_Action
        virtual std::vector<uint8_t> SetBlockEntityData(Position pos, int8_t action, ENBT data) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0A);
            pos.WritePos(packet);
            packet.push_back(action);
            {
                std::vector <uint8_t> temp = NBT(data);
                packet.insert(packet.end(), temp.begin(), temp.end());
            }
            return packet;
        }
        virtual std::vector<uint8_t> SetBlockEntityData(Position pos, uint8_t action,uint8_t param,int32_t block_type_id ) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0B);
            pos.WritePos(packet);
            packet.push_back(action);
            packet.push_back(param);
            WriteVar(block_type_id, packet);
            return packet;
        }
        virtual std::vector<uint8_t> ChangeBlock(Position pos,int32_t block_state_id) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0B);
            pos.WritePos(packet);
            WriteVar(block_state_id, packet);
            return packet;
        }


        struct BlockChange {
            Position pos;

        };
        virtual std::vector<uint8_t> TransferBlockChange(std::shared_ptr<BlockChange> block_changed) {

        }

        enum class SetBlockEndityDataType : uint8_t {
            spawner = 1,
            commandblock,
            beacon,
            head,
            conduit,
            banner,
            tile_entity,
            gateway,
            sign,
            bed = 11,
            jigsaw,
            campfire,
            beehive,
        };
        virtual std::vector<uint8_t> SetblockEntityData(SetBlockEndityDataType set_type) {

        }

    #pragma region bossbar
        enum class BossbarColor :int8_t {
            Pink,
            Blue,
            Red,
            Green,
            Yellow,
            Purple,
            White
        };
        enum class BossbarDevision :int8_t {
            none,
            notches_6,
            notches_10,
            notches_12,
            notches_20,
        };
        enum class BossbarFlags :int8_t {
            none = 0,
            dark_sky = 1,
            is_dragon_bar = 2,
            is_dragon_bar_and_dark_sky = 3,
            enable_fog = 4,
            enable_fog_and_dark_sky = 5,
            enable_fog_and_is_dragon_bar = 6,
            enable_all = 7,
        };
        virtual std::vector<uint8_t> BossbarAdd(ENBT::UUID uuid, Chat title, float health, BossbarColor color, BossbarDevision devision, BossbarFlags flags) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(0);
            WriteUUID(uuid, packet);
            std::string str = title.ToStr();
            for (char it : str)
                packet.push_back(it);
            uint8_t* bytes = (uint8_t*)&health;
            for (int8_t i = 0; i < 4; i++)
                packet.push_back(bytes[i]);
            packet.push_back((uint8_t)color);
            packet.push_back((uint8_t)devision);
            packet.push_back((uint8_t)flags);
            return packet;
        }
        virtual std::vector<uint8_t> BossbarRemove(ENBT::UUID uuid) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(1);
            WriteUUID(uuid, packet);
            return packet;
        }
        virtual std::vector<uint8_t> BossbarSetHealth(ENBT::UUID uuid, float health) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(2);
            WriteUUID(uuid, packet);
            health = ENBT::ConvertExternEndian(std::endian::big, health);
            uint8_t* bytes = (uint8_t*)&health;
            for (int8_t i = 0; i < 4; i++)
                packet.push_back(bytes[i]);
            return packet;
        }
        virtual std::vector<uint8_t> BossbarSetTitle(ENBT::UUID uuid, Chat title) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(3);
            WriteUUID(uuid, packet);
            std::string str = title.ToStr();
            for (char it : str)
                packet.push_back(it);
            return packet;
        }
        virtual std::vector<uint8_t> BossbarSetStyle(ENBT::UUID uuid, BossbarColor color, BossbarDevision devision) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(4);
            WriteUUID(uuid, packet);
            packet.push_back((uint8_t)color);
            packet.push_back((uint8_t)devision);
            return packet;
        }
        virtual std::vector<uint8_t> BossbarSetFlags(ENBT::UUID uuid, BossbarFlags flags) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0D);
            packet.push_back(5);
            WriteUUID(uuid, packet);
            packet.push_back((uint8_t)flags);
            return packet;
        }
    #pragma endregion

        enum class Difficulty {
            peaceful,
            easy,
            normal,
            hard
        };
        virtual std::vector<uint8_t> SetDifficulty(Difficulty diff, bool is_locked) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0E);
            packet.push_back((uint8_t)diff);
            packet.push_back(is_locked);
            return packet;
        }

        enum class TellPlayerMode {
            chat,
            system_message,
            game_info
        };
        virtual std::vector<uint8_t> TellPlayer(Chat message, ENBT::UUID sender, TellPlayerMode mode = TellPlayerMode::chat) {
            std::vector<uint8_t> packet;
            packet.push_back(0x0F);
            if(mode == TellPlayerMode::chat)
                if (chat_color_enabled) 
                    message.removeColor();
            if (text_filter_enabled && text_filter)
                WriteString(packet, text_filter(message.ToStr()));
            else
                WriteString(packet,message.ToStr());
            packet.push_back((uint8_t)mode);
            WriteUUID(sender, packet);
            return packet;
        }
        virtual std::vector<uint8_t> ClearTitles(bool reset) {
            return { 0x10,reset };
        }

        struct TabCompleteAnswerItem {
            std::string match;//max len 32767
            Chat* tool_tip; //set nullptr if not present
        };
        virtual std::vector<uint8_t> TabCompleteAnswer(int32_t transaction_id,int32_t start_text_replace_pos,int32_t replacable_text_len,std::vector<TabCompleteAnswerItem> matches) {
            std::vector<uint8_t> packet;
            packet.push_back(0x11);
            WriteVar(transaction_id, packet);
            WriteVar(start_text_replace_pos, packet);
            WriteVar(replacable_text_len, packet);
            WriteVar((int32_t)matches.size(), packet);
            for (auto& it : matches){
                WriteString(packet, it.match, 32767);
                WriteValue((bool)it.tool_tip,packet);
                if(it.tool_tip)
                    WriteString(packet, it.tool_tip->ToStr());
            }
            return packet;
        }


    #pragma region ItemsAndInvrntory
        virtual std::vector<uint8_t> CloseWindows(WindowID wind_id) {
            return {0x13,(uint8_t)wind_id};
        }
        virtual std::vector<uint8_t> UpdateInventory(WindowID wind_id, int32_t state_id, const std::vector<Slot>& slots_def) {
            std::vector<uint8_t> packet;
            packet.push_back(0x14);
            packet.push_back((uint8_t)wind_id);
            WriteVar(state_id, packet);
            WriteVar(slots_def.size(), packet);
            for (auto& it : slots_def)
                it.WriteSlot(packet);
            return packet;
        }
        virtual std::vector<uint8_t> SetWindProperty(WindowID wind_id, int16_t prop, int16_t val) {
            std::vector<uint8_t> packet;
            packet.push_back(0x15);
            packet.push_back((uint8_t)wind_id);
            WriteValue(prop, packet);
            WriteValue(val, packet);
            return packet;
        }
        virtual std::vector<uint8_t> SetSlot(WindowID wind_id, int32_t state_id, int16_t slot, Slot* item) {
            std::vector<uint8_t> packet;
            packet.push_back(0x16);
            packet.push_back((uint8_t)wind_id);
            WriteVar(state_id, packet);
            WriteValue(slot, packet);
            if (item) {
                packet.push_back(true);
                item->WriteSlot(packet);
            }
            else
                packet.push_back(0);
            return packet;
        }
        virtual std::vector<uint8_t> SetColdown(uint32_t item_id, int32_t time) {
            std::vector<uint8_t> packet;
            packet.push_back(0x17);
            WriteVar(item_id, packet);
            WriteVar(time, packet);
            return packet;
        }
    #pragma endregion
        virtual std::vector<uint8_t> PluginMsg(std::string chanel_name,std::vector<uint8_t> data) {
            std::vector<uint8_t> packet;
            packet.push_back(0x18);
            WriteString(packet, chanel_name, 32767);
            packet.insert(packet.end(), data.begin(), data.end());
            return packet;
        }
        virtual std::vector<uint8_t> PlayNamedSound(std::string name,int32_t category, int32_t x, int32_t y, int32_t z,float volume_percent,float pitch) {
            std::vector<uint8_t> packet;
            packet.push_back(0x19);
            WriteString(packet, name, 32767);
            WriteVar(category, packet);
            WriteValue(x<<3, packet);//multiply by 8
            WriteValue(y<<3, packet);
            WriteValue(z<<3, packet);
            WriteValue(volume_percent, packet);
            WriteValue(pitch, packet);
            return packet;
        }
        
        //send packet, wait recuive and close connection
        virtual std::vector<uint8_t> CloseConnection(Chat reason) {
            std::vector<uint8_t>* packet = new std::vector<uint8_t>();
            packet->push_back(0x1A);
            WriteString(*packet, reason.ToStr(), 32767);
            bai::async_write(client_sock->sock, bai::buffer(*packet), [this, packet](const boost::system::error_code& ex, size_t) {
                delete packet;
                client_sock->disconnect();
            });
        }
        virtual std::vector<uint8_t> UpdateEntityStatus(int32_t entity_id,int8_t status) {
            std::vector<uint8_t> packet;
            packet.push_back(0x1B);
            WriteValue(entity_id, packet);
            packet.push_back(status);
            return packet;
        }
        virtual std::vector<uint8_t> ExplosionOccured(float x,float y,float z,float strength,const std::vector<calc::XYZ<int8_t>>& affected_blocks, float push_montion_x, float push_montion_y, float push_montion_z) {
            std::vector<uint8_t> packet;
            packet.push_back(0x1C);
            WriteValue(x, packet);
            WriteValue(y, packet);
            WriteValue(z, packet);
            WriteValue(strength, packet);
            WriteVar((int32_t)affected_blocks.size(), packet);
            for (auto& it : affected_blocks)
            {
                packet.push_back(it.x);
                packet.push_back(it.y);
                packet.push_back(it.z);
            }
            WriteValue(push_montion_x, packet);
            WriteValue(push_montion_y, packet);
            WriteValue(push_montion_z, packet);
            return packet;
        }
        virtual std::vector<uint8_t> UnloadChunk(int32_t chunk_pos_x, int32_t chunk_pos_z) {
            std::vector<uint8_t> packet;
            packet.push_back(0x1D);
            WriteValue(chunk_pos_x, packet);
            WriteValue(chunk_pos_z, packet);
            return packet;
        }
        //use https://wiki.vg/Protocol#Change_Game_State
        virtual std::vector<uint8_t> GameStateChanged(int8_t reason,float value) {
            std::vector<uint8_t> packet;
            packet.push_back(0x1E);
            packet.push_back(reason);
            WriteValue(value, packet);
            return packet;
        }
        virtual std::vector<uint8_t> OpenHorseWindow(int8_t id, int32_t numb_slots,int32_t entity_id) {
            std::vector<uint8_t> packet;
            packet.push_back(0x1E);
            packet.push_back(id);
            WriteVar(numb_slots, packet);
            WriteValue(entity_id, packet);
            return packet;
        }
        virtual std::vector<uint8_t> InitOrDefineWorldBorder(double center_x,double center_z,double old_diamether,double new_diamether,int64_t ms_speed_change,int32_t portal_teleport_bound,int32_t warn_blocks,int32_t warn_time) {
            std::vector<uint8_t> packet;
            packet.push_back(0x20);
            WriteValue(center_x, packet);
            WriteValue(center_z, packet);
            WriteValue(old_diamether, packet);
            WriteValue(new_diamether, packet);
            WriteVar(ms_speed_change, packet);
            WriteVar(portal_teleport_bound, packet);
            WriteVar(warn_blocks, packet);
            WriteVar(warn_time, packet);
            return packet;
        }
        //keepalive defined
    #pragma region Chunk
        virtual int64_t* SerializeHeightMap(mcCore::ChunkCore& chunk) {
            int64_t* bin = new int64_t[37];
            union PROXY {
                struct Splited {
                    int64_t v0 : 9;
                    int64_t v1 : 9;
                    int64_t v2 : 9;
                    int64_t v3 : 9;
                    int64_t v4 : 9;
                    int64_t v5 : 9;
                    int64_t v6 : 9;
                } v;
                int64_t actual_val;
            } vp;

            char i = 0;
            char j = 0;
            for (char x = 0; x < 16; x++) {
                for (char z = 0; z < 16; z++) {
                    for (int y = 256; y >= 0; y--) {
                        if (chunk(x, y, z).id != 0 || y == 0) {
                            switch (j++)
                            {
                            case 0:
                                vp.v.v0 = y;
                                break;
                            case 1:
                                vp.v.v1 = y;
                                break;
                            case 2:
                                vp.v.v2 = y;
                                break;
                            case 3:
                                vp.v.v3 = y;
                                break;
                            case 4:
                                vp.v.v4 = y;
                                break;
                            case 5:
                                vp.v.v5 = y;
                                break;
                            case 6:
                            default:
                                vp.v.v6 = y;
                                bin[i++] = vp.actual_val;
                                break;
                            }
                            break;
                        }
                    }//y
                }//z
            }//x
            vp.v.v3 = vp.v.v5 = vp.v.v6 = 0;
            bin[i] = vp.actual_val;

            return bin;
        }
        struct ChunkSection {
            int16_t non_air_blocks;
            uint8_t bit_p_block;
            struct def_if_present {
                int32_t len;
                int32_t* values;
                def_if_present(size_t siz) {
                    len = siz;
                    values = new int32_t[siz];
                }
                ~def_if_present() {
                    if (values)
                        delete[] values;
                }
            }*IndirectPalette;//set nullptr if pallete is direct
            int32_t sub_chunk_len;
            int64_t* sub_chunk;
            ChunkSection(mcCore::ChunkFragment& fragm, int32_t protocol, uint16_t included_subchunks = 0x0) {
                non_air_blocks = 0;
                for (int x = 0; x < 16; x++)
                    for (int y = 0; y < 16; y++)
                        for (int z = 0; z < 16; z++)
                            if (!fragm[x][y][z].inTagFamily("air"))
                                ++non_air_blocks;
                auto&& it = UniqueBlocks(fragm);
                bit_p_block = pallate_bit(it.size());
                auto& current_protocol_pallete = TCPClientHandlePlay::clients_protocol_data[protocol].BlockPallete;
                if (bit_p_block == 15) {
                    std::vector<PR> tmp;
                    PR tm;
                    tm.rv = 0;
    #pragma region TO_TEST
                    {
                        uint8_t i = 0;
                        uint8_t mi = PR::max(bit_p_block);
                        for (int x = 0; x < 16; x++) {
                            for (int y = 0; y < 16; y++) {
                                for (int z = 0; z < 16; z++) {
                                    tm(bit_p_block, i++, current_protocol_pallete[fragm[x][y][z]]);
                                    if (i >= mi) {
                                        i = 0;
                                        tmp.push_back(tm);
                                    }
                                }
                            }
                        }
                        if (i != 0)
                            tmp.push_back(tm);
                    }
    #pragma endregion
                    sub_chunk_len = tmp.size();
                    sub_chunk = new int64_t[sub_chunk_len];
                    for (size_t i = 0; i < sub_chunk_len; i++)
                        sub_chunk[i] = tmp[i].rv;
                }
                else {
                    auto&& temp = SelectIndirectPalette(it, current_protocol_pallete);
                    int32_t inter_v = temp.size();
                    IndirectPalette = new def_if_present(inter_v);
                    int32_t* values = IndirectPalette->values;
                    for (int32_t i = 0; i < inter_v; i++)
                        values[i] = temp[i];

                    std::vector<PR> tmp;
                    PR tm;
                    tm.rv = 0;
    #pragma region TO_TEST
                    {
                        uint8_t i = 0;
                        uint8_t mi = PR::max(bit_p_block);
                        for (int x = 0; x < 16; x++) {
                            for (int y = 0; y < 16; y++) {
                                for (int z = 0; z < 16; z++) {
                                    tm(bit_p_block, i++, std::find(temp.begin(), temp.end(), current_protocol_pallete[fragm[x][y][z]]) - temp.begin());
                                    if (i >= mi) {
                                        i = 0;
                                        tmp.push_back(tm);
                                    }
                                }
                            }
                        }
                        if (i != 0)
                            tmp.push_back(tm);
                    }
    #pragma endregion
                    sub_chunk_len = tmp.size();
                    sub_chunk = new int64_t[sub_chunk_len];
                    for (size_t i = 0; i < sub_chunk_len; i++)
                        sub_chunk[i] = tmp[i].rv;
                }
            }
            ChunkSection() {}
        private:
            union PR {
                int64_t rv;
                struct {
                    int64_t v0 : 4;
                    int64_t v1 : 4;
                    int64_t v2 : 4;
                    int64_t v3 : 4;
                    int64_t v4 : 4;
                    int64_t v5 : 4;
                    int64_t v6 : 4;
                    int64_t v7 : 4;
                    int64_t v8 : 4;
                    int64_t v9 : 4;
                    int64_t v10 : 4;
                    int64_t v11 : 4;
                    int64_t v12 : 4;
                    int64_t v13 : 4;
                    int64_t v14 : 4;
                    int64_t v15 : 4;
                } v4;
                struct {
                    int64_t v0 : 5;
                    int64_t v1 : 5;
                    int64_t v2 : 5;
                    int64_t v3 : 5;
                    int64_t v4 : 5;
                    int64_t v5 : 5;
                    int64_t v6 : 5;
                    int64_t v7 : 5;
                    int64_t v8 : 5;
                    int64_t v9 : 5;
                    int64_t v10 : 5;
                    int64_t v11 : 5;
                } v5;
                struct {
                    int64_t v0 : 6;
                    int64_t v1 : 6;
                    int64_t v2 : 6;
                    int64_t v3 : 6;
                    int64_t v4 : 6;
                    int64_t v5 : 6;
                    int64_t v6 : 6;
                    int64_t v7 : 6;
                    int64_t v8 : 6;
                    int64_t v9 : 6;
                } v6;
                struct {
                    int64_t v0 : 7;
                    int64_t v1 : 7;
                    int64_t v2 : 7;
                    int64_t v3 : 7;
                    int64_t v4 : 7;
                    int64_t v5 : 7;
                    int64_t v6 : 7;
                    int64_t v7 : 7;
                    int64_t v8 : 7;
                } v7;
                struct {
                    int64_t v0 : 8;
                    int64_t v1 : 8;
                    int64_t v2 : 8;
                    int64_t v3 : 8;
                    int64_t v4 : 8;
                    int64_t v5 : 8;
                    int64_t v6 : 8;
                    int64_t v7 : 8;
                } v8;
                struct {
                    int64_t v0 : 15;
                    int64_t v1 : 15;
                    int64_t v2 : 15;
                    int64_t v3 : 15;
                } v15;
                void operator()(int8_t bm, int8_t i, int64_t val) {
                    switch (bm) {
                    case 4:
                        switch (i)
                        {
                        case 0: v4.v0 = val; break;
                        case 1: v4.v1 = val; break;
                        case 2: v4.v2 = val; break;
                        case 3: v4.v3 = val; break;
                        case 4: v4.v4 = val; break;
                        case 5: v4.v5 = val; break;
                        case 6: v4.v6 = val; break;
                        case 7: v4.v7 = val; break;
                        case 8: v4.v8 = val; break;
                        case 9: v4.v9 = val; break;
                        case 10: v4.v10 = val; break;
                        case 11: v4.v11 = val; break;
                        case 12: v4.v12 = val; break;
                        case 13: v4.v13 = val; break;
                        case 14: v4.v14 = val; break;
                        case 15: v4.v15 = val; break;
                        }
                        break;
                    case 5:
                        switch (i)
                        {
                        case 0: v5.v0 = val; break;
                        case 1: v5.v1 = val; break;
                        case 2: v5.v2 = val; break;
                        case 3: v5.v3 = val; break;
                        case 4: v5.v4 = val; break;
                        case 5: v5.v5 = val; break;
                        case 6: v5.v6 = val; break;
                        case 7: v5.v7 = val; break;
                        case 8: v5.v8 = val; break;
                        case 9: v5.v9 = val; break;
                        case 10: v5.v10 = val; break;
                        case 11: v5.v11 = val; break;
                        }
                        break;
                    case 6:
                        switch (i)
                        {
                        case 0: v6.v0 = val; break;
                        case 1: v6.v1 = val; break;
                        case 2: v6.v2 = val; break;
                        case 3: v6.v3 = val; break;
                        case 4: v6.v4 = val; break;
                        case 5: v6.v5 = val; break;
                        case 6: v6.v6 = val; break;
                        case 7: v6.v7 = val; break;
                        case 8: v6.v8 = val; break;
                        case 9: v6.v9 = val; break;
                        }
                        break;
                    case 7:
                        switch (i)
                        {
                        case 0: v7.v0 = val; break;
                        case 1: v7.v1 = val; break;
                        case 2: v7.v2 = val; break;
                        case 3: v7.v3 = val; break;
                        case 4: v7.v4 = val; break;
                        case 5: v7.v5 = val; break;
                        case 6: v7.v6 = val; break;
                        case 7: v7.v7 = val; break;
                        case 8: v7.v8 = val; break;
                        }
                        break;
                    case 8:
                        switch (i)
                        {
                        case 0: v8.v0 = val; break;
                        case 1: v8.v1 = val; break;
                        case 2: v8.v2 = val; break;
                        case 3: v8.v3 = val; break;
                        case 4: v8.v4 = val; break;
                        case 5: v8.v5 = val; break;
                        case 6: v8.v6 = val; break;
                        case 7: v8.v7 = val; break;
                        }
                        break;
                    case 15:
                        switch (i)
                        {
                        case 0: v15.v0 = val; break;
                        case 1: v15.v1 = val; break;
                        case 2: v15.v2 = val; break;
                        case 3: v15.v3 = val; break;
                        }
                    }
                }
                static int8_t max(int8_t bm) {
                    switch (bm) {
                    case 4:
                        return 16;
                    case 5:
                        return 12;
                    case 6:
                        return 10;
                    case 7:
                        return 9;
                    case 8:
                        return 8;
                    case 15:
                        return 4;
                    }
                }
            };
            std::list<mcCore::Block> UniqueBlocks(mcCore::ChunkFragment& fragm) {
                std::list<mcCore::Block> ub;
                for (int x = 0; x < 16; x++) {
                    for (int y = 0; y < 16; y++) {
                        for (int z = 0; z < 16; z++) {
                            bool exists = false;
                            for (auto& it : ub)
                                if (it.id_and_state_eq(fragm[x][y][z])) {
                                    exists = true;
                                    break;
                                }
                            if (!exists)
                                ub.push_back(fragm[x][y][z]);
                        }
                    }
                }
                return ub;
            }
            int8_t pallate_bit(int16_t unique_blocks_count) {
                int8_t i = 0;
                while (unique_blocks_count)
                    unique_blocks_count >>= 1;
                if (i < 4)
                    i = 4;
                else if (i > 8)
                    i = 15;
                return i;
            }
            std::vector<int32_t> SelectIndirectPalette(std::list<mcCore::Block>& tmp, std::unordered_map<mcCore::Block, uint16_t, mcCore::BlockHash>& current_protocol_pallete) {
                std::vector<int32_t> res;
                for (auto& it : tmp)
                    res.push_back(current_protocol_pallete[it]);
                return res;
            }
        };
        virtual std::vector<uint8_t> SendChunk(int32_t x, int32_t z, mcCore::ChunkCore& chunk, bool is_full_chunk = false, uint16_t included_subchunks = 0x0, uint32_t biome_ids[1024] = 0) {
            std::vector<uint8_t> packet;
            packet.push_back(0x22);
            WriteVar(x, packet);
            WriteVar(z, packet);
            WriteValue(is_full_chunk, packet);
            WriteVar((int32_t)included_subchunks, packet);
            //write height map
            {
                ENBT tmp(ENBT::Type_ID{ ENBT::Type_ID::Type::compound , ENBT::Type_ID::LenType::Tiny });
                ENBT& MOTION_BLOCKING = tmp["MOTION_BLOCKING"] = ENBT(ENBT::Type_ID{ ENBT::Type_ID::Type::array , ENBT::Type_ID::LenType::Tiny }, 37);//TAG_Long_Array 
                tmp["WORLD_SURFACE"] = ENBT();

                int64_t* temp = SerializeHeightMap(chunk);
                for (char i = 0; i < 37; i++)
                    MOTION_BLOCKING[i] = temp[i];
                delete[] temp;

                NBT serializer(tmp);
                for (uint8_t i : (std::vector<uint8_t>)serializer)
                    WriteValue(i, packet);
            }
            if (is_full_chunk) {
                WriteVar(1024, packet);
                for (int i = 0; i < 1024; i++)
                    WriteVar(biome_ids[i], packet);
            }
            //set data
            {
                std::vector<uint8_t> data;
                uint16_t j = 1;
                for (uint8_t i = 0; j != 0; i++) {
                    if (!(included_subchunks & j)) {
                        j <<= 1;
                        continue;
                    }
                    ChunkSection tmp = { chunk.chunk_fragments[i],protocol_version };
                    WriteValue(tmp.non_air_blocks, data);
                    WriteValue(tmp.bit_p_block, data);
                    if (tmp.IndirectPalette) {
                        int32_t len = tmp.IndirectPalette->len;
                        WriteValue(len, data);
                        for (int32_t l = 0; l < len; l++)
                            WriteValue(tmp.IndirectPalette->values[l], data);
                    }
                    uint32_t sub_chunk_len = tmp.sub_chunk_len;
                    WriteValue(sub_chunk_len, data);
                    for (int32_t l = 0; l < sub_chunk_len; l++)
                        WriteValue(tmp.sub_chunk[l], data);
                    j <<= 1;
                }
                WriteVar((int32_t)data.size(), packet);
                packet.insert(packet.end(), data.begin(), data.end());
            }
            packet.push_back(0);//do not send block entities nbt when it not neded,
                                // send if player physicaly, send request to get nbt, ex: opened container
            return packet;
        }
    #pragma endregion

        virtual std::vector<uint8_t> PlayEffect(int32_t effect_id,Position pos,int32_t data,bool disable_matching_distance) {
            std::vector<uint8_t> packet;
            packet.push_back(0x23);
            WriteValue(effect_id, packet);
            pos.WritePos(packet);
            WriteValue(data, packet);
            packet.push_back(disable_matching_distance);
            return packet;
        }
    #pragma region Particle
        virtual std::vector<uint8_t> PlayParticle(int32_t particle_id,bool far_render, double x, double y, double z, double off_x, double off_y, double off_z,float particle_data,int32_t count) {
            std::vector<uint8_t> packet;
            packet.push_back(0x24);
            WriteValue(particle_id, packet);
            packet.push_back(far_render);
            WriteValue(x, packet);
            WriteValue(y, packet);
            WriteValue(z, packet);
            WriteValue(off_x, packet);
            WriteValue(off_y, packet);
            WriteValue(off_z, packet);
            WriteValue(particle_data, packet);
            WriteValue(count, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleBlock(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count,int32_t block_id) {
            std::vector<uint8_t> packet(PlayParticle(4,far_render,x,y,z,off_x,off_y,off_z,particle_data,count));
            WriteVar(block_id, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleDust(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count,float red,float green,float blue,float scale) {
            std::vector<uint8_t> packet(PlayParticle(15, far_render, x, y, z, off_x, off_y, off_z, particle_data, count));
            WriteValue(red, packet);
            WriteValue(green, packet);
            WriteValue(blue, packet);
            WriteValue(scale, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleDustTransition(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count, float from_red, float from_green, float from_blue, float scale, float to_red, float to_green, float to_blue) {
            std::vector<uint8_t> packet(PlayParticle(16, far_render, x, y, z, off_x, off_y, off_z, particle_data, count));
            WriteValue(from_red, packet);
            WriteValue(from_green, packet);
            WriteValue(from_blue, packet);
            WriteValue(scale, packet);
            WriteValue(to_red, packet);
            WriteValue(to_green, packet);
            WriteValue(to_blue, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleFallDust(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count, int32_t block_id) {
            std::vector<uint8_t> packet(PlayParticle(25, far_render, x, y, z, off_x, off_y, off_z, particle_data, count));
            WriteVar(block_id, packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleItem(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count, Slot item) {
            std::vector<uint8_t> packet(PlayParticle(36, far_render, x, y, z, off_x, off_y, off_z, particle_data, count));
            item.WriteSlot(packet);
            return packet;
        }
        virtual std::vector<uint8_t> PlayParticleVibration(bool far_render, double x, double y, double z, double off_x, double off_y, double off_z, float particle_data, int32_t count, double o_x, double o_y, double o_z, double d_x, double d_y, double d_z,int32_t move_ticks) {
            std::vector<uint8_t> packet(PlayParticle(37, far_render, x, y, z, off_x, off_y, off_z, particle_data, count));
            WriteValue(o_x, packet);
            WriteValue(o_y, packet);
            WriteValue(o_z, packet);
            WriteValue(d_x, packet);
            WriteValue(d_y, packet);
            WriteValue(d_z, packet);
            WriteValue(move_ticks, packet);
            return packet;
        }
    #pragma endregion

        virtual std::vector<uint8_t> UpdateLight(int32_t chunk_x, int32_t chunk_z, bool trust_edges, std::vector<int64_t> sky_light_mask, std::vector<int64_t> block_light_mask, std::vector<int64_t> empty_sky_light_mask, std::vector<int64_t> empty_block_light_mask, std::vector<std::vector<int64_t>> sky_light, std::vector<std::vector<int64_t>> block_light) {
            std::vector<uint8_t> packet;
            packet.push_back(0x23);
            WriteVar(chunk_x, packet);
            WriteVar(chunk_z, packet);
            packet.push_back(trust_edges);
            WriteVar((int32_t)sky_light_mask.size(), packet);
            for (auto& it : sky_light_mask)
                WriteValue(it, packet);
            WriteVar((int32_t)block_light_mask.size(), packet);
            for (auto& it : block_light_mask)
                WriteValue(it, packet);
            WriteVar((int32_t)empty_sky_light_mask.size(), packet);
            for (auto& it : empty_sky_light_mask)
                WriteValue(it, packet);
            WriteVar((int32_t)empty_block_light_mask.size(), packet);
            for (auto& it : empty_block_light_mask)
                WriteValue(it, packet);
            WriteVar((int32_t)sky_light.size(), packet);
            for (auto& it : sky_light)
            {
                WriteVar((int32_t)it.size(), packet);
                for (auto& iit : it)
                    WriteValue(iit, packet);
            }
            WriteVar((int32_t)block_light.size(), packet);
            for (auto& it : block_light)
            {
                WriteVar((int32_t)it.size(), packet);
                for (auto& iit : it)
                    WriteValue(iit, packet);
            }
            return packet;
        }


#pragma region InitClientRegion
        struct IC_DimensionDATA {
            std::string infiniburn;
            std::string effects;
            int64_t fixed_time = -1;//set -1 if undefined
            int32_t min_y;
            int32_t max_y;
            int32_t max_teleport_height;
            float ambient_light;
            float coordinate_scale;
            bool piglin_safe : 1;
            bool is_natural : 1;
            bool has_raids : 1;
            bool ultrawarm : 1;
            bool has_ceiling : 1;
            bool bed_works : 1;
            bool has_skylight : 1;
            bool respawn_anchor_works : 1;
        };
        struct IC_BiomeDATA {
            std::string precipitation;
            std::string category;
            float deph;
            float temperature;
            float scale;
            float downfall;
            struct effectsStruct{
                bool use_foliage_color = false;
                bool use_grass_color = false;
                bool use_music = false;
                int32_t sky_color;
                int32_t fog_color;
                int32_t water_color;
                int32_t water_fog_color;


                int32_t foliage_color;
                int32_t grass_color;
                struct Music {
                    std::string sound_name;
                    int32_t max_delay;
                    int32_t min_delay;
                    bool replace_current_music;
                } music;
                // if addition_sound empty addition_sound_change_tick not used
                std::string addition_sound_name;
                double addition_sound_change_tick;
                //
                std::string ambient_sound;
                std::string addition_sound;
                std::string grass_color_modifier;
            } effects;
            struct Particle {
                bool use_particle;
                float probability;
                std::string type;
            } particle;
        };
        struct IC_Biome {
            std::string name;
            int32_t id;
            IC_BiomeDATA data;
        };
        struct IC_Dimension {
            std::string name;
            int32_t id;
            IC_DimensionDATA data;
        };
        struct IC_DimensionsCodec {
            std::vector<IC_Dimension> dimensions;
            std::vector<IC_Biome> biomes;
        };
        // in join game
        virtual std::vector<uint8_t> InitClient(int32_t player_eid,bool player_in_hardcore, Gamemode cur_mode, Gamemode prev_mode, std::vector<std::string> worlds_names, IC_DimensionsCodec dimension_codec, IC_Dimension dimension,std::string loading_dimension_name,int64_t seed_hash_part,int32_t max_players = 0,int32_t render_distance = 2,bool force_reduced_debug_info = false,bool respawn_screen = true,bool world_is_debug = false,bool is_flat = false) {
        
        
        
        
        }
#pragma endregion












        //alias TellPlayer with system_message mode  
        virtual std::vector<uint8_t> SetActionBar(Chat text) {
            std::vector<uint8_t> packet;
            packet.push_back(0x41);
            if(text_filter_enabled && text_filter)
                WriteString(packet, text_filter(text.ToStr()));
            else
                WriteString(packet, text.ToStr());
            return packet;
        }
    #pragma region WordlBorder
        virtual std::vector<uint8_t> WorldBorderSetCenter(double x, double z) {
            std::vector<uint8_t> packet;
            packet.push_back(0x42);
            WriteValue(x, packet);
            WriteValue(z, packet);
            return packet;
        }
        virtual std::vector<uint8_t> WorldBorderSetSize(double diamether) {
            std::vector<uint8_t> packet;
            packet.push_back(0x43);
            WriteValue(diamether, packet);
            return packet;
        }
        virtual std::vector<uint8_t> WorldBorderChangeSize(double old_diamether, double new_diamether, int64_t speed_ms) {
            std::vector<uint8_t> packet;
            packet.push_back(0x44);
            WriteValue(old_diamether, packet);
            WriteValue(new_diamether, packet);
            WriteVar(speed_ms, packet);
            return packet;
        }
        virtual std::vector<uint8_t> WorldBorderWarnTime(int32_t time_s) {
            std::vector<uint8_t> packet;
            packet.push_back(0x45);
            WriteVar(time_s, packet);
            return packet;
        }
        virtual std::vector<uint8_t> WorldBorderWarnReach(int32_t methres) {
            std::vector<uint8_t> packet;
            packet.push_back(0x46);
            WriteVar(methres, packet);
            return packet;
        }
    #pragma endregion
           
        virtual std::vector<uint8_t> SetSpectatorCamera(uint32_t entity_id) {
            std::vector<uint8_t> packet;
            packet.push_back(0x47);
            WriteVar(entity_id, packet);
            return packet;
        }
        //0-8
        virtual std::vector<uint8_t> SetSelectedItemSlot(uint8_t slot) {
            TransferPacket({0x48,slot});
        }


        virtual std::vector<uint8_t> ConnectLeashEntity(int32_t connected_entity_id, int32_t holder_entity_id) {
            std::vector<uint8_t> packet;
            packet.push_back(0x4E);
            WriteValue(connected_entity_id, packet);
            WriteValue(holder_entity_id, packet);
            return packet;
        }
        virtual std::vector<uint8_t> DisconnectLeashEntity(int32_t connected_entity_id) {
            std::vector<uint8_t> packet;
            packet.push_back(0x4E);
            WriteValue(connected_entity_id, packet);
            WriteValue((int32_t)-1, packet);
            return packet;
        }

        virtual std::vector<uint8_t> SetEntityVelocity(int32_t entity_id, int16_t vx, int16_t vy, int16_t vz) {
            std::vector<uint8_t> packet;
            packet.push_back(0x4F);
            WriteVar(entity_id, packet);
            WriteValue(vx, packet);
            WriteValue(vy, packet);
            WriteValue(vz, packet);
            return packet;
        }

        enum class EntityEquipmentSlots :uint8_t {
            main_hand,
            off_hand,
            boots,
            leggings,
            chestplate,
            helmet
        };
        virtual std::vector<uint8_t> SetEntityEquipment(int32_t entity_id, std::vector<std::pair<Slot, EntityEquipmentSlots>> items) {
            std::vector<uint8_t> packet;
            packet.push_back(0x50);
            WriteVar(entity_id, packet);
            size_t siz = items.size();
            for (size_t i = 0; i < siz; i++)
            {
                WriteValue(
                    (int8_t)items[i].second & (i + 1 < siz) << 7,//set last bit true if contains next item
                    packet
                );
                items[i].first.WriteSlot(packet);
            }
            return packet;
        }

        //percent will be filled from 0 to 1
        virtual std::vector<uint8_t> SetExperience(float bar_fill_percent, int32_t level, int32_t total_experience) {
            std::vector<uint8_t> packet;
            packet.push_back(0x4F);
            WriteValue(bar_fill_percent, packet);
            WriteVar(level, packet);
            WriteVar(total_experience, packet);
            return packet;
        }

        //food will be in 0..20 range 
        virtual std::vector<uint8_t> SetHealthyState(float health,int8_t food,float saturation)
        {
            std::vector<uint8_t> packet;
            packet.push_back(0x52);
            WriteValue(health, packet);
            WriteVar((int32_t)food, packet);
            WriteValue(saturation, packet);
            return packet;
        }

    #pragma region ScoreBoardObjective
        virtual std::vector<uint8_t> CreateScoreBoardObjective(std::string name, Chat display_name,bool show_value_as_hearts) {
            std::vector<uint8_t> packet;
            packet.push_back(0x53);
            WriteString(packet, name, 16);
            packet.push_back(0);
            WriteString(packet, display_name.ToStr());
            packet.push_back(show_value_as_hearts);
            return packet;
        }
        virtual std::vector<uint8_t> RemoveScoreBoardObjective(std::string name) {
            std::vector<uint8_t> packet;
            packet.push_back(0x53);
            WriteString(packet, name, 16);
            packet.push_back(1);
            return packet;
        }
        virtual std::vector<uint8_t> ModifyScoreBoardObjective(std::string name, Chat display_name, bool show_value_as_hearts) {
            std::vector<uint8_t> packet;
            packet.push_back(0x53);
            WriteString(packet, name, 16);
            packet.push_back(2);
            WriteString(packet, display_name.ToStr());
            packet.push_back(show_value_as_hearts);
            return packet;
        }
    #pragma endregion

        virtual std::vector<uint8_t> PresentPassengers(int32_t car_entity_id,std::vector<int32_t> eid_passengers) {
            std::vector<uint8_t> packet;
            packet.push_back(0x54);
            WriteVar(car_entity_id, packet);
            WriteVar((int32_t)eid_passengers.size(), packet);
            for(int32_t it : eid_passengers)
                WriteVar(it, packet);
            return packet;
        }


    #pragma region Team
        enum class TeamNameTagVisibility {
            always,
            never,
            hideForOtherTeams,
            hideForOwnTeam
        };
        std::string to_string(TeamNameTagVisibility nt_visibility) {
            switch (nt_visibility)
            {
            case TCPClientHandlePlay::TeamNameTagVisibility::always:
                return "always";
            case TCPClientHandlePlay::TeamNameTagVisibility::never:
                return "never";
            case TCPClientHandlePlay::TeamNameTagVisibility::hideForOtherTeams:
                return "hideForOtherTeams";
            case TCPClientHandlePlay::TeamNameTagVisibility::hideForOwnTeam:
                return "hideForOwnTeam";
            default:
                return "always";
            }
        }
        enum class TeamCollisionRule {
            always,
            never,
            pushOtherTeams,
            pushOwnTeam
        };
        std::string to_string(TeamCollisionRule nt_visibility) {
            switch (nt_visibility)
            {
            case TCPClientHandlePlay::TeamCollisionRule::always:
                return "always";
            case TCPClientHandlePlay::TeamCollisionRule::never:
                return "never";
            case TCPClientHandlePlay::TeamCollisionRule::pushOtherTeams:
                return "pushOtherTeams";
            case TCPClientHandlePlay::TeamCollisionRule::pushOwnTeam:
                return "pushOwnTeam";
            default:
                return "always";
            }
        }
        enum class TeamColor : uint8_t {
            black,
            dark_blue,
            dark_green,
            dark_aqua, 
            dark_red, 
            dark_purple,
            gold, gray,
            dark_gray,
            blue,
            green,
            aqua,
            red,
            light_purple,
            yellow,
            white,
            obfuscated,
            bold,
            strikethrough,
            underlined,
            italic,
            reset
        };

        // tpnaeu is team_player_names_and_entity_uuids
        virtual std::vector<uint8_t> TeamCreate(std::string name, Chat display_name, bool can_friendly_fire, bool can_see_invis_teampate, TeamNameTagVisibility ntv_rule, TeamCollisionRule c_rule, TeamColor color, Chat prefix, Chat sufix, std::vector<std::string> tpnaeu) {
            std::vector<uint8_t> packet;
            packet.push_back(0x55);
            WriteString(packet, name, 16);
            packet.push_back(0);
            WriteString(packet, display_name.ToStr());
            packet.push_back(can_friendly_fire & can_see_invis_teampate << 1);
            WriteString(packet, to_string(ntv_rule));
            WriteString(packet, to_string(c_rule));
            packet.push_back((uint8_t)color);
            WriteString(packet, prefix.ToStr());
            WriteString(packet, sufix.ToStr());
            WriteVar((int32_t)tpnaeu.size(), packet);
            for (auto& it : tpnaeu)
                WriteString(packet, it, 40);
            return packet;
        }
        virtual std::vector<uint8_t> TeamRemove(std::string name) {
            std::vector<uint8_t> packet;
            packet.push_back(0x55);
            WriteString(packet, name, 16);
            packet.push_back(1);
            return packet;
        }
        virtual std::vector<uint8_t> TeamUpdate(std::string name, Chat display_name, bool can_friendly_fire, bool can_see_invis_teampate, TeamNameTagVisibility ntv_rule, TeamCollisionRule c_rule, TeamColor color, Chat prefix, Chat sufix) {
            std::vector<uint8_t> packet;
            packet.push_back(0x55);
            WriteString(packet, name, 16);
            packet.push_back(2);
            WriteString(packet, display_name.ToStr());
            packet.push_back(can_friendly_fire & can_see_invis_teampate << 1);
            WriteString(packet, to_string(ntv_rule));
            WriteString(packet, to_string(c_rule));
            packet.push_back((uint8_t)color);
            WriteString(packet, prefix.ToStr());
            WriteString(packet, sufix.ToStr());
            return packet;
        }
        virtual std::vector<uint8_t> TeamAddEntitys(std::string name, std::vector<std::string> tpnaeu) {
            std::vector<uint8_t> packet;
            packet.push_back(0x55);
            WriteString(packet, name, 16);
            packet.push_back(3);
            WriteVar((int32_t)tpnaeu.size(), packet);
            for (auto& it : tpnaeu)
                WriteString(packet, it, 40);
            return packet;
        }
        virtual std::vector<uint8_t> TeamRemoveEntitys(std::string name, std::vector<std::string> tpnaeu) {
            std::vector<uint8_t> packet;
            packet.push_back(0x55);
            WriteString(packet, name, 16);
            packet.push_back(4);
            WriteVar((int32_t)tpnaeu.size(), packet);
            for (auto& it : tpnaeu)
                WriteString(packet, it, 40);
            return packet;
        }
    #pragma endregion

        std::vector<uint8_t> DefineScore(std::string endity_uuid_or_player_name,std::string objective_name,int32_t value) {
            std::vector<uint8_t> packet;
            packet.push_back(0x56);
            WriteString(packet, endity_uuid_or_player_name, 40);
            packet.push_back(0);
            WriteString(packet, objective_name,16);
            WriteVar(value,packet);
            return packet;
        }
        std::vector<uint8_t> RemoveScore(std::string endity_uuid_or_player_name, std::string objective_name) {
            std::vector<uint8_t> packet;
            packet.push_back(0x56);
            WriteString(packet, endity_uuid_or_player_name, 40);
            packet.push_back(1);
            WriteString(packet, objective_name, 16);
            return packet;
        }

        virtual std::vector<uint8_t> SetTitleSubText(Chat subtext) {
            std::vector<uint8_t> packet;
            packet.push_back(0x57);
            std::string str = subtext.ToStr();
            for (char it : str)
                packet.push_back(it);
            return packet;
        }

        //if day_time == -1 sun and moon stop moving
        virtual std::vector<uint8_t> TimeUpdate(int64_t world_time, int64_t day_time) {
            std::vector<uint8_t> packet;
            packet.push_back(0x58);
            WriteValue(world_time, packet);
            WriteValue(day_time, packet);
            return packet;
        }

        virtual std::vector<uint8_t> SetTitleText(Chat text) {
            std::vector<uint8_t> packet;
            packet.push_back(0x59);
            std::string str = text.ToStr();
            for (char it : str)
                packet.push_back(it);
            return packet;
        }

        virtual std::vector<uint8_t> SetTitleTime(int32_t showing, int32_t staying, int32_t hiding) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5A);
            WriteValue(showing, packet);
            WriteValue(staying, packet);
            WriteValue(hiding, packet);
            return packet;
        }


    #pragma region Sound
        enum class SoundCategory {
            master, music, record, weather, block, hostile, neutral, player, ambient, voice
        };
        //volume will be betwen 0.0 - 1.0, pitch will be betwen 0.5 - 2.0
        virtual std::vector<uint8_t> CallHardcodedSoundFromEntity(int32_t sound_id, SoundCategory sound_category, int32_t entity_id, float volume, float pitch) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5B);
            WriteVar(sound_id, packet);
            WriteVar((int32_t)sound_category, packet);
            WriteVar(entity_id, packet);
            WriteValue(volume, packet);
            WriteValue(pitch, packet);
            return packet;
        }
        //source_# will be multipled by 8, volume will be betwen 0.0 - 1.0, pitch will be betwen 0.5 - 2.0
        virtual std::vector<uint8_t> CallHardcodedSound(int32_t sound_id, SoundCategory sound_category, int32_t source_x, int32_t source_y, int32_t source_z, float volume, float pitch) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5C);
            WriteVar(sound_id, packet);
            WriteVar((int32_t)sound_category, packet);
            WriteValue(source_x * 8, packet);
            WriteValue(source_y * 8, packet);
            WriteValue(source_z * 8, packet);
            WriteValue(volume, packet);
            WriteValue(pitch, packet);
            return packet;
        }
        virtual std::vector<uint8_t> StopAllSound() {
            TransferPacket({0x5D,0});
        }
        virtual std::vector<uint8_t> StopSound(int32_t source) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5D);
            packet.push_back(1);
            WriteVar(source, packet);
            return packet;
        }
        virtual std::vector<uint8_t> StopSound(std::string sound_name) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5D);
            packet.push_back(2);
            WriteString(packet,sound_name,INT16_MAX);
            return packet;
        }
        virtual std::vector<uint8_t> StopSound(int32_t source, std::string sound_name) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5D);
            packet.push_back(3);
            WriteVar(source, packet);
            WriteString(packet, sound_name, INT16_MAX);
            return packet;
        }
    #pragma endregion
        virtual std::vector<uint8_t> DecoratePlayersList(Chat header, Chat footer) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5E);
            for (char it : header.ToStr())
                packet.push_back(it);
            for (char it : footer.ToStr())
                packet.push_back(it);
            return packet;
        }

        virtual std::vector<uint8_t> NbtQueryAnswer(int32_t transaction_id,Position block_pos) {
            std::vector<uint8_t> packet;
            packet.push_back(0x5F);
            WriteVar(transaction_id, packet);
            {
                std::vector<uint8_t> encoded = NBT(BlockRequest(block_pos.x, block_pos.y, block_pos.z).get_nbt());
                packet.insert(packet.end(), encoded.begin(), encoded.end());
            }
            return packet;
        }
        virtual std::vector<uint8_t> NbtQueryAnswer(int32_t transaction_id, int32_t entity_id) {
            //TO-DO
        }

        //used for items and xp orbs
        virtual std::vector<uint8_t> EntityCollectedEntity(int32_t collector_entity_id, int32_t collected_entity_id, int32_t collected_count) {
            std::vector<uint8_t> packet;
            packet.push_back(0x60);
            WriteValue(collected_entity_id, packet);
            WriteValue(collector_entity_id, packet);
            WriteValue(collected_count, packet);
            return packet;
        }

        virtual std::vector<uint8_t> TeleportEntity(const ENBT::UUID& entity, double x, double y, double z, uint8_t yaw_angle, uint8_t pitch_angle, bool on_ground) {
            std::vector<uint8_t> packet;
            packet.push_back(0x61);
            WriteUUID(entity, packet);
            WriteValue(x, packet);
            WriteValue(y, packet);
            WriteValue(z, packet);
            packet.push_back(yaw_angle);
            packet.push_back(pitch_angle);
            packet.push_back(on_ground);
            return packet;
        }
#pragma endregion


        TCPClientHandlePlay(TCPsession& session, WorldClusters& world, block_pos_t cluster_x, block_pos_t cluster_z, block_pos_t cluster_handle_distance) : WorldClusterTalker(world, cluster_x, cluster_z, cluster_handle_distance){
            client_sock = &session;
            mc_keep_alive_timeout = new boost::asio::deadline_timer(session.sock.get_executor(), boost::posix_time::seconds(2));
        }
        TCPClientHandlePlay(TCPsession* session, WorldClusters& world, block_pos_t cluster_x, block_pos_t cluster_z, block_pos_t cluster_handle_distance) : WorldClusterTalker(world, cluster_x, cluster_z, cluster_handle_distance) {
            client_sock = session;
            mc_keep_alive_timeout = new boost::asio::deadline_timer(session->sock.get_executor(), boost::posix_time::seconds(2));
        }
        ~TCPClientHandlePlay() override {
            if (mc_keep_alive_timeout)
                delete mc_keep_alive_timeout;
        }
        TCPclient* DefineOurself(baip::tcp::socket& sock) override {
            return nullptr;
        }

        Response WorkClient(const std::vector<std::vector<uint8_t>>& clientData, uint64_t timeout_ms) override {
            if (!client_sock)
                return Response({}, 1);
            if (keep_alive_enabled) 
                if (TimeoutCheck(timeout_ms))
                    return Response({}, 1);
            return WorkPackets(clientData);
        }
        bool DoDisconnect(baip::address ip) override {
            return banned_players.contains(ip);
        }


        boost::asio::deadline_timer* mc_keep_alive_timeout;
    };
}
//to implement
// 0x26
// 0x27
// 0x28
// 0x29
// 0x2A
// 0x2B
// 0x2C
// 0x2D
// 0x2E
// 0x2F
// 0x31
// 0x32
// 0x35
// 0x36
// 0x37
// 0x38
// 0x39
// 0x3A
// 0x3B
// 0x3C
// 0x3D
// 0x3E
// 0x3F
// 0x40
// 0x49
// 0x4B
// 0x4C
// 0x4D
// 0x62
// 0x63
// 0x64
// 0x65
// 0x66
class TCPClientHandleLogin : public TCPClientHandle {
public:
    static bool offline_mode;
protected:
    static int compres_threshold;
    static bool compression_enabled;
    //enable compression, returns -1 if disable or maximum compressed data before compress
    virtual int32_t CompressionEnabled() {
        return -1;
    }
    virtual Chat* AllowPlayersName(std::string nick) {
        return new Chat("server closed");
    }

    int8_t waitLoginStage = 0;
    virtual Response WorkPacket(std::vector<uint8_t> packet) {
        ArrayStream data(packet.begin()._Ptr, packet.size());
        int8_t actual_login_stage = data.read();
        if (waitLoginStage != actual_login_stage)
            return Response({}, 1);
        switch (actual_login_stage) {
        case 0:
        {
            std::string nickname = ReadString(data, 16);
            Chat* kick_reason_chat = AllowPlayersName(nickname);
            if (kick_reason_chat != nullptr) {
                std::string kick_reason = kick_reason_chat->ToStr();
                delete kick_reason_chat;
                std::vector<uint8_t> response;
                response.push_back(0);
                WriteVar((int32_t)kick_reason.size(), response);
                for (char it : kick_reason)
                    response.push_back(it);
                return Response({ response }, 0, 1);
            }
            waitLoginStage = 1;
        }
        break;
        case 1:
            break;
        }
        return Response({});
    }


    bool player_authed = false;
    virtual bool allowUnauthed() { return true; };
    virtual std::vector<std::string> authServers() {
        return { {} };
    };
    virtual bool enableEncryption() { return false; };
    baip::tcp::socket* client_sock;
public:
    TCPClientHandleLogin(baip::tcp::socket& sock) {
        client_sock = &sock;
    }
    TCPClientHandleLogin(baip::tcp::socket* sock, int32_t protocol) {
        client_sock = sock;
        protocol_version = protocol;
    }
    TCPClientHandleLogin() {
        client_sock = nullptr;
    }
    TCPclient* DefineOurself(baip::tcp::socket& sock) override {
        return new TCPClientHandleLogin(sock);
    }
};
class TCPClientHandleStatus : public TCPClientHandle {
protected:
    enum class IllegalDataResponse {
        none,
        disconnect,
        switch_to_next_handler
    };
    virtual IllegalDataResponse AllowProtocolVersion(int proto_version) {
        if (756 == proto_version)
            return IllegalDataResponse::none;
        else
            return IllegalDataResponse::none;
    }
    virtual IllegalDataResponse AllowServerAdressAndPort(std::string& str, uint16_t port) {
        return IllegalDataResponse::none;
    }
    //return empty chat string if al normal
    virtual std::string AllowPlayersName(std::string& nick) {
        // return "{\"text\":\"Server closed!\"}";
        return "";
    }

    //response status
    virtual std::string buildResponse() {
        uint32_t max_online = 0;
        uint32_t online = 0;
        bool online_show = true;
        if (PlayersHidden()) {
            max_online = StatusResponseMaxPlayers();
            online = StatusResponseOnline();
            online_show = false;
        }
        //auto&& sample = StatusResponseOnlinePlayers();
        //bool sample_show = sample.size();
        std::string res =
            "{"
            "\"version\": {"
            "\"name\": \"" + StatusResponseVersionName() + "\","
            "\"protocol\": " + StatusResponseProtocol() + 
            "},";

        if (online_show /*| sample_show*/) {
            res += "\"players\":{";
            if (online_show) {
                res += "\"max\":" + max_online;
                res += ",\"online\":" + online;
            }
            res += "},";
        }




        res += "\"description\":" + StatusResponseDescription().ToStr();

        std::string base64_fav = StatusResponseFavicon();
        if (base64_fav == "")
            res += "\n}";
        else
            res += ",\"favicon\": \"data:image/png" +
            ';' + base64_fav +
            ",<data>\"\n}";
        return res;
    }
    //  base64 encoded png Favicon
    virtual std::string StatusResponseFavicon() {
        return "";
    }
    virtual Chat StatusResponseDescription() {
        Chat hello;
        hello.SetText("Hello world!");
        hello.SetColor("blue");

        return hello;
    }
    virtual std::string StatusResponseVersionName() {
        return "1.17.1";
    }
    virtual std::string StatusResponseProtocol() {
        return std::to_string(protocol_version);
    }
    virtual uint32_t StatusResponseMaxPlayers() {
        return 1;
    }
    virtual uint32_t StatusResponseOnline() {
        return 0;
    }
    virtual bool PlayersHidden() {
        return true;
    }
    //if return empty, sample will not be shown
    virtual std::vector<std::string> StatusResponseOnlinePlayers() {
        //return {"Test World","Hello!:)"};
        return {};
    }

    Response WorkPacket(std::vector<uint8_t> packet) override {
        if(packet.size()==1){
            std::vector<uint8_t> responge;
            responge.push_back('\0');
            std::string&& tmp = buildResponse();
            WriteVar((int32_t)tmp.size(), responge);
            responge.insert(responge.end(), tmp.begin(), tmp.end());
            return Response({ responge });
        }
        else
            return Response({ packet },0,1);
    }
public:
    TCPClientHandleStatus(int32_t protocol) {
        protocol_version = protocol;
    }
    TCPclient* DefineOurself(baip::tcp::socket& sock) override {
        return new TCPClientHandleStatus(-1);
    }
};
class TCPClientHandleHandshaking : public TCPClientHandle {
protected:
    virtual bool AllowProtocolVersion(int proto_version) {
        if (756 == proto_version)
            return true;
        else
            return true;
    }
    virtual bool AllowServerAdressAndPort(std::string& str, uint16_t port) {
        return true;
    }
    //return empty chat string if al normal
    virtual std::string AllowPlayersName(std::string& nick) {
        // return "{\"text\":\"Server closed!\"}";
        return "";
    }
    template<class T>
    T readValue(ArrayStream& data) {
        uint8_t tmp[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return ENBT::ConvertNativeEndian(std::endian::big, *(T*)tmp);
    }
    Response WorkPacket(std::vector<uint8_t> packet) override {
        ArrayStream data(packet.begin()._Ptr, packet.size());
        uint8_t tmp = data.read();
        if (tmp != '\0')
            return Response({}, 1);

        int32_t protocol_version = ReadVar<int32_t>(data);

        if(!AllowProtocolVersion(protocol_version))
            return Response({}, 1);
        

        std::string server_adress = ReadString(data, 255);
        if (!AllowServerAdressAndPort(server_adress, readValue<uint16_t>(data)))
            return Response({}, 1);

        switch (ReadVar<int32_t>(data)) {
        case 1:
            next_handler = new TCPClientHandleStatus(protocol_version);
            return Response({});
        case 2:
            next_handler = new TCPClientHandleLogin(client_sock, protocol_version);
            return Response({});
        default:
            return Response({}, 1);
        }
    }
    baip::tcp::socket* client_sock;
public:
    TCPClientHandleHandshaking(baip::tcp::socket& sock) {
        client_sock = &sock;
    }
    TCPClientHandleHandshaking() {
        client_sock = nullptr;
    }

    TCPclient* DefineOurself(baip::tcp::socket& sock ) override {
        return new TCPClientHandleHandshaking(sock);
    }
    TCPclient* RedefineHandler() override {
        return next_handler;
    }
    Response WorkClient(const std::vector<std::vector<uint8_t>>& clientData, uint64_t timeout_ms) override {
        if (!client_sock)
            return Response({}, 1);
        return WorkPackets(clientData);
    }
    bool DoDisconnect(baip::address ip) override {
        return banned_players.contains(ip);
    }
};
