/*
   Copyright (C) 2013-2014 Ron Norman

   This file is part of GNU Cobol.

   The GNU Cobol runtime library is free software: you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GNU Cobol is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GNU Cobol.  If not, see <http://www.gnu.org/licenses/>.
*/

/*************************************************************************
 * This module and Report Writer code added by Ron Norman, November 2013 *
 * including parts of sources found in 'cobc / *'                        *
 ************************************************************************/

#include "config.h"

#define _LFS64_LARGEFILE		1
#define _LFS64_STDIO			1
#define _FILE_OFFSET_BITS		64
#define _LARGEFILE64_SOURCE		1
#ifdef	_AIX
#define _LARGE_FILES			1
#endif	/* _AIX */
#if defined(__hpux__) && !defined(__LP64__)
#define _APP32_64BIT_OFF_T		1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef	HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef	_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#ifndef __WATCOMC__
#define	fdcobsync	_commit
#else
#define	fdcobsync	fsync
#endif
#if !defined(__BORLANDC__) && !defined(__WATCOMC__)
#define	getcwd		_getcwd
#define	chdir		_chdir
#define	mkdir		_mkdir
#define	rmdir		_rmdir
#define	open		_open
#define	close		_close
#define	unlink		_unlink
#define	fdopen		_fdopen
#define lseek		_lseeki64
#endif
#define off_t		cob_s64_t

#elif	defined(HAVE_FDATASYNC)
#define	fdcobsync	fdatasync
#else
#define	fdcobsync	fsync

#endif

#ifndef	_O_TEMPORARY
#define	_O_TEMPORARY	0
#endif

#ifndef	O_BINARY
#define	O_BINARY	0
#endif

/* Force symbol exports */
#define	COB_LIB_EXPIMP

#include "libcob.h"
#include "coblocal.h"

#ifdef	WORDS_BIGENDIAN
#define	COB_MAYSWAP_16(x)	((unsigned short)(x))
#define	COB_MAYSWAP_32(x)	((unsigned int)(x))
#else
#define	COB_MAYSWAP_16(x)	(COB_BSWAP_16((unsigned short)(x)))
#define	COB_MAYSWAP_32(x)	(COB_BSWAP_32((unsigned int)(x)))
#endif

#ifdef DEBUG
#define LOG	fdbg == NULL ? 0 : reportLog
static	FILE	*fdbg = NULL;
#endif
static	int	bDidReportInit = 0;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef DEBUG
/*
 * Log information to file for debugging 
 */
static void
reportLog(const char *fmt, ...)
{
	va_list         ap;
	if(fdbg == NULL)
		return;
	va_start (ap, fmt);
	vfprintf(fdbg,fmt,ap);
	va_end (ap);
	fflush(fdbg);
}
#endif

#define ND1 COB_REPORT_HEADING|COB_REPORT_FOOTING|COB_REPORT_PAGE_HEADING|COB_REPORT_PAGE_FOOTING
#define ND2 COB_REPORT_CONTROL_HEADING|COB_REPORT_CONTROL_HEADING_FINAL
#define ND3 COB_REPORT_CONTROL_FOOTING|COB_REPORT_CONTROL_FOOTING_FINAL
#define NOTDETAIL(f) ( f & (ND1|ND2|ND3))

static int report_line_type(cob_report *r, cob_report_line *l, int type);

static const cob_field_attr	const_alpha_attr =
				{COB_TYPE_ALPHANUMERIC, 0, 0, 0, NULL};
static const cob_field_attr	const_num_attr =
				{COB_TYPE_NUMERIC, 0, 0, 0, NULL};
/*
 * Move "String" to 'dst' field
 */
static void
cob_str_move (cob_field *dst, unsigned char *src, const int size)
{
	cob_field	temp;

	temp.size = size;
	temp.data = src;
	temp.attr = &const_alpha_attr;
	cob_move (&temp, dst);
}

/*
 * Initialize data field
 */
static cob_field *
cob_field_init (cob_field *f)
{
	cob_field	temp;

	if(f == NULL)
		return NULL;
	temp.size = 1;
	if(COB_FIELD_IS_NUMERIC(f)) {
		temp.data = (unsigned char*)"0";	/* MOVE ZERO to field */
		temp.attr = &const_num_attr;
	} else {
		temp.data = (unsigned char*)" ";	/* MOVE SPACES to field */
		temp.attr = &const_alpha_attr;
	}
	cob_move (&temp, f);
	return f;
}


/*
 * Make a new field the same format as that given
 */
static cob_field *
cob_field_dup (cob_field *f, int incr)
{
	cob_field	temp;
	cob_field	*fld = calloc(1,sizeof(cob_field));

	fld->size = f->size+incr;
	fld->data = calloc(1,f->size+incr);
	fld->attr = f->attr;

	temp.size = 1;
	if(COB_FIELD_IS_NUMERIC(f)) {
		temp.data = (unsigned char*)"0";	/* MOVE ZERO to new field */
		temp.attr = &const_num_attr;
	} else {
		temp.data = (unsigned char*)" ";	/* MOVE SPACES to new field */
		temp.attr = &const_alpha_attr;
	}
	cob_move (&temp, fld);
	return fld;
}

/*
 * Free a field created by cob_field_dup or cob_field_save
 */
static void
cob_field_free (cob_field *f)
{
	if(f == NULL)
		return;
	if(f->data)
		free((void*)f->data);
	free((void*)f);
	return ;
}

/*
 * Clear the 'group_indicate' flag for all fields
 */
static void
clear_group_indicate(cob_report_line *l)
{
	cob_report_field *f;
	for(f=l->fields; f; f=f->next) {
		f->group_indicate = FALSE;
	}
	if(l->child)
		clear_group_indicate(l->child);
	if(l->sister)
		clear_group_indicate(l->sister);
}

/*
 * Clear the 'suppress' flag for all fields
 */
static void
clear_suppress(cob_report_line *l)
{
	cob_report_field *f;
	l->suppress = FALSE;
	for(f=l->fields; f; f=f->next) {
		f->suppress = FALSE;
	}
	if(l->child)
		clear_suppress(l->child);
	if(l->sister)
		clear_suppress(l->sister);
}

/*
 * If this line has NEXT GROUP .... then set info in report header
 */
