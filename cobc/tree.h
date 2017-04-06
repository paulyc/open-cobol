/*
   Copyright (C) 2001-2012, 2014-2017 Free Software Foundation, Inc.
   Written by Keisuke Nishida, Roger While, Simon Sobisch, Sergey Kashyrin

   This file is part of GnuCOBOL C++.

   The GnuCOBOL C++ compiler is free software: you can redistribute it

   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GnuCOBOL C++ is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GnuCOBOL C++.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef CB_TREE_H
#define CB_TREE_H

#define CB_BEFORE		cb_int0
#define CB_AFTER		cb_int1

#define COB_MAX_SUBSCRIPTS	16

#define CB_PREFIX_ATTR		"_a_"	/* Field attribute(cob_field_attr) */
#define CB_PREFIX_BASE		"_b_"	/* Base address(unsigned char *) */
#define CB_PREFIX_CONST		"_c_"	/* Constant or literal(cob_field) */
#define CB_PREFIX_DECIMAL	"_d_"	/* Decimal number(cob_decimal) */
#define CB_PREFIX_FIELD		"_f_"	/* Field(cob_field) */
#define CB_PREFIX_FILE		"_h_"	/* File(cob_file) */
#define CB_PREFIX_KEYS		"_k_"	/* File keys(cob_file_key []) */
#define CB_PREFIX_LABEL		"_l_"	/* Label */
#define CB_PREFIX_PIC		"_p_"	/* PICTURE string */
#define CB_PREFIX_SEQUENCE	"_s_"	/* Collating sequence */
#define CB_PREFIX_STRING	"_st_"	/* String */

#define CB_PROGRAM_TYPE		0
#define CB_FUNCTION_TYPE	1

#define CB_CALL_BY_REFERENCE	1
#define CB_CALL_BY_CONTENT		2
#define CB_CALL_BY_VALUE		3

#define CB_SIZE_AUTO		0
#define CB_SIZE_1			1
#define CB_SIZE_2			2
#define CB_SIZE_4			3
#define CB_SIZE_8			4
#define CB_SIZE_UNSIGNED	8

/* Hash values */
/* Power of 2 - see hash function in tree.c */
#define CB_WORD_HASH_SIZE	(1U << 11)
#define CB_WORD_HASH_MASK	(CB_WORD_HASH_SIZE - 1U)

/* Basic tree tag */
enum cb_tag {
	/* Primitives */
	CB_TAG_CONST = 0,	/* 0 Constant value */
	CB_TAG_INTEGER,		/* 1 Integer constant */
	CB_TAG_STRING,		/* 2 String constant */
	CB_TAG_ALPHABET_NAME,	/* 3 Alphabet-name */
	CB_TAG_CLASS_NAME,	/* 4 Class-name */
	CB_TAG_LOCALE_NAME,	/* 5 Locale-name */
	CB_TAG_SYSTEM_NAME,	/* 6 System-name */
	CB_TAG_LITERAL,		/* 7 Numeric/alphanumeric literal */
	CB_TAG_DECIMAL,		/* 8 Decimal number */
	CB_TAG_FIELD,		/* 9 User-defined variable */
	CB_TAG_FILE,		/* 10 File description */
	CB_TAG_REPORT,		/* 11 Report description */
	CB_TAG_CD,			/* 12 Communication description */
	/* Expressions */
	CB_TAG_REFERENCE,	/* 13 Reference to a field, file, or label */
	CB_TAG_BINARY_OP,	/* 14 Binary operation */
	CB_TAG_FUNCALL,		/* 15 Run-time function call */
	CB_TAG_CAST,		/* 16 Type cast */
	CB_TAG_INTRINSIC,	/* 17 Intrinsic function */
	/* Statements */
	CB_TAG_LABEL,		/* 18 Label statement */
	CB_TAG_ASSIGN,		/* 19 Assignment statement */
	CB_TAG_INITIALIZE,	/* 20 INITIALIZE statement */
	CB_TAG_SEARCH,		/* 21 SEARCH statement */
	CB_TAG_CALL,		/* 22 CALL statement */
	CB_TAG_GOTO,		/* 23 GO TO statement */
	CB_TAG_IF,			/* 24 IF statement */
	CB_TAG_PERFORM,		/* 25 PERFORM statement */
	CB_TAG_STATEMENT,	/* 26 General statement */
	CB_TAG_CONTINUE,	/* 27 CONTINUE statement */
	CB_TAG_CANCEL,		/* 28 CANCEL statement */
	CB_TAG_ALTER,		/* 29 ALTER statement */
	CB_TAG_SET_ATTR,	/* 30 SET ATTRIBUTE statement */
	/* Miscellaneous */
	CB_TAG_PERFORM_VARYING,	/* 31 PERFORM VARYING parameter */
	CB_TAG_PICTURE,		/* 32 PICTURE clause */
	CB_TAG_LIST,		/* 33 List */
	CB_TAG_DIRECT,		/* 34 Code output or comment */
	CB_TAG_DEBUG,		/* 35 Debug item set */
	CB_TAG_DEBUG_CALL,	/* 36 Debug callback */
	CB_TAG_PROGRAM,		/* 37 Program */
	CB_TAG_PROTOTYPE	/* 38 Prototype */
};

/* Alphabet type */
#define CB_ALPHABET_NATIVE	0
#define CB_ALPHABET_ASCII	1
#define CB_ALPHABET_EBCDIC	2
#define CB_ALPHABET_CUSTOM	3

/* Call convention bits */
/* Bit number	Meaning			Value */
/*	0	currently ignored by GC			*/
/*		Parameter order		0 - Right to left		*/
/*					1 - Left to right		*/
/*	1	currently ignored by GC			*/
/*		Stack manipulation	0 - Caller removes params	*/
/*					1 - Callee removes params	*/
/*	2	RETURN-CODE update	0 - Updated			*/
/*					1 - Not updated			*/
/*	3	Linking behaviour	0 - Normal linking		*/
/*					1 - Static CALL linking		*/
/*	4	currently ignored by GC + MF		*/
/*		OS/2 Optlink		0 - ??				*/
/*					1 - ??				*/
/*	5	currently ignored by GC + MF		*/
/*		Thunked to 16 bit	0 - No thunk			*/
/*					1 - Thunk			*/
/*	6	GC: works bith with static/dynamic calls */
/*		MF: this has his has no effect on dynamic calls	*/
/*		STDCALL convention	0 - CDECL			*/
/*					1 - STDCALL			*/
/*	7	currently ignored by GC + MF		*/
/*	8	currently ignored by GC			*/
/*		parameter-count for individual entry points	0 - checked	*/
/*					1 - not checked			*/
/*	9	currently ignored by GC			*/
/*		case of call + program names	0 - disregareded (depending on compile time flags)		*/
/*					1 - regarded			*/
/*	10	currently ignored by GC			*/
/*		RETURN-CODE storage	0 - passed as return value		*/
/*					1 - passed in the first parameter			*/
/*	11-14	currently ignored by GC+MF			*/
/*	15	GC: enabling COBOL parameter handling for external callers	*/
/*		currently ignored by MF			*/
/*					0 - external callers don't set cob_call_params	*/
/*					1 - external callers set cob_call_params	- standard (!)*/

#define CB_CONV_L_TO_R			(1 << 0)
#define CB_CONV_CALLEE_STACK	(1 << 1)
#define CB_CONV_NO_RET_UPD		(1 << 2)
#define CB_CONV_STATIC_LINK		(1 << 3)
#define CB_CONV_OPT_LINK		(1 << 4)
#define CB_CONV_THUNK_16		(1 << 5)
#define CB_CONV_STDCALL			(1 << 6)
#define CB_CONV_COBOL			(1 << 15)

/* System category */
enum cb_system_name_category {
	CB_DEVICE_NAME = 0,
	CB_SWITCH_NAME,
	CB_FEATURE_NAME,
	CB_CALL_CONVENTION_NAME,
	CB_CODE_NAME,
	CB_COMPUTER_NAME,
	CB_EXTERNAL_LOCALE_NAME,
	CB_LIBRARY_NAME,
	CB_TEXT_NAME
};

/* Mnemonic defines */
/* Devices */
#define CB_DEVICE_SYSIN		0
#define CB_DEVICE_SYSOUT	1
#define CB_DEVICE_SYSERR	2
#define CB_DEVICE_CONSOLE	3
/* Switches (max. must match COB_SWITCH_MAX) */
#define CB_SWITCH_0			0
#define CB_SWITCH_1			1
#define CB_SWITCH_2			2
#define CB_SWITCH_3			3
#define CB_SWITCH_4			4
#define CB_SWITCH_5			5
#define CB_SWITCH_6			6
#define CB_SWITCH_7			7
#define CB_SWITCH_8			8
#define CB_SWITCH_9			9
#define CB_SWITCH_10		10
#define CB_SWITCH_11		11
#define CB_SWITCH_12		12
#define CB_SWITCH_13		13
#define CB_SWITCH_14		14
#define CB_SWITCH_15		15
#define CB_SWITCH_16		16
#define CB_SWITCH_17		17
#define CB_SWITCH_18		18
#define CB_SWITCH_19		19
#define CB_SWITCH_20		20
#define CB_SWITCH_21		21
#define CB_SWITCH_22		22
#define CB_SWITCH_23		23
#define CB_SWITCH_24		24
#define CB_SWITCH_25		25
#define CB_SWITCH_26		26
#define CB_SWITCH_27		27
#define CB_SWITCH_28		28
#define CB_SWITCH_29		29
#define CB_SWITCH_30		30
#define CB_SWITCH_31		31
#define CB_SWITCH_32		32
#define CB_SWITCH_33		33
#define CB_SWITCH_34		34
#define CB_SWITCH_35		35
#define CB_SWITCH_36		36
/* Features */
#define CB_FEATURE_FORMFEED	0
#define CB_FEATURE_CONVENTION	1
#define CB_FEATURE_C01		2
#define CB_FEATURE_C02		3
#define CB_FEATURE_C03		4
#define CB_FEATURE_C04		5
#define CB_FEATURE_C05		6
#define CB_FEATURE_C06		7
#define CB_FEATURE_C07		8
#define CB_FEATURE_C08		9
#define CB_FEATURE_C09		10
#define CB_FEATURE_C10		11
#define CB_FEATURE_C11		12
#define CB_FEATURE_C12		13


