/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <stdexcept>

#include "portab.h"
#include "parse.h"


bool parse_PBN(std::ostream &out,
  const std::vector<std::string>& list,
  int& dealer,
  int& vul,
  dealPBN * dl);

bool parse_FUT(std::ostream &out,
  const std::vector<std::string>& list,
  futureTricks * fut);

bool parse_TABLE(std::ostream &out,
  const std::vector<std::string>& list,
  ddTableResults * table);

bool parse_PAR(std::ostream &out,
  const std::vector<std::string>& list,
  parResults * par);

bool parse_DEALERPAR(std::ostream &out,
  const std::vector<std::string>& list,
  parResultsDealer * par);

bool parse_PLAY(std::ostream &out,
  const std::vector<std::string>& list,
  playTracePBN * play);

bool parse_TRACE(std::ostream &out,
  const std::vector<std::string>& list,
  solvedPlay * solved);

bool parseable_GIB(const std::string& line);

bool parse_GIB(
  const std::string& line,
  dealPBN * dl,
  ddTableResults * table);

bool get_any_line(std::ostream &out,
  std::ifstream& fin,
 std::vector<std::string>& list,
  const std::string& tag,
  const int n);

bool get_head_element(std::ostream &out,
  const std::string& elem,
  const std::string& expected);

bool get_int_element(std::ostream &out,
  const std::string& elem,
  int& res,
  const std::string& errtext);

bool strip_quotes(std::ostream &out,
  const std::string& st,
  char * cstr,
  const std::string& errtag);

bool strip_quotes(std::ostream &out,
  const std::string& st,
  int& res,
  const std::string& errtag);

std::string trimTrailing(
  const std::string& st,
  const char c);

void splitIntoWords(
  const std::string& text,
 std::vector<std::string>& words);

bool str2int(
  const std::string& text,
  int& res);


