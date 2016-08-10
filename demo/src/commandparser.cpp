// commandparser.cpp                                                  -*-C++-*-
#include "commandparser.hpp"

#include <gungi/gungi.hpp>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

namespace {

static std::string readFile(std::ifstream& ifs);
  // Reads the contents of the file pointed to by the given 'ifs', and returns
  // the contents.  Modifies the given input file stream, 'ifs', such that it
  // points at the beginning of the stream.

std::string readFile(std::ifstream& ifs) {
  unsigned int length = 0;
  ifs.seekg(0, ifs.end);
  length = ifs.tellg();
  ifs.seekg(0, ifs.beg);

  std::string contents(length, ' ');
  ifs.read(&contents[0], length);

  return contents;
}

}  // close unnamed namespace

// CREATORS
CommandParser::CommandParser(int argc, char *argv[])
: progName(argv[0])
, inputFile()
, outputFile()
, controller()
, metadata()
{
  std::stringstream ss;
  for (int i = 1; i < argc; i++) {
    ss << argv[i] << " ";
  }

  std::string opt, value;
  while (true) {
    ss >> opt;
    if (ss.fail()) {
      break;
    }

    if (opt == "-h" || opt == "--help") {
      std::cout
        << usage()
        << std::endl;
      exit(0);
    }

    ss >> value;
    if (ss.fail()) {
      std::cerr
        << "Invalid command or missing argument: "
        << opt
        << std::endl
        << usage()
        << std::endl;
      exit(-1);
    }

    if (opt == "-o" || opt == "--output") {
      outputFile.open(value.c_str());
      if (outputFile.fail()) {
        std::cerr
          << "Invalid input file: "
          << value
          << std::endl
          << usage()
          << std::endl;
        exit(-3);
      }
    } else if (opt == "-i" || opt == "--input") {
      inputFile.open(value.c_str());
      if (inputFile.fail()) {
        std::cerr
          << "Invalid input file: "
          << value
          << std::endl
          << usage()
          << std::endl;
        exit(-4);
      }

      if (!gungi::GNDecoder::decode(readFile(inputFile), metadata, controller)) {
        std::cerr
          << "Malformed input file: "
          << value
          << std::endl
          << usage()
          << std::endl;
        exit(-5);
      }
    } else if (opt == "-l" || opt == "--location") {
      metadata.setLocation(value);
    } else if (opt == "-e" || opt == "--event") {
      metadata.setEvent(value);
    } else if (opt == "-w" || opt == "--white") {
      metadata.setWhite(value);
    } else if (opt == "-b" || opt == "--black") {
      metadata.setBlack(value);
    } else {
      exit(-2);
    }
  }
}

CommandParser::~CommandParser(void) {
  // DO NOTHING
}

// ACCESSORS
std::string CommandParser::usage(void) const {
  std::ostringstream oss;
  oss
    << progName
    << " [ -h | --help ]"
    << " [ -o | --output file ] [ -i | --input file]"
    << " [ -l | --location location ] [ -e | --event event ]"
    << " [ -w | --white name ] [ -b | --black name ]"
    << std::endl
    << std::endl
    << "Optional Args:"
    << std::endl
    << "  -h\\--help                 show this dialog"
    << std::endl
    << "  -o\\--output file          output '.gn' file"
    << std::endl
    << "  -i\\--input file           input '.gn' file to load"
    << std::endl
    << "  -l\\--location location    match location"
    << std::endl
    << "  -e\\--event event          name of the match"
    << std::endl
    << "  -w\\--white name           name of the whie player"
    << std::endl
    << "  -b\\--black name           name of the black player";
  return oss.str();
}