/* Class category */
enum cb_class {
	CB_CLASS_UNKNOWN = 0,	/* 0 */
	CB_CLASS_ALPHABETIC,	/* 1 */
	CB_CLASS_ALPHANUMERIC,	/* 2 */
	CB_CLASS_BOOLEAN,		/* 3 */
	CB_CLASS_INDEX,			/* 4 */
	CB_CLASS_NATIONAL,		/* 5 */
	CB_CLASS_NUMERIC,		/* 6 */
	CB_CLASS_OBJECT,		/* 7 */
	CB_CLASS_POINTER		/* 8 */
};

/* Category */
enum cb_category {
	CB_CATEGORY_UNKNOWN = 0,		/* 0 */
	CB_CATEGORY_ALPHABETIC,			/* 1 */
	CB_CATEGORY_ALPHANUMERIC,		/* 2 */
	CB_CATEGORY_ALPHANUMERIC_EDITED,/* 3 */
	CB_CATEGORY_BOOLEAN,			/* 4 */
	CB_CATEGORY_INDEX,				/* 5 */
	CB_CATEGORY_NATIONAL,			/* 6 */
	CB_CATEGORY_NATIONAL_EDITED,	/* 7 */
	CB_CATEGORY_NUMERIC,			/* 8 */
	CB_CATEGORY_NUMERIC_EDITED,		/* 9 */
	CB_CATEGORY_OBJECT_REFERENCE,	/* 10 */
	CB_CATEGORY_DATA_POINTER,		/* 11 */
	CB_CATEGORY_PROGRAM_POINTER		/* 12 */
};

/* Storage sections */
enum cb_storage {
	CB_STORAGE_CONSTANT = 0,	/* Constants */
	CB_STORAGE_FILE,			/* FILE SECTION */
	CB_STORAGE_WORKING,			/* WORKING-STORAGE SECTION */
	CB_STORAGE_LOCAL,			/* LOCAL-STORAGE SECTION */
	CB_STORAGE_LINKAGE,			/* LINKAGE SECTION */
	CB_STORAGE_SCREEN,			/* SCREEN SECTION */
	CB_STORAGE_REPORT,			/* REPORT SECTION */
	CB_STORAGE_COMMUNICATION	/* COMMUNICATION SECTION */
};

/* Field types */
enum cb_usage {
	CB_USAGE_BINARY = 0,		/* 0 */
	CB_USAGE_BIT,				/* 1 */
	CB_USAGE_COMP_5,			/* 2 */
	CB_USAGE_COMP_X,			/* 3 */
	CB_USAGE_DISPLAY,			/* 4 */
	CB_USAGE_FLOAT,				/* 5 */
	CB_USAGE_DOUBLE,			/* 6 */
	CB_USAGE_INDEX,				/* 7 */
	CB_USAGE_NATIONAL,			/* 8 */
	CB_USAGE_OBJECT,			/* 9 */
	CB_USAGE_PACKED,			/* 10 */
	CB_USAGE_POINTER,			/* 11 */
	CB_USAGE_LENGTH,			/* 12 */
	CB_USAGE_PROGRAM_POINTER,	/* 13 */
	CB_USAGE_UNSIGNED_CHAR,		/* 14 */
	CB_USAGE_SIGNED_CHAR,		/* 15 */
	CB_USAGE_UNSIGNED_SHORT,	/* 16 */
	CB_USAGE_SIGNED_SHORT,		/* 17 */
	CB_USAGE_UNSIGNED_INT,		/* 18 */
	CB_USAGE_SIGNED_INT,		/* 19 */
	CB_USAGE_UNSIGNED_LONG,		/* 20 */
	CB_USAGE_SIGNED_LONG,		/* 21 */
	CB_USAGE_COMP_6,			/* 22 */
	CB_USAGE_FP_DEC64,			/* 23 */
	CB_USAGE_FP_DEC128,			/* 24 */
	CB_USAGE_FP_BIN32,			/* 25 */
	CB_USAGE_FP_BIN64,			/* 26 */
	CB_USAGE_FP_BIN128,			/* 27 */
	CB_USAGE_LONG_DOUBLE,		/* 28 */
	CB_USAGE_UNION				/* 29 */
};


/* Cast type */
enum cb_cast_type {
	CB_CAST_INTEGER = 0,	/* 0 */
	CB_CAST_LONG_INT,		/* 1 */
	CB_CAST_ADDRESS,		/* 2 */
	CB_CAST_ADDR_OF_ADDR,	/* 3 */
	CB_CAST_LENGTH,			/* 4 */
	CB_CAST_PROGRAM_POINTER	/* 5 */
};

/* Intrinsic functions */
enum cb_intr_enum {
	CB_INTR_ABS = 1,
	CB_INTR_ACOS,
	CB_INTR_ANNUITY,
	CB_INTR_ASIN,
	CB_INTR_ATAN,
	CB_INTR_BOOLEAN_OF_INTEGER,
	CB_INTR_BYTE_LENGTH,
	CB_INTR_CHAR,
	CB_INTR_CHAR_NATIONAL,
	CB_INTR_COMBINED_DATETIME,
	CB_INTR_CONCATENATE,
	CB_INTR_COS,
	CB_INTR_CURRENCY_SYMBOL,
	CB_INTR_CURRENT_DATE,
	CB_INTR_DATE_OF_INTEGER,
	CB_INTR_DATE_TO_YYYYMMDD,
	CB_INTR_DAY_OF_INTEGER,
	CB_INTR_DAY_TO_YYYYDDD,
	CB_INTR_DISPLAY_OF,
	CB_INTR_E,
	CB_INTR_EXCEPTION_FILE,
	CB_INTR_EXCEPTION_FILE_N,
	CB_INTR_EXCEPTION_LOCATION,
	CB_INTR_EXCEPTION_LOCATION_N,
	CB_INTR_EXCEPTION_STATEMENT,
	CB_INTR_EXCEPTION_STATUS,
	CB_INTR_EXP,
	CB_INTR_EXP10,
	CB_INTR_FACTORIAL,
	CB_INTR_FORMATTED_CURRENT_DATE,
	CB_INTR_FORMATTED_DATE,
	CB_INTR_FORMATTED_DATETIME,
	CB_INTR_FORMATTED_TIME,
	CB_INTR_FRACTION_PART,
	CB_INTR_HIGHEST_ALGEBRAIC,
	CB_INTR_INTEGER,
	CB_INTR_INTEGER_OF_BOOLEAN,
	CB_INTR_INTEGER_OF_DATE,
	CB_INTR_INTEGER_OF_DAY,
	CB_INTR_INTEGER_OF_FORMATTED_DATE,
	CB_INTR_INTEGER_PART,
	CB_INTR_LENGTH,
	CB_INTR_LOCALE_COMPARE,
	CB_INTR_LOCALE_DATE,
	CB_INTR_LOCALE_TIME,
	CB_INTR_LOCALE_TIME_FROM_SECS,
	CB_INTR_LOG,
	CB_INTR_LOG10,
	CB_INTR_LOWER_CASE,
	CB_INTR_LOWEST_ALGEBRAIC,
	CB_INTR_MAX,
	CB_INTR_MEAN,
	CB_INTR_MEDIAN,
	CB_INTR_MIDRANGE,
	CB_INTR_MIN,
	CB_INTR_MOD,
	CB_INTR_MODULE_CALLER_ID,
	CB_INTR_MODULE_DATE,
	CB_INTR_MODULE_FORMATTED_DATE,
	CB_INTR_MODULE_ID,
	CB_INTR_MODULE_PATH,
	CB_INTR_MODULE_SOURCE,
	CB_INTR_MODULE_TIME,
	CB_INTR_MON_DECIMAL_POINT,
	CB_INTR_MON_THOUSANDS_SEP,
	CB_INTR_NATIONAL_OF,
	CB_INTR_NUM_DECIMAL_POINT,
	CB_INTR_NUM_THOUSANDS_SEP,
	CB_INTR_NUMVAL,
	CB_INTR_NUMVAL_C,
	CB_INTR_NUMVAL_F,
	CB_INTR_ORD,
	CB_INTR_ORD_MAX,
	CB_INTR_ORD_MIN,
	CB_INTR_PI,
	CB_INTR_PRESENT_VALUE,
	CB_INTR_RANDOM,
	CB_INTR_RANGE,
	CB_INTR_REM,
	CB_INTR_REVERSE,
	CB_INTR_SECONDS_FROM_FORMATTED_TIME,
	CB_INTR_SECONDS_PAST_MIDNIGHT,
	CB_INTR_SIGN,
	CB_INTR_SIN,
	CB_INTR_SQRT,
	CB_INTR_STANDARD_COMPARE,
	CB_INTR_STANDARD_DEVIATION,
	CB_INTR_STORED_CHAR_LENGTH,
	CB_INTR_SUBSTITUTE,
	CB_INTR_SUBSTITUTE_CASE,
	CB_INTR_SUM,
	CB_INTR_TAN,
	CB_INTR_TEST_DATE_YYYYMMDD,
	CB_INTR_TEST_DAY_YYYYDDD,
	CB_INTR_TEST_FORMATTED_DATETIME,
	CB_INTR_TEST_NUMVAL,
	CB_INTR_TEST_NUMVAL_C,
	CB_INTR_TEST_NUMVAL_F,
	CB_INTR_TRIM,
	CB_INTR_UPPER_CASE,
	CB_INTR_USER_FUNCTION,
	CB_INTR_VARIANCE,
	CB_INTR_WHEN_COMPILED,
	CB_INTR_YEAR_TO_YYYY
};

/* Perform type */
enum cb_perform_type {
	CB_PERFORM_EXIT = 0,
	CB_PERFORM_ONCE,
	CB_PERFORM_TIMES,
	CB_PERFORM_UNTIL,
	CB_PERFORM_FOREVER
};

/* Reserved word list structure */
struct cobc_reserved {
	const char *	name;			/* Word */
	unsigned short	nodegen;		/* Statement with END-xxx */
	unsigned short	context_sens;	/* Context sensitive */
	int				token;			/* Token */
	unsigned int	context_set;	/* Set context sensitive */
	unsigned int	context_test;	/* Test context sensitive */
};

/* Basic common tree structure */

