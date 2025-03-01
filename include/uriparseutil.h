// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#pragma once
#ifndef uriparseutil_h
#define uriparseutil_h

#include <cstdint>
#include <optional>
#include <string>
#include <uriparseutil_export.h>
#include <version.h>

namespace vtpl {
namespace utilities {
class URIPARSEUTIL_EXPORT Channel {
public:
  std::optional<uint32_t> site_id;
  std::optional<uint32_t> channel_id;
  std::optional<uint32_t> app_id;
  std::optional<uint32_t> live_or_rec;
  std::optional<uint32_t> stream_type;
  std::optional<uint64_t> start_ts;
  std::optional<uint32_t> media_type;
};

class URIPARSEUTIL_EXPORT UriDetails {
public:
  std::string                scheme;
  std::optional<std::string> url;
  std::optional<std::string> host;
  std::optional<uint32_t>    port;
  std::optional<std::string> username;
  std::optional<std::string> password;
  std::optional<std::string> relative_path;

  Channel channel;
};

std::string URIPARSEUTIL_EXPORT normalizeUri(const std::string& uri);
UriDetails URIPARSEUTIL_EXPORT  parseUri(const std::string& uri);
} // namespace utilities
} // namespace vtpl
#endif // uriparseutil_h
