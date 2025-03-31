#include <iostream>
#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>
#include <cstdint>
#include <regex>

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

    static Channel from_json(const std::unordered_map<std::string, std::string>& map) {
        Channel channel;
        if (map.find("site_id") != map.end()) channel.site_id = std::stoul(map.at("site_id"));
        if (map.find("channel_id") != map.end()) channel.channel_id = std::stoul(map.at("channel_id"));
        if (map.find("app_id") != map.end()) channel.app_id = std::stoul(map.at("app_id"));
        if (map.find("live_or_rec") != map.end()) channel.live_or_rec = std::stoul(map.at("live_or_rec"));
        if (map.find("stream_type") != map.end()) channel.stream_type = std::stoul(map.at("stream_type"));
        if (map.find("media_type") != map.end()) channel.media_type = std::stoul(map.at("media_type"));
        if (map.find("start_ts") != map.end()) channel.start_ts = std::stoull(map.at("start_ts"));
        return channel;
    }
};

class UriDetails {
public:
    std::string scheme;
    std::optional<std::string> url;
    std::optional<std::string> host;
    std::optional<uint32_t> port;
    std::optional<std::string> username;
    std::optional<std::string> password;
    std::optional<std::string> relative_path;
    Channel channel;

    void print() const {
        std::cout << "URI Details:\n";
        std::cout << "scheme: " << scheme << "\n";
        if (url) std::cout << "url: " << *url << "\n";
        if (host) std::cout << "host: " << *host << "\n";
        if (port) std::cout << "port: " << *port << "\n";
        if (username) std::cout << "username: " << *username << "\n";
        if (password) std::cout << "password: " << *password << "\n";
        if (relative_path) std::cout << "relative_path: " << *relative_path << "\n";
        channel.print();
    }

    std::string to_json() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;

        auto add_field = [&](const std::string& key, const auto& value) {
            if (value) {
                if (!first) oss << ", ";
                oss << "\"" << key << "\": \"" << *value << "\"";
                first = false;
            }
        };

        oss << "\"scheme\": \"" << scheme << "\"";
        add_field("url", url);
        add_field("host", host);
        add_field("username", username);
        add_field("password", password);
        add_field("relative_path", relative_path);
        if (port) {
            if (!first) oss << ", ";
            oss << "\"port\": " << *port;
            first = false;
        }
        oss << ", \"channel\": " << channel.to_json();

        oss << "}";
        return oss.str();
    }

    static UriDetails from_json(const std::unordered_map<std::string, std::string>& map) {
        UriDetails uri;
        if (map.find("scheme") != map.end()) uri.scheme = map.at("scheme");
        if (map.find("url") != map.end()) uri.url = map.at("url");
        if (map.find("host") != map.end()) uri.host = map.at("host");
        if (map.find("port") != map.end()) uri.port = std::stoul(map.at("port"));
        if (map.find("username") != map.end()) uri.username = map.at("username");
        if (map.find("password") != map.end()) uri.password = map.at("password");
        if (map.find("relative_path") != map.end()) uri.relative_path = map.at("relative_path");
        uri.channel = Channel::from_json(map);
        return uri;
    }
};