static void
set_next_info(cob_report *r, cob_report_line *l)
{
	if(l->flags & COB_REPORT_NEXT_GROUP_LINE) {
		r->next_value = l->next_group_line;
		r->next_line = TRUE;
		r->next_just_set = TRUE;
#ifdef DEBUG
		LOG(" Save NEXT GROUP LINE %d\n",r->next_value);
#endif
	}
	if(l->flags & COB_REPORT_NEXT_GROUP_PLUS) {
		r->next_value = l->next_group_line;
		r->next_line = FALSE;
		r->next_line_plus = TRUE;
		r->next_just_set = TRUE;
#ifdef DEBUG
		LOG(" Save NEXT GROUP PLUS %d\n",r->next_value);
#endif
	}
	if(l->flags & COB_REPORT_NEXT_GROUP_PAGE) {
		r->next_value = l->next_group_line;
		r->next_line = FALSE;
		r->next_page = TRUE;
		r->next_just_set = TRUE;
#ifdef DEBUG
		LOG(" Save NEXT GROUP PAGE\n");
#endif
	}
}

static cob_report_line *
get_print_line(cob_report_line *l)
{
	while(l
	&& l->fields == NULL
	&& l->child != NULL)
		l = l->child;				/* Find line with data fields */
	return l;
}

/*
 * Do any global initialization needed
 */
static void
reportInitialize()
{
#ifdef DEBUG
	char	*env;
#endif
	if(bDidReportInit)
		return;
	bDidReportInit = 1;
#ifdef DEBUG
	if ((env = getenv("OCREPORTDEBUG")) &&
		(toupper(*env) == 'Y')) {
		fdbg = fopen("/tmp/ocreport.log","w");
	}
#endif
}

/*
 * Add Two Fields together giving Result
 */
static void
cob_add_fields(cob_field *op1, cob_field *op2, cob_field *rslt)
{
	cob_field_attr	attr1, attr2;
	char		data1[30],data2[30];
	cob_field	fld1,fld2;
#ifdef DEBUG
	char		wrk[32];
#endif

	/* Copy data to local PIC 9 DISPLAY fields */
	/* As cob_add does not handle NUMERIC_EDITED very well */
	fld1.size	= op1->size;
	attr1		= *op1->attr;
	fld1.attr	= &attr1;
	attr1.type	= COB_TYPE_NUMERIC_DISPLAY;
	fld1.data	= (unsigned char*)data1;
	memset(data1,'0',fld1.size);
	cob_move(op1, &fld1);

#ifdef DEBUG
	if(fdbg) {
		cob_field_to_string(op1, wrk, sizeof(wrk)-1);
		LOG("    Add '%s' ",wrk);
	}
#endif

	fld2.size	= op2->size;
	attr2		= *op2->attr;
	fld2.attr	= &attr2;
	attr2.type	= COB_TYPE_NUMERIC_DISPLAY;
	fld2.data	= (unsigned char*)data2;
	memset(data2,'0',fld2.size);
	cob_move(op2, &fld2);

#ifdef DEBUG
	if(fdbg) {
		cob_field_to_string(op2, wrk, sizeof(wrk)-1);
		LOG("TO '%s' ",wrk);
	}
#endif

	cob_add(&fld1,&fld2,0);

	cob_move(&fld1, rslt);			/* Copy SUM back to result field */

#ifdef DEBUG
	if(fdbg) {
		cob_field_to_string(&fld1, wrk, sizeof(wrk)-1);
		LOG("GIVING '%s' ",wrk);
		LOG("  PIC 9(%d)",rslt->attr->digits);
		if(rslt->attr->scale > 0)
			LOG("V9(%d)",rslt->attr->scale);
		LOG("\n");
	}
#endif
}

#ifdef DEBUG
static void
dumpFlags(int flags, int ln, char *name)
{
	if(fdbg == NULL)
		return;
	if(name == NULL)
		name = (char*)"";
	if(flags & COB_REPORT_HEADING)		LOG("REPORT HEADING ");
	if(flags & COB_REPORT_FOOTING)		LOG("REPORT FOOTING ");
	if(flags & COB_REPORT_PAGE_HEADING)	LOG("PAGE HEADING ");
	if(flags & COB_REPORT_PAGE_FOOTING)	LOG("PAGE FOOTING ");
	if(flags & COB_REPORT_CONTROL_HEADING)	LOG("CONTROL HEADING %s ",name);
	if(flags & COB_REPORT_CONTROL_HEADING_FINAL) LOG("CONTROL HEADING FINAL ");
	if(flags & COB_REPORT_CONTROL_FOOTING)	LOG("CONTROL FOOTING %s ",name);
	if(flags & COB_REPORT_CONTROL_FOOTING_FINAL) LOG("CONTROL FOOTING FINAL ");
	if(flags & COB_REPORT_DETAIL)		LOG("DETAIL ");
	if(flags & COB_REPORT_LINE_PLUS)	{if(ln > 0) LOG("LINE PLUS %d ",ln);}
	else if(flags & COB_REPORT_LINE)	LOG("LINE %d ",ln);
	if(flags & COB_REPORT_LINE_NEXT_PAGE)	LOG("LINE NEXT PAGE ");
	if(flags & COB_REPORT_NEXT_PAGE)	LOG("NEXT PAGE ");
	if(flags & COB_REPORT_GROUP_INDICATE)	LOG("GROUP INDICATE ");
	if(flags & COB_REPORT_COLUMN_PLUS)	LOG("COLUMN PLUS ");
	if(flags & COB_REPORT_RESET_FINAL)	LOG("RESET FINAL ");
}

