#include <cstdint>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

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
    if (site_id)
      std::cout << "site_id: " << *site_id << "\n";
    if (channel_id)
      std::cout << "channel_id: " << *channel_id << "\n";
    if (app_id)
      std::cout << "app_id: " << *app_id << "\n";
    if (live_or_rec)
      std::cout << "live_or_rec: " << *live_or_rec << "\n";
    if (stream_type)
      std::cout << "stream_type: " << *stream_type << "\n";
    if (media_type)
      std::cout << "media_type: " << *media_type << "\n";
    if (start_ts)
      std::cout << "start_ts: " << *start_ts << "\n";
  }
};

class UriDetails {
public:
  std::string                scheme;
  std::optional<std::string> url;
  std::optional<std::string> host;
  std::optional<uint32_t>    port;
  std::optional<std::string> username;
  std::optional<std::string> password;
  std::optional<std::string> relative_path;
  Channel                    channel;

  void print() const {
    std::cout << "URI Details:\n";
    std::cout << "scheme: " << scheme << "\n";
    if (url)
      std::cout << "url: " << *url << "\n";
    if (host)
      std::cout << "host: " << *host << "\n";
    if (port)
      std::cout << "port: " << *port << "\n";
    if (username)
      std::cout << "username: " << *username << "\n";
    if (password)
      std::cout << "password: " << *password << "\n";
    if (relative_path)
      std::cout << "relative_path: " << *relative_path << "\n";
    channel.print();
  }
};

std::unordered_map<std::string, std::string> parse_json_to_map(const std::string& json_str) {
  std::unordered_map<std::string, std::string> result;
  std::string                                  key, value;
  bool                                         in_key = false, in_value = false;
  bool                                         is_string = false;

  for (size_t i = 0; i < json_str.length(); ++i) {
    char c = json_str[i];

    if (c == '"') {
      if (!in_key) {
        in_key = true;
        key.clear();
      } else if (in_key && !in_value) {
        in_key   = false;
        in_value = true;
        value.clear();
        if (json_str[i + 1] == '"')
          is_string = true;
      } else if (in_value && is_string) {
        in_value    = false;
        result[key] = value;
        is_string   = false;
      }
    } else if (in_key) {
      key += c;
    } else if (in_value) {
      if (c == ',' || c == '}' || c == ' ') {
        if (!is_string) {
          in_value    = false;
          result[key] = value;
        }
      } else {
        value += c;
      }
    }
  }
  return result;
}

// std::unordered_map<std::string, std::string> parse_json_to_map(const std::string& json_str) {
//     std::unordered_map<std::string, std::string> result;
//     std::string key, value;
//     bool in_key = false, in_value = false;
//     bool is_string = false;

//     for (size_t i = 0; i < json_str.length(); ++i) {
//         char c = json_str[i];
//         if (c == '"') {
//             is_string = !is_string;
//             continue;
//         }
//         if (is_string) {
//             if (in_key)
//                 key += c;
//             else if (in_value)
//                 value += c;
//         } else {
//             if (c == ':') {
//                 in_key = false;
//                 in_value = true;
//             } else if (c == ',' || c == '}') {
//                 if (!key.empty() && !value.empty()) {
//                     result[key] = value;
//                 }
//                 key.clear();
//                 value.clear();
//                 in_key = true;
//                 in_value = false;
//             } else if (c != '{' && c != ' ') {
//                 if (in_key)
//                     key += c;
//                 else if (in_value)
//                     value += c;
//             }
//         }
//     }
//     return result;
// }

int main() {
  std::string json_str =
      R"({"scheme": "https", "url": "https://example.com", "host": "example.com", "port": "443", "username": "user", "password": "pass", "relative_path": "/path", "channel": {"site_id": "101", "channel_id": "5", "app_id": "42", "live_or_rec": "1", "stream_type": "2", "media_type": "3", "start_ts": "1711824000"}})";
  auto parsed_data = parse_json_to_map(json_str);
  for (const auto& [key, value] : parsed_data) {
    std::cout << key << " : " << value << "\n";
  }
  return 0;
}
