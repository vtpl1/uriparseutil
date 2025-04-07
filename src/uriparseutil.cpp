// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#include "uriparseutil.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
// #include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
// trim from start (in place)
inline void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
inline void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string& s) {
  rtrim(s);
  ltrim(s);
}

std::string normalizedIpAddress(const std::string& ip) {
  std::stringstream  ss;
  std::istringstream iss(ip);
  std::string        token;
  int                i = 0;
  while (std::getline(iss, token, '.')) {
    try {
      ss << (((i++) != 0) ? "_" : "") << std::to_string(std::stoi(token)).insert(0, 3 - token.length(), '0');
    } catch (...) {
      return ip;
    }
  }
  return ss.str();
}

std::string normalizedQuery(std::string query) {
  std::replace_if(
      query.begin(), query.end(),
      [](char c) {
        return c == '?' || c == '%' || c == '&' || c == '=' || c == '/' || c == '.' || c == '-' || c == '<' ||
               c == '>' || c == ':' || c == '"' || c == '\\' || c == '|' || c == '*';
      },
      '_');
  std::transform(query.begin(), query.end(), query.begin(), ::tolower);
  return query;
}

std::unordered_map<std::string, std::string> parseJson(const std::string& json_str) {
  std::unordered_map<std::string, std::string> result;

  std::string key, value;
  bool        in_key = false, in_value = false;
  bool        is_string = false;

  for (size_t i = 0; i < json_str.length(); ++i) {
    char c = json_str[i];
    if (c == '"') {
      is_string = !is_string;
      continue;
    }
    if (is_string) {
      if (in_key)
        key += c;
      else if (in_value)
        value += c;
    } else {
      if (c == ':') {
        in_key   = false;
        in_value = true;
      } else if (c == ',' || c == '}') {
        if (!key.empty() && !value.empty()) {
          result[key] = value;
        }
        key.clear();
        value.clear();
        in_key   = true;
        in_value = false;
      } else if (c != '{' && c != ' ') {
        if (in_key)
          key += c;
        else if (in_value)
          value += c;
      }
    }
  }
  return result;
}

std::string uriEncode(const std::string& value) {
  std::ostringstream encoded;
  encoded << std::hex << std::uppercase;

  for (unsigned char c : value) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else {
      encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
  }

  return encoded.str();
}
std::string uriDecode(const std::string& value) {
  std::ostringstream decoded;
  size_t             len = value.length();

  for (size_t i = 0; i < len; ++i) {
    if (value[i] == '%' && i + 2 < len) {
      int                hexValue;
      std::istringstream hexStream(value.substr(i + 1, 2));
      if (hexStream >> std::hex >> hexValue) {
        decoded << static_cast<char>(hexValue);
        i += 2;
      } else {
        decoded << '%';
      }
    } else if (value[i] == '+') {
      decoded << ' ';
    } else {
      decoded << value[i];
    }
  }

  return decoded.str();
}

} // namespace

