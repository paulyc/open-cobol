/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_EOF = 0,
     ACCEPT = 258,
     ACCESS = 259,
     ADD = 260,
     ADDRESS = 261,
     ADVANCING = 262,
     AFTER = 263,
     ALL = 264,
     ALLOCATE = 265,
     ALPHABET = 266,
     ALPHABETIC = 267,
     ALPHABETIC_LOWER = 268,
     ALPHABETIC_UPPER = 269,
     ALPHANUMERIC = 270,
     ALPHANUMERIC_EDITED = 271,
     ALSO = 272,
     ALTER = 273,
     ALTERNATE = 274,
     AND = 275,
     ANY = 276,
     ARE = 277,
     AREA = 278,
     AREAS = 279,
     ARGUMENT_NUMBER = 280,
     ARGUMENT_VALUE = 281,
     AS = 282,
     ASCENDING = 283,
     ASCII = 284,
     ASSIGN = 285,
     AT = 286,
     ATTRIBUTE = 287,
     AUTO = 288,
     AUTOMATIC = 289,
     AWAY_FROM_ZERO = 290,
     BACKGROUND_COLOR = 291,
     BASED = 292,
     BEFORE = 293,
     BELL = 294,
     BINARY = 295,
     BINARY_C_LONG = 296,
     BINARY_CHAR = 297,
     BINARY_DOUBLE = 298,
     BINARY_LONG = 299,
     BINARY_SHORT = 300,
     BLANK = 301,
     BLINK = 302,
     BLOCK = 303,
     BOTTOM = 304,
     BY = 305,
     BYTE_LENGTH = 306,
     CALL = 307,
     CANCEL = 308,
     CAPACITY = 309,
     CD = 310,
     CF = 311,
     CH = 312,
     CHAINING = 313,
     CHARACTER = 314,
     CHARACTERS = 315,
     CLASS = 316,
     CLASSIFICATION = 317,
     CLASS_NAME = 318,
     CLOSE = 319,
     CODE = 320,
     CODE_SET = 321,
     COLLATING = 322,
     COL = 323,
     COLS = 324,
     COLUMN = 325,
     COLUMNS = 326,
     COMMA = 327,
     COMMAND_LINE = 328,
     COMMA_DELIM = 329,
     COMMIT = 330,
     COMMON = 331,
     COMMUNICATION = 332,
     COMP = 333,
     COMPUTE = 334,
     COMP_1 = 335,
     COMP_2 = 336,
     COMP_3 = 337,
     COMP_4 = 338,
     COMP_5 = 339,
     COMP_6 = 340,
     COMP_X = 341,
     CONCATENATE_FUNC = 342,
     CONDITION = 343,
     CONFIGURATION = 344,
     CONSTANT = 345,
     CONTAINS = 346,
     CONTENT = 347,
     CONTINUE = 348,
     CONTROL = 349,
     CONTROLS = 350,
     CONVERSION = 351,
     CONVERTING = 352,
     COPY = 353,
     CORRESPONDING = 354,
     COUNT = 355,
     CRT = 356,
     CRT_UNDER = 357,
     CURRENCY = 358,
     CURRENT_DATE_FUNC = 359,
     CURSOR = 360,
     CYCLE = 361,
     DATA = 362,
     DATE = 363,
     DAY = 364,
     DAY_OF_WEEK = 365,
     DE = 366,
     DEBUGGING = 367,
     DECIMAL_POINT = 368,
     DECLARATIVES = 369,
     DEFAULT = 370,
     DELETE = 371,
     DELIMITED = 372,
     DELIMITER = 373,
     DEPENDING = 374,
     DESCENDING = 375,
     DETAIL = 376,
     DISC = 377,
     DISK = 378,
     DISPLAY = 379,
     DISPLAY_OF_FUNC = 380,
     DIVIDE = 381,
     DIVISION = 382,
     DOWN = 383,
     DUPLICATES = 384,
     DYNAMIC = 385,
     EBCDIC = 386,
     EC = 387,
     ECHO = 388,
     EIGHTY_EIGHT = 389,
     ELSE = 390,
     END = 391,
     END_ACCEPT = 392,
     END_ADD = 393,
     END_CALL = 394,
     END_COMPUTE = 395,
     END_DELETE = 396,
     END_DISPLAY = 397,
     END_DIVIDE = 398,
     END_EVALUATE = 399,
     END_FUNCTION = 400,
     END_IF = 401,
     END_MULTIPLY = 402,
     END_PERFORM = 403,
     END_PROGRAM = 404,
     END_READ = 405,
     END_RETURN = 406,
     END_REWRITE = 407,
     END_SEARCH = 408,
     END_START = 409,
     END_STRING = 410,
     END_SUBTRACT = 411,
     END_UNSTRING = 412,
     END_WRITE = 413,
     ENTRY = 414,
     ENVIRONMENT = 415,
     ENVIRONMENT_NAME = 416,
     ENVIRONMENT_VALUE = 417,
     EOL = 418,
     EOP = 419,
     EOS = 420,
     EQUAL = 421,
     ERASE = 422,
     ERROR = 423,
     ESCAPE = 424,
     EVALUATE = 425,
     EVENT_STATUS = 426,
     EXCEPTION = 427,
     EXCEPTION_CONDITION = 428,
     EXCLUSIVE = 429,
     EXIT = 430,
     EXPONENTIATION = 431,
     EXTEND = 432,
     EXTERNAL = 433,
     F = 434,
     FD = 435,
     FILE_CONTROL = 436,
     FILE_ID = 437,
     FILLER = 438,
     FINAL = 439,
     FIRST = 440,
     FIXED = 441,
     FLOAT_BINARY_128 = 442,
     FLOAT_BINARY_32 = 443,
     FLOAT_BINARY_64 = 444,
     FLOAT_DECIMAL_16 = 445,
     FLOAT_DECIMAL_34 = 446,
     FLOAT_DECIMAL_7 = 447,
     FLOAT_EXTENDED = 448,
     FLOAT_LONG = 449,
     FLOAT_SHORT = 450,
     FOOTING = 451,
     FOR = 452,
     FOREGROUND_COLOR = 453,
     FOREVER = 454,
     FORMATTED_DATE_FUNC = 455,
     FORMATTED_DATETIME_FUNC = 456,
     FORMATTED_TIME_FUNC = 457,
     FREE = 458,
     FROM = 459,
     FROM_CRT = 460,
     FULL = 461,
     FUNCTION = 462,
     FUNCTION_ID = 463,
     FUNCTION_NAME = 464,
     GENERATE = 465,
     GIVING = 466,
     GLOBAL = 467,
     GO = 468,
     GOBACK = 469,
     GREATER = 470,
     GREATER_OR_EQUAL = 471,
     GRID = 472,
     GROUP = 473,
     HEADING = 474,
     HIGHLIGHT = 475,
     HIGH_VALUE = 476,
     ID = 477,
     IDENTIFICATION = 478,
     IF = 479,
     IGNORE = 480,
     IGNORING = 481,
     IN = 482,
     INDEX = 483,
     INDEXED = 484,
     INDICATE = 485,
     INITIALIZE = 486,
     INITIALIZED = 487,
     INITIATE = 488,
     INPUT = 489,
     INPUT_OUTPUT = 490,
     INSPECT = 491,
     INTO = 492,
     INTRINSIC = 493,
     INVALID = 494,
     INVALID_KEY = 495,
     IS = 496,
     I_O = 497,
     I_O_CONTROL = 498,
     JUSTIFIED = 499,
     KEPT = 500,
     KEY = 501,
     KEYBOARD = 502,
     LABEL = 503,
     LAST = 504,
     LEADING = 505,
     LEFT = 506,
     LEFTLINE = 507,
     LENGTH = 508,
     LENGTH_OF = 509,
     LESS = 510,
     LESS_OR_EQUAL = 511,
     LIMIT = 512,
     LIMITS = 513,
     LINAGE = 514,
     LINAGE_COUNTER = 515,
     LINE = 516,
     LINE_COUNTER = 517,
     LINES = 518,
     LINKAGE = 519,
     LITERAL = 520,
     LOCALE = 521,
     LOCALE_DATE_FUNC = 522,
     LOCALE_TIME_FUNC = 523,
     LOCALE_TIME_FROM_FUNC = 524,
     LOCAL_STORAGE = 525,
     LOCK = 526,
     LOWER = 527,
     LOWER_CASE_FUNC = 528,
     LOWLIGHT = 529,
     LOW_VALUE = 530,
     MANUAL = 531,
     MEMORY = 532,
     MERGE = 533,
     MINUS = 534,
     MNEMONIC_NAME = 535,
     MODE = 536,
     MOVE = 537,
     MULTIPLE = 538,
     MULTIPLY = 539,
     NAME = 540,
     NATIONAL = 541,
     NATIONAL_EDITED = 542,
     NATIONAL_OF_FUNC = 543,
     NATIVE = 544,
     NEAREST_AWAY_FROM_ZERO = 545,
     NEAREST_EVEN = 546,
     NEAREST_TOWARD_ZERO = 547,
     NEGATIVE = 548,
     NEXT = 549,
     NEXT_PAGE = 550,
     NO = 551,
     NO_ECHO = 552,
     NORMAL = 553,
     NOT = 554,
     NOTHING = 555,
     NOT_END = 556,
     NOT_EOP = 557,
     NOT_ESCAPE = 558,
     NOT_EQUAL = 559,
     NOT_EXCEPTION = 560,
     NOT_INVALID_KEY = 561,
     NOT_OVERFLOW = 562,
     NOT_SIZE_ERROR = 563,
     NO_ADVANCING = 564,
     NUMBER = 565,
     NUMBERS = 566,
     NUMERIC = 567,
     NUMERIC_EDITED = 568,
     NUMVALC_FUNC = 569,
     OBJECT_COMPUTER = 570,
     OCCURS = 571,
     OF = 572,
     OFF = 573,
     OMITTED = 574,
     ON = 575,
     ONLY = 576,
     OPEN = 577,
     OPTIONAL = 578,
     OR = 579,
     ORDER = 580,
     ORGANIZATION = 581,
     OTHER = 582,
     OUTPUT = 583,
     OVERLINE = 584,
     PACKED_DECIMAL = 585,
     PADDING = 586,
     PAGE = 587,
     PAGE_COUNTER = 588,
     PARAGRAPH = 589,
     PERFORM = 590,
     PH = 591,
     PF = 592,
     PICTURE = 593,
     PICTURE_SYMBOL = 594,
     PLUS = 595,
     POINTER = 596,
     POSITION = 597,
     POSITIVE = 598,
     PRESENT = 599,
     PREVIOUS = 600,
     PRINT = 601,
     PRINTER = 602,
     PRINTER_1 = 603,
     PRINTING = 604,
     PROCEDURE = 605,
     PROCEDURES = 606,
     PROCEED = 607,
     PROGRAM = 608,
     PROGRAM_ID = 609,
     PROGRAM_NAME = 610,
     PROGRAM_POINTER = 611,
     PROHIBITED = 612,
     PROMPT = 613,
     PROTECTED = 614,
     QUOTE = 615,
     RANDOM = 616,
     RD = 617,
     READ = 618,
     READY_TRACE = 619,
     RECORD = 620,
     RECORDING = 621,
     RECORDS = 622,
     RECURSIVE = 623,
     REDEFINES = 624,
     REEL = 625,
     REFERENCE = 626,
     REFERENCES = 627,
     RELATIVE = 628,
     RELEASE = 629,
     REMAINDER = 630,
     REMOVAL = 631,
     RENAMES = 632,
     REPLACE = 633,
     REPLACING = 634,
     REPORT = 635,
     REPORTING = 636,
     REPORTS = 637,
     REPOSITORY = 638,
     REQUIRED = 639,
     RESERVE = 640,
     RESET = 641,
     RESET_TRACE = 642,
     RETRY = 643,
     RETURN = 644,
     RETURNING = 645,
     REVERSE = 646,
     REVERSE_FUNC = 647,
     REVERSE_VIDEO = 648,
     REVERSED = 649,
     REWIND = 650,
     REWRITE = 651,
     RF = 652,
     RH = 653,
     RIGHT = 654,
     ROLLBACK = 655,
     ROUNDED = 656,
     RUN = 657,
     S = 658,
     SAME = 659,
     SCREEN = 660,
     SCREEN_CONTROL = 661,
     SCROLL = 662,
     SD = 663,
     SEARCH = 664,
     SECONDS = 665,
     SECTION = 666,
     SECURE = 667,
     SEGMENT_LIMIT = 668,
     SELECT = 669,
     SEMI_COLON = 670,
     SENTENCE = 671,
     SEPARATE = 672,
     SEQUENCE = 673,
     SEQUENTIAL = 674,
     SET = 675,
     SEVENTY_EIGHT = 676,
     SHARING = 677,
     SIGN = 678,
     SIGNED = 679,
     SIGNED_INT = 680,
     SIGNED_LONG = 681,
     SIGNED_SHORT = 682,
     SIXTY_SIX = 683,
     SIZE = 684,
     SIZE_ERROR = 685,
     SORT = 686,
     SORT_MERGE = 687,
     SOURCE = 688,
     SOURCE_COMPUTER = 689,
     SPACE = 690,
     SPECIAL_NAMES = 691,
     STANDARD = 692,
     STANDARD_1 = 693,
     STANDARD_2 = 694,
     START = 695,
     STATIC = 696,
     STATUS = 697,
     STDCALL = 698,
     STEP = 699,
     STOP = 700,
     STRING = 701,
     SUBSTITUTE_FUNC = 702,
     SUBSTITUTE_CASE_FUNC = 703,
     SUBTRACT = 704,
     SUM = 705,
     SUPPRESS = 706,
     SYMBOLIC = 707,
     SYNCHRONIZED = 708,
     SYSTEM_DEFAULT = 709,
     SYSTEM_OFFSET = 710,
     TAB = 711,
     TALLYING = 712,
     TAPE = 713,
     TERMINATE = 714,
     TEST = 715,
     THAN = 716,
     THEN = 717,
     THRU = 718,
     TIME = 719,
     TIME_OUT = 720,
     TIMES = 721,
     TO = 722,
     TOK_AMPER = 723,
     TOK_CLOSE_PAREN = 724,
     TOK_COLON = 725,
     TOK_DIV = 726,
     TOK_DOT = 727,
     TOK_EQUAL = 728,
     TOK_FALSE = 729,
     TOK_FILE = 730,
     TOK_GREATER = 731,
     TOK_INITIAL = 732,
     TOK_LESS = 733,
     TOK_MINUS = 734,
     TOK_MUL = 735,
     TOK_NULL = 736,
     TOK_OVERFLOW = 737,
     TOK_OPEN_PAREN = 738,
     TOK_PLUS = 739,
     TOK_TRUE = 740,
     TOP = 741,
     TOWARD_GREATER = 742,
     TOWARD_LESSER = 743,
     TRAILING = 744,
     TRANSFORM = 745,
     TRIM_FUNC = 746,
     TRUNCATION = 747,
     TYPE = 748,
     U = 749,
     UNDERLINE = 750,
     UNIT = 751,
     UNLOCK = 752,
     UNSIGNED = 753,
     UNSIGNED_INT = 754,
     UNSIGNED_LONG = 755,
     UNSIGNED_SHORT = 756,
     UNSTRING = 757,
     UNTIL = 758,
     UP = 759,
     UPDATE = 760,
     UPON = 761,
     UPON_ARGUMENT_NUMBER = 762,
     UPON_COMMAND_LINE = 763,
     UPON_ENVIRONMENT_NAME = 764,
     UPON_ENVIRONMENT_VALUE = 765,
     UPPER = 766,
     UPPER_CASE_FUNC = 767,
     USAGE = 768,
     USE = 769,
     USER = 770,
     USER_DEFAULT = 771,
     USER_FUNCTION_NAME = 772,
     USING = 773,
     V = 774,
     VALUE = 775,
     VARIABLE = 776,
     VARYING = 777,
     WAIT = 778,
     WHEN = 779,
     WHEN_COMPILED_FUNC = 780,
     WITH = 781,
     WORD = 782,
     WORDS = 783,
     WORKING_STORAGE = 784,
     WRITE = 785,
     YYYYDDD = 786,
     YYYYMMDD = 787,
     ZERO = 788,
     SHIFT_PREFER = 789,
     OVERFLOW = 790
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_PARSER_H_INCLUDED  */
