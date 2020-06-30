/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "print.h"
#include "cst.h"


static const std::array<unsigned short, 16> dbitMapRank{
  0x0000,
  0x0000,
  0x0001,
  0x0002,
  0x0004,
  0x0008,
  0x0010,
  0x0020,
  0x0040,
  0x0080,
  0x0100,
  0x0200,
  0x0400,
  0x0800,
  0x1000,
  0x2000,
};
static const std::array<unsigned char, 16> dcardRank{
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  'T',
  'J',
  'Q',
  'K',
  'A',
  '-'
};
static const std::array<unsigned char, 5> dcardSuit{
  'S',
  'H',
  'D',
  'C',
  'N'
};

std::string equals_to_string(const int equals);

void print_PBN(std::ostream &out, const dealPBN& dl)
{
  out << std::setw(10) << std::left << "trump" << dl.trump << "\n";
  out << std::setw(10) << "first" << dl.first << "\n";
  out << std::setw(10) << "cards" << dl.remainCards << "\n";
}


void print_FUT(std::ostream &out, const futureTricks& fut)
{
  out << std::setw(6) << std::left << "cards" << fut.cards << "\n";
  out << std::setw(6) << std::right <<  "No." <<
    std::setw(7) << "suit" <<
    std::setw(7) << "rank" <<
    std::setw(7) << "equals" <<
    std::setw(7) << "score" << "\n";

  for (int i = 0; i < fut.cards; i++)
  {
    out << std::setw(6) << std::right << i <<
      std::setw(7) << dcardSuit[ fut.suit[i] ] <<
      std::setw(7) << dcardRank[ fut.rank[i] ] <<
      std::setw(7) << equals_to_string(fut.equals[i]) <<
      std::setw(7) << fut.score[i] << "\n";
  }
}


std::string equals_to_string(const int equals)
{
  std::string st = "";
  for (unsigned i = 15; i >= 2; i--)
  {
    if (equals & dbitMapRank[i])
      st += static_cast<char>(dcardRank[i]);
  }
  return (st == "" ? "-" : st);
}


void print_TABLE(std::ostream &out, const ddTableResults& table)
{
  out << std::setw(5) << std::right << "" <<
    std::setw(6) << "North" <<
    std::setw(6) << "South" <<
    std::setw(6) << "East" <<
    std::setw(6) << "West" << "\n";

  out << std::setw(5) << std::right << "NT" <<
    std::setw(6) << table.resTable[4][0] <<
    std::setw(6) << table.resTable[4][2] <<
    std::setw(6) << table.resTable[4][1] <<
    std::setw(6) << table.resTable[4][3] << "\n";

  for (int suit = 0; suit <= 3; suit++)
  {
    out << std::setw(5) << std::right << dcardSuit[suit] <<
      std::setw(6) << table.resTable[suit][0] <<
      std::setw(6) << table.resTable[suit][2] <<
      std::setw(6) << table.resTable[suit][1] <<
      std::setw(6) << table.resTable[suit][3] << "\n";
  }
}


void print_PAR(std::ostream &out, const parResults& par)
{
  out << std::setw(9) << std::left << "NS score" << par.parScore[0] << "\n";
  out << std::setw(9) << "EW score" << par.parScore[1] << "\n";
  out << std::setw(9) << "NS list" << par.parContractsString[0] << "\n";
  out << std::setw(9) << "EW list" << par.parContractsString[1] << "\n";
}


void print_DEALERPAR(std::ostream &out, const parResultsDealer& par)
{
  out << std::setw(6) << std::left << "Score" << par.score << "\n";
  out << std::setw(6) << std::left << "Pars" << par.number << "\n";

  for (int i = 0; i < par.number; i++)
    out << std::left << "Par " << std::setw(2) << i << par.contracts[i] << "\n";
}


void print_PLAY(std::ostream &out, const playTracePBN& play)
{
  out << std::setw(6) << std::right << "Number" <<
    std::setw(5) << play.number << "\n";

  for (int i = 0; i < play.number; i++)
     out << std::setw(6) << i <<  "   " <<
       play.cards[2*i] << play.cards[2*i+1] << "\n";
}


void print_TRACE(std::ostream &out, const solvedPlay& solved)
{
  out << std::setw(6) << std::right << "Number" <<
    std::setw(5) << solved.number << "\n";

  for (int i = 0; i < solved.number; i++)
     out << std::setw(6) << i <<
       std::setw(5) << solved.tricks[i] << "\n";
}


void print_double_TRACE(std::ostream &out,
  const solvedPlay& solved,
  const solvedPlay& ref)
{
  out << "Number solved vs ref: " << solved.number << " vs. " <<
    ref.number << "\n";

  const int m = std::min(solved.number, ref.number);
  for (int i = 0; i < m; i++)
  {
    out << "Trick " << i << ": " <<
      solved.tricks[i] << " vs " <<
      ref.tricks[i] <<
      (solved.tricks[i] == ref.tricks[i] ? "" : " ERROR") << "\n";
  }

  if (solved.number > m)
  {
    for (int i = m; i < solved.number; i++)
      out << "Solved " << i << ": " << solved.tricks[i] << "\n";
  }
  else if (ref.number > m)
  {
    for (int i = m; i < ref.number; i++)
      out << "Ref    " << i << ": " << ref.tricks[i] << "\n";
  }
}