struct cb_tree_common {
	enum cb_tag			tag;			/* TAG - see below */
	enum cb_category	category;		/* Category */
	const char		*	 source_file;	/* Source file */
	int					source_line;	/* Line */
	int					source_column;	/* Column */
};

/* Define common cb_tree/CB_TREE for following defines */

typedef cb_tree_common * cb_tree;

/////////////////////////////////////////////////////////////////
// Several prototypes needed below
extern cb_tree	cb_error_node;
char 	*		cb_name(cb_tree);
cb_class		cb_tree_class(cb_tree);
cb_category		cb_tree_category(cb_tree);
cb_tree			cb_ref(cb_tree);
size_t			cb_check_index_p(cb_tree x);
// Several prototypes needed below
/////////////////////////////////////////////////////////////////

inline cb_tag CB_TREE_TAG(cb_tree x)
{
	return x->tag;
}
inline cb_class CB_TREE_CLASS(cb_tree x)
{
	return cb_tree_class(x);
}
inline cb_category CB_TREE_CATEGORY(cb_tree x)
{
	return cb_tree_category(x);
}

inline bool CB_VALID_TREE(cb_tree x)
{
	return (x && x != cb_error_node);
}
inline bool CB_INVALID_TREE(cb_tree x)
{
	return (!(x) || x == cb_error_node);
}

#ifdef	COB_TREE_DEBUG

#ifdef	COB_HAVE_STEXPR
#define CB_TREE_CAST(tg,ty,x)								\
	({															\
		cb_tree _x = (x);										\
		if(unlikely(!_x || CB_TREE_TAG(_x) != tg)) {			\
			cobc_tree_cast_error(_x, __FILE__, __LINE__, tg);	\
		}														\
		((ty *)(_x));											\
	})
#else
#define CB_TREE_CAST(tg,ty,x)	\
	((ty *)cobc_tree_cast_check(x, __FILE__, __LINE__, tg))
#endif

#else
#define CB_TREE_CAST(tg,ty,x)	((ty *)(x))
#endif


/* xref entries */
struct cb_xref_elem {
	struct cb_xref_elem	* next;
	int			line;
	int			receive;
};

struct cb_xref {
	struct cb_xref_elem	* head;
	struct cb_xref_elem	* tail;
	int			skip;
};

/* Constant */

struct cb_const : cb_tree_common {
	const char	*	val;		/* Constant value */
};

inline cb_const * CB_CONST(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CONST, cb_const, x);
}
inline bool CB_CONST_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CONST);
}

/* Code output or comment */

struct cb_direct : cb_tree_common {
	const char *	line;			/* Line redirect */
	cob_u32_t		flag_is_direct;	/* Is directed */
	cob_u32_t		flag_new_line;	/* Need new line */
};

inline cb_direct * CB_DIRECT(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_DIRECT, cb_direct, x);
}
inline bool CB_DIRECT_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_DIRECT);
}

/* DEBUG */

struct cb_debug : cb_tree_common {
	cb_tree			target;		/* Target for debug */
	const char *	value;		/* Value for debug */
	cb_tree			fld;		/* Reference */
	size_t			size;		/* Size if relevant */
};

inline cb_debug * CB_DEBUG(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_DEBUG, cb_debug, x);
}
inline bool CB_DEBUG_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_DEBUG);
}

/* DEBUG Callback */

struct cb_debug_call : cb_tree_common {
	cb_label *	target;	/* Target label */
};

inline cb_debug_call * CB_DEBUG_CALL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_DEBUG_CALL, cb_debug_call, x);
}
inline bool CB_DEBUG_CALL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_DEBUG_CALL);
}

/* Integer */

struct cb_integer : cb_tree_common {
	int				val;		/* Integer value */
	unsigned int	hexval;		/* Output hex value */
};

inline cb_integer * CB_INTEGER(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_INTEGER, cb_integer, x);
}
inline bool CB_INTEGER_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_INTEGER);
}

/* String */

struct cb_string : cb_tree_common {
	const unsigned char * data;		/* Data */
	size_t				size;		/* Data size */
};

inline cb_string * CB_STRING(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_STRING, cb_string, x);
}
inline bool CB_STRING_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_STRING);
}

/* Alphabet-name */

struct cb_alphabet_name : cb_tree_common {
	const char *	name;			/* Original name */
	char 	*		cname;			/* Name used in C */
	cb_tree			custom_list;	/* Custom ALPHABET */
	unsigned int	alphabet_type;	/* ALPHABET type */
	int				low_val_char;	/* LOW-VALUE */
	int				high_val_char;	/* HIGH-VALUE */
	int				values[256];	/* Collating values */
	int				alphachr[256];	/* Actual values */
};

inline cb_alphabet_name * CB_ALPHABET_NAME(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_ALPHABET_NAME, cb_alphabet_name, x);
}
inline bool CB_ALPHABET_NAME_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_ALPHABET_NAME);
}

/* Class-name */

struct cb_class_name : cb_tree_common {
	const char *	name;		/* Original name */
	char 	*		cname;		/* Name used in C */
	cb_tree			list;		/* List of CLASS definitions */
};

inline cb_class_name * CB_CLASS_NAME(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CLASS_NAME, cb_class_name, x);
}
inline bool CB_CLASS_NAME_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CLASS_NAME);
}

/* Locale name */

struct cb_locale_name : cb_tree_common {
	const char *	name;		/* Original name */
	char 	*		cname;		/* Name used in C */
	cb_tree			list;		/* List of locale definitions */
};

inline cb_locale_name * CB_LOCALE_NAME(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_LOCALE_NAME, cb_locale_name, x);
}
inline bool CB_LOCALE_NAME_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_LOCALE_NAME);
}

/* System-name */

struct cb_system_name : cb_tree_common {
	cb_tree	value;							/* System value */
	enum cb_system_name_category category;	/* System category */
	int		token;							/* Device attributes */
};

inline cb_system_name * CB_SYSTEM_NAME(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_SYSTEM_NAME, cb_system_name, x);
}
inline bool CB_SYSTEM_NAME_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_SYSTEM_NAME);
}

/* Literal */

struct cb_literal : cb_tree_common {
	unsigned char *	data;	/* Literal data */
	cob_u32_t		size;	/* Literal size */
	int				scale;	/* Numeric scale */
	cob_u32_t		llit;	/* 'L' literal */
	short			sign;	/* unsigned: 0 negative: -1 positive: 1 */
	short			all;	/* ALL */
};

inline cb_literal * CB_LITERAL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_LITERAL, cb_literal, x);
}
inline bool CB_LITERAL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_LITERAL);
}
inline bool CB_NUMERIC_LITERAL_P(cb_tree x)
{
	return (CB_LITERAL_P(x) && CB_TREE_CATEGORY(x) == CB_CATEGORY_NUMERIC);
}

/* Decimal */

struct cb_decimal : cb_tree_common {
	int			id;		/* Id for this decimal */
};

inline cb_decimal * CB_DECIMAL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_DECIMAL, cb_decimal, x);
}
inline bool CB_DECIMAL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_DECIMAL);
}

/* Picture */

struct cb_picture : cb_tree_common {
	char 	*	orig;			/* Original picture string */
	cob_pic_symbol * str;		/* Packed picture string */
	int			size;			/* Byte size */
	int			lenstr;			/* Length of picture string */
	enum cb_category category;	/* Field category */
	cob_u32_t	digits;			/* Number of digit places */
	int			scale;			/* 1/10^scale */
	cob_u32_t	have_sign;		/* Have 'S' */
	cob_u32_t	real_digits;	/* Real number of digits */
};

inline cb_picture * CB_PICTURE(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_PICTURE, cb_picture, x);
}
inline bool CB_PICTURE_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_PICTURE);
}

/* Field */

struct cb_key {
	cb_tree	key;			/* KEY */
	cb_tree	ref;			/* Reference used in SEARCH ALL */
	cb_tree	val;			/* Value to be compared in SEARCH ALL */
	int	dir;				/* ASCENDING or DESCENDING */
};

struct cb_field : cb_tree_common {
	const char *	name;			/* Original name */
	const char *	ename;			/* Externalized name */
	//////////////// SKA /////////////////////////
	const char *	sName;			// COBOL name(adjusted)
	const char *	sRefName;		// Full reference name - for printout only
	const char *	sType;			// struct/union/class name
	cb_field 	*	upsister;		// Make bi-directional list
	bool			bOutDone;		// "struct" was written to storage
	bool			bPointer;		// main name is a pointer to "struct"
	bool			bUseName;		// Use COBOL naming
	//////////////// SKA /////////////////////////
	cb_tree			depending;		/* OCCURS ... DEPENDING ON */
	cb_tree			values;			/* VALUE */
	cb_tree			false_88;		/* 88 FALSE clause */
	cb_tree			index_list;		/* INDEXED BY */
	cb_field 	*	parent;			/* Upper level field(if any) */
	cb_field 	*	children;		/* Top of lower level fields */
	cb_field 	*	validation;		/* First level 88 field (if any) */
	cb_field 	*	sister;			/* Fields at the same level */
	cb_field 	*	redefines;		/* REDEFINES */
	cb_field 	*	rename_thru;	/* RENAMES THRU */
	cb_field 	*	index_qual;		/* INDEXED BY qualifier */
	struct cb_file * file;			/* FD section file name */
	struct cb_cd *	cd;				/* CD name */
	cb_key 	*	keys;			/* SEARCH key */
	cb_picture *	pic;			/* PICTURE */
	cb_field 	*	vsize;			/* Variable size cache */
	cb_label 	*	debug_section;	/* DEBUG section */
	cb_xref			xref;			/* xref elements */

	cb_tree			screen_line;	/* LINE */
	cb_tree			screen_column;	/* COLUMN */
	cb_tree			screen_from;	/* TO and USING */
	cb_tree			screen_to;		/* FROM and USING */
	cb_tree			screen_foreg;	/* FOREGROUND */
	cb_tree			screen_backg;	/* BACKGROUND */
	cb_tree			screen_prompt;	/* PROMPT */

	int				id;				/* Field id */
	int				size;			/* Field size */
	int				level;			/* Level number */
	int				memory_size;	/* Memory size */
	int				offset;			/* Byte offset from 01 level */
	int				occurs_min;		/* OCCURS <min> */
	int				occurs_max;		/* OCCURS [... TO] <max> */
	int				indexes;		/* Indices count(OCCURS) */