static void
reportDumpOneLine(const cob_report *r, cob_report_line *fl, int indent, int dumpdata)
{
	cob_report_field	*rf;
	cob_report_control	*c;
	cob_report_control_ref	*rr;
	char	idnt[32], wrk[64];

	sprintf(idnt,"%.*s",indent>30?30:indent,"..................................");
	LOG("%s ",idnt);
	if(dumpdata) {
		LOG("Line# %d of Page# %d; ",r->curr_line,r->curr_page);
	}
	if(r->controls) {
		for(c=r->controls; c; c = c->next) {
			for(rr=c->control_ref; rr; rr=rr->next) {
				if(rr->ref_line == fl) {
					strcpy(wrk,c->name);
					break;
				}
			}
		}
	}
	dumpFlags(fl->report_flags,fl->line,wrk);
	if(fl->step_count)	LOG("Step %d ",fl->step_count);
	if(fl->suppress)	LOG("Suppress Line ");
	if(fl->next_group_line)	{
		LOG("Next ",fl->next_group_line);
		if(fl->report_flags & COB_REPORT_NEXT_GROUP_LINE)	LOG("GROUP LINE ");
		if(fl->report_flags & COB_REPORT_NEXT_GROUP_PLUS)	LOG("GROUP PLUS ");
		if(fl->report_flags & COB_REPORT_NEXT_GROUP_PAGE)	LOG("GROUP PAGE ");
		LOG("%d ",fl->next_group_line);
	}
	if(fl->control) {
		cob_field_to_string(fl->control, wrk, sizeof(wrk)-1);
		if(wrk[0] >= ' ')
			LOG("Control is '%s' ",wrk);
	}
	LOG("\n");
	if(!(fl->flags & COB_REPORT_DETAIL)) dumpdata = 1;
	for(rf = fl->fields; rf; rf = rf->next) {
		LOG("%s   Field ",idnt);
		if(rf->line)		LOG("Line %2d ",rf->line);
		if(rf->column)		LOG("Col %3d ",rf->column);
		if(rf->step_count)	LOG("Step %d ",rf->step_count);
		if(rf->next_group_line)	LOG("NextGrp %d ",rf->next_group_line);
		if(dumpdata) {
			if(rf->f) {
				if(rf->litval) {
					LOG("   \"%s\" ",rf->litval);
				} else {
					cob_field_to_string(rf->f, wrk, sizeof(wrk)-1);
					LOG("   '%s' ",wrk);
				}
			}
			if(rf->control) {
				cob_field_to_string(rf->control, wrk, sizeof(wrk)-1);
				if(wrk[0] >= ' ')
					LOG("Control is '%s' ",wrk);
			}
			if(rf->source
			&& cob_cmp(rf->f,rf->source) != 0) {
				if(rf->source == r->page_counter) {
					LOG("Source PAGE-COUNTER ");
				} else if(rf->source == r->line_counter) {
					LOG("Source LINE-COUNTER ");
				}
			} 
			dumpFlags(rf->flags,rf->line,NULL);
		}
		if(rf->suppress)	LOG("Suppress ");
		LOG("\n");
	}
}

/*
 * Dump REPORT line and walk down tree
 */
static void
reportDumpLine(const cob_report *r, cob_report_line *fl, int indent)
{
	reportDumpOneLine(r,fl,indent,0);
	if(fl->child)
		reportDumpLine(r,fl->child,indent+2);
	if(fl->sister)
		reportDumpLine(r,fl->sister,indent);
}

/*
 * Dump entire REPORT definition tables
 */
static void
reportDump(const cob_report *r, const char *msg)
{
	cob_report_control *c;
	char		wrk[80];

	if(fdbg == NULL)
		return;
	LOG("Dump of Report '%s' for %s\n",r->report_name,msg);
	if(r->report_file) {
		LOG("Using File %s ",r->report_file->select_name);
		if(r->report_file->assign
		&& r->report_file->assign->data) {
			LOG(" ASSIGNed to %s",r->report_file->assign->data);
		}
		LOG(" Rcsz min %d max %d ",r->report_file->record_min,r->report_file->record_max);
#if 0
		/* 
		 * TODO: This needs more work. Cross check how fileio.c handles print files
		 * and exactly what operations should be used
		 */
		if(r->report_file->flag_select_features & COB_SELECT_LINAGE) {
			LOG("has LINAGE");
		} else {
			/*
			 * Create LINAGE clause fields for fileio.c so that
			 * the output file looks more like what Micro Focus would create
			 */
			cob_linage      *lingptr;
			if(r->report_file->linorkeyptr == NULL) {
				r->report_file->linorkeyptr = calloc(1,sizeof(cob_linage));
				lingptr = r->report_file->linorkeyptr;
				lingptr->lin_top = r->def_heading;
				lingptr->lin_bot = r->def_footing;
				lingptr->linage = cob_field_dup(r->line_counter,0);
				lingptr->linage_ctr = cob_field_dup(r->line_counter,0);
				r->report_file->flag_select_features |= COB_SELECT_LINAGE;
			}
			LOG("had NO LINAGE!");
		}
#endif
		LOG("\n");
	}
	LOG("\n");
	LOG("Default   Lines: %4d  Columns: %4d\n",r->def_lines,r->def_cols);
	LOG("        Heading: %4d  Footing: %4d\n",r->def_heading,r->def_footing);
	LOG("         Detail: %4d  Control: %4d  Last detail: %4d\n",r->def_first_detail,
						r->def_last_control,r->def_last_detail);
	if((r->curr_page+r->curr_status+r->curr_line+r->curr_cols) > 0) {
		LOG("Current    Page: %4d   Status: %4d\n",r->curr_page,r->curr_status);
		LOG("           Line: %4d   Column: %4d\n",r->curr_line,r->curr_cols);
	}
	LOG("\n");
	if(r->controls) {
		for(c=r->controls; c; c = c->next) {
			LOG(" Control %s ",c->name);
			if(c->f) {
				cob_field_to_string(c->f, wrk, sizeof(wrk)-1);
				if(wrk[0] >= ' ')
					LOG("has '%s' ",wrk);
			}
			if(c->val) {
				cob_field_to_string(c->val, wrk, sizeof(wrk)-1);
				if(wrk[0] >= ' ')
					LOG("Value '%s' ",wrk);
			}
			LOG("\n");
		}
	}
	reportDumpLine(r,r->first_line,0);
	LOG("\n");
}
#endif /* DEBUG */

/*
 * Verify that each LINE # is within PAGE LIMITS
 */
