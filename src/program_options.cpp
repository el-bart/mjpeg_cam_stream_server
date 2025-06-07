#include "program_options.hpp"
#include <regex>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;


namespace
{
auto parse_resolution(std::string const& str)
{
  std::regex const re{R"(^(\d+)x(\d+)$)"};
  std::smatch match;
  if( not std::regex_search(str, match, re) )
    throw std::runtime_error{"failed to parse resolution: " + str};

  return Resolution{
    .x_ = boost::lexical_cast<unsigned>(match.str(1)),
    .y_ = boost::lexical_cast<unsigned>(match.str(2))
  };
}
}


Program_options parse_program_options(int argc, char** argv)
{
  po::options_description desc("usage");
  desc.add_options()
    ("help,h", "show this helpful help")
    ("device,d", po::value<std::string>()->default_value("/dev/video0"), "camera device to use")
    ("resolution,r", po::value<std::string>()->default_value("1920x1080"), "set capture resolution for the camera")
    ("port,p", po::value<uint16_t>()->default_value(8080), "set the port to listen on (TCP)")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  Program_options po;

  if( vm.count("help") )
  {
    std::stringstream ss;
    ss << desc << "\n";
    po.show_help_ = ss.str();
    return po;
  }

  po.camera_config_.video_device_ = vm["device"].as<std::string>();
  if( not fs::exists(po.camera_config_.video_device_) )
    throw std::runtime_error{"video device " + po.camera_config_.video_device_.string() + " does not exist"};

  po.camera_config_.capture_resolution_ = parse_resolution( vm["resolution"].as<std::string>() );
  po.server_config_.port_ vm["port"].as<uint16_t>();

  return po;
}

