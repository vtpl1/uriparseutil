// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#include "uriparseutil.h"

#include <Poco/Path.h>
#include <Poco/URI.h>
#include <absl/strings/str_split.h>
#include <fmt/format.h>
#include <string>
#include <vector>

Channel parseChannel(const std::string& str, const char delim) {
  Channel channel;

  std::vector<std::string> fragments = absl::StrSplit(str, delim, absl::SkipWhitespace());
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
      } else if (parts.at(0) == "timestamp") {
        channel.start_ts = std::stol(parts.at(1));
      } else if (parts.at(0) == "media") {
        channel.media_type = std::stoi(parts.at(1));
      }
    }
  }

  return channel;
}

// rtsp://172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4#site=1#channel=2#app=3
// rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4?extra=%23channel%3D2%23app%3D3#site=1#channel=2#app=3
// vms://172.16.1.173:3005/44/0
// vms://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// grpc://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2
// videos/1.avf

UriDetails vtpl::utilities::parseUri(const std::string& uri) {
  UriDetails uri_details;

  const Poco::URI uri_p(uri);
  std::cout << uri << "\n";

  std::cout << "scheme:[" << uri_p.getScheme() << "], authority:[" << uri_p.getAuthority() << "], path:["
            << uri_p.getPath() << "], path_query:[" << uri_p.getPathAndQuery() << "], path_etc:[" << uri_p.getPathEtc()
            << "], query:[" << uri_p.getQuery() << "], fragment:[" << uri_p.getFragment() << "]\n";

  if (uri_p.getScheme().empty()) {
    const Poco::Path path_p(uri);

    if (path_p.getExtension().empty()) {
      return uri_details;
    }
    uri_details.scheme = path_p.getExtension();
    uri_details.relative_path = path_p.toString();
    uri_details.url           = path_p.toString();
  }

  if (uri_details.scheme.empty()) {
    uri_details.scheme = uri_p.getScheme();
    uri_details.host   = uri_p.getHost();
    uri_details.port   = uri_p.getPort();

    uri_details.relative_path = uri_p.getPathAndQuery();
    uri_details.url           = fmt::format("{}://{}:{}{}", uri_p.getScheme(), uri_p.getHost(), uri_p.getPort(),
                                            uri_details.relative_path.value_or(""));

    if (!uri_p.getUserInfo().empty()) {
      std::vector<std::string> elems = absl::StrSplit(uri_p.getUserInfo(), ':');
      if (elems.size() == 2) {
        uri_details.username = elems.at(0);
        uri_details.password = elems.at(1);
      }
    }

    uri_details.channel = parseChannel(uri_p.getFragment(), '#');
  }

  if (uri_details.scheme == "rtsp") {

  } else if (uri_details.scheme == "vms") {

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

  } else if (uri_details.scheme == "grpc") {

    std::vector<std::string> parts = absl::StrSplit(uri_p.getPath(), '/', absl::SkipWhitespace());
    for (size_t i = 0; i < parts.size() - 1; i = i + 2) {
      if (parts.at(i) == "site") {
        uri_details.channel.site_id = std::stoi(parts.at(i + 1));
      } else if (parts.at(i) == "channel") {
        uri_details.channel.channel_id = std::stoi(parts.at(i + 1));
      }
    }
  }

  return uri_details;
}