// rtsp://172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4#site=1#channel=2#app=3
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4?extra=%23channel%3D2%23app%3D3#site=1#channel=2#app=3
// vms://172.16.1.173:3005/44/0
// vms://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// grpc://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// videos/1.avf

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
vtpl::utilities::UriDetails vtpl::utilities::parseUri(const std::string& uri_in) {
  vtpl::utilities::UriDetails uri_details;
  std::istringstream          iss(uri_in);
  std::vector<std::string>    v;
  std::string                 token;
  while (iss >> token) {
    v.push_back(token);
  }

  std::string uri = uri_in;
  if (v.size() >= 3) {
    uri                  = v[0];
    uri_details.username = uriDecode(v[1]);
    uri_details.password = uriDecode(v[2]);
  }

  std::string relative_path_and_fragment;

  auto pos_scheme = uri.find("://");
  if (pos_scheme == std::string::npos) {
    auto pos_dot = uri.rfind('.');
    if (pos_dot != std::string::npos) {
      uri_details.scheme = uri.substr(pos_dot + 1);
    }
    relative_path_and_fragment = uri;
  } else {
    uri_details.scheme = uri.substr(0, pos_scheme);
    relative_path_and_fragment = uri.substr(pos_scheme + 3);
    if ((uri_details.scheme.find("vms") == 0) || (uri_details.scheme.find("grpc") == 0) ||
        (uri_details.scheme.find("http") == 0) || (uri_details.scheme.find("rtsp") == 0)) {
      auto pos_host = pos_scheme + 3;
      auto pos_path = uri.find('/', pos_host);

      std::string host_port_and_user_pass = uri.substr(pos_host, pos_path - pos_host);

      relative_path_and_fragment = (pos_path != std::string::npos) ? uri.substr(pos_path) : "";

      auto pos_at = host_port_and_user_pass.rfind('@');
      if (pos_at != std::string::npos) {
        const std::string userinfo = host_port_and_user_pass.substr(0, pos_at);
        host_port_and_user_pass    = host_port_and_user_pass.substr(pos_at + 1);
        auto pos_colon             = userinfo.find(':');
        if (pos_colon != std::string::npos) {
          uri_details.username = uriDecode(userinfo.substr(0, pos_colon));
          uri_details.password = uriDecode(userinfo.substr(pos_colon + 1));
        } else {
          uri_details.username = uriDecode(userinfo);
        }
      }
      auto pos_port = host_port_and_user_pass.rfind(':');
      if (pos_port != std::string::npos) {
        uri_details.port = std::stoi(host_port_and_user_pass.substr(pos_port + 1));
        uri_details.host = host_port_and_user_pass.substr(0, pos_port);
      } else {
        uri_details.host = host_port_and_user_pass;
      }
    }
  }

  auto pos_fragment = relative_path_and_fragment.find('#');
  if (pos_fragment != std::string::npos) {
    uri_details.relative_path  = relative_path_and_fragment.substr(0, pos_fragment);
    const std::string fragment = relative_path_and_fragment.substr(pos_fragment + 1);
    uri_details.channel        = vtpl::utilities::Channel::fromString(fragment);
  } else {
    if (!relative_path_and_fragment.empty()) {
      uri_details.relative_path = relative_path_and_fragment;
    }
  }

  if (uri_details.relative_path) {
    const std::string relative_path = uri_details.relative_path.value();
    if ((uri_details.scheme.find("vms") == 0) || (uri_details.scheme.find("grpc") == 0)) {
      std::istringstream       path_stream(relative_path);
      std::vector<std::string> parts;
      std::string              part;
      while (std::getline(path_stream, part, '/')) {
        if (!part.empty()) {
          parts.push_back(part);
        }
      }
      if (parts.size() == 2) {
        uri_details.channel.channel_id  = std::stoi(parts.at(0));
        uri_details.channel.stream_type = std::stoi(parts.at(1));
      } else {
        for (size_t i = 0; i < parts.size() - 1; i += 2) {
          if (parts.at(i) == "site") {
            uri_details.channel.site_id = std::stoi(parts.at(i + 1));
          } else if (parts.at(i) == "channel") {
            uri_details.channel.channel_id = std::stoi(parts.at(i + 1));
          }
        }
      }
    }

    if (uri_details.scheme.empty()) { // scheme is still empty
      auto pos_dot = relative_path.rfind('.');
      if (pos_dot != std::string::npos) {
        uri_details.scheme = relative_path.substr(pos_dot + 1);
      }
    }
  }

  if (uri_details.scheme == "grpc") {
    uri_details.scheme = "grpcframe";
  }

  if (!uri_details.scheme.empty()) {
    std::stringstream ss;
    ss << uri_details.scheme << "://";
    if (uri_details.host) {
      ss << uri_details.host.value();
    }
    if (uri_details.port) {
      ss << ":";
      ss << uri_details.port.value();
    }
    if (uri_details.relative_path) {
      ss << uri_details.relative_path.value();
    }
    uri_details.url = ss.str();
  }

  return uri_details;
}

std::string vtpl::utilities::normalizeUri(const std::string& uri) {
  const vtpl::utilities::UriDetails details = vtpl::utilities::parseUri(uri);
  return normalizedIpAddress(details.host.value_or("")) + "_" + std::to_string(details.port.value_or(0)) + "_" +
         normalizedQuery(details.relative_path.value_or(""));
}

std::string vtpl::utilities::Channel::toString() {
  std::ostringstream ss;
  if (site_id) {
    ss << "#site=" << *site_id;
  }
  if (channel_id) {
    ss << "#channel=" << *channel_id;
  }
  if (app_id) {
    ss << "#app=" << *app_id;
  }
  if (live_or_rec) {
    ss << "#live=" << *live_or_rec;
  }
  if (stream_type) {
    ss << "#stream=" << *stream_type;
  }
  if (media_type) {
    ss << "#media=" << *media_type;
  }
  if (start_ts) {
    ss << "#timestamp=" << *start_ts;
  }
  if (session_id) {
    ss << "#session=" << *session_id;
  }
  return ss.str();
}

vtpl::utilities::Channel vtpl::utilities::Channel::fromString(std::string str) {
  vtpl::utilities::Channel ch;
  std::istringstream       iss(str);
  std::string              segment;
  while (std::getline(iss, segment, '#')) {
    auto pos = segment.find('=');
    if (pos != std::string::npos) {
      const std::string key = segment.substr(0, pos);
      if (key == "site") {
        ch.site_id = std::stoi(segment.substr(pos + 1));
      } else if (key == "channel") {
        ch.channel_id = std::stoi(segment.substr(pos + 1));
      } else if (key == "app") {
        ch.app_id = std::stoi(segment.substr(pos + 1));
      } else if (key == "live") {
        ch.live_or_rec = std::stoi(segment.substr(pos + 1));
      } else if (key == "stream") {
        ch.stream_type = std::stoi(segment.substr(pos + 1));
      } else if (key == "media") {
        ch.media_type = std::stoi(segment.substr(pos + 1));
      } else if (key == "timestamp") {
        ch.start_ts = std::stoull(segment.substr(pos + 1));
      } else if (key == "session") {
        ch.session_id = segment.substr(pos + 1);
      }
    }
  }
  return ch;
}