bool read_file(std::ostream &out,
  const std::string& fname,
  int& number,
  bool& GIBmode,
  int ** dealer_list,
  int ** vul_list,
  dealPBN ** deal_list,
  futureTricks ** fut_list,
  ddTableResults ** table_list,
  parResults ** par_list,
  parResultsDealer ** dealerpar_list,
  playTracePBN ** play_list,
  solvedPlay ** trace_list)
{
  std::ifstream fin;
  fin.open(fname);

  std::string line;
  if (! getline(fin, line))
  {
    out << "First line bad: '" << line << "'" << std::endl;
    return false;
  }

 std::vector<std::string> list;
  list.clear();
  splitIntoWords(line, list);

  if (list.size() == 2 && get_head_element(out, list[0], "NUMBER"))
  {
    // Hopefully a txt-style file.
    if (! str2int(list[1], number))
    {
      out << "Not a number of hands: '" << list[1] << "'" << std::endl;
      return false;
    }
    else if (number <= 0 || number > 100000)
    {
      out << "Suspect number of hands: " << number << std::endl;
      return false;
    }
  }
  else if (! parseable_GIB(line))
  {
    out << "Not a GIB-style start: '" << line << "'" << std::endl;
    return false;
  }
  else
  {
    // Count the lines, then start over.
    GIBmode = 1;
    number = 1;
    while (1)
    {
      if (! getline(fin, line))
        break;
      number++;
    }
    fin.close();
    fin.open(fname);
  }


  // Make enough room for the hands.

  const size_t number_t = static_cast<size_t>(number);

  if ((*dealer_list = static_cast<int *>
      (calloc(number_t, sizeof(int)))) == NULL)
    return false;

  if ((*vul_list = static_cast<int *>
      (calloc(number_t, sizeof(int)))) == NULL)
    return false;

  if ((*deal_list = static_cast<dealPBN *>
      (calloc(number_t, sizeof(dealPBN)))) == NULL)
    return false;

  if ((*fut_list = static_cast<futureTricks *>
      (calloc(number_t, sizeof(futureTricks)))) == NULL)
    return false;

  if ((*table_list = static_cast<ddTableResults *>
      (calloc(number_t, sizeof(ddTableResults)))) == NULL)
    return false;

  if ((*par_list = static_cast<parResults *>
      (calloc(number_t, sizeof(parResults)))) == NULL)
    return false;

  if ((*dealerpar_list = static_cast<parResultsDealer *>
      (calloc(number_t, sizeof(parResultsDealer)))) == NULL)
    return false;

  if ((*play_list = static_cast<playTracePBN *>
      (calloc(number_t, sizeof(playTracePBN)))) == NULL)
    return false;

  if ((*trace_list = static_cast<solvedPlay *>
      (calloc(number_t, sizeof(solvedPlay)))) == NULL)
    return false;

  if (GIBmode)
  {
    for (int n = 0; n < number; n++)
    {
      if (! getline(fin, line))
      {
        out << "Expected GIB line " << n << std::endl;
        return false;
      }
      if (! parse_GIB(line, &(*deal_list)[n], &(*table_list)[n]))
        return false;
    }
  }
  else
  {
    for (int n = 0; n < number; n++)
    {
      if (! get_any_line(out, fin, list, "PBN", n))
        return false;
      if (! parse_PBN(out, list, (*dealer_list)[n],
          (*vul_list)[n], &(*deal_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "FUT", n))
        return false;
      if (! parse_FUT(out, list, &(*fut_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "TABLE", n))
        return false;
      if (! parse_TABLE(out, list, &(*table_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "PAR", n))
        return false;
      if (! parse_PAR(out, list, &(*par_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "DEALERPAR", n))
        return false;
      if (! parse_DEALERPAR(out, list, &(*dealerpar_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "PLAY", n))
        return false;
      if (! parse_PLAY(out, list, &(*play_list)[n]))
        return false;

      if (! get_any_line(out, fin, list, "TRACE", n))
        return false;
      if (! parse_TRACE(out, list, &(*trace_list)[n]))
        return false;
    }
  }

  fin.close();
  return true;
}


bool parse_PBN(std::ostream &out,
  const std::vector<std::string>& list,
  int& dealer,
  int& vul,
  dealPBN * dl)
{
  if (list.size() != 9)
  {
    out << "PBN list does not have 9 elements: " << list.size() << "\n";
    return false;
  }

  if (! get_head_element(out, list[0], "PBN"))
    return false;
  if (! get_int_element(out, list[1], dealer, "PBN dealer failed"))
    return false;
  if (! get_int_element(out, list[2], vul, "PBN vul failed"))
    return false;
  if (! get_int_element(out, list[3], dl->trump, "PBN trump failed"))
    return false;
  if (! get_int_element(out, list[4], dl->first, "PBN trump failed"))
    return false;

  for (int i = 0; i < 3; i++)
  {
    dl->currentTrickSuit[i] = 0;
    dl->currentTrickRank[i] = 0;
  }

  if (! strip_quotes(out,
      list[5] + " " + list[6] + " " + list[7] + " " + list[8],
      dl->remainCards, "PBN std::string"))
    return false;

  return true;
}


bool parse_FUT(std::ostream &out,
  const std::vector<std::string>& list,
  futureTricks * fut)
{
  if (list.size() < 2)
  {
    out << "PBN list does not have 2+ elements: " << list.size() << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "FUT"))
    return false;
  if (! get_int_element(out, list[1], fut->cards, "FUT cards"))
    return false;

  if (static_cast<int>(list.size()) != 4 * fut->cards + 2)
  {
    out << "PBN list does not have right length: " << list.size() << std::endl;
    return false;
  }

  const unsigned nu = static_cast<unsigned>(fut->cards);
  for (unsigned c = 0; c < nu; c++)
    if (! get_int_element(out, list[c+2], fut->suit[c], "FUT suit"))
      return false;

  for (unsigned c = 0; c < nu; c++)
    if (! get_int_element(out, list[c+nu+2], fut->rank[c], "FUT rank"))
      return false;

  for (unsigned c = 0; c < nu; c++)
    if (! get_int_element(out, list[c+2*nu+2], fut->equals[c], "FUT equals"))
      return false;

  for (unsigned c = 0; c < nu; c++)
    if (! get_int_element(out, list[c+3*nu+2], fut->score[c], "FUT score"))
      return false;

  return true;
}


bool parse_TABLE(std::ostream &out,
  const std::vector<std::string>& list,
  ddTableResults * table)
{
  if (list.size() != 21)
  {
    out << "Table list does not have 21 elements: " << list.size() << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "TABLE"))
    return false;

  for (unsigned suit = 0; suit < DDS_STRAINS; suit++)
  {
    for (unsigned pl = 0; pl < DDS_HANDS; pl++)
    {
      if (! get_int_element(out, list[DDS_HANDS * suit + pl + 1],
          table->resTable[suit][pl], "TABLE entry"))
        return false;
    }
  }

  return true;
}


bool parse_PAR(std::ostream &out,
  const std::vector<std::string>& list,
  parResults * par)
{
  if (list.size() < 9)
  {
    out << "PAR list does not have 9+ elements: " << list.size() << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "PAR"))
    return false;

  if (! strip_quotes(out, list[1] + " " + list[2], par->parScore[0],
      "PAR score 0"))
    return false;

  if (! strip_quotes(out, list[3] + " " + list[4], par->parScore[1],
      "PAR score 1"))
    return false;

  unsigned i = 5;
  std::string st = "";
  while (i < list.size())
  {
    st += " " + list[i++];
    if (st.back() == '"')
      break;
  }

  if (! strip_quotes(out, st.substr(1), par->parContractsString[0],
      "PAR contract 0"))
    return false;

  st = "";
  while (i < list.size())
  {
    st += " " + list[i++];
    if (st.back() == '"')
      break;
  }

  if (! strip_quotes(out, st.substr(1), par->parContractsString[1],
      "PAR contract 1"))
    return false;

  return true;
}


bool parse_DEALERPAR(std::ostream &out,
  const std::vector<std::string>& list,
  parResultsDealer * par)
{
  const size_t l = list.size();
  if (l < 3)
  {
    out << "PAR2 list does not have 3+ elements: " << l << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "PAR2"))
    return false;

  if (! strip_quotes(out, list[1], par->score, "PBN std::string"))
    return false;

  unsigned no = 0;
  while (no+2 < l)
  {
    if (! strip_quotes(out, list[no+2], par->contracts[no], "PAR2 contract"))
      break;
    no++;
  }

  par->number = static_cast<int>(no);
  return true;
}


bool parse_PLAY(std::ostream &out,
  const std::vector<std::string>& list,
  playTracePBN * playp)
{
  if (list.size() != 3)
  {
    out << "PLAY list does not have 3 elements: " << list.size() << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "PLAY"))
    return false;

  if (! get_int_element(out, list[1], playp->number, "PLAY number"))
    return false;

  if (! strip_quotes(out, list[2], playp->cards, "PLAY std::string"))
    return false;

  return true;
}