	int				count;			/* Reference count */
	int				mem_offset;		/* Memory offset */
	int				nkeys;			/* Number of keys */
	int				param_num;		/* CHAINING param number */
	cob_flags_t		screen_flag;	/* Flags used in SCREEN SECTION */
	int				step_count;		/* STEP in REPORT */
	unsigned int	vaddr;			/* Variable address cache */
	unsigned int	odo_level;		/* ODO level (0 = no ODO item)
									   could be direct ODO (check via depending)
									   or via subordinate) */
	cob_u32_t		special_index;	/* Special field */

	enum cb_storage		storage;	/* Storage section */
	enum cb_usage		usage;		/* USAGE */

	/* Flags */
	unsigned char flag_base;			/* Has memory allocation */
	unsigned char flag_external;		/* EXTERNAL */
	unsigned char flag_local_storage;	/* LOCAL storage */
	unsigned char flag_is_global;		/* Is GLOBAL */

	unsigned int flag_local			: 1;	/* Has local scope */
	unsigned int flag_occurs		: 1;	/* OCCURS */
	unsigned int flag_sign_clause	: 1;	/* Any SIGN clause */
	unsigned int flag_sign_separate	: 1;	/* SIGN IS SEPARATE */
	unsigned int flag_sign_leading	: 1;	/* SIGN IS LEADING */
	unsigned int flag_blank_zero	: 1;	/* BLANK WHEN ZERO */
	unsigned int flag_justified		: 1;	/* JUSTIFIED RIGHT */
	unsigned int flag_binary_swap	: 1;	/* Binary byteswap */
	unsigned int flag_real_binary	: 1;	/* BINARY-CHAR/SHORT/LONG/DOUBLE */

	unsigned int flag_is_pointer	: 1;	/* Is POINTER */
	unsigned int flag_item_78		: 1;	/* Is 78 level */
	unsigned int flag_any_length	: 1;	/* Is ANY LENGTH */
	unsigned int flag_item_based	: 1;	/* Is BASED */
	unsigned int flag_filler		: 1;	/* Implicit/explicit filler */
	unsigned int flag_synchronized	: 1;	/* SYNCHRONIZED */
	unsigned int flag_invalid		: 1;	/* Is broken */
	unsigned int flag_field			: 1;	/* Has been internally cached */

	unsigned int flag_chained		: 1;	/* CHAINING item */
	unsigned int flag_anylen_done	: 1;	/* ANY LENGTH is set up */
	unsigned int flag_indexed_by	: 1;	/* INDEXED BY item */
	unsigned int flag_is_verified	: 1;	/* Has been verified */
	unsigned int flag_is_c_long		: 1;	/* Is BINARY-C-LONG */
	unsigned int flag_is_pdiv_parm	: 1;	/* Is PROC DIV USING */
	unsigned int flag_is_pdiv_opt	: 1;	/* Is PROC DIV USING OPTIONAL */
	unsigned int flag_local_alloced	: 1;	/* LOCAL storage is allocated */
	unsigned int flag_no_init		: 1;	/* No initialize unless used */

	unsigned int flag_vsize_done	: 1;	/* Variable size cached */
	unsigned int flag_vaddr_done	: 1;	/* Variable address cached */
	unsigned int flag_odo_relative	: 1;	/* complex-odo: item address depends
											   on size of a different (ODO) item */
	unsigned int flag_field_debug	: 1;	/* DEBUGGING */
	unsigned int flag_all_debug		: 1;	/* DEBUGGING */
	unsigned int flag_no_field		: 1;	/* SCREEN dummy field */
	unsigned int flag_any_numeric	: 1;	/* Is ANY NUMERIC */
	unsigned int flag_is_returning	: 1;	/* Is RETURNING item */
	unsigned int flag_unbounded		: 1;	/* OCCURS UNBOUNDED */
};

inline cb_field * CB_FIELD(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_FIELD, cb_field, x);
}
inline bool CB_FIELD_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_FIELD);
}

/* Index */

inline size_t CB_INDEX_P(cb_tree x)
{
	return cb_check_index_p(x);
}

/* Label */

struct cb_para_label {
	cb_para_label	* next;
	cb_label	*	para;
};

struct cb_alter_id {
	cb_alter_id	* next;
	int			goto_id;
};

struct cb_label : cb_tree_common {
	const char *	name;			/* Name */
	const char *	orig_name;		/* Original name */
	cb_label 	*	section;		/* Parent SECTION */
	cb_label 	*	debug_section;	/* DEBUG SECTION */
	cb_para_label *	para_label;		/* SECTION Paragraphs */
	struct cb_xref	xref;			/* xref elements */
	cb_tree			exit_label;		/* EXIT label */
	cb_alter_id *	alter_gotos;	/* ALTER ids */
	int				id;				/* Unique id */
	int				section_id;		/* SECTION id */
	int				segment;		/* Segment number */

	unsigned int	flag_section		: 1;	/* Section */
	unsigned int	flag_entry			: 1;	/* Entry */
	unsigned int	flag_begin			: 1;	/* Begin label */
	unsigned int	flag_return			: 1;	/* End label */
	unsigned int	flag_real_label		: 1;	/* Is real label */
	unsigned int	flag_global			: 1;	/* GLOBAL */
	unsigned int	flag_declarative_exit : 1;	/* Final EXIT */
	unsigned int	flag_declaratives	: 1;	/* DECLARATIVES */

	unsigned int	flag_fatal_check	: 1;	/* Fatal check */
	unsigned int	flag_dummy_section	: 1;	/* Dummy MAIN */
	unsigned int	flag_dummy_paragraph : 1;	/* Dummy MAIN */
	unsigned int	flag_dummy_exit		: 1;	/* Dummy EXIT */
	unsigned int	flag_next_sentence	: 1;	/* NEXT SENTENCE */
	unsigned int	flag_default_handler : 1;	/* Error handler */
	unsigned int	flag_statement		: 1;	/* Has statement */
	unsigned int	flag_first_is_goto	: 1;	/* 1st is GO TO */

	unsigned int	flag_alter			: 1;	/* ALTER code */
	unsigned int	flag_debugging_mode	: 1;	/* DEBUGGING MODE */
	unsigned int	flag_is_debug_sect	: 1;	/* DEBUGGING sect */
	unsigned int	flag_skip_label		: 1;	/* Skip label gen */
};

inline cb_label * CB_LABEL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_LABEL, cb_label, x);
}
inline bool CB_LABEL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_LABEL);
}

struct handler_struct {
	cb_label	* handler_label;	/* Handler label */
	cb_program	* handler_prog;		/* Handler program */
};

/* File */

struct cb_alt_key {
	cb_alt_key	* next;			/* Pointer to next */
	cb_tree		key;			/* Key item */
	int			duplicates;		/* DUPLICATES */
	int			offset;			/* Offset from start */
};

struct cb_file : cb_tree_common {
	const char *	name;			/* Original name */
	char 	*		cname;			/* Name used in C */
	/* SELECT */
	cb_tree			assign;			/* ASSIGN */
	cb_tree			file_status;	/* FILE STATUS */
	cb_tree			sharing;		/* SHARING */
	cb_tree			key;			/* RECORD KEY */
	cb_alt_key *	alt_key_list;	/* ALTERNATE RECORD KEY */
	/* FD/SD */
	cb_field 	*	record;			/* Record descriptions */
	cb_tree			record_depending; /* RECORD DEPENDING */
	cb_tree			reports;		/* REPORTS */
	cb_tree			linage;			/* LINAGE */
	cb_tree			linage_ctr;		/* LINAGE COUNTER */
	cb_tree			latfoot;		/* LINAGE FOOTING */
	cb_tree			lattop;			/* LINAGE TOP */
	cb_tree			latbot;			/* LINAGE BOTTOM */
	cb_label 	*	handler;		/* Error handler */
	cb_program *	handler_prog;	/* Prog where defined */
	cb_label 	*	debug_section;	/* DEBUG SECTION */
	struct cb_alphabet_name * code_set;	/* CODE-SET */
	struct cb_list * code_set_items;	/* CODE-SET FOR items */
	struct cb_xref	xref;			/* xref elements */
	int				record_min;		/* RECORD CONTAINS */
	int				record_max;		/* RECORD CONTAINS */
	int				optional;		/* OPTIONAL */
	int				organization;	/* ORGANIZATION */
	int				access_mode;	/* ACCESS MODE */
	cob_flags_t		lock_mode;		/* LOCK MODE */
	int				special;		/* Special file */
	int				same_clause;	/* SAME clause */
	unsigned int	flag_finalized	: 1;	/* Is finalized */
	unsigned int	flag_external	: 1;	/* Is EXTERNAL */
	unsigned int	flag_ext_assign	: 1;	/* ASSIGN EXTERNAL */
	unsigned int	flag_fileid		: 1;	/* ASSIGN DISK */
	unsigned int	flag_global		: 1;	/* Is GLOBAL */
	unsigned int	flag_fl_debug	: 1;	/* DEBUGGING */
	unsigned int	flag_line_adv	: 1;	/* LINE ADVANCING */
};

inline cb_file * CB_FILE(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_FILE, cb_file, x);
}
inline bool CB_FILE_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_FILE);
}

struct cb_cd : cb_tree_common {
	const char *	name;				/* Name */
	cb_field 	*	record;				/* Record descriptions */
	cb_label 	*	debug_section;		/* DEBUG section */
	int				flag_field_debug;	/* DEBUGGING */
};

inline cb_cd * CB_CD(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CD, cb_cd, x);
}
inline bool CB_CD_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CD);
}

/* Reference */

struct cb_word {
	cb_word *	next;		/* Next word with the same hash value */
	const char * name;		/* Word name */
	cb_tree		items;		/* Objects associated with this word */
	int			count;		/* Number of words with the same name */
	int			error;		/* Set to 1 if error detected */
};

#define CB_WORD_TABLE_SIZE	(CB_WORD_HASH_SIZE * sizeof(cb_word))

