#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", po::value<std::string>()->implicit_value(""),
     "Show help. Optionally takes a topic string.")
    ("stuff", po::value<std::string>(),
     "An option that requires one argument");

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (const po::error& e) {
    std::cerr << "Error parsing options: " << e.what() << "\n";
    return 1;
  }

  if (vm.count("help")) {
    std::string topic = vm["help"].as<std::string>();
    std::cout << desc << "\n";
    if (!topic.empty()) {
      std::cout << "Help topic requested: " << topic << "\n";
    }
    return 0;
  }

  if (vm.count("stuff")) {
    std::string stuff_arg = vm["stuff"].as<std::string>();
    std::cout << "Stuff argument: " << stuff_arg << "\n";
  } else {
    std::cout << "No --stuff argument provided.\n";
  }

  return 0;
}

