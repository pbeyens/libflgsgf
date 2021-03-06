/*
Copyright (c) 2010, Pieter Beyens
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sgf.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

static int is_lcletter(const char *sgf)
{
	if(*sgf>='a' && *sgf<='z')
		return 1;
	return 0;
}

static int is_point(const char *sgf)
{
	if(is_lcletter(sgf) && is_lcletter(sgf+1))
		return 1;
	else
		return 0;
}

static int is_whitespace(const char *sgf)
{
	if(*sgf==' ' || *sgf=='\r' || *sgf=='\n' || *sgf=='\t' || *sgf=='(' || *sgf==')')
		return 1;
	return 0;
}
static int is_ucletter(const char *sgf)
{
	if(*sgf>='A' && *sgf<='Z')
		return 1;
	return 0;
}

/*
static int is_digit(const char *sgf)
{
	if(*sgf>='1' && *sgf<='9')
		return 1;
	return 0;
}

static int is_double(const char *sgf)
{
	if(*sgf=='1' || *sgf=='2') return 1;
	else return 0;
}

static int is_color(const char *sgf)
{
	if(*sgf=='B' || *sgf=='W') return 1;
	else return 0;
}

static int is_number(const char *sgf)
{
	if(is_digit(sgf))
		return 1;
	if((*sgf=='+' || *sgf=='-') && is_digit(sgf+1))
		return 1;
	return 0;
}

static int is_text(const char *sgf)
{
	if(is_ucletter(sgf) || is_lcletter(sgf) || is_whitespace(sgf))
		return 1;
	return 0;
}
*/

static const char *valuetype(const char *sgf)
{
	while(strlen(sgf) && *sgf != ']')
		++sgf;
	return sgf;
}

static const char *cvaluetype(const char *sgf)
{
	return valuetype(sgf);
}

static const char *propident(const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	while(is_ucletter(sgf))
		++sgf;
	return sgf;
}

static const char *propvalue(const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf;
	if(*sgf!='[')
		return start;
	++sgf;
	sgf = cvaluetype(sgf);
	if(*sgf!=']')
		return start;
	++sgf;
	return sgf;
}

static const char *whitespace(const char *sgf)
{
	while(is_whitespace(sgf))
		++sgf;
	return sgf;
}

static const char *sz(const struct sgf_cb *cbs, const char *sgf)
{
	if(!cbs->sz)
		return sgf;
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf;
	if(strncmp("SZ",sgf,2))
		return sgf;
	sgf += 2;
	sgf = whitespace(sgf);
	if(*sgf != '[')
		return start;
	++sgf;
	if(strncmp("19",sgf,2))
		return start;
	sgf += 2;
	if(*sgf != ']')
		return start;
	++sgf;
	cbs->sz(19);
	return sgf;
}

static const char *move(const struct sgf_cb *cbs, const char *sgf)
{
	if(!cbs->b || !cbs->w)
		return sgf;
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf;
	char a, b, color;
	if(*sgf!='B' && *sgf!='W')
		return start;
	color = *sgf;
	++sgf;
	sgf = whitespace(sgf);
	if(*sgf != '[')
		return start;
	++sgf;

	if(*sgf == ']') {
		a = 't';
		b = 't';
	}
	else if(is_point(sgf)) {
		a = *sgf; ++sgf;
		b = *sgf; ++sgf;
	}
	else
		return start;

	if(*sgf != ']')
		return start;
	(color=='B') ? cbs->b(a,b) : cbs->w(a,b);
	++sgf;
	return sgf;
}

static const char *add(const struct sgf_cb *cbs, const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf;
	char color;
	if(*sgf!='A')
		return start;
	++sgf;
	if(*sgf!='B' && *sgf!='W' && *sgf!='E')
		return start;
	color = *sgf;
	++sgf;
	sgf = whitespace(sgf);
	if(*sgf != '[')
		return start;
	while(*sgf == '[') {
		char a, b;
		++sgf;
		if(is_point(sgf)) {
			a = *sgf; ++sgf;
			b = *sgf; ++sgf;
		}
		else
			return start;
		if(*sgf != ']')
			return start;
		if(color=='B' && cbs->ab) cbs->ab(a,b);
		else if(color=='W' && cbs->aw) cbs->aw(a,b);
		else if(color=='E' && cbs->ae) cbs->ae(a,b);
		++sgf;
		sgf = whitespace(sgf);
	}
	return sgf;
}

static const char *cr(const struct sgf_cb *cbs, const char *sgf)
{
	if(!cbs->cr)
		return sgf;
	//printf("%s:%s\n",__FUNCTION__,sgf);
	char a, b;
	const char *start = sgf;
	if(strncmp("CR",sgf,2))
		return sgf;
	sgf += 2;
	sgf = whitespace(sgf);
	if(*sgf != '[')
		return start;
	++sgf;
	if(is_point(sgf)) {
		a = *sgf; ++sgf;
		b = *sgf; ++sgf;
	}
	else
		return start;
	if(*sgf != ']')
		return start;
	cbs->cr(a,b);
	++sgf;
	return sgf;
}

static const char *key(const struct sgf_cb *cbs, const char *sgf)
{
	char k;
	if(!cbs->key)
		return sgf;
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf;
	if(strncmp("KEY",sgf,3))
		return sgf;
	sgf += 3;
	sgf = whitespace(sgf);
	if(*sgf != '[')
		return start;
	++sgf;

	k = *sgf;
	++sgf;

	if(*sgf != ']')
		return start;
	++sgf;
	cbs->key(k);
	return sgf;
}

static const char *unknown(const struct sgf_cb *cbs, const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *start = sgf, *tmp;

	tmp = sgf;
	sgf = propident(sgf);
	if(sgf == tmp)
		return start;
	sgf = whitespace(sgf);
	tmp = sgf;
	sgf = propvalue(sgf);
	if(sgf == tmp)
		return start;
	
	if(cbs->prop_unknown)
		cbs->prop_unknown(start,sgf-start);

	return sgf;
}

static const char *node(const struct sgf_cb *cbs, const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	if(*sgf != ';')
		return sgf;
	if(cbs->node_new) cbs->node_new();
	return ++sgf;
}

static const char *property(const struct sgf_cb *cbs, const char *sgf)
{
	//printf("%s:%s\n\n",__FUNCTION__,sgf);
	const char *start = sgf;
	sgf = node(cbs,sgf);
	if(sgf==start) sgf = sz(cbs,sgf);
	if(sgf==start) sgf = move(cbs,sgf);
	if(sgf==start) sgf = add(cbs,sgf);
	if(sgf==start) sgf = cr(cbs,sgf);
	if(sgf==start) sgf = key(cbs,sgf);
	if(sgf==start) sgf = unknown(cbs,sgf);

	return sgf;
}

const char *sgf_parse_fast(const struct sgf_cb *cbs, const char *sgf)
{
	//printf("%s:%s\n",__FUNCTION__,sgf);
	const char *tmp = 0;
	while(sgf != tmp) {
		sgf = whitespace(sgf);
		tmp = sgf;
		sgf = property(cbs, sgf);
	}
	return sgf;
}