struct cb_reference : cb_tree_common {
	cb_tree			chain;		/* Next qualified name */
	cb_tree			value;		/* Item referred to */
	cb_tree			subs;		/* List of subscripts */
	cb_tree			offset;		/* Reference mod offset */
	cb_tree			length;		/* Reference mod length */
	cb_tree			check;		/* Runtime checks */
	cb_word 	*	word;		/* Pointer to word list */
	cb_label 	*	section;	/* Current section */
	cb_label 	*	paragraph;	/* Current paragraph */
	cb_label 	*	debug_section;	/* Debug section */
	size_t			hashval;	/* Hash value of name */

	unsigned int	flag_receiving	: 1;	/* Reference target */
	unsigned int	flag_all		: 1;	/* ALL */
	unsigned int	flag_in_decl	: 1;	/* In DECLARATIVE */
	unsigned int	flag_decl_ok	: 1;	/* DECLARATIVE ref OK  */
	unsigned int	flag_alter_code	: 1;	/* Needs ALTER code */
	unsigned int	flag_debug_code	: 1;	/* Needs DEBUG code */
	unsigned int	flag_all_debug	: 1;	/* Needs ALL DEBUG code */
	unsigned int	flag_target		: 1;	/* DEBUG item is target */

	unsigned int	flag_optional	: 1;	/* Definition optional */
	unsigned int	flag_filler_ref	: 1;	/* Ref to FILLER */
	unsigned int	flag_duped		: 1;	/* Duplicate name */
};

inline cb_reference * CB_REFERENCE(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_REFERENCE, cb_reference, x);
}
inline bool CB_REFERENCE_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_REFERENCE);
}

inline bool CB_REF_OR_FIELD_P(cb_tree x)
{
	return (CB_REFERENCE_P(x) || CB_FIELD_P(x));
}
inline cb_field * CB_FIELD_PTR(cb_tree x)
{
	return CB_REFERENCE_P(x) ? CB_FIELD(cb_ref(x)) : CB_FIELD(x);
}

#define CB_NAME(x)		(CB_REFERENCE(x)->word->name)
#define CB_WORD_COUNT(x)	(CB_REFERENCE(x)->word->count)
#define CB_WORD_ITEMS(x)	(CB_REFERENCE(x)->word->items)

/* Binary operation */

/*
  '+'	x + y
  '-'	x - y
  '*'	x * y
  '/'	x / y
  '^'	x ** y
  '='	x = y
  '>'	x > y
  '<'	x < y
  '['	x <= y
  ']'	x >= y
  '~'	x != y
  '!'	not x
  '&'	x and y
  '|'	x or y
  '@'	( x )
*/

struct cb_binary_op : cb_tree_common {
	cb_tree			x;		/* LHS */
	cb_tree			y;		/* RHS */
	int			op;		/* Operation */
	unsigned int		flag;		/* Special usage */
};

inline cb_binary_op * CB_BINARY_OP(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_BINARY_OP, cb_binary_op, x);
}
inline bool CB_BINARY_OP_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_BINARY_OP);
}

/* Function call */

struct cb_funcall : cb_tree_common {
	const char *	name;		/* Function name */
	cb_tree			argv[11];	/* Function arguments */
	int				argc;		/* Number of arguments */
	int				varcnt;		/* Variable argument count */
	unsigned int	screenptr;	/* SCREEN usage */
	unsigned int	nolitcast;	/* No cast for literals */
};

inline cb_funcall * CB_FUNCALL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_FUNCALL, cb_funcall, x);
}
inline bool CB_FUNCALL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_FUNCALL);
}

/* Type cast */

struct cb_cast : cb_tree_common {
	cb_tree			val;
	enum cb_cast_type	cast_type;
};

inline cb_cast * CB_CAST(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CAST, cb_cast, x);
}
inline bool CB_CAST_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CAST);
}

/* Assign */

struct cb_assign : cb_tree_common {
	cb_tree			var;
	cb_tree			val;
};

inline cb_assign * CB_ASSIGN(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_ASSIGN, cb_assign, x);
}
inline bool CB_ASSIGN_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_ASSIGN);
}

/* Intrinsic FUNCTION */

struct cb_intrinsic_table {
	const char 	*	name;			/* FUNCTION NAME */
	const char 	*	intr_routine;	/* Routine name */
	const cb_intr_enum	intr_enum;		/* Enum intrinsic */
	const int			token;			/* Token value */
	const int			implemented;	/* Have we implemented it? */
	const int			args;			/* Maximum number of arguments, -1 = unlimited */
	const int			min_args;		/* Minimum number of arguments */
	const cb_category	category;		/* Category */
	const unsigned int	refmod;			/* Can be refmodded */
};

struct cb_intrinsic : cb_tree_common {
	cb_tree				name;		/* INTRINSIC name */
	cb_tree				args;		/* Arguments */
	cb_tree				intr_field;	/* Field to use */
	cb_intrinsic_table * intr_tab;	/* Table pointer */
	cb_tree				offset;		/* Reference mod */
	cb_tree				length;		/* Reference mod */
	int					isuser;		/* User function */
};

inline cb_intrinsic * CB_INTRINSIC(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_INTRINSIC, cb_intrinsic, x);
}
inline bool CB_INTRINSIC_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_INTRINSIC);
}

/* INITIALIZE */

struct cb_initialize : cb_tree_common {
	cb_tree			var;					/* Field */
	cb_tree			val;					/* Value */
	cb_tree			rep;					/* Replacing */
	unsigned char	flag_default;			/* Default */
	unsigned char	flag_init_statement;	/* INITIALIZE statement */
	unsigned char	flag_no_filler_init;	/* No FILLER initialize */
	unsigned char	padding;				/* Padding */
};

inline cb_initialize * CB_INITIALIZE(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_INITIALIZE, cb_initialize, x);
}
inline bool CB_INITIALIZE_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_INITIALIZE);
}

/* SEARCH */

struct cb_search : cb_tree_common {
	cb_tree		table;		/* Table name */
	cb_tree		var;		/* Varying */
	cb_tree		end_stmt;	/* AT END */
	cb_tree		whens;		/* WHEN */
	int			flag_all;	/* SEARCH ALL */
};

inline cb_search * CB_SEARCH(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_SEARCH, cb_search, x);
}
inline bool CB_SEARCH_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_SEARCH);
}

/* CALL */

struct cb_call : cb_tree_common {
	cb_tree		name;			/* CALL name */
	cb_tree		args;			/* Arguments */
	cb_tree		stmt1;			/* ON EXCEPTION */
	cb_tree		stmt2;			/* NOT ON EXCEPTION */
	cb_tree		call_returning;	/* RETURNING */
	cob_u32_t	is_system;		/* System call */
	int			convention;		/* CALL convention */
};

inline cb_call * CB_CALL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CALL, cb_call, x);
}
inline bool CB_CALL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CALL);
}

/* CANCEL */

struct cb_cancel : cb_tree_common {
	cb_tree			target;		/* CANCEL target(s) */
};

inline cb_cancel * CB_CANCEL(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CANCEL, cb_cancel, x);
}
inline bool CB_CANCEL_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CANCEL);
}

/* ALTER */

struct cb_alter : cb_tree_common {
	cb_tree			source;		/* ALTER source paragraph */
	cb_tree			target;		/* ALTER target GO TO paragraph */
};

inline cb_alter * CB_ALTER(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_ALTER, cb_alter, x);
}
inline bool CB_ALTER_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_ALTER);
}

/* GO TO */

struct cb_goto : cb_tree_common {
	cb_tree			target;		/* Procedure name(s) */
	cb_tree			depending;	/* DEPENDING */
};

inline cb_goto * CB_GOTO(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_GOTO, cb_goto, x);
}
inline bool CB_GOTO_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_GOTO);
}

/* IF */

struct cb_if : cb_tree_common {
	cb_tree			test;		/* Condition */
	cb_tree			stmt1;		/* Statement list */
	cb_tree			stmt2;		/* ELSE/WHEN statement list */
	unsigned int	is_if;		/* From IF */
};

inline cb_if * CB_IF(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_IF, cb_if, x);
}
inline bool CB_IF_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_IF);
}

/* PERFORM */

struct cb_perform_varying : cb_tree_common {
	cb_tree			name;		/* VARYING item */
	cb_tree			from;		/* FROM */
	cb_tree			step;		/* Increment */
	cb_tree			until;		/* UNTIL */
};

struct cb_perform : cb_tree_common {
	cb_tree			test;		/* Condition */
	cb_tree			body;		/* Statements */
	cb_tree			data;		/* TIMES or procedure */
	cb_tree			varying;	/* VARYING */
	cb_tree			exit_label;	/* Implicit exit label */
	cb_tree			cycle_label;	/* EXIT PERFORM CYCLE */
	enum cb_perform_type perform_type;	/* Perform type */
};

inline cb_perform_varying * CB_PERFORM_VARYING(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_PERFORM_VARYING, cb_perform_varying, x);
}

inline cb_perform * CB_PERFORM(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_PERFORM, cb_perform, x);
}
inline bool CB_PERFORM_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_PERFORM);
}

/* Struct for extended ACCEPT / DISPLAY */

struct cb_attr_struct {
	cb_tree		fgc;		/* FOREGROUND COLOR */
	cb_tree		bgc;		/* BACKGROUND COLOR */
	cb_tree		scroll;		/* SCROLL */
	cb_tree		timeout;	/* TIMEOUT */
	cb_tree		prompt;		/* PROMPT */
	cb_tree		size_is;	/* [PROTECTED] SIZE [IS] */
	cob_flags_t	dispattrs;	/* Attributes */
};

/* Exception handler type */

enum cb_handler_type {
	NO_HANDLER = 0,
	DISPLAY_HANDLER,
	ACCEPT_HANDLER,
	SIZE_ERROR_HANDLER,
	OVERFLOW_HANDLER,
	AT_END_HANDLER,
	EOP_HANDLER,
	INVALID_KEY_HANDLER
};

/* Statement */

struct cb_statement : cb_tree_common {
	const char *	name;			/* Statement name */
	cb_tree			body;			/* Statement body */
	cb_tree			file;			/* File reference */
	cb_tree			ex_handler;		/* Exception handler */
	cb_tree			not_ex_handler;	/* Exception handler */
	cb_tree			handler3;		/* INTO clause */
	cb_tree			null_check;		/* NULL check */
	cb_tree			debug_check;	/* Field DEBUG */
	cb_tree			debug_nodups;	/* Field DEBUG dups */
	cb_attr_struct * attr_ptr;		/* Attributes */
	cb_handler_type	handler_type;	/* Handler type */
	unsigned int	flag_no_based	: 1;	/* Check BASED */
	unsigned int	flag_in_debug	: 1;	/* In DEBUGGING */
	unsigned int	flag_merge		: 1;	/* Is MERGE */
	unsigned int	flag_callback	: 1;	/* DEBUG Callback */
};