static void
limitCheckOneLine(cob_report *r, cob_report_line *fl)
{
	cob_report_field	*rf;

	if((fl->line > 0 && r->def_lines > 0 && fl->line > r->def_lines)) {
		fprintf(stderr,_("ERROR INITIATE %s LINE %d exceeds PAGE LIMIT %d\n"),r->report_name,fl->line,r->def_lines);
#ifdef DEBUG
		LOG("PAGE LIMITs is incorrect; LINE %d > LIMIT %d\n",fl->line,r->def_lines);
#endif
		cob_set_exception (COB_EC_REPORT_PAGE_LIMIT);
		r->initiate_done = FALSE;
		return;
	}
	if((fl->next_group_line > 0 && r->def_lines > 0 && fl->next_group_line > r->def_lines)) {
		fprintf(stderr,_("ERROR INITIATE %s NEXT GROUP %d exceeds PAGE LIMIT\n"),r->report_name,fl->next_group_line);
#ifdef DEBUG
		LOG("PAGE LIMITs is incorrect; NEXT GROUP %d > LIMIT %d\n",fl->next_group_line,r->def_lines);
#endif
		cob_set_exception (COB_EC_REPORT_PAGE_LIMIT);
		r->initiate_done = FALSE;
		return;
	}
	for(rf = fl->fields; rf; rf = rf->next) {
		if((rf->line && rf->line > r->def_lines)) {
			fprintf(stderr,_("ERROR INITIATE %s LINE %d exceeds PAGE LIMIT\n"),r->report_name,rf->line);
#ifdef DEBUG
			LOG("PAGE LIMITs is incorrect; LINE %d > LIMIT %d\n",rf->line,r->def_lines);
#endif
			cob_set_exception (COB_EC_REPORT_PAGE_LIMIT);
			r->initiate_done = FALSE;
			return;
		}
		if((rf->next_group_line && rf->next_group_line > r->def_lines)) {
			fprintf(stderr,_("ERROR INITIATE %s NEXT GROUP %d exceeds PAGE LIMIT\n"),r->report_name,rf->next_group_line);
#ifdef DEBUG
			LOG("PAGE LIMITs is incorrect; NEXT GROUP %d > LIMIT %d\n",rf->next_group_line,r->def_lines);
#endif
			cob_set_exception (COB_EC_REPORT_PAGE_LIMIT);
			r->initiate_done = FALSE;
			return;
		}
	}
}

/*
 * Verify that LINE # is within PAGE LIMITS
 */
static void
limitCheckLine(cob_report *r, cob_report_line *fl)
{
	limitCheckOneLine(r,fl);
	if(fl->child)
		limitCheckLine(r,fl->child);
	if(fl->sister)
		limitCheckLine(r,fl->sister);
}

/*
 * Verify that all LINE # are within PAGE LIMITS
 */
static void
limitCheck(cob_report *r)
{
	limitCheckLine(r,r->first_line);
}

static void
saveLineCounter(cob_report *r)
{
	int	ln = r->curr_line;
	if(ln > r->def_lines)
		ln = 0;
	if(ln < 0)
		ln = 0;

	cob_set_int(r->page_counter,r->curr_page);
	cob_set_int(r->line_counter,ln);
}

/*
 * Write the Page Footing
 */
static void
do_page_footing(cob_report *r)
{
	cob_file	*f = r->report_file;
	char		*rec;

	if(r->in_page_footing)
		return;
	rec = (char *)f->record->data;
	r->in_page_footing = TRUE;
	report_line_type(r,r->first_line,COB_REPORT_PAGE_FOOTING);
	memset(rec,' ',f->record_max);
	if(r->curr_line < r->def_lines) {
		cob_write(f, f->record, COB_WRITE_BEFORE|COB_WRITE_LINES|(r->def_lines-r->curr_line), NULL, 0);
		r->curr_line = r->def_lines;
		r->incr_line = FALSE;
	} else {
		r->curr_line = 1;
	}
	saveLineCounter(r);
	r->first_detail = TRUE;
	r->in_page_footing = FALSE;
}

/*
 * Write the Page Heading
 */
static void
do_page_heading(cob_report *r)
{
	cob_file	*f = r->report_file;
	char		*rec;
	int		opt;

	if(r->in_page_heading)
		return;
	opt = COB_WRITE_BEFORE | COB_WRITE_LINES | 1;
	rec = (char *)f->record->data;
	memset(rec,' ',f->record_max);
	r->in_page_heading = TRUE;
	if(!r->first_generate) {
		r->curr_page++;
	}
	r->first_detail = FALSE;
	while(r->curr_line < r->def_heading) {		/* Skip to Heading position on page */
		cob_write(f, f->record, opt, NULL, 0);
		r->curr_line++;
		saveLineCounter(r);
	}
	report_line_type(r,r->first_line,COB_REPORT_PAGE_HEADING);
	memset(rec,' ',f->record_max);
	while(r->curr_line < r->def_first_detail) {
		cob_write(f, f->record, opt, NULL, 0);
		r->curr_line++;
		saveLineCounter(r);
	}
	clear_group_indicate(r->first_line);
	r->in_page_heading = FALSE;
}

/*
 * GENERATE one report-line
 */
