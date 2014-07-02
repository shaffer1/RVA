/*=========================================================================

Program:   RVA
Module:    Utility

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <cassert>
#include <cmath>
#include <sstream>
#include <string>

template<class T>
T StringToNumeric(const std::string& string)
{
  T ret = 0; // Safe since T should only be a numeric type
  std::stringstream stream;
  stream << string;
  stream >> ret;
  return ret;
}

template<class T, class P>
T NumericToString(P x)
{
  T ret;
  std::stringstream ss;
  ss << x;
  ss >> ret;
  return ret;
}

template<class X, class Y>
static Y convertXToY(X val)
{
  Y ret;
  std::stringstream out;
  out << val;
  out >> ret;
  return ret;
}

static std::string toUpperCaseFileExtension(const char* theFileName)
{
  std::string result;
  const char* chrptr = strrchr(theFileName ? theFileName : "",'.'); //finds last period, or Null
  if(chrptr) // null if no period. Skip over period, then add upper case character
    for(chrptr++; *chrptr; chrptr++)
      result.push_back( toupper(*chrptr) );
  return result;
}

static float makeExponential(float base, char** ptr)
{
  if(ptr == NULL)
    return 0.f;

  char* startptr = *ptr;
  int exp = strtod(startptr, ptr);

  return (float)(pow(10.f, exp) * base);
}

static int startsWith(const char* s, const char* sub)
{
  return s && sub && 0==strncmp(s,sub,strlen(sub));
}

static int contains(const char* s, const char* sub)
{
  return s && sub && NULL!=strstr(s,sub);
}

static const char* skipWhiteSpace(const char *s) {
  if(!s) return NULL;
  while(*s == ' ' || *s =='\t')
    s++;
  return s;
}

static const char* skipNonWhiteSpace(const char *s) {
  if(!s) return NULL;
  while( *s && *s != ' ' && *s !='\t')
    s++;
  return s;
}

static char* removeTrailingChar(char* s,char c) {
	if(!s) return s;
	size_t len = strlen(s);
	while(len && s[len-1]==c) len--;
	s[len]='\0';
	return s;
}
static char* removeTrailingChars(char* s,const char* unwanted) {
	if(!unwanted || !s) return s;
	for(size_t i = strlen(unwanted)-1; i>=0;i--)
		removeTrailingChar(s,unwanted[i]);
	return s;
}