inline cb_statement * CB_STATEMENT(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_STATEMENT, cb_statement, x);
}
inline bool CB_STATEMENT_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_STATEMENT);
}

/* CONTINUE */

struct cb_continue : cb_tree_common {
};

inline cb_continue * CB_CONTINUE(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_CONTINUE, cb_continue, x);
}
inline bool CB_CONTINUE_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_CONTINUE);
}

/* SET ATTRIBUTE */

struct cb_set_attr : cb_tree_common {
	cb_field	*	fld;
	cob_flags_t		val_on;
	cob_flags_t		val_off;
};

inline cb_set_attr * CB_SET_ATTR(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_SET_ATTR, cb_set_attr, x);
}
inline bool CB_SET_ATTR_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_SET_ATTR);
}

/* List */

struct cb_list : cb_tree_common {
	cb_tree		chain;		/* Next in list */
	cb_tree		value;		/* Reference to item(s) */
	cb_tree		purpose;	/* Purpose */
	int			sizes;		/* BY VALUE SIZE */
};

inline cb_list * CB_LIST(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_LIST, cb_list, x);
}
inline bool CB_LIST_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_LIST);
}

#define CB_PURPOSE(x)			(CB_LIST(x)->purpose)
#define CB_VALUE(x)			(CB_LIST(x)->value)
#define CB_CHAIN(x)			(CB_LIST(x)->chain)
#define CB_SIZES(x)			(CB_LIST(x)->sizes)

#define CB_PURPOSE_INT(x)		(CB_INTEGER(CB_PURPOSE(x))->val)

#define CB_SIZES_INT(x)			((CB_LIST(x)->sizes) & 0x07)
#define CB_SIZES_INT_UNSIGNED(x)	((CB_LIST(x)->sizes) & CB_SIZE_UNSIGNED)

/* Pair */

#define CB_PAIR_P(x)		(CB_LIST_P(x) && CB_PAIR_X(x))
#define CB_PAIR_X(x)		CB_PURPOSE(x)
#define CB_PAIR_Y(x)		CB_VALUE(x)

/* Report */

struct cb_report : cb_tree_common {
	const char *	name;			/* Original name */
	char 	*		cname;			/* Name used in C */
	cb_file 	*	file;			/* File */
	cb_tree			line_counter;	/* LINE-COUNTER */
	cb_tree			page_counter;	/* PAGE-COUNTER */
	cb_tree			code_clause;	/* CODE */
	cb_tree			controls;		/* CONTROLS */
	int				lines;			/* PAGE LIMIT LINES */
	int				columns;		/* PAGE LIMIT COLUMNS */
	int				heading;		/* HEADING */
	int				first_detail;	/* FIRST DE */
	int				last_control;	/* LAST CH */
	int				last_detail;	/* LAST DE */
	int				footing;		/* FOOTING */
};

inline cb_report * CB_REPORT(cb_tree x)
{
	return CB_TREE_CAST(CB_TAG_REPORT, cb_report, x);
}
inline bool CB_REPORT_P(cb_tree x)
{
	return (CB_TREE_TAG(x) == CB_TAG_REPORT);
}

/* Program */

struct nested_list {
	nested_list	* next;
	cb_program	* nested_prog;
};

struct cb_program : cb_tree_common {
	/* Program variables */
	cb_program *	next_program;		/* Nested/contained */
	const char *	program_name;		/* Internal program-name */
	const char *	program_id;			/* Demangled PROGRAM-ID */
	char 	*		source_name;		/* Source name */
	char 	*		orig_program_id;	/* Original PROGRAM-ID */
	cb_word 	**	word_table;			/* Name hash table */
	local_filename * local_include;		/* Local include info */
	nested_list *	nested_prog_list;	/* Callable contained */
	nested_list *	common_prog_list;	/* COMMON contained */
	cb_tree			entry_list;			/* Entry point list */
	cb_tree			file_list;			/* File list */
	cb_tree			cd_list;			/* CD list */
	cb_tree			exec_list;			/* Executable statements */
	cb_tree			label_list;			/* Label list */
	cb_tree			reference_list;		/* Reference list */
	cb_tree			alphabet_name_list;	/* ALPHABET list */
	cb_tree			symbolic_char_list;	/* SYMBOLIC list */
	cb_tree			class_name_list;	/* CLASS list */
	cb_tree			parameter_list;		/* USING parameters */
	cb_tree			locale_list;		/* LOCALE list */
	cb_tree			global_list;		/* GLOBAL list */
	cb_tree			report_list;		/* REPORT list */
	cb_tree			alter_list;			/* ALTER list */
	cb_tree			debug_list;			/* DEBUG ref list */
	cb_tree			cb_return_code;		/* RETURN-CODE */
	cb_tree			cb_sort_return;		/* SORT-RETURN */
	cb_tree			cb_call_params;		/* Number of CALL params */
	cb_tree			mnemonic_spec_list;	/* MNEMONIC spec */
	cb_tree			class_spec_list;	/* CLASS spec */
	cb_tree			interface_spec_list;/* INTERFACE spec */
	cb_tree			function_spec_list;	/* FUNCTION spec */
	cb_tree			user_spec_list;		/* User FUNCTION spec */
	cb_tree			program_spec_list;	/* PROGRAM spec */
	cb_tree			property_spec_list;	/* PROPERTY spec */
	cb_alter_id *	alter_gotos;		/* ALTER ids */
	cb_field 	*	working_storage;	/* WORKING-STORAGE */
	cb_field 	*	local_storage;		/* LOCAL-STORAGE */
	cb_field 	*	linkage_storage;	/* LINKAGE */
	cb_field 	*	screen_storage;		/* SCREEN */
	cb_field 	*	report_storage;		/* REPORT */
	cb_tree			local_file_list;	/* Local files */
	cb_tree			global_file_list;	/* Global files */
	handler_struct	global_handler[5];	/* Global handlers */
	cb_tree			collating_sequence;	/* COLLATING */
	cb_tree			classification;		/* CLASSIFICATION */
	cb_tree			cursor_pos;			/* CURSOR */
	cb_tree			crt_status;			/* CRT STATUS */
	cb_tree			returning;			/* RETURNING */
	cb_label 	*	all_procedure;		/* DEBUGGING */

	/* Internal variables */
	int				loop_counter;		/* Loop counters */
	int				decimal_index;		/* cob_decimal count */
	int				decimal_index_max;	/* cob_decimal max */
	int				nested_level;		/* Nested program level */
	int				num_proc_params;	/* PROC DIV params */
	int				toplev_count;		/* Top level source count */
	int				max_call_param;		/* Max params */

	unsigned char	decimal_point;		/* '.' or ',' */
	unsigned char	currency_symbol;	/* '$' or user-specified */
	unsigned char	numeric_separator;	/* ',' or '.' */
	unsigned char	prog_type;			/* Program type */
	cb_tree			entry_convention;	/* ENTRY convention / PROCEDURE convention */

	unsigned int	flag_main			: 1;	/* Gen main function */
	unsigned int	flag_common			: 1;	/* COMMON PROGRAM */
	unsigned int	flag_initial		: 1;	/* INITIAL PROGRAM */
	unsigned int	flag_recursive		: 1;	/* RECURSIVE PROGRAM */
	unsigned int	flag_screen			: 1;	/* Have SCREEN SECTION */
	unsigned int	flag_validated		: 1;	/* End program validate */
	unsigned int	flag_chained		: 1;	/* PROCEDURE CHAINING */
	unsigned int	flag_global_use		: 1;	/* USE GLOBAL */

	unsigned int	flag_gen_error		: 1;	/* Gen error routine */
	unsigned int	flag_file_global	: 1;	/* Global FD */
	unsigned int	flag_has_external	: 1;	/* Has EXTERNAL */
	unsigned int	flag_segments		: 1;	/* Has segments */
	unsigned int	flag_trailing_separate : 1;	/* TRAILING SEPARATE */
	unsigned int	flag_console_is_crt	: 1;	/* CONSOLE IS CRT */
	unsigned int	flag_debugging		: 1;	/* DEBUGGING MODE */
	unsigned int	flag_gen_debug		: 1;	/* DEBUGGING MODE */

	unsigned int	flag_save_exception	: 1;	/* Save execption code */
	unsigned int	flag_report			: 1;	/* Have REPORT SECTION */
	unsigned int	flag_void			: 1;	/* void return for subprogram */
};

#define CB_PROGRAM(x)	(CB_TREE_CAST (CB_TAG_PROGRAM, struct cb_program, x))

/* Function prototype */

struct cb_prototype : cb_tree_common {
	/* Name of prototype in the REPOSITORY */
	const char	*	name;
	/* External name of the prototype/definition */
	const char	*	ext_name;
	int		        type;
};

#define CB_PROTOTYPE(x)		(CB_TREE_CAST (CB_TAG_PROTOTYPE, struct cb_prototype, x))
#define CB_PROTOTYPE_P(x)	(CB_TREE_TAG (x) == CB_TAG_PROTOTYPE)

/* DISPLAY type */

enum cb_display_type {
	UNKNOWN_DISPLAY,
	SCREEN_DISPLAY,
	FIELD_ON_SCREEN_DISPLAY,
	DEVICE_DISPLAY,
	MIXED_DISPLAY
};

/* INSPECT clauses */

enum cb_inspect_clause {
	TALLYING_CLAUSE,
	REPLACING_CLAUSE,
	CONVERTING_CLAUSE,
	/* This is what happens when you support OS/VS COBOL. */
	TRANSFORM_STATEMENT
};

/* Functions/variables */

/* tree.c */

