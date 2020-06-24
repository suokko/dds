/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <algorithm>

#include "print.h"
#include "cst.h"


static unsigned short dbitMapRank[16];
static unsigned char dcardRank[16];
static unsigned char dcardSuit[5];

std::string equals_to_string(const int equals);


void set_constants()
{
  dbitMapRank[15] = 0x2000;
  dbitMapRank[14] = 0x1000;
  dbitMapRank[13] = 0x0800;
  dbitMapRank[12] = 0x0400;
  dbitMapRank[11] = 0x0200;
  dbitMapRank[10] = 0x0100;
  dbitMapRank[ 9] = 0x0080;
  dbitMapRank[ 8] = 0x0040;
  dbitMapRank[ 7] = 0x0020;
  dbitMapRank[ 6] = 0x0010;
  dbitMapRank[ 5] = 0x0008;
  dbitMapRank[ 4] = 0x0004;
  dbitMapRank[ 3] = 0x0002;
  dbitMapRank[ 2] = 0x0001;
  dbitMapRank[ 1] = 0;
  dbitMapRank[ 0] = 0;

  dcardRank[ 2] = '2';
  dcardRank[ 3] = '3';
  dcardRank[ 4] = '4';
  dcardRank[ 5] = '5';
  dcardRank[ 6] = '6';
  dcardRank[ 7] = '7';
  dcardRank[ 8] = '8';
  dcardRank[ 9] = '9';
  dcardRank[10] = 'T';
  dcardRank[11] = 'J';
  dcardRank[12] = 'Q';
  dcardRank[13] = 'K';
  dcardRank[14] = 'A';
  dcardRank[15] = '-';

  dcardSuit[0] = 'S';
  dcardSuit[1] = 'H';
  dcardSuit[2] = 'D';
  dcardSuit[3] = 'C';
  dcardSuit[4] = 'N';
}


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

