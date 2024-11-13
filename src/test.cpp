#include <cassert>
#include <iostream>
#include <uriparseutil.h>

int main(int argc, char const* argv[]) {

  std::cout << "tests started" << std::endl;
  {
    auto a = vtpl::utilities::parseUri("");
    assert(a.scheme == "");
    assert(a.host == std::nullopt);
    assert(a.port == std::nullopt);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == std::nullopt);
    assert(a.channel.channel_id == std::nullopt);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == std::nullopt);
    assert(a.url == std::nullopt);
  }
  {
    auto a = vtpl::utilities::parseUri("rtsp://172.20.1.160:8554/videos/1.mp4");
    assert(a.scheme == "rtsp");
    assert(a.host == "172.20.1.160");
    assert(a.port == 8554);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == std::nullopt);
    assert(a.channel.channel_id == std::nullopt);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "/videos/1.mp4");
    assert(a.url == "rtsp://172.20.1.160:8554/videos/1.mp4");
  }
  {
    auto a = vtpl::utilities::parseUri("rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4");
    assert(a.scheme == "rtsp");
    assert(a.host == "172.20.1.160");
    assert(a.port == 8554);
    assert(a.username == "admin");
    assert(a.password == "AdmiN1234");
    assert(a.channel.site_id == std::nullopt);
    assert(a.channel.channel_id == std::nullopt);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "/videos/1.mp4");
    assert(a.url == "rtsp://172.20.1.160:8554/videos/1.mp4");
  }
  {
    auto a = vtpl::utilities::parseUri("rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4#site=1#channel=2#app=0");
    assert(a.scheme == "rtsp");
    assert(a.host == "172.20.1.160");
    assert(a.port == 8554);
    assert(a.username == "admin");
    assert(a.password == "AdmiN1234");
    assert(a.channel.site_id == 1);
    assert(a.channel.channel_id == 2);
    assert(a.channel.app_id == 0);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "/videos/1.mp4");
    assert(a.url == "rtsp://172.20.1.160:8554/videos/1.mp4");
  }
  {
    auto a = vtpl::utilities::parseUri(
        "rtsp://admin:AdmiN1234@172.20.1.160:8554/videos/1.mp4?extra=%23channel%3D2%23app%3D3#site=1#channel=2#app=0");
    assert(a.scheme == "rtsp");
    assert(a.host == "172.20.1.160");
    assert(a.port == 8554);
    assert(a.username == "admin");
    assert(a.password == "AdmiN1234");
    assert(a.channel.site_id == 1);
    assert(a.channel.channel_id == 2);
    assert(a.channel.app_id == 0);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "/videos/1.mp4?extra=%23channel%3D2%23app%3D3");
    assert(a.url == "rtsp://172.20.1.160:8554/videos/1.mp4?extra=%23channel%3D2%23app%3D3");
  }
  {
    auto a = vtpl::utilities::parseUri("vms://172.16.1.173:3005/44/0");
    assert(a.scheme == "vms");
    assert(a.host == "172.16.1.173");
    assert(a.port == 3005);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == std::nullopt);
    assert(a.channel.channel_id == 44);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == 0);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "/44/0");
    assert(a.url == "vms://172.16.1.173:3005/44/0");
  }
  {
    auto a = vtpl::utilities::parseUri("vms://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#media=2");
    assert(a.scheme == "vms");
    assert(a.host == "172.16.1.22");
    assert(a.port == 20006);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == 1);
    assert(a.channel.channel_id == 3);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == 1);
    assert(a.channel.stream_type == 0);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == 2);
    assert(a.relative_path == "/site/1/channel/3");
    assert(a.url == "vms://172.16.1.22:20006/site/1/channel/3");
  }
  {
    auto a = vtpl::utilities::parseUri(
        "grpc://172.16.1.22:20006/site/1/channel/3#live=1#stream=0#timestamp=1731174970344#media=2");
    assert(a.scheme == "grpc");
    assert(a.host == "172.16.1.22");
    assert(a.port == 20006);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == 1);
    assert(a.channel.channel_id == 3);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == 1);
    assert(a.channel.stream_type == 0);
    assert(a.channel.start_ts == 1731174970344);
    assert(a.channel.media_type == 2);
    assert(a.relative_path == "/site/1/channel/3");
    assert(a.url == "grpc://172.16.1.22:20006/site/1/channel/3");
  }
  {
    auto a = vtpl::utilities::parseUri("videos/3/1.avf");
    assert(a.scheme == "avf");
    assert(a.host == std::nullopt);
    assert(a.port == std::nullopt);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == std::nullopt);
    assert(a.channel.channel_id == std::nullopt);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "videos/3/1.avf");
    assert(a.url == "videos/3/1.avf");
  }
  {
    auto a = vtpl::utilities::parseUri("videos/3/1.avf#site=1#channel=1");
    assert(a.scheme == "avf");
    assert(a.host == std::nullopt);
    assert(a.port == std::nullopt);
    assert(a.username == std::nullopt);
    assert(a.password == std::nullopt);
    assert(a.channel.site_id == 1);
    assert(a.channel.channel_id == 1);
    assert(a.channel.app_id == std::nullopt);
    assert(a.channel.live_or_rec == std::nullopt);
    assert(a.channel.stream_type == std::nullopt);
    assert(a.channel.start_ts == std::nullopt);
    assert(a.channel.media_type == std::nullopt);
    assert(a.relative_path == "videos/3/1.avf");
    assert(a.url == "videos/3/1.avf");
  }
  std::cout << "tests end" << std::endl;
  return 0;
}