bool parse_TRACE(std::ostream &out,
  const std::vector<std::string>& list,
  solvedPlay * solvedp)
{
  if (list.size() < 2)
  {
    out << "TRACE list does not have 2+ elements: " << list.size() << std::endl;
    return false;
  }

  if (! get_head_element(out, list[0], "TRACE"))
    return false;

  if (! get_int_element(out, list[1], solvedp->number, "TRACE number"))
    return false;

  for (unsigned i = 0; i < static_cast<unsigned>(solvedp->number); i++)
    if (! get_int_element(out, list[2+i], solvedp->tricks[i], "TRACE element"))
      return false;

  return true;
}


bool parseable_GIB(const std::string& line)
{
  if (line.size() != 88)
    return false;

  if (line.substr(67, 1) != ":")
    return false;

  return true;
}

int GIB_TO_DDS[4] = {1, 0, 3, 2};

bool parse_GIB(
  const std::string& line,
  dealPBN * dl,
  ddTableResults * table)
{
  std::string st = "W:" + line.substr(0, 67);
  strcpy(dl->remainCards, st.c_str());

  int dds_strain, dds_hand;
  for (int s = 0; s < DDS_STRAINS; s++)
  {
    dds_strain = (s == 0 ? 4 : s - 1);
    for (unsigned h = 0; h < DDS_HANDS; h++)
    {
      dds_hand = GIB_TO_DDS[h];
      char const c = (line.substr(
        68 + 4*static_cast<unsigned>(s) + h, 1).c_str())[0];
      int d;
      if (c >= 48 && c <= 57) // 0, 9
        d = c-48;
      else if (c >= 65 && c <= 70) // A, F
        d = c-55;
      else
        return false;

      if (dds_hand & 1)
        d = 13 - d;

      table->resTable[dds_strain][dds_hand] = d;
    }
  }
  return true;
}


