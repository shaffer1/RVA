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
#include <cstring>
#include <cstdlib>

static std::string getFileExtension(const char* theFileName)
{
  std::string extension(theFileName);
  return extension.substr(extension.rfind(".") + 1);
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

// MVM: Check to see if these can be replaced with normal C++ stream usage.
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