static void
report_line(cob_report *r, cob_report_line *l)
{
	cob_report_field *rf;
	cob_file	*f = r->report_file;
	char		*rec,wrk[250];
	int		opt;

	opt = COB_WRITE_BEFORE | COB_WRITE_LINES | 1;
	rec = (char *)f->record->data;
	if(rec) {
		memset(rec,' ',f->record_max);
		memset(wrk,0,sizeof(wrk));
		if(r->curr_line > r->def_last_detail
		&& !r->in_report_footing
		&& !r->in_page_footing) {	/* Page overflow */
			do_page_footing(r);
		}
		if(!r->next_just_set && r->next_line_plus) {
#ifdef DEBUG
			LOG(" Execute NEXT GROUP PLUS %d\n",r->next_value);
#endif
			opt = COB_WRITE_BEFORE | COB_WRITE_LINES | (r->next_value);
			cob_write(f, f->record, opt, NULL, 0);
			r->curr_line += r->next_value;
			r->next_line_plus = FALSE;
		} else
		if(!r->next_just_set && r->next_line) {
#ifdef DEBUG
			LOG(" Execute NEXT GROUP LINE %d\n",r->next_value);
#endif
			if(r->curr_line > r->next_value) {
				do_page_footing(r);
				do_page_heading(r);
			}
			while(r->curr_line < r->next_value) {
				cob_write(f, f->record, opt, NULL, 0);
				r->curr_line++;
			}
			r->next_line = FALSE;
		} else
		if(!r->next_just_set && r->next_page) {
#ifdef DEBUG
			LOG(" Execute NEXT GROUP PAGE\n");
#endif
			do_page_footing(r);
			do_page_heading(r);
			r->next_page = FALSE;
		} else
		if( !(l->flags & COB_REPORT_LINE_PLUS)
		&&   (l->flags & COB_REPORT_LINE)) {
			if(r->curr_line > l->line) {
#ifdef DEBUG
				LOG(" Eject Page %d from line %d for Line %d\n",r->curr_page,r->curr_line,l->line);
#endif
				do_page_footing(r);
				if(r->in_report_footing) {
					r->curr_page++;		/* Now on next page */
					r->curr_line = 1;
				} else {
					do_page_heading(r);
				}
				r->first_detail = FALSE;
			}
			while(r->curr_line < l->line) {
				cob_write(f, f->record, opt, NULL, 0);
				r->curr_line++;
			}
		} else
		if((l->flags & COB_REPORT_LINE_PLUS)
		&& l->line > 1) {
			if(r->curr_line != r->def_first_detail
			|| r->def_first_detail == 0) {
				opt = COB_WRITE_BEFORE | COB_WRITE_LINES | (l->line - 1);
				cob_write(f, f->record, opt, NULL, 0);
				r->curr_line += l->line - 1;
			}
		}
		saveLineCounter(r);
		if(l->fields == NULL) {
			set_next_info(r,l);
			return;
		}
		if(l->suppress) {
#ifdef DEBUG
			if(fdbg) {
				reportDumpOneLine(r,l,0,1);
				LOG("   ^^^ Suppressed ^^^\n\n");
			}
#endif
			set_next_info(r,l);
			return;
		}

		/*
		 * Copy fields to print line area
		 */
		for(rf = l->fields; rf; rf = rf->next) {
			if(rf->suppress || rf->group_indicate) {
				if(rf->source) {		/* Copy source field in */
					cob_move(rf->source,rf->f);
				}
				continue;
			}
			if(rf->source) {		/* Copy source field in */
				cob_move(rf->source,rf->f);
				cob_field_to_string(rf->f, wrk, sizeof(wrk)-1);
				memcpy(&rec[rf->column-1], wrk, strlen(wrk));
			} else if(rf->litval) {		/* Refresh literal value */
				if(rf->f) {
					cob_str_move(rf->f, (unsigned char*)rf->litval, rf->litlen);
				}
				memcpy(&rec[rf->column-1], rf->litval, rf->litlen);
			} else if(rf->f) {
				cob_field_to_string(rf->f, wrk, sizeof(wrk)-1);
				memcpy(&rec[rf->column-1], wrk, strlen(wrk));
			}
			if((rf->flags & COB_REPORT_GROUP_INDICATE)) {	/* Suppress subsequent printings */
				rf->group_indicate = TRUE;
			}
		}
	}
#ifdef DEBUG
	if(fdbg) {
		reportDumpOneLine(r,l,0,1);
		LOG("\n");
	}
#endif
	if(rec) {
		opt = COB_WRITE_BEFORE | COB_WRITE_LINES | 1;
		cob_write(f, f->record, opt, NULL, 0);
		r->curr_line ++;
		saveLineCounter(r);
	}

	set_next_info(r,l);
}

/*
 * GENERATE one report-line
 */
static void
report_line_and(cob_report *r, cob_report_line *l, int type)
{
	if(l == NULL)
		return;
	if(l->fields == NULL
	&& l->child != NULL) {
		if(l->flags & type) {
			report_line(r,l);
			if(l->child) {
				report_line_type(r,l->child,COB_REPORT_LINE);
			}
			return;
		} 
		l = l->child;
	}
	report_line_type(r,l,type);
}

/*
 * Find Report Line of given type
 */
static cob_report_line *
get_line_type(cob_report *r, cob_report_line *l, int type)
{
	cob_report_line *t;
	if(l == NULL)
		return NULL;
	if(l->flags & type) {
		return l;
	}
	if(l->child)
		if ((t = get_line_type(r,l->child,type)) != NULL)
			return t;
	if(l->sister)
		return get_line_type(r,l->sister,type);
	return NULL;
}


/*
 * GENERATE report-line(s) of type 
 */
static int
report_line_type(cob_report *r, cob_report_line *l, int type)
{
	if(l == NULL)
		return 0;
	if(l->flags & type) {
		report_line(r,l);
		if(l->child) {
			report_line_type(r,l->child,COB_REPORT_LINE);
		}
		if(l->sister) {
			report_line_type(r,l->sister,type);
		}
		return 1;
	}
	if(l->child)
		if(report_line_type(r,l->child,type))
			return 1;
	if(l->sister)
		return report_line_type(r,l->sister,type);
	return 0;
}

/*
 * SUM all DETAIL counters
 */
static void
sum_all_detail(cob_report *r)
{
	cob_report_sum_ctr	*sc;
	cob_report_sum		*rs;
#ifdef DEBUG
	int			bHasSum = FALSE;
#endif

	/*
	 * Add up all SUM counter values
	 */
	for(sc = r->sum_counters; sc; sc = sc->next) {
		for(rs = sc->sum; rs && !sc->subtotal; rs = rs->next) {
#ifdef DEBUG
			if(!bHasSum) {
				bHasSum = TRUE;
				LOG(" Do SUM detail counters:\n");
			}
			LOG(" .. %-20s ",sc->name);
#endif
			cob_add_fields(sc->counter,rs->f,sc->counter);
		}
	}
}

/*
 * If the counter is part of another SUM then it is 'rolling forward'
 */
static void
sum_this_counter(cob_report *r, cob_field *counter)
{
	cob_report_sum_ctr	*sc;
	cob_report_sum		*rs;

	for(sc = r->sum_counters; sc; sc = sc->next) {
		for(rs = sc->sum; rs; rs = rs->next) {
			if(rs->f == counter) {
#ifdef DEBUG
				LOG("SUM %s forward ",sc->name);
#endif
				for(rs = sc->sum; rs; rs = rs->next) {
					cob_add_fields(sc->counter,rs->f,sc->counter);
				}
				break;
			}
		}
	}
}

/*
 * ZERO counters for a given control level
 */
