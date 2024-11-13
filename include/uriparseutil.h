// *****************************************************
//    Copyright 2024 Videonetics Technology Pvt Ltd
// *****************************************************

#pragma once
#ifndef uriparseutil_h
#define uriparseutil_h

#include <string>
#include <optional>
#include <uriparseutil_export.h>
#include <version.h>
#include <cstdint>

struct UriDetails {
  std::string scheme;
  std::optional<uint32_t> site_id;
  std::optional<uint32_t> channel_id;
};

namespace vtpl {
namespace utilities {
UriDetails URIPARSEUTIL_EXPORT parseUri(const std::string& uri);
} // namespace utilities
} // namespace vtpl
#endif // uriparseutil_h