extern cb_tree		cb_any;
extern cb_tree		cb_true;
extern cb_tree		cb_false;
extern cb_tree		cb_null;
extern cb_tree		cb_zero;
extern cb_tree		cb_one;
extern cb_tree		cb_space;
extern cb_tree		cb_low;
extern cb_tree		cb_high;
extern cb_tree		cb_norm_low;
extern cb_tree		cb_norm_high;
extern cb_tree		cb_quote;
extern cb_tree		cb_int0;
extern cb_tree		cb_int1;
extern cb_tree		cb_int2;
extern cb_tree		cb_int3;
extern cb_tree		cb_int4;
extern cb_tree		cb_int5;
extern cb_tree		cb_i[COB_MAX_SUBSCRIPTS];
extern cb_tree		cb_error_node;

extern cb_tree		cb_intr_whencomp;

extern cb_tree		cb_standard_error_handler;
extern cb_tree		cb_depend_check;

extern unsigned int	gen_screen_ptr;

int			cb_tree_type(const cb_tree, const cb_field *);
int			cb_category_is_alpha(cb_tree);
int			cb_fits_int(const cb_tree);
int			cb_fits_long_long(const cb_tree);
int			cb_get_int(const cb_tree);
cob_s64_t	cb_get_long_long(const cb_tree);
cob_u64_t	cb_get_u_long_long(const cb_tree);

void		cb_init_constants(void);

cb_tree		cb_int(const int);
cb_tree		cb_int_hex(const int);

cb_tree		cb_build_string(const void *, const size_t);

cb_tree		cb_flags_t(const cob_flags_t);

cb_tree		cb_build_class_name(cb_tree, cb_tree);

cb_tree		cb_build_locale_name(cb_tree, cb_tree);

cb_tree		cb_build_numeric_literal(const int, const void *, const int);
cb_tree		cb_build_alphanumeric_literal(const void *, const size_t);
cb_tree		cb_build_national_literal(const void *, const size_t);
cb_tree		cb_build_numsize_literal(const void *, const size_t, const int);
cb_tree		cb_concat_literals(const cb_tree, const cb_tree);

cb_tree		cb_build_decimal(const int);

cb_tree		cb_build_picture(const char *);
cb_tree		cb_build_comment(const char *);
cb_tree		cb_build_direct(const char *, const unsigned int);
cb_tree		cb_build_debug(const cb_tree, const char *, const cb_tree);
cb_tree		cb_build_debug_call(cb_label *);

cb_picture * cb_build_binary_picture(const char *, const cob_u32_t, const cob_u32_t);

cb_tree		cb_build_field(cb_tree);
cb_tree		cb_build_implicit_field(cb_tree, const int);
cb_tree		cb_build_constant(cb_tree, cb_tree);

void		cb_build_symbolic_chars(const cb_tree, const cb_tree);

cb_field *	cb_field_add(cb_field *, cb_field *);
cb_field *	cb_field_founder(const cb_field *);
cb_field *	cb_field_variable_size(const cb_field *);
unsigned int cb_field_variable_address(const cb_field *);
int			cb_field_subordinate(const cb_field *, const cb_field *);

cb_tree		cb_build_label(cb_tree, cb_label *);

cb_file *	build_file(cb_tree);
void		validate_file(cb_file *, cb_tree);
void		finalize_file(cb_file *, cb_field *);
cb_cd 	*	cb_build_cd(cb_tree name);
void		cb_finalize_cd(cb_cd *, cb_field *);

cb_tree		cb_build_filler(void);
cb_tree		cb_build_reference(const char *);
cb_tree		cb_build_field_reference(cb_field *, cb_tree);
const char * cb_define(cb_tree, cb_tree);
char 	*	cb_to_cname(const char *);
void		cb_set_system_names(void);

cb_tree		cb_build_binary_op(cb_tree, const int, cb_tree);
cb_tree		cb_build_binary_list(cb_tree, const int);

cb_tree		cb_build_funcall(const char *, const int, const cb_tree, const cb_tree, const cb_tree, const cb_tree,
							 const cb_tree, const cb_tree, const cb_tree, const cb_tree, const cb_tree, const cb_tree, const cb_tree);

cb_tree		cb_build_cast(const enum cb_cast_type, const cb_tree);
cb_tree		cb_build_cast_int(const cb_tree);
cb_tree		cb_build_cast_llint(const cb_tree);

cb_tree		cb_build_assign(const cb_tree, const cb_tree);

cb_tree		cb_build_intrinsic(cb_tree, cb_tree, cb_tree, const int);
cb_tree		cb_build_prototype(const cb_tree, const cb_tree, const int);
cb_tree		cb_build_any_intrinsic(cb_tree);

cb_tree		cb_build_search(const int, const cb_tree, const cb_tree, const cb_tree, const cb_tree);

cb_tree		cb_build_call(const cb_tree, const cb_tree, const cb_tree, const cb_tree, const cb_tree, const cob_u32_t, const int);

cb_tree		cb_build_alter(const cb_tree, const cb_tree);

cb_tree		cb_build_cancel(const cb_tree);

cb_tree		cb_build_goto(const cb_tree, const cb_tree);

cb_tree		cb_build_if(const cb_tree, const cb_tree, const cb_tree, const unsigned int);

cb_tree		cb_build_perform(const enum cb_perform_type);
cb_tree		cb_build_perform_varying(cb_tree, cb_tree, cb_tree, cb_tree);

cb_statement * cb_build_statement(const char *);

cb_tree		cb_build_continue(void);

cb_tree		cb_build_list(cb_tree, cb_tree, cb_tree);
cb_tree		cb_list_add(cb_tree, cb_tree);
cb_tree		cb_pair_add(cb_tree, cb_tree, cb_tree);
cb_tree		cb_list_append(cb_tree, cb_tree);
cb_tree		cb_list_reverse(cb_tree);
int			cb_list_length(cb_tree);

cb_report *	build_report(cb_tree);

void		cb_add_common_prog(cb_program *);
void		cb_insert_common_prog(cb_program *, cb_program *);


cb_intrinsic_table * lookup_intrinsic(const char *, const int);

cb_tree		cb_build_alphabet_name(cb_tree);

cb_tree		cb_build_initialize(const cb_tree, const cb_tree, const cb_tree, const unsigned int, const unsigned int, const unsigned int);

cb_literal * build_literal(const enum cb_category, const void *, const size_t);

cb_tree		cb_build_system_name(const enum cb_system_name_category, const int);

const char * cb_get_usage_string(const enum cb_usage);

/* parser.y */
extern cb_tree		cobc_printer_node;
extern int			non_const_word;
extern int			suppress_data_exceptions;
extern unsigned int	cobc_repeat_last_token;
extern unsigned int	cobc_in_id;
extern unsigned int	cobc_in_procedure;
extern unsigned int	cobc_in_repository;
extern unsigned int	cobc_force_literal;
extern unsigned int	cobc_cs_check;
extern unsigned int	cobc_allow_program_name;

/* reserved.c */
int			is_reserved_word(const char *);
int			is_default_reserved_word(const char *);
void		remove_context_sensitivity(const char *, const int);
cobc_reserved * lookup_reserved_word(const char *);
cb_tree		lookup_system_name(const char *);
void		cb_list_reserved(void);
void		cb_list_intrinsics(void);
void		cb_list_mnemonics(void);
void		cb_list_system(void);
void		cb_list_map(cb_tree(*)(cb_tree), cb_tree);

/* error.c */
void		cb_warning_x(const cb_tree, const char *, ...) COB_A_FORMAT23;
void		cb_error_x(const cb_tree, const char *, ...) COB_A_FORMAT23;
unsigned int cb_verify_x(cb_tree, const enum cb_support, const char *);
void		listprint_suppress(void);
void		listprint_restore(void);

void		redefinition_error(cb_tree);
void		redefinition_warning(cb_tree, cb_tree);
void		undefined_error(cb_tree);
void		ambiguous_error(cb_tree);
void		group_error(cb_tree, const char *);
void		level_redundant_error(cb_tree, const char *);
void		level_require_error(cb_tree, const char *);
void		level_except_error(cb_tree, const char *);

/* field.c */
extern size_t	cb_needs_01;
int			cb_get_level(cb_tree);
cb_tree		cb_build_field_tree(cb_tree, cb_tree, cb_field *, enum cb_storage, cb_file *, const int);
cb_field *	cb_resolve_redefines(cb_field *, cb_tree);
void		cb_validate_field(cb_field *);
void		cb_validate_88_item(cb_field *);
cb_field *	cb_validate_78_item(cb_field *, const cob_u32_t);
void		cb_validate_renames_item(cb_field *);
cb_field *	cb_get_real_field(void);
void		cb_clear_real_field(void);

/* typeck.c */
extern cb_tree	cb_debug_item;
extern cb_tree	cb_debug_line;
extern cb_tree	cb_debug_name;
extern cb_tree	cb_debug_sub_1;
extern cb_tree	cb_debug_sub_2;
extern cb_tree	cb_debug_sub_3;
extern cb_tree	cb_debug_contents;

