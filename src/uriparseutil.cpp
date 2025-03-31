// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#include "uriparseutil.h"

#include <Poco/Path.h>
#include <Poco/URI.h>
#include <absl/strings/ascii.h>
#include <absl/strings/str_replace.h>
#include <absl/strings/str_split.h>
#include <cstddef>
#include <fmt/core.h>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string normalizedIpAddress(const std::string& ip) {
  std::stringstream        ss;
  std::vector<std::string> v = absl::StrSplit(absl::string_view(ip.data(), ip.size()), '.', absl::SkipEmpty());
  auto                     i = 0;
  try {
    for (auto&& str : v) {
      ss << fmt::format("{:03}", std::stoi(str));
      if (++i < v.size()) {
        ss << "_";
      }
    }
    return ss.str();
  } catch (const std::invalid_argument& e) {
  }
  return ip;
}

std::string normalizedQuery(const std::string& query) {
  return absl::StrReplaceAll(absl::AsciiStrToLower(query), {{"?", "_"},
                                                            {"%", "_"},
                                                            {"&", "_"},
                                                            {"=", "_"},
                                                            {"/", "_"},
                                                            {".", "_"},
                                                            {"-", "_"},
                                                            {"<", "_"},
                                                            {">", "_"},
                                                            {":", "_"},
                                                            {"\"", "_"},
                                                            {"\\", "_"},
                                                            {"|", "_"},
                                                            {"*", "_"}});
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
  std::vector<std::string>    v = absl::StrSplit(uri_in, absl::ByAnyChar(" ,\t"), absl::SkipEmpty());
  std::string                 uri(uri_in);
  if (v.size() >= 3) {
    uri                  = v[0];
    uri_details.username = v[1];
    uri_details.password = v[2];
  }
  const Poco::URI uri_p(uri);
  // std::cout << uri << "\n";

  // std::cout << "scheme:[" << uri_p.getScheme() << "], authority:[" << uri_p.getAuthority() << "], path:["
  //           << uri_p.getPath() << "], path_query:[" << uri_p.getPathAndQuery() << "], path_etc:[" <<
  //           uri_p.getPathEtc()
  //           << "], query:[" << uri_p.getQuery() << "], fragment:[" << uri_p.getFragment() << "]\n";

  std::vector<std::string> known_schemes = {"rtsp", "vms", "grpc", "http"};

  bool is_known_schemes = false;
  for (auto&& known_scheme : known_schemes) {
    if (absl::StartsWith(uri_p.getScheme(), known_scheme)) {
      is_known_schemes = true;
      break;
    }
  }

  if (!is_known_schemes) {
    const Poco::Path path_p(uri);

    if (path_p.getExtension().empty()) {
      return uri_details;
    }
    uri_details.scheme        = absl::AsciiStrToLower(path_p.getExtension());
    uri_details.relative_path = path_p.toString();
    uri_details.url           = path_p.toString();
  }

  if (uri_details.scheme.empty()) {
    uri_details.scheme = uri_p.getScheme();
    uri_details.host   = uri_p.getHost();
    uri_details.port   = uri_p.getPort();

    uri_details.relative_path = uri_p.getPathAndQuery();

    if (!uri_p.getUserInfo().empty()) {
      std::vector<std::string> elems = absl::StrSplit(uri_p.getUserInfo(), ':');
      if (elems.size() == 2) {
        uri_details.username = elems.at(0);
        uri_details.password = elems.at(1);
      }
    }

    uri_details.channel = parseChannel(uri_p.getFragment());
  }

  uri_details.scheme = absl::AsciiStrToLower(uri_details.scheme);
  if (absl::StartsWith(uri_details.scheme, "rtsp")) {

  } else if (absl::StartsWith(uri_details.scheme, "vms")) {

    std::vector<std::string> parts = absl::StrSplit(uri_p.getPath(), '/', absl::SkipWhitespace());
    if (parts.size() == 2) {
      uri_details.channel.channel_id  = std::stoi(parts.at(0));
      uri_details.channel.stream_type = std::stoi(parts.at(1));

    } else {
      for (size_t i = 0; i < parts.size() - 1; i = i + 2) {
        if (parts.at(i) == "site") {
          uri_details.channel.site_id = std::stoi(parts.at(i + 1));
        } else if (parts.at(i) == "channel") {
          uri_details.channel.channel_id = std::stoi(parts.at(i + 1));
        }
      }
    }

  } else if (absl::StartsWith(uri_details.scheme, "grpc")) {
    if (uri_details.scheme == "grpc") {
      uri_details.scheme = "grpcframe";
    }

    std::vector<std::string> parts = absl::StrSplit(uri_p.getPath(), '/', absl::SkipWhitespace());
    for (size_t i = 0; i < parts.size() - 1; i = i + 2) {
      if (parts.at(i) == "site") {
        uri_details.channel.site_id = std::stoi(parts.at(i + 1));
      } else if (parts.at(i) == "channel") {
        uri_details.channel.channel_id = std::stoi(parts.at(i + 1));
      }
    }
  }
  if (uri_details.port || uri_details.host) {
    uri_details.url = fmt::format("{}://{}:{}{}", uri_details.scheme, uri_details.host.value_or(""),
                                  uri_details.port.value_or(0), uri_details.relative_path.value_or(""));
  } else {
    uri_details.url = fmt::format("{}://{}", uri_details.scheme, uri_details.relative_path.value_or(""));
  }

  return uri_details;
}

std::string vtpl::utilities::normalizeUri(const std::string& uri) {
  Poco::URI uri1(uri);
  return fmt::format("{}_{}_{}", normalizedIpAddress(uri1.getHost()), uri1.getPort(),
                     normalizedQuery(uri1.getPathAndQuery()));
}

std::string vtpl::utilities::Channel::toString() {
  std::stringstream ss;
  if (site_id) {
    ss << "#site=" << site_id.value();
  }
  if (channel_id) {
    ss << "#channel=" << channel_id.value();
  }
  if (app_id) {
    ss << "#app=" << app_id.value();
  }
  if (live_or_rec) {
    ss << "#live=" << live_or_rec.value();
  }
  if (stream_type) {
    ss << "#stream=" << stream_type.value();
  }
  if (media_type) {
    ss << "#media=" << media_type.value();
  }
  if (start_ts) {
    ss << "#timestamp=" << start_ts.value();
  }
  return ss.str();
}

vtpl::utilities::Channel parseChannelInternal(const std::string& str, const char delim) {
  vtpl::utilities::Channel channel;

  const std::vector<std::string> fragments = absl::StrSplit(str, delim, absl::SkipWhitespace());
  for (auto&& elem : fragments) {
    std::vector<std::string> parts = absl::StrSplit(elem, '=');
    if (parts.size() == 2) {
      if (parts.at(0) == "site") {
        channel.site_id = std::stoi(parts.at(1));
      } else if (parts.at(0) == "channel") {
        channel.channel_id = std::stoi(parts.at(1));
      } else if (parts.at(0) == "app") {
        channel.app_id = std::stoi(parts.at(1));
      } else if (parts.at(0) == "live") {
        channel.live_or_rec = std::stoi(parts.at(1));
      } else if (parts.at(0) == "stream") {
        channel.stream_type = std::stoi(parts.at(1));
      } else if (parts.at(0) == "media") {
        channel.media_type = std::stoi(parts.at(1));
      } else if (parts.at(0) == "timestamp") {
        channel.start_ts = std::stoull(parts.at(1));
      }
    }
  }

  return channel;
}

vtpl::utilities::Channel vtpl::utilities::parseChannel(const std::string& channel) {
  return parseChannelInternal(channel, '#');
}