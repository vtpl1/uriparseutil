// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#include "uriparseutil.h"
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string normalizedIpAddress(const std::string& ip) {
  std::stringstream  ss;
  std::istringstream iss(ip);
  std::string        token;
  int                i = 0;
  while (std::getline(iss, token, '.')) {
    try {
      ss << (i++ ? "_" : "") << std::to_string(std::stoi(token)).insert(0, 3 - token.length(), '0');
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

} // namespace

// rtsp://172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4#site=1#channel=2#app=3
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4?extra=%23channel%3D2%23app%3D3#site=1#channel=2#app=3
// vms://172.16.1.173:3005/44/0
// vms://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// grpc://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// videos/1.avf

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
    uri_details.username = v[1];
    uri_details.password = v[2];
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
    auto pos_host      = pos_scheme + 3;
    auto pos_path      = uri.find('/', pos_host);

    std::string host_port_and_user_pass = uri.substr(pos_host, pos_path - pos_host);

    relative_path_and_fragment = (pos_path != std::string::npos) ? uri.substr(pos_path) : "";

    auto pos_at = host_port_and_user_pass.rfind('@');
    if (pos_at != std::string::npos) {
      std::string userinfo    = host_port_and_user_pass.substr(0, pos_at);
      host_port_and_user_pass = host_port_and_user_pass.substr(pos_at + 1);
      auto pos_colon          = userinfo.find(':');
      if (pos_colon != std::string::npos) {
        uri_details.username = userinfo.substr(0, pos_colon);
        uri_details.password = userinfo.substr(pos_colon + 1);
      } else {
        uri_details.username = userinfo;
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

  auto pos_fragment = relative_path_and_fragment.find('#');
  if (pos_fragment != std::string::npos) {
    uri_details.relative_path = relative_path_and_fragment.substr(0, pos_fragment);
    std::string fragment      = relative_path_and_fragment.substr(pos_fragment + 1);
    uri_details.channel       = parseChannel(fragment);
  } else {
    if (!relative_path_and_fragment.empty()) {
      uri_details.relative_path = relative_path_and_fragment;
    }
  }

  if (uri_details.relative_path) {
    std::string relative_path = uri_details.relative_path.value();
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
  vtpl::utilities::UriDetails details = vtpl::utilities::parseUri(uri);
  return normalizedIpAddress(details.host.value_or("")) + "_" + std::to_string(details.port.value_or(0)) + "_" +
         normalizedQuery(details.relative_path.value_or(""));
}

std::string vtpl::utilities::Channel::toString() {
  std::ostringstream ss;
  if (site_id)
    ss << "#site=" << *site_id;
  if (channel_id)
    ss << "#channel=" << *channel_id;
  if (app_id)
    ss << "#app=" << *app_id;
  if (live_or_rec)
    ss << "#live=" << *live_or_rec;
  if (stream_type)
    ss << "#stream=" << *stream_type;
  if (media_type)
    ss << "#media=" << *media_type;
  if (start_ts)
    ss << "#timestamp=" << *start_ts;
  return ss.str();
}

vtpl::utilities::Channel vtpl::utilities::parseChannel(const std::string& channel) {
  vtpl::utilities::Channel ch;
  std::istringstream       iss(channel);
  std::string              segment;
  while (std::getline(iss, segment, '#')) {
    auto pos = segment.find('=');
    if (pos != std::string::npos) {
      std::string key = segment.substr(0, pos);
      if (key == "site")
        ch.site_id = std::stoi(segment.substr(pos + 1));
      else if (key == "channel")
        ch.channel_id = std::stoi(segment.substr(pos + 1));
      else if (key == "app")
        ch.app_id = std::stoi(segment.substr(pos + 1));
      else if (key == "live")
        ch.live_or_rec = std::stoi(segment.substr(pos + 1));
      else if (key == "stream")
        ch.stream_type = std::stoi(segment.substr(pos + 1));
      else if (key == "media")
        ch.media_type = std::stoi(segment.substr(pos + 1));
      else if (key == "timestamp")
        ch.start_ts = std::stoull(segment.substr(pos + 1));
    }
  }
  return ch;
}