static void
zero_all_counters(cob_report *r, int	flag, cob_report_line *l)
{
	cob_report_sum_ctr	*sc;
	cob_report_sum		*rs;
	cob_report_control	*rc;
	cob_report_control_ref	*rr;

	l = get_print_line(l);
	/*
	 * ZERO SUM counter 
	 */
	for(sc = r->sum_counters; sc; sc = sc->next) {
		for(rs = sc->sum; rs; rs = rs->next) {
			if((flag & COB_REPORT_CONTROL_FOOTING_FINAL)) {
				if(sc->control_final) {
#ifdef DEBUG
					LOG("ZERO SUM Counter %s for FOOTING FINAL\n",sc->name);
#endif
					cob_field_init(sc->counter);
				}
			} else if(sc->control) {
				rc = sc->control;
				for(rr = rc->control_ref; rr; rr=rr->next) {
					if(rr->ref_line
					&& (rr->ref_line->flags & COB_REPORT_CONTROL_HEADING))
						continue;
					if(rr->ref_line
					&& (rr->ref_line->flags & COB_REPORT_CONTROL_HEADING_FINAL))
						continue;
					if(l != NULL
					&& l != get_print_line(rr->ref_line))
						continue;
					if(rr->ref_line
					&& (rr->ref_line->flags & flag)) {
						sum_this_counter(r,sc->counter);
#ifdef DEBUG
						LOG("ZERO SUM counter %s for ",sc->name); 
						dumpFlags(rr->ref_line->flags,0,(char*)rc->name); 
						LOG("\n");
#endif
						cob_field_init(sc->counter);
					}
				}
			}
		}
	}
}

/*
 * INITIATE report
 */
void
cob_report_initiate(cob_report *r)
{
	cob_report_control	*rc;
	cob_report_control_ref	*rr;
	cob_report_sum_ctr	*sc;

	reportInitialize();
	if(r->initiate_done) {
		fprintf(stderr,_("ERROR INITIATE %s was already done\n"),r->report_name);
#ifdef DEBUG
		LOG("REPORT was already INITIATEd\n");
#endif
		cob_set_exception (COB_EC_REPORT_ACTIVE);
		return;
	}
#if 1
	if((r->def_first_detail > 0 && !(r->def_first_detail >= r->def_heading))
	|| (r->def_last_detail > 0 && !(r->def_last_detail >= r->def_first_detail))
	|| (r->def_footing > 0 && !(r->def_footing >= r->def_heading))
	|| (r->def_footing > 0 && !(r->def_footing >= r->def_last_detail))
	|| (r->def_lines > 0 && !(r->def_lines >= r->def_heading))
	|| (r->def_lines > 0 && !(r->def_lines >= r->def_footing))) {
		fprintf(stderr,_("ERROR INITIATE %s PAGE LIMIT problem\n"),r->report_name);
#ifdef DEBUG
		LOG("PAGE LIMITs is incorrect\n");
		reportDump(r,"INITIATE");
#endif
		cob_set_exception (COB_EC_REPORT_PAGE_LIMIT);
		return;
	}
#endif
	r->curr_page = 1;
	r->curr_line = 0;
	r->incr_line = TRUE;
	saveLineCounter(r);
#ifdef DEBUG
	reportDump(r,"INITIATE");
#endif
	r->initiate_done = TRUE;
	limitCheck(r);
	if(!r->initiate_done)	/* Problem during LIMIT check */
		return;
	r->first_detail = TRUE;
	r->first_generate = TRUE;
	r->next_value = 0;
	r->next_line = 0;
	r->next_line_plus = 0;
	r->next_page = 0;
	/*
	 * Allocate temp area for each control field
	 */
	for(rc = r->controls; rc; rc = rc->next) {
		if(rc->val) {
			cob_field_free(rc->val);
			rc->val = NULL;
		}
		if(rc->sf) {
			cob_field_free(rc->sf);
			rc->sf = NULL;
		}
		rc->val = cob_field_dup(rc->f,0);
		rc->sf  = cob_field_dup(rc->f,0);
		rc->has_heading = FALSE;
		rc->has_footing = FALSE;
		for(rr = rc->control_ref; rr; rr = rr->next) {
			if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING)
				rc->has_heading = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING_FINAL)
				rc->has_heading = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING)
				rc->has_footing = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING_FINAL)
				rc->has_footing = TRUE;
		}
	}
	for(sc = r->sum_counters; sc; sc = sc->next) {
		cob_field_init(sc->counter);
	}
}

/*
 * TERMINATE report
 */
int
cob_report_terminate(cob_report *r, int ctl)
{
	cob_report_control	*rc;
	cob_report_control_ref	*rr;
	cob_report_line		*pl;

	if(!r->initiate_done) {
#ifdef DEBUG
		LOG("INITIATE was never done!\n");
#endif
		fprintf(stderr,_("ERROR TERMINATE %s but No INITIATE was done\n"),r->report_name);
		cob_set_exception (COB_EC_REPORT_INACTIVE);
		return 0;
	}
	if(r->first_generate) {
#ifdef DEBUG
		LOG("No GENERATE was ever done!\n");
#endif
		return 0;
	}
	if(ctl > 0) {	 /* Continue Processing Footings from last point */
		for(rc = r->controls; rc; rc = rc->next) {
			for(rr = rc->control_ref; rr; rr = rr->next) {
				if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING) {
					pl = get_print_line(rr->ref_line);
					if(rr->ref_line->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFooting;	/* Continue Footings */
					}
					if(pl != rr->ref_line
					&& pl->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFooting;	/* Continue Footings */
					}
				}
				if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING_FINAL) {
					pl = get_print_line(rr->ref_line);
					if(rr->ref_line->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFootingFinal;	/* Continue Footings */
					}
					if(pl != rr->ref_line
					&& pl->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFootingFinal;	/* Continue Footings */
					}
				}
				if(rr->ref_line->flags & COB_REPORT_FOOTING) {
					pl = get_print_line(rr->ref_line);
					if(rr->ref_line->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintReportFooting;/* Continue Footings */
					}
					if(pl != rr->ref_line
					&& pl->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintReportFooting;/* Continue Footings */
					}
				}
			}
		}
#ifdef DEBUG
		LOG("Could not find Declarative %d\n",ctl);