cb_program * cb_build_program(cb_program *, const int);
cb_tree		cb_check_numeric_value(cb_tree);
void		cb_build_registers(void);
void		cb_build_debug_item(void);
void		cb_check_field_debug(cb_tree);
void		cb_trim_program_id(cb_tree);
char 	*	cb_encode_program_id(const char *);
char 	*	cb_build_program_id(cb_tree, cb_tree, const cob_u32_t);
cb_tree		cb_define_switch_name(cb_tree, cb_tree, const int);
cb_tree		cb_build_section_name(cb_tree, const int);
cb_tree		cb_build_assignment_name(cb_file *, cb_tree);
cb_tree		cb_build_index(cb_tree, cb_tree, const unsigned int, cb_field *);
cb_tree		cb_build_identifier(cb_tree, const int);
cb_tree		cb_build_length(cb_tree);
cb_tree		cb_build_const_length(cb_tree);
cb_tree		cb_build_address(cb_tree);
cb_tree		cb_build_ppointer(cb_tree);
void		cb_validate_program_environment(cb_program *);
void		cb_validate_program_data(cb_program *);
void		cb_validate_program_body(cb_program *);
cb_tree		cb_build_expr(cb_tree);
cb_tree		cb_build_cond(cb_tree);
void		cb_emit_arithmetic(cb_tree, const int, cb_tree);
cb_tree		cb_build_add(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_sub(cb_tree, cb_tree, cb_tree);
void		cb_emit_corresponding(cb_tree(*)(cb_tree, cb_tree, cb_tree), cb_tree, cb_tree, cb_tree);
void		cb_emit_tab_arithmetic(cb_tree(*)(cb_tree, cb_tree, cb_tree),
								   cb_tree, cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_move_corresponding(cb_tree, cb_tree);
void		cb_emit_accept(cb_tree, cb_tree, cb_attr_struct *);
void		cb_emit_accept_line_or_col(cb_tree, const int);
void		cb_emit_accept_escape_key(cb_tree);
void		cb_emit_accept_exception_status(cb_tree);
void		cb_emit_accept_user_name(cb_tree);
void		cb_emit_accept_date(cb_tree);
void		cb_emit_accept_date_yyyymmdd(cb_tree);
void		cb_emit_accept_day(cb_tree);
void		cb_emit_accept_day_yyyyddd(cb_tree);
void		cb_emit_accept_day_of_week(cb_tree);
void		cb_emit_accept_time(cb_tree);
void		cb_emit_accept_command_line(cb_tree);
void		cb_emit_accept_environment(cb_tree);
void		cb_emit_accept_mnemonic(cb_tree, cb_tree);
void		cb_emit_accept_name(cb_tree, cb_tree);
void		cb_emit_accept_arg_number(cb_tree);
void		cb_emit_accept_arg_value(cb_tree);
void		cb_emit_get_environment(cb_tree, cb_tree);
void		cb_emit_allocate(cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_alter(cb_tree, cb_tree);
void		cb_emit_free(cb_tree);
void		cb_emit_call(cb_tree, cb_tree, cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_cancel(cb_tree);
void		cb_emit_close(cb_tree, cb_tree);
void		cb_emit_commit(void);
void		cb_emit_continue(void);
void		cb_emit_delete(cb_tree);
void		cb_emit_delete_file(cb_tree);
void		cb_emit_display(cb_tree, cb_tree, cb_tree, cb_tree, cb_attr_struct *, int, enum cb_display_type);
cb_tree		cb_build_display_mnemonic(cb_tree);
cb_tree		cb_build_display_name(cb_tree);
void		cb_emit_env_name(cb_tree);
void		cb_emit_env_value(cb_tree);
void		cb_emit_arg_number(cb_tree);
void		cb_emit_command_line(cb_tree);
void		cb_emit_divide(cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_evaluate(cb_tree, cb_tree);
void		cb_emit_goto(cb_tree, cb_tree);
void		cb_emit_exit(const unsigned int);
void		cb_emit_if(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_if_check_break(cb_tree, cb_tree);
void		cb_emit_initialize(cb_tree, cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_inspect(cb_tree, cb_tree, const enum cb_inspect_clause);
void		cb_init_tallying(void);
cb_tree		cb_build_tallying_data(cb_tree);
cb_tree		cb_build_tallying_characters(cb_tree);
cb_tree		cb_build_tallying_all(void);
cb_tree		cb_build_tallying_leading(void);
cb_tree		cb_build_tallying_trailing(void);
cb_tree		cb_build_tallying_value(cb_tree, cb_tree);
cb_tree		cb_build_replacing_characters(cb_tree, cb_tree);
cb_tree		cb_build_replacing_all(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_replacing_leading(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_replacing_first(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_replacing_trailing(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_converting(cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_inspect_region_start(void);
int			validate_move(cb_tree, cb_tree, bool);
cb_tree		cb_build_move(cb_tree, cb_tree);
void		cb_emit_move(cb_tree, cb_tree);
void		cb_emit_open(cb_tree, cb_tree, cb_tree);
void		cb_emit_perform(cb_tree, cb_tree);
cb_tree		cb_build_perform_once(cb_tree);
cb_tree		cb_build_perform_times(cb_tree);
cb_tree		cb_build_perform_until(cb_tree, cb_tree);
cb_tree		cb_build_perform_forever(cb_tree);
cb_tree		cb_build_perform_exit(cb_label *);
void		cb_emit_read(cb_tree, cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_ready_trace(void);
void		cb_emit_rewrite(cb_tree, cb_tree, cb_tree);
void		cb_emit_release(cb_tree, cb_tree);
void		cb_emit_reset_trace(void);
void		cb_emit_return(cb_tree, cb_tree);
void		cb_emit_rollback(void);
void		cb_emit_search(cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_search_all(cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_setenv(cb_tree, cb_tree);
void		cb_emit_set_to(cb_tree, cb_tree);
void		cb_emit_set_up_down(cb_tree, cb_tree, cb_tree);
void		cb_emit_set_on_off(cb_tree, cb_tree);
void		cb_emit_set_true(cb_tree);
void		cb_emit_set_false(cb_tree);
void		cb_emit_set_attribute(cb_tree, const cob_flags_t, const cob_flags_t);
cb_tree		cb_build_set_attribute(const cb_field *, const cob_flags_t, const cob_flags_t);
void		cb_emit_set_last_exception_to_off(void);
void		cb_emit_sort_init(cb_tree, cb_tree, cb_tree);
void		cb_emit_sort_using(cb_tree, cb_tree);
void		cb_emit_sort_input(cb_tree);
void		cb_emit_sort_giving(cb_tree, cb_tree);
void		cb_emit_sort_output(cb_tree);
void		cb_emit_sort_finish(cb_tree);
void		cb_emit_start(cb_tree, cb_tree, cb_tree, cb_tree);
void		cb_emit_stop_run(cb_tree);
void		cb_emit_string(cb_tree, cb_tree, cb_tree);
void		cb_emit_unlock(cb_tree);
void		cb_emit_unstring(cb_tree, cb_tree, cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_unstring_delimited(cb_tree, cb_tree);
cb_tree		cb_build_unstring_into(cb_tree, cb_tree, cb_tree);
void		cb_emit_write(cb_tree, cb_tree, cb_tree, cb_tree);
cb_tree		cb_build_write_advancing_lines(cb_tree, cb_tree);
cb_tree		cb_build_write_advancing_mnemonic(cb_tree, cb_tree);
cb_tree		cb_build_write_advancing_page(cb_tree);

DECLNORET void cobc_tree_cast_error(const cb_tree, const char *, const int, const enum cb_tag) COB_A_NORETURN;

#if	!defined(__GNUC__) && defined(COB_TREE_DEBUG)
	cb_tree		cobc_tree_cast_check(const cb_tree, const char *, const int, const enum cb_tag);
#endif


/* codegen.c */
void		codegen(cb_program *, const int);

/* scanner.l */
void		cb_unput_dot(void);
void		cb_add_78(cb_field *);
void		cb_reset_78(void);
void		cb_reset_global_78(void);
cb_field *	check_level_78(const char *);

cb_program * cb_find_defined_program_by_name(const char *);
cb_program * cb_find_defined_program_by_id(const char *);

/* cobc.c */
void		cobc_xref_link(cb_xref *, const int, const int);
void		cobc_xref_link_parent(const cb_field *);
void		cobc_xref_set_receiving(const cb_tree);

/* Function defines */

#define CB_BUILD_FUNCALL_0(f)								\
	cb_build_funcall(f, 0, NULL, NULL, NULL, NULL, NULL,	\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_1(f,a1)						\
	cb_build_funcall(f, 1, a1, NULL, NULL, NULL, NULL,	\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_2(f,a1,a2)						\
	cb_build_funcall(f, 2, a1, a2, NULL, NULL, NULL,	\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_3(f,a1,a2,a3)					\
	cb_build_funcall(f, 3, a1, a2, a3, NULL, NULL,		\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_4(f,a1,a2,a3,a4)				\
	cb_build_funcall(f, 4, a1, a2, a3, a4, NULL,		\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_5(f,a1,a2,a3,a4,a5)			\
	cb_build_funcall(f, 5, a1, a2, a3, a4, a5,			\
					 NULL, NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_6(f,a1,a2,a3,a4,a5,a6)			\
	cb_build_funcall(f, 6, a1, a2, a3, a4, a5, a6,		\
					 NULL, NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_7(f,a1,a2,a3,a4,a5,a6,a7)		\
	cb_build_funcall(f, 7, a1, a2, a3, a4, a5, a6, a7,	\
					 NULL, NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_8(f,a1,a2,a3,a4,a5,a6,a7,a8)		\
	cb_build_funcall(f, 8, a1, a2, a3, a4, a5, a6, a7, a8,	\
					 NULL, NULL, NULL)

#define CB_BUILD_FUNCALL_9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)	\
	cb_build_funcall(f, 9, a1, a2, a3, a4, a5, a6, a7, a8,	\
					 a9, NULL, NULL)

#define CB_BUILD_FUNCALL_10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)		\
	cb_build_funcall(f, 10, a1, a2, a3, a4, a5, a6, a7, a8,	\
					 a9, a10, NULL)

#define CB_BUILD_FUNCALL_11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)	\
	cb_build_funcall(f, 11, a1, a2, a3, a4, a5, a6, a7, a8,	\
					 a9, a10, a11)

/* Miscellanous defines */

#define CB_BUILD_CAST_ADDRESS(x)	cb_build_cast(CB_CAST_ADDRESS, x)
#define CB_BUILD_CAST_ADDR_OF_ADDR(x)	cb_build_cast(CB_CAST_ADDR_OF_ADDR, x)
#define CB_BUILD_CAST_LENGTH(x)		cb_build_cast(CB_CAST_LENGTH, x)
#define CB_BUILD_CAST_PPOINTER(x)	cb_build_cast(CB_CAST_PROGRAM_POINTER, x)

#define CB_BUILD_PARENTHESES(x)	cb_build_binary_op (x, '@', NULL)
#define CB_BUILD_NEGATION(x)	cb_build_binary_op(x, '!', NULL)

#define CB_BUILD_STRING0(str)	cb_build_string(str, strlen((char *)(str)))

#define CB_LIST_INIT(x)			cb_build_list(NULL, x, NULL)
#define CB_BUILD_CHAIN(x,y)		cb_build_list(NULL, x, y)
#define CB_BUILD_PAIR(x,y)		cb_build_list(x, y, NULL)
#define CB_ADD_TO_CHAIN(x,y)	y = CB_BUILD_CHAIN(x, y)
#define CB_CHAIN_PAIR(x,y,z)	x = cb_pair_add(x, y, z)
#define CB_FIELD_ADD(x,y)		x = cb_field_add(x, y)


#endif /* CB_TREE_H */