std::string vtpl::utilities::Channel::toJSON() const {
  std::ostringstream oss;
  oss << "{";
  bool first = true;

  auto add_field = [&](const std::string& key, const auto& value) {
    if (value) {
      if (!first)
        oss << ", ";
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
  add_field("session_id", session_id);

  oss << "}";
  return oss.str();
}

vtpl::utilities::Channel vtpl::utilities::Channel::fromJSON(std::string jsonStr) {
  auto    data = parseJson(jsonStr);
  Channel channel;
  if (data.find("site_id") != data.end()) {
    channel.site_id = std::stoul(data.at("site_id"));
  }
  if (data.find("channel_id") != data.end()) {
    channel.channel_id = std::stoul(data.at("channel_id"));
  }
  if (data.find("app_id") != data.end()) {
    channel.app_id = std::stoul(data.at("app_id"));
  }
  if (data.find("live_or_rec") != data.end()) {
    channel.live_or_rec = std::stoul(data.at("live_or_rec"));
  }
  if (data.find("stream_type") != data.end()) {
    channel.stream_type = std::stoul(data.at("stream_type"));
  }
  if (data.find("media_type") != data.end()) {
    channel.media_type = std::stoul(data.at("media_type"));
  }
  if (data.find("start_ts") != data.end()) {
    channel.start_ts = std::stoull(data.at("start_ts"));
  }
  if (data.find("session_id") != data.end()) {
    channel.session_id = data.at("session_id");
  }
  return channel;
}

std::string vtpl::utilities::UriDetails::toString() {
  std::stringstream ss;
  ss << "URI Details:\n";
  ss << "scheme: " << scheme << "\n";
  if (url) {
    ss << "url: " << *url << "\n";
  }
  if (host) {
    ss << "host: " << *host << "\n";
  }
  if (port) {
    ss << "port: " << *port << "\n";
  }
  if (username) {
    ss << "username: " << *username << "\n";
  }
  if (password) {
    ss << "password: " << *password << "\n";
  }
  if (relative_path) {
    ss << "relative_path: " << *relative_path << "\n";
  }
  ss << "channel: " << channel.toString();
  return ss.str();
}

std::string vtpl::utilities::UriDetails::toJSON() {
  std::ostringstream oss;
  oss << "{";
  bool first = true;

  auto add_field = [&](const std::string& key, const auto& value) {
    if (value) {
      if (!first)
        oss << ", ";
      oss << "\"" << key << "\": \"" << *value << "\"";
      first = false;
    }
  };
  add_field("scheme", std::optional(scheme));
  // oss << "\"scheme\": \"" << scheme << "\"";
  add_field("url", url);
  add_field("host", host);
  add_field("username", username);
  add_field("password", password);
  add_field("relative_path", relative_path);
  if (port) {
    if (!first)
      oss << ", ";
    oss << "\"port\": " << *port;
    first = false;
  }
  oss << ", \"channel\": " << channel.toJSON();

  oss << "}";
  return oss.str();
}

vtpl::utilities::Channel vtpl::utilities::parseChannel(const std::string& channel) {
  return vtpl::utilities::Channel::fromString(channel);
}

std::string vtpl::utilities::UriDetails::getComositeString() {
  std::stringstream ss;
  if (!scheme.empty()) {
    ss << scheme;
    ss << "://";
    if (username) {
      ss << uriEncode(username.value());
      if (password) {
        ss << ':';
        ss << uriEncode(password.value());
      }
      ss << '@';
    }
  }
  if (host) {
    ss << host.value();
    if (port) {
      ss << ":" << port.value();
    }
  }
  if (relative_path.has_value() == false) {
    if ((scheme.find("vms") == 0) || (scheme.find("grpc") == 0)) {
      std::stringstream sss;
      if (channel.site_id) {
        sss << "/site/";
        sss << channel.site_id.value();
        if (channel.channel_id) {
          sss << "/channel/";
          sss << channel.channel_id.value();
        }
        if (!sss.str().empty()) {
          relative_path = sss.str();
        }
      } else {
        if (channel.channel_id) {
          sss << '/';
          sss << channel.channel_id.value();
        }
        if (channel.stream_type) {
          sss << '/';
          sss << channel.stream_type.value();
        }
        if (!sss.str().empty()) {
          relative_path = sss.str();
        }
      }
    }
  }

  if (relative_path) {
    ss << relative_path.value();
  }

  if (!channel.toString().empty()) {
    ss << channel.toString();
  }
  // std::cout << "ddddddddd " << ss.str() << std::endl;
  return ss.str();
}