#endif
		pl = get_line_type(r, r->first_line,COB_REPORT_CONTROL_FOOTING_FINAL);
		if(pl
		&& pl->use_decl == ctl) {
#ifdef DEBUG
			LOG("  Continue after Final Declaratives %d\n",ctl);
#endif
			goto PrintFootingFinal;	/* Continue Footings */
		}
		pl = get_line_type(r, r->first_line,COB_REPORT_FOOTING);
		if(pl
		&& pl->use_decl == ctl) {
#ifdef DEBUG
			LOG("  Continue after Report Declaratives %d\n",ctl);
#endif
			goto PrintReportFooting;	/* Continue Footings */
		}
	} else {
		reportInitialize();
#ifdef DEBUG
		reportDump(r,"TERMINATE");
#endif
		/* Do CONTROL FOOTING breaks */
		for(rc = r->controls; rc; rc = rc->next) {
			for(rr = rc->control_ref; rr; rr = rr->next) {
				if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING) {
					if(rr->ref_line->use_decl) {
#ifdef DEBUG
						LOG("  Return for %s Footing Declaratives %d\n",
								rc->name,rr->ref_line->use_decl);
#endif
						return rr->ref_line->use_decl;
					}
					pl = get_print_line(rr->ref_line);
					if(pl != rr->ref_line
					&& pl->use_decl) {
#ifdef DEBUG
						LOG("  Return for %s Footing Declaratives %d.\n",
								rc->name,pl->use_decl);
#endif
						return pl->use_decl;	/* Back for DECLARATIVES */
					}
PrintFooting:
					if(!rc->suppress)
						report_line_and(r,rr->ref_line,COB_REPORT_CONTROL_FOOTING);
					rc->suppress = FALSE;
					zero_all_counters(r, COB_REPORT_CONTROL_FOOTING,pl);
				}
			}
		}

	}

	/* Do CONTROL FOOTING FINAL */
	pl = get_line_type(r, r->first_line,COB_REPORT_CONTROL_FOOTING_FINAL);
	if(pl) {
		if(pl->use_decl) {
#ifdef DEBUG
			LOG("  Return for Footing Final Declaratives %d.\n", pl->use_decl);
#endif
			return pl->use_decl;	/* Back for DECLARATIVES */
		}
PrintFootingFinal:
		report_line_type(r,r->first_line,COB_REPORT_CONTROL_FOOTING_FINAL);
	}
	zero_all_counters(r, COB_REPORT_CONTROL_FOOTING_FINAL,NULL);

	do_page_footing(r);

	pl = get_line_type(r, r->first_line,COB_REPORT_FOOTING);
	if(pl) {
		if(pl->use_decl) {
#ifdef DEBUG
			LOG("  Return for Report Footing Declaratives %d.\n", pl->use_decl);
#endif
			return pl->use_decl;	/* Back for DECLARATIVES */
		}
PrintReportFooting:
		r->in_report_footing = TRUE;
		report_line_type(r,r->first_line,COB_REPORT_FOOTING);
		r->in_report_footing = FALSE;
	}

	/*
	 * Free control temp areas
	 */
	for(rc = r->controls; rc; rc = rc->next) {
		if(rc->val) {
			cob_field_free(rc->val);
			rc->val = NULL;
		}
		if(rc->sf) {
			cob_field_free(rc->sf);
			rc->sf = NULL;
		}
		rc->has_heading = FALSE;
		rc->has_footing = FALSE;
		for(rr = rc->control_ref; rr; rr = rr->next) {
			if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING)
				rc->has_heading = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING_FINAL)
				rc->has_heading = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING)
				rc->has_footing = TRUE;
			if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING_FINAL)
				rc->has_footing = TRUE;
		}
	}
	r->initiate_done = FALSE;
	return 0;
}

/*
 * GENERATE report-line
 */
int
cob_report_generate(cob_report *r, cob_report_line *l, int ctl)
{
	cob_report_control	*rc, *rp;
	cob_report_control_ref	*rr;
	cob_report_line		*pl;
	int			maxctl,ln,num;
#ifdef DEBUG
	char			wrk[128];
#endif

	reportInitialize();
	if(!r->initiate_done) {
		fprintf(stderr,_("ERROR GENERATE %s but No INITIATE was done\n"),r->report_name);
		cob_set_exception (COB_EC_REPORT_INACTIVE);
		return 0;
	}

#ifdef DEBUG
	LOG("   Enter GENERATE with ctl == %d\n",ctl);
#endif
	if(ctl > 0) {	 /* Continue Processing Footings from last point */
		for(rc = r->controls; rc; rc = rc->next) {
			for(rr = rc->control_ref; rr; rr = rr->next) {
				if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING) {
					pl = get_print_line(rr->ref_line);
					if(rr->ref_line->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFooting;	/* Continue Footings */
					}
					if(pl != rr->ref_line
					&& pl->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						goto PrintFooting;	/* Continue Footings */
					}
				}
				if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING) {
					pl = get_print_line(rr->ref_line);
					if(rr->ref_line->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						if(r->first_generate)
							goto PrintFirstHeading;
						goto PrintHeading;	/* Continue Footings */
					}
					if(pl != rr->ref_line
					&& pl->use_decl == ctl) {
#ifdef DEBUG
						LOG("  Continue after Declaratives %d\n",ctl);
#endif
						if(r->first_generate)
							goto PrintFirstHeading;
						goto PrintHeading;	/* Continue Headings */
					}
				}
			}
		}
#ifdef DEBUG
		LOG("Could not find Declarative %d\n",ctl);
