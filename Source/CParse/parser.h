/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     HBLOCK = 259,
     POUND = 260,
     STRING = 261,
     INCLUDE = 262,
     IMPORT = 263,
     INSERT = 264,
     CHARCONST = 265,
     NUM_INT = 266,
     NUM_FLOAT = 267,
     NUM_UNSIGNED = 268,
     NUM_LONG = 269,
     NUM_ULONG = 270,
     NUM_LONGLONG = 271,
     NUM_ULONGLONG = 272,
     NUM_BOOL = 273,
     TYPEDEF = 274,
     TYPE_INT = 275,
     TYPE_UNSIGNED = 276,
     TYPE_SHORT = 277,
     TYPE_LONG = 278,
     TYPE_FLOAT = 279,
     TYPE_DOUBLE = 280,
     TYPE_CHAR = 281,
     TYPE_WCHAR = 282,
     TYPE_VOID = 283,
     TYPE_SIGNED = 284,
     TYPE_BOOL = 285,
     TYPE_COMPLEX = 286,
     TYPE_TYPEDEF = 287,
     TYPE_RAW = 288,
     TYPE_NON_ISO_INT8 = 289,
     TYPE_NON_ISO_INT16 = 290,
     TYPE_NON_ISO_INT32 = 291,
     TYPE_NON_ISO_INT64 = 292,
     LPAREN = 293,
     RPAREN = 294,
     COMMA = 295,
     SEMI = 296,
     EXTERN = 297,
     INIT = 298,
     LBRACE = 299,
     RBRACE = 300,
     PERIOD = 301,
     CONST_QUAL = 302,
     VOLATILE = 303,
     REGISTER = 304,
     STRUCT = 305,
     UNION = 306,
     EQUAL = 307,
     SIZEOF = 308,
     MODULE = 309,
     LBRACKET = 310,
     RBRACKET = 311,
     BEGINFILE = 312,
     ENDOFFILE = 313,
     ILLEGAL = 314,
     CONSTANT = 315,
     NAME = 316,
     RENAME = 317,
     NAMEWARN = 318,
     EXTEND = 319,
     PRAGMA = 320,
     FEATURE = 321,
     VARARGS = 322,
     ENUM = 323,
     CLASS = 324,
     TYPENAME = 325,
     PRIVATE = 326,
     PUBLIC = 327,
     PROTECTED = 328,
     COLON = 329,
     STATIC = 330,
     VIRTUAL = 331,
     FRIEND = 332,
     THROW = 333,
     CATCH = 334,
     EXPLICIT = 335,
     USING = 336,
     NAMESPACE = 337,
     NATIVE = 338,
     INLINE = 339,
     TYPEMAP = 340,
     EXCEPT = 341,
     ECHO = 342,
     APPLY = 343,
     CLEAR = 344,
     SWIGTEMPLATE = 345,
     FRAGMENT = 346,
     WARN = 347,
     LESSTHAN = 348,
     GREATERTHAN = 349,
     DELETE_KW = 350,
     LESSTHANOREQUALTO = 351,
     GREATERTHANOREQUALTO = 352,
     EQUALTO = 353,
     NOTEQUALTO = 354,
     QUESTIONMARK = 355,
     TYPES = 356,
     PARMS = 357,
     NONID = 358,
     DSTAR = 359,
     DCNOT = 360,
     TEMPLATE = 361,
     OPERATOR = 362,
     COPERATOR = 363,
     PARSETYPE = 364,
     PARSEPARM = 365,
     PARSEPARMS = 366,
     DOXYGENSTRING = 367,
     DOXYGENPOSTSTRING = 368,
     C_COMMENT_STRING = 369,
     CAST = 370,
     LOR = 371,
     LAND = 372,
     OR = 373,
     XOR = 374,
     AND = 375,
     RSHIFT = 376,
     LSHIFT = 377,
     MINUS = 378,
     PLUS = 379,
     MODULO = 380,
     SLASH = 381,
     STAR = 382,
     LNOT = 383,
     NOT = 384,
     UMINUS = 385,
     DCOLON = 386
   };
