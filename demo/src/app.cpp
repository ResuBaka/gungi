// app.cpp                                                            -*-C++-*-
#include "app.hpp"

#include "commandparser.hpp"
#include "term.h"

#include <gungi/gungi.hpp>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define FOREVER for(;;)

// PRIVATE MANIPULATORS
void App::print(void) {
  save_cursor();
  {
    move_cursor(0, 0);
    std::cout
      << m_controller
      << std::endl
      << std::endl;

    // Print out the hands.
    const gungi::SharedUnitPtrVector vecs[] = {
      m_controller.black().inactiveUnits(),
      m_controller.white().inactiveUnits()
    };

    for (unsigned int i = 0; i < 2; i++) {
      std::cout
        << (i == 0 ? "Black's" : "White's")
        << " Hand:"
        << std::endl;

      unsigned int count = 0;
      for (const gungi::SharedUnitPtr& unit : vecs[i]) {
        if (count > 0 && count % 5 == 0) {
          std::cout << std::endl;
        }
        std::cout
          << std::left
          << std::setw(18)
          << unit->code();
        count++;
      }
      std::cout
        << std::endl
        << std::endl;
    }
  }
  restore_cursor();
}

void App::handleInput(const std::string& input) {
  char front[2] = { 0 };
  char back[2] = { 0 };
  unsigned int col1, row1, tier1, col2, row2, tier2;
  gungi::error_t error = gungi::GUNGI_ERROR_NONE;
  m_errorString = "";
  if (sscanf(input.c_str(),
             "d %c%c %u-%u",
             front,
             back,
             &col1,
             &row1) == 4) {
    // Drop
    gungi::piece_id_t frontPiece = gungi::gn_identifier_to_piece(front);
    gungi::piece_id_t backPiece = gungi::gn_identifier_to_piece(back);
    gungi::Posn to(col1, row1);
    m_controller.dropUnit(frontPiece, backPiece, to, error);
  } else if (sscanf(input.c_str(),
                    "m %u-%u-%u %u-%u",
                    &col1,
                    &row1,
                    &tier1,
                    &col2,
                    &row2) == 5) {
    // Move
    gungi::Posn from(col1, row1);
    gungi::Posn to(col2, row2);
    m_controller.moveUnit(from, tier1, to, error);
  } else if (sscanf(input.c_str(),
                    "i %u-%u-%u %u",
                    &col1,
                    &row1,
                    &tier1,
                    &tier2) == 4) {
    // Immobile Strike
    gungi::Posn posn(col1, row1);
    m_controller.immobileStrike(posn, tier1, tier2, error);
  } else if (input == "fr") {
    // Forced Recovery
    if (input.substr(2).find_first_not_of(" \r\n\t") != std::string::npos) {
      m_errorString = "Invalid command.";
      return;
    }
    m_controller.forceRecover(true, error);
  } else if (input == "nfr") {
    // Reject Forced Recovery
    if (input.substr(3).find_first_not_of(" \r\n\t") != std::string::npos) {
      m_errorString = "Invalid command.";
      return;
    }
    m_controller.forceRecover(false, error);
  } else if (sscanf(input.c_str(),
                    "s %u-%u-%u %u-%u-%u",
                    &col1,
                    &row1,
                    &tier1,
                    &col2,
                    &row2,
                    &tier2) == 6) {
    // Substitution
    gungi::effect_t exch = gungi::GUNGI_EFFECT_SUBSTITUTION;
    gungi::Posn from(col1, row1);
    gungi::Posn to(col2, row2);
    m_controller.exchangeUnits(exch, from, tier1, to, tier2, error);
  } else if (sscanf(input.c_str(),
                    "t %u-%u-%u %u",
                    &col1,
                    &row1,
                    &tier1,
                    &tier2) == 4) {
    // Tier Exchange
    gungi::effect_t exch = gungi::GUNGI_EFFECT_1_3_TIER_EXCHANGE;
    gungi::Posn from(col1, row1);
    gungi::Posn to = from;
    m_controller.exchangeUnits(exch, from, tier1, to, tier2, error);
  } else {
    m_errorString = "Invalid command.";
  }

  if (error != gungi::GUNGI_ERROR_NONE) {
    m_errorString = gungi::error_to_string(error);
  }
}

// CREATORS
App::App(CommandParser& commandParser)
: m_commandParser(commandParser)
, m_controller(commandParser.controller)
{
  // DO NOTHING
}

// MANIPULATORS
int App::run(void) {
  FOREVER {
    // Main application loop.  Runs indefinitely until it is quit.
    clear_screen();
    move_cursor(0, TERMINAL_HEIGHT);
    print();

    move_cur_up(1);
    std::cout
      << (m_controller.isPlayersTurn(gungi::BLACK) ? "Black" : "White")
      << "'s Turn:"
      << std::endl;

    if (m_errorString.size()) {
      // Print out the error message.
      move_cur_down(1);
      {
        std::cout
          << m_errorString
          << std::endl;
      }
      move_cur_up(2);
    }

    std::string input;
    std::cout << "PROMPT> ";
    std::getline(std::cin, input);
    if (input == "q" || input == "quit") {
      break;
    }

    handleInput(input);
    if (!m_controller.isOver()) {
      continue;
    }

    if (m_controller.isDraw()) {
      std::cout << "DRAW!" << std::endl;
    } else if (m_controller.isInCheckmate(gungi::WHITE)) {
      std::cout << "WHITE WINS!" << std::endl;
    } else {
      std::cout << "BLACK WINS!" << std::endl;
    }

    break;
  }

  if (m_commandParser.outputFile.good()) {
    std::string data = m_commandParser.metadata.header(m_controller) +
                       std::string("\n") +
                       m_outputStream.str();
    unsigned int len = data.size();
    m_commandParser.outputFile.write(data.c_str(), len);
  }

  return 0;
}