#endif
	}

	if(r->incr_line) {
		r->incr_line = FALSE;
		r->curr_line++;
		saveLineCounter(r);
	}
	if(r->first_generate) {
		/* 
		 * First GENERATE of the report
		 */
		report_line_type(r,r->first_line,COB_REPORT_HEADING);
		do_page_heading(r);
		/* do CONTROL Headings */
		for(rc = r->controls; rc; rc = rc->next) {
			for(rr = rc->control_ref; rr; rr = rr->next) {
				if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING) {
					if(rr->ref_line->use_decl) {
#ifdef DEBUG
						LOG("  Return first %s Heading Declaratives %d\n",
								rc->name,rr->ref_line->use_decl);
#endif
						return rr->ref_line->use_decl;
					}
					pl = get_print_line(rr->ref_line);
					if(pl != rr->ref_line
					&& pl->use_decl) {
#ifdef DEBUG
						LOG("  Return first %s Heading Declaratives %d.\n",
								rc->name,pl->use_decl);
#endif
						return pl->use_decl;	/* Back for DECLARATIVES */
					}
PrintFirstHeading:
					report_line_and(r,rr->ref_line,COB_REPORT_CONTROL_HEADING);
				}
			}
			cob_move (rc->f,rc->val);	/* Save current field data */
			rc->data_change = FALSE;
		}

	} else {

		if(r->curr_line > r->def_last_detail) {	/* Page overflow */
			do_page_footing(r);
			r->curr_line = 1;
			do_page_heading(r);
			r->first_detail = FALSE;
		} else
		if(r->curr_line <= 1
		|| r->first_detail) {
			if(r->first_detail) {
				r->curr_line = 1;
			}
			do_page_heading(r);
			r->first_detail = FALSE;
		}

		/* 
		 * Check for FOOTINGs on other GENERATEs 
		 */
		maxctl = 0;
		for(rc = r->controls; rc; rc = rc->next) {
			rc->data_change = (cob_cmp(rc->f,rc->val) != 0);
			if(rc->data_change) {	/* Data change, implies control break at lower levels */
#ifdef DEBUG
				LOG(" Control Break %s order %d changed from ",
						rc->name,rc->sequence);
				cob_field_to_string(rc->val, wrk, sizeof(wrk)-1);
				LOG("'%s' to ",wrk);
				cob_field_to_string(rc->f, wrk, sizeof(wrk)-1);
				LOG("'%s'\n",wrk);
#endif
				cob_move(rc->f, rc->sf);	/* Save new CONTROL value */
				cob_move(rc->val,rc->f);	/* Prev value for FOOTING */
				if(rc->sequence > maxctl)
					maxctl = rc->sequence;

			}
		}
		if(maxctl > 0) {
			for(rp = r->controls; rp; rp = rp->next) {
				if(rp->sequence < maxctl
				&& !rp->data_change) {
					rp->data_change = TRUE;
#ifdef DEBUG
					LOG(" Control Break %s order %d also ...\n",
							rp->name,rp->sequence);
#endif
					cob_move(rp->f, rp->sf); /* Save CONTROL value */
					cob_move(rp->val,rp->f); /* Prev value for FOOTING */
				}
			}
		}

		for(rc = r->controls; rc; rc = rc->next) {
			if(rc->data_change) {
				for(rr = rc->control_ref; rr; rr = rr->next) {
					if(rr->ref_line->flags & COB_REPORT_CONTROL_FOOTING) {
						if(rr->ref_line->use_decl) {
#ifdef DEBUG
							LOG("  Return for %s Footing Declaratives %d\n",
									rc->name,rr->ref_line->use_decl);
#endif
							return rr->ref_line->use_decl;
						}
						pl = get_print_line(rr->ref_line);
						if(pl != rr->ref_line
						&& pl->use_decl) {
#ifdef DEBUG
							LOG("  Return for %s Footing Declaratives %d.\n",
									rc->name,pl->use_decl);
#endif
							return pl->use_decl;	/* Back for DECLARATIVES */
						}
PrintFooting:
						if(!rc->suppress
						&& !rr->ref_line->suppress)
							report_line_and(r,rr->ref_line,COB_REPORT_CONTROL_FOOTING);
						rc->suppress = FALSE;
						rr->ref_line->suppress = FALSE;
						zero_all_counters(r, COB_REPORT_CONTROL_FOOTING,pl);
						clear_group_indicate(r->first_line);
						r->next_just_set = FALSE;
					}
				}
				cob_move(rc->sf,rc->f);	/* Put new CONTROL value back */
			}
		}
		/* 
		 * Check for Control Headings
		 */
		for(rc = r->controls; rc; rc = rc->next) {
			if(rc->data_change) {
				for(rr = rc->control_ref; rr; rr = rr->next) {
					if(rr->ref_line->flags & COB_REPORT_CONTROL_HEADING) {
						if(rr->ref_line->use_decl) {
#ifdef DEBUG
							LOG("  Return for %s Heading Declaratives %d\n",
									rc->name,rr->ref_line->use_decl);
#endif
							return rr->ref_line->use_decl;
						}
						pl = get_print_line(rr->ref_line);
						if(pl != rr->ref_line
						&& pl->use_decl) {
#ifdef DEBUG
							LOG("  Return for %s Heading Declaratives %d.\n",
									rc->name,pl->use_decl);
#endif
							return pl->use_decl;	/* Back for DECLARATIVES */
						}
PrintHeading:
						if(!rr->ref_line->suppress)
							report_line_and(r,rr->ref_line,COB_REPORT_CONTROL_HEADING);
						rr->ref_line->suppress = FALSE;
					}
				}
				cob_move (rc->f,rc->val);	/* Save current field data */
			}
			rc->data_change = FALSE;
		}
	}

	sum_all_detail(r);			/* SUM detail counters */
	if(l == NULL)	{			/* GENERATE <report-name> */

	} else if(l->suppress) {
		l->suppress = FALSE;
	} else {
		if(l->sister == NULL
		&& l->fields == NULL
		&& l->child != NULL
		&& l->child->sister != NULL)
			l = l->child;		/* Multiple Detail Lines in group */

		num = ln = 0;
		for(pl = l; pl; pl = pl->sister) {
			if( NOTDETAIL(pl->flags) )
				break;
			if((pl->flags & COB_REPORT_LINE_PLUS)
			&& pl->line > 1) {
				ln += pl->line;
			}
			num++;
		}
		if(num > 1
		&& (r->curr_line + ln) > r->def_last_detail) {	/* Page overflow */
			do_page_footing(r);
			r->curr_line = 1;
			do_page_heading(r);
			r->first_detail = FALSE;
			saveLineCounter(r);
		}

		for(pl = l; pl; pl = pl->sister) {
			if( NOTDETAIL(pl->flags) )
				break;
			l = get_print_line(pl);		/* Find line with data fields */
			if(!l->suppress) 
				report_line(r,l);	/* Generate this DETAIL line */
			l->suppress = FALSE;
		}
	}

	/*
	 * Zero out SUM counters
	 */
	zero_all_counters(r, COB_REPORT_DETAIL, NULL);
	clear_suppress(r->first_line);
	r->first_generate = FALSE;
	r->next_just_set = FALSE;
	r->curr_line--;
	r->incr_line = TRUE;
	saveLineCounter(r);
	return 0;
}

/*
 * SUPPRESS printing of this CONTROL level
 */
void
cob_report_suppress(cob_report *r, cob_report_line *l)
{
	cob_report_control	*rc;
	cob_report_control_ref	*rr;
	cob_report_line		*pl;

	for(rc = r->controls; rc; rc = rc->next) {
		for(rr = rc->control_ref; rr; rr = rr->next) {
			if(rr->ref_line == l) {
				rc->suppress = TRUE;
				return;
			}
			pl = get_print_line(rr->ref_line);
			if(pl == l) {
				rc->suppress = TRUE;
				return;
			}
		}
	}
	fprintf(stderr,_("ERROR Could not find line to suppress in report %s\n"),r->report_name);
}
