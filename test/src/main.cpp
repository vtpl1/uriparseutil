#include <iostream>
#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>
#include <cstdint>

class Channel {
public:
    std::optional<uint32_t> site_id;
    std::optional<uint32_t> channel_id;
    std::optional<uint32_t> app_id;
    std::optional<uint32_t> live_or_rec;
    std::optional<uint32_t> stream_type;
    std::optional<uint32_t> media_type;
    std::optional<uint64_t> start_ts;

    void print() const {
        std::cout << "Channel Info:\n";
        if (site_id) std::cout << "site_id: " << *site_id << "\n";
        if (channel_id) std::cout << "channel_id: " << *channel_id << "\n";
        if (app_id) std::cout << "app_id: " << *app_id << "\n";
        if (live_or_rec) std::cout << "live_or_rec: " << *live_or_rec << "\n";
        if (stream_type) std::cout << "stream_type: " << *stream_type << "\n";
        if (media_type) std::cout << "media_type: " << *media_type << "\n";
        if (start_ts) std::cout << "start_ts: " << *start_ts << "\n";
    }

    std::string to_json() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;

        auto add_field = [&](const std::string& key, const auto& value) {
            if (value) {
                if (!first) oss << ", ";
                oss << "\"" << key << "\": " << *value;
                first = false;
            }
        };

        add_field("site_id", site_id);
        add_field("channel_id", channel_id);
        add_field("app_id", app_id);
        add_field("live_or_rec", live_or_rec);
        add_field("stream_type", stream_type);
        add_field("media_type", media_type);
        add_field("start_ts", start_ts);

        oss << "}";
        return oss.str();
    }
};

std::unordered_map<std::string, std::string> parse_json(const std::string& json) {
    std::unordered_map<std::string, std::string> map;
    std::string key, value;
    bool in_key = false, in_value = false;
    bool is_number = false;

    for (size_t i = 0; i < json.size(); ++i) {
        if (json[i] == '"') {
            if (!in_key && !in_value) {
                in_key = true;
                key.clear();
            } else if (in_key) {
                in_key = false;
            } else if (in_value && !is_number) {
                in_value = false;
                map[key] = value;
            }
        } else if (in_key) {
            key += json[i];
        } else if (json[i] == ':') {
            in_value = true;
            value.clear();
            is_number = (json[i + 1] >= '0' && json[i + 1] <= '9');
        } else if (json[i] == ',' || json[i] == '}') {
            if (in_value) {
                in_value = false;
                map[key] = value;
            }
        } else if (in_value) {
            value += json[i];
        }
    }
    return map;
}

Channel create_channel_from_json(const std::string& json) {
    auto data = parse_json(json);
    Channel channel;

    auto assign_if_present = [&](const std::string& key, auto& member) {
        auto it = data.find(key);
        if (it != data.end()) {
            std::stringstream ss(it->second);
            uint64_t value;
            ss >> value;
            if constexpr (std::is_same_v<std::decay_t<decltype(member)>, std::optional<uint32_t>>) {
                if (value <= std::numeric_limits<uint32_t>::max()) {
                    member = static_cast<uint32_t>(value);
                } else {
                    std::cerr << "Warning: Value out of range for " << key << "\n";
                }
            } else {
                member = value;
            }
        }
    };

    assign_if_present("site_id", channel.site_id);
    assign_if_present("channel_id", channel.channel_id);
    assign_if_present("app_id", channel.app_id);
    assign_if_present("live_or_rec", channel.live_or_rec);
    assign_if_present("stream_type", channel.stream_type);
    assign_if_present("media_type", channel.media_type);
    assign_if_present("start_ts", channel.start_ts);

    return channel;
}

int main() {
    std::string json = R"({"site_id": 101, "channel_id": 5, "app_id": 42, "live_or_rec": 1, "stream_type": 2, "media_type": 3, "start_ts": 1711824000})";
    Channel channel = create_channel_from_json(json);
    channel.print();
    std::cout << "JSON Output: " << channel.to_json() << "\n";
    return 0;
}