bool get_any_line(std::ostream &out,
  std::ifstream& fin,
 std::vector<std::string>& list,
  const std::string& tag,
  const int n)
{
  std::string line;
  if (! getline(fin, line))
  {
    out << "Expected txt " << tag << " line " << n << std::endl;
    return false;
  }

  list.clear();
  splitIntoWords(line, list);
  return true;
}


bool get_head_element(std::ostream &out,
  const std::string& elem,
  const std::string& expected)
{
  if (elem != expected)
  {
    out << "PBN list does not start with " << expected <<
      ": '" << elem << "'" << std::endl;
    return false;
  }
  else
    return true;
}


bool get_int_element(std::ostream &out,
  const std::string& elem,
  int& res,
  const std::string& errtext)
{
  if (! str2int(elem, res))
  {
    out << errtext << ": '" << elem << "'\n";
    return false;
  }
  else
    return true;
}


bool strip_quotes(std::ostream &out,
  const std::string& st,
  char * cstr,
  const std::string& errtag)
{
  // Could just be past the last one.
  if (st.size() == 0)
    return false;

  if (st.front() != '\"' || st.back() != '\"')
  {
    out << errtag << " not in quotations: '" << st << "'\n";
    return false;
  }
  strcpy(cstr, st.substr(1, st.size()-2).c_str());
  return true;
}


bool strip_quotes(std::ostream &out,
  const std::string& st,
  int& res,
  const std::string& errtag)
{
  if (st.front() != '\"' || st.back() != '\"')
  {
    out << errtag << " not in quotations: '" << st << "'" << std::endl;
    return false;
  }

  if (! str2int(st.substr(1, st.size()-2).c_str(), res))
  {
    out << st << " not an int" << std::endl;
    return false;
  }

  return true;
}


std::string trimTrailing(
  const std::string& text,
  const char c)
{
  unsigned pos = static_cast<unsigned>(text.length());
  while (pos >= 1 && text.at(pos-1) == c)
    pos--;

  if (pos == 0)
    return "";
  else
    return text.substr(0, pos);
}


void splitIntoWords(
  const std::string& text,
 std::vector<std::string>& words)
{
  // Split into words (split on \s+, effectively).
  unsigned pos = 0;
  unsigned startPos = 0;
  bool isSpace = true;

  // It seems compilers have different ideas about files.
  const size_t tl = text.length();
  std::string ttext;
  if (text.back() == ' ')
    ttext = text.substr(0, tl-1);
  else if (text.at(tl-2) == ' ')
    ttext = text.substr(0, tl-2);
  else
    ttext = text;

  const unsigned l = static_cast<unsigned>(ttext.length());

  while (pos < l)
  {
    if (ttext.at(pos) == ' ')
    {
      if (! isSpace)
      {
        words.push_back(ttext.substr(startPos, pos-startPos));
        isSpace = true;
      }
    }
    else if (isSpace)
    {
      isSpace = false;
      startPos = pos;
    }
    pos++;
  }

  if (! isSpace)
    words.push_back(ttext.substr(startPos, pos-startPos));
}


bool str2int(
  const std::string& text,
  int& res)
{
  int i;
  size_t pos;
  try
  {
    i = stoi(text, &pos);
    if (pos != text.size())
      return false;

  }
  catch (const std::invalid_argument& ia)
  {
    UNUSED(ia);
    return false;
  }
  catch (const std::out_of_range& ia)
  {
    UNUSED(ia);
    return false;
  }

  res = i;
  return true;
}