#endif
/* Tokens.  */
#define ID 258
#define HBLOCK 259
#define POUND 260
#define STRING 261
#define INCLUDE 262
#define IMPORT 263
#define INSERT 264
#define CHARCONST 265
#define NUM_INT 266
#define NUM_FLOAT 267
#define NUM_UNSIGNED 268
#define NUM_LONG 269
#define NUM_ULONG 270
#define NUM_LONGLONG 271
#define NUM_ULONGLONG 272
#define NUM_BOOL 273
#define TYPEDEF 274
#define TYPE_INT 275
#define TYPE_UNSIGNED 276
#define TYPE_SHORT 277
#define TYPE_LONG 278
#define TYPE_FLOAT 279
#define TYPE_DOUBLE 280
#define TYPE_CHAR 281
#define TYPE_WCHAR 282
#define TYPE_VOID 283
#define TYPE_SIGNED 284
#define TYPE_BOOL 285
#define TYPE_COMPLEX 286
#define TYPE_TYPEDEF 287
#define TYPE_RAW 288
#define TYPE_NON_ISO_INT8 289
#define TYPE_NON_ISO_INT16 290
#define TYPE_NON_ISO_INT32 291
#define TYPE_NON_ISO_INT64 292
#define LPAREN 293
#define RPAREN 294
#define COMMA 295
#define SEMI 296
#define EXTERN 297
#define INIT 298
#define LBRACE 299
#define RBRACE 300
#define PERIOD 301
#define CONST_QUAL 302
#define VOLATILE 303
#define REGISTER 304
#define STRUCT 305
#define UNION 306
#define EQUAL 307
#define SIZEOF 308
#define MODULE 309
#define LBRACKET 310
#define RBRACKET 311
#define BEGINFILE 312
#define ENDOFFILE 313
#define ILLEGAL 314
#define CONSTANT 315
#define NAME 316
#define RENAME 317
#define NAMEWARN 318
#define EXTEND 319
#define PRAGMA 320
#define FEATURE 321
#define VARARGS 322
#define ENUM 323
#define CLASS 324
#define TYPENAME 325
#define PRIVATE 326
#define PUBLIC 327
#define PROTECTED 328
#define COLON 329
#define STATIC 330
#define VIRTUAL 331
#define FRIEND 332
#define THROW 333
#define CATCH 334
#define EXPLICIT 335
#define USING 336
#define NAMESPACE 337
#define NATIVE 338
#define INLINE 339
#define TYPEMAP 340
#define EXCEPT 341
#define ECHO 342
#define APPLY 343
#define CLEAR 344
#define SWIGTEMPLATE 345
#define FRAGMENT 346
#define WARN 347
#define LESSTHAN 348
#define GREATERTHAN 349
#define DELETE_KW 350
#define LESSTHANOREQUALTO 351
#define GREATERTHANOREQUALTO 352
#define EQUALTO 353
#define NOTEQUALTO 354
#define QUESTIONMARK 355
#define TYPES 356
#define PARMS 357
#define NONID 358
#define DSTAR 359
#define DCNOT 360
#define TEMPLATE 361
#define OPERATOR 362
#define COPERATOR 363
#define PARSETYPE 364
#define PARSEPARM 365
#define PARSEPARMS 366
#define DOXYGENSTRING 367
#define DOXYGENPOSTSTRING 368
#define C_COMMENT_STRING 369
#define CAST 370
#define LOR 371
#define LAND 372
#define OR 373
#define XOR 374
#define AND 375
#define RSHIFT 376
#define LSHIFT 377
#define MINUS 378
#define PLUS 379
#define MODULO 380
#define SLASH 381
#define STAR 382
#define LNOT 383
#define NOT 384
#define UMINUS 385
#define DCOLON 386




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 1710 "parser.y"
{
  char  *id;
  List  *bases;
  struct Define {
    String *val;
    String *rawval;
    int     type;
    String *qualifier;
    String *bitfield;
    Parm   *throws;
    String *throwf;
  } dtype;
  struct {
    char *type;
    String *filename;
    int   line;
  } loc;
  struct {
    char      *id;
    SwigType  *type;
    String    *defarg;
    ParmList  *parms;
    short      have_parms;
    ParmList  *throws;
    String    *throwf;
  } decl;
  Parm         *tparms;
  struct {
    String     *method;
    Hash       *kwargs;
  } tmap;
  struct {
    String     *type;
    String     *us;
  } ptype;
  SwigType     *type;
  String       *str;
  Parm         *p;
  ParmList     *pl;
  int           intvalue;
  Node         *node;
}
/* Line 1529 of yacc.c.  */
#line 354 "CParse/parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

