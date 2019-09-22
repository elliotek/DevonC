  // DevonC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <time.h>

   /*
Full, free support by email, phone and text. Register now or call 0845 638 1421 to sign up and start using SMS text messaging today.
*/

#include "../PEGTL-master/include/tao/pegtl.hpp"

namespace pegtl = TAO_PEGTL_NAMESPACE;
using namespace pegtl;


template< typename Rule > struct maction {};

struct pp_blank_line : until< eol, blank > {};
struct pp_comment : seq< one<'/'>, one<'/'>, until< eolf > > {};
struct pp_long_comment : seq< one<'/'>, one<'*'>, until < sor < eof, seq< one<'*'>, one<'/'> > >>> {};
struct pp_code : any {};
struct preprocess : star< sor<pp_comment, pp_long_comment, pp_code>> {};

template<> struct maction< pp_comment >
{
    template< typename Input > static void apply( const Input& in , std::string & out)
    {
		std::cout << "PP_COMMENT : "<< in.string() << std::endl;
    }
};

template<> struct maction< pp_long_comment >
{
    template< typename Input > static void apply( const Input& in , std::string & out)
    {
		std::cout << "PP_LONG_COMMENT : "<< in.string() << std::endl;
    }
};

template<> struct maction< pp_code >
{
    template< typename Input > static void apply( const Input& in , std::string & out)
    {
		out += in.string();
    }
};

template<> struct maction< preprocess >
{
    template< typename Input > static void apply( const Input& in , std::string & out)
    {
		std::cout << "PREPROCESS" << std::endl;
    }
};


struct blank_line : until< eol, blank > {};
struct sblk : star<sor<blank, eol>> {};
struct pblk : plus<sor<blank, eol>> {};
struct directive : seq< sblk, one<'#'>, until< eol , any > > {};
struct Id : seq< alpha, star<alnum> > {};

struct type_int   : TAO_PEGTL_STRING("int") {};
struct type_char  : TAO_PEGTL_STRING("char") {};
struct type_short : TAO_PEGTL_STRING("short") {};
struct type_void  : TAO_PEGTL_STRING("void") {};
struct type_bool  : TAO_PEGTL_STRING("bool") {};
struct type_base  : sor< type_int, type_char, type_short, type_void, type_bool > {};
struct type_pointer : seq< type_base, plus< sblk, one<'*'>> > {};
struct typespecifier : sor< type_pointer, type_base > {};

struct literalchar : seq< one<'\''>, plus<seven>, one<'\''>> {};
struct literalhexa : seq< one<'0'>, one<'x'>, must<plus<xdigit>> > {};
struct literaldecimal : seq< opt< one<'-'> >, plus<digit>> {};
struct literaltrue : TAO_PEGTL_STRING("true") {};
struct literalfalse : TAO_PEGTL_STRING("false") {};
struct literalnullptr : TAO_PEGTL_STRING("nullptr") {};
struct staticarraysize : sor< literalchar, literalhexa, literaldecimal > {};
struct vardeclid : seq< identifier, star< sblk, one<'['>, sblk, staticarraysize, sblk, one<']'> >> {};
struct literalexp : sor<literaltrue, literalfalse, literalnullptr, literalchar, literalhexa, literaldecimal> {};
struct varinit : seq< sblk, one<'='>, sblk, literalexp> {};
struct vartype : typespecifier {};
struct vardecl : seq<sblk, vartype, pblk, list< seq< vardeclid, opt<varinit> >, seq< sblk, one<','>, sblk > > > {};
struct globalvardecl : seq< vardecl, one<';'> > {};
struct localvardecl : seq< vardecl, one<';'> > {};
struct forvardecl : vardecl {};

struct memberid : identifier {};
struct varid : identifier {};
struct arrayindex;
struct arrayaccess : seq< one<'['>, sblk, arrayindex, sblk, one<']'> >{};
struct varaccess : seq<varid, star<sblk, arrayaccess>, star< sblk, one<'.'>, sblk, memberid, star<sblk, arrayaccess> >> {};
struct lvalue : varaccess {};

enum ERelopType
{
	LowerEq,
	Lower,
	GreaterEq,
	Greater,
	Equal,
	NotEqual,
};

template<ERelopType RelopType> struct relop {};
template<> struct relop<LowerEq>	: seq<one<'<'>, one<'='>> {};
template<> struct relop<Lower>		: one<'<'> {};
template<> struct relop<GreaterEq>	: seq<one<'>'>, one<'='>> {};
template<> struct relop<Greater>	: one<'>'> {};
template<> struct relop<Equal>		: two<'='> {};
template<> struct relop<NotEqual>	: seq<one<'!'>, one<'='>> {};

struct expressionerror : failure {};
struct expression;
struct subexpression;
struct parenthesedexpression : seq<one<'('>, sblk, expression, sblk, one<')'>> {};
struct funcid;
struct funcargexpression;
struct funcarglist : list< funcargexpression, seq<sblk, one<','>, sblk> > {};
struct funccall : seq< funcid, sblk, one<'('>, sblk, opt<funcarglist>, sblk, one<')'> > {};
struct rvalue : sor< parenthesedexpression, funccall, literalexp, expressionerror> {};
struct assignment : seq<lvalue, sblk, one<'='>, sblk, expression> {};
struct reloperator : sor<relop<LowerEq>, relop<Lower>, relop<GreaterEq>, relop<Greater>, relop<Equal>, relop<NotEqual>> {};
struct addop : one<'+'> {};
struct subop : one<'-'> {};
struct mulop : one<'*'> {};
struct divop : one<'/'> {};
struct modop : one<'%'> {};
struct minusop : one<'-'> {};
struct indirectop : one<'*'> {};
struct addressop : one<'&'> {};
struct unaryop : sor<minusop, indirectop, addressop> {};
struct sumop : sor<addop, subop> {};
struct prodop : sor<mulop, divop, modop> {};
struct factor : sor<rvalue, lvalue> {};
struct applyunaryexpression;
struct unaryexpression : if_then_else<unaryop, seq<sblk, applyunaryexpression>, factor> {};
struct applyunaryexpression : unaryexpression {};
struct productexpression : list< if_then_else< at<unaryexpression>, unaryexpression, expressionerror>, seq<sblk, prodop, sblk> > {};
struct sumexpression : list< if_then_else< at<productexpression>, productexpression, expressionerror>, seq<sblk, sumop, sblk> > {};
struct applyrelexpression : seq<sumexpression, sblk, reloperator, sblk, sumexpression> {};
struct relexpression : if_then_else< at<applyrelexpression>, applyrelexpression, sumexpression> {};
struct applynotexpression;
struct notexpression : if_then_else<one<'!'>, seq<sblk, applynotexpression>, relexpression> {};
struct applynotexpression : notexpression {};
struct andexpression : list< if_then_else< at<notexpression>, notexpression, expressionerror>, seq<sblk, two<'&'>, sblk> > {};
struct orexpression : list< if_then_else< at<andexpression>, andexpression, expressionerror>, seq<sblk, two<'|'>, sblk> > {};
struct subexpression : if_then_else<at<assignment>, assignment, if_then_else< at<orexpression>, orexpression, expressionerror>> {};
struct funcargexpression : subexpression {};
struct expression : list< subexpression, seq<sblk, one<','>, sblk> > {};
struct arrayindex : expression {};

struct whilecond : expression {};
struct dowhilecond : expression {};
struct ifcond : expression {};
struct forcond : expression {};

struct functype : typespecifier {};
struct funcid : identifier {};
struct labelid : identifier {};
struct label : seq< labelid, sblk, one<':'>> {};
struct statement;
struct unknownstatement : seq<plus<alnum>, sblk, one<';'> > {};
struct expressionstatement : seq< opt<expression, sblk>, one<';'> > {};
struct gotostatement : seq<TAO_PEGTL_STRING("goto"), sblk, labelid, sblk, one<';'> > {};
struct returnstatement : seq<TAO_PEGTL_STRING( "return" ), opt< sblk, expression>, sblk, one<';'> > {};
struct breakstatement : seq<TAO_PEGTL_STRING( "break" ), sblk, one<';'> > {};
struct whilestatement : seq<TAO_PEGTL_STRING( "while" ), must< sblk, one<'('>, sblk, plus< whilecond, sblk>, one<')'>, sblk, statement > > {};
struct nextstatement : expression {};
struct forstatement : seq<TAO_PEGTL_STRING( "for" ), must< sblk, one<'('>, sblk, sor< forvardecl, expression >, sblk, one<';'>, sblk, forcond, sblk, one<';'>, sblk, nextstatement, sblk, one<')'>, sblk, statement > > {};
struct dowhilestatement : seq<TAO_PEGTL_STRING( "do" ), must< sblk, statement, sblk, TAO_PEGTL_STRING( "while" ), sblk, one<'('>, sblk, plus< dowhilecond, sblk>, one<')'>, sblk, one<';'> > > {};
struct elsestatement : seq<TAO_PEGTL_STRING( "else" ), sblk, statement > {};
struct ifstatement : seq<TAO_PEGTL_STRING( "if" ), sblk, one<'('>, sblk, plus< ifcond, sblk>, one<')'>, sblk, statement, opt< sblk, elsestatement > > {};
struct localscope;
struct statement : sor< localscope, localvardecl, breakstatement, returnstatement, forstatement, dowhilestatement, whilestatement, ifstatement, gotostatement, expressionstatement, unknownstatement > {};
struct scopestart : one<'{'> {};
struct scope : seq< scopestart, star< sblk, if_then_else< at<label>, label, statement >>, sblk, one<'}'>> {};
struct funcscope : scope {};
struct localscope : scope {};

struct paramtype : typespecifier {};
struct paramid : identifier {};
struct funcparam : seq< paramtype, pblk, paramid> {};
struct funcparamlist : seq< sblk, funcparam, star<sblk, one<','>, sblk, funcparam>, sblk > {};
struct funcdecl : seq<sblk, functype, pblk, funcid, sblk, one<'('>, opt<funcparamlist>, one<')'>, sblk, sor<funcscope, one<';'>> > {};

struct declaration : sor<funcdecl, globalvardecl> {};

struct unknown : until< one<';'> , any > {};
struct program : until< eof, sor<	blank_line, 
									directive,
									declaration,
									unknown
								> > {};


template<> struct maction< blank_line >
{
    template< typename Input > static void apply( const Input& in )
    {
//		std::cout << "BLANK LINE" << std::endl;
    }
};

template<> struct maction< sblk >
{
    template< typename Input > static void apply( const Input& in )
    {
//		std::cout << "SBLK(" << in.string() << ")" << std::endl;
    }
};

template<> struct maction< funcargexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "FUNCARGEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< funccall >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "FUNCCALL : " << in.string() << std::endl;
	}
};

template<> struct maction< expressionerror >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "EXPRESSIONERROR : " << in.string() << std::endl;
	}
};

template<> struct maction< applyunaryexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "APPLYUNARYEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< productexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "PRODUCTEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< sumexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "SUMEXPRESSION : " << in.string() << std::endl;
	}
};

template<ERelopType T> struct maction< relop<T> >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "RELOP : " << in.string() << std::endl;
	}
};

template<> struct maction< literaldecimal >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "LITERALDECIMAL : " << in.string() << std::endl;
	}
};

template<> struct maction< literalhexa >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "LITERALHEXA : " << in.string() << std::endl;
	}
};

template<> struct maction< applynotexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "! EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< relexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "REL EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< orexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "|| EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< andexpression >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "&& EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< literalexp >
{
	template< typename Input > static void apply(const Input& in)
	{
//		std::cout << "LITERALEXP : " << in.string() << std::endl;
	}
};

template<> struct maction< gotostatement >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "GOTOSTATEMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< ifcond >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "IFCOND : " << in.string() << std::endl;
	}
};

template<> struct maction< dowhilecond >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "DOWHILECOND : " << in.string() << std::endl;
    }
};

template<> struct maction< whilecond >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "WHILECOND : " << in.string() << std::endl;
	}
};

template<> struct maction< memberid >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "MEMBERID : " << in.string() << std::endl;
	}
};

template<> struct maction< arrayindex >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "ARRAYINDEX : " << in.string() << std::endl;
	}
};

template<> struct maction< arrayaccess >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "ARRAYACCESS : " << in.string() << std::endl;
	}
};

template<> struct maction< varid >
{
	template< typename Input > static void apply(const Input& in)
	{
		//std::cout << "VARID : " << in.string() << std::endl;
	}
};

template<> struct maction< lvalue >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "LVALUE : " << in.string() << std::endl;
	}
};

template<> struct maction< assignment >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "ASSIGNMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< forstatement >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "FORSTATEMENT" << std::endl;
	}
};

template<> struct maction< forcond >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "FORCOND : " << in.string() << std::endl;
	}
};

template<> struct maction< nextstatement >
{
	template< typename Input > static void apply(const Input& in)
	{
		std::cout << "NEXTSTATEMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< ifstatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "IFSTATEMENT" << std::endl;
    }
};

template<> struct maction< elsestatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "ELSESTATEMENT" << std::endl;
    }
};

template<> struct maction< literaltrue >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LITERAL TRUE" << std::endl;
    }
};

template<> struct maction< literalfalse >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LITERAL FALSE" << std::endl;
    }
};

template<> struct maction< literalnullptr >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LITERAL NULLPTR" << std::endl;
    }
};

template<> struct maction< dowhilestatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "DO WHILE STATEMENT : " << in.string() << std::endl;
    }
};
    
template<> struct maction< whilestatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "WHILE STATEMENT : " << in.string() << std::endl;
    }
    
	template< typename Input > static void failure( Input& in)
    {
		std::cout << "!!!! WHILE STATEMENT FAILURE: " << in.string() << std::endl;
    }
};

template<> struct maction< breakstatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "BREAK STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< unknownstatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "UNKNOWN STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< returnstatement >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "RETURN STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< funcparam >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "FUNCPARAM : " << in.string() << std::endl;
    }
};

template<> struct maction< paramtype >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "PARAMTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< paramid >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "PARAMID : " << in.string() << std::endl;
    }
};

template<> struct maction< localscope >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LOCALSCOPE END : " << in.string() << std::endl;
    }
};

template<> struct maction< labelid >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LABELID : " << in.string() << std::endl;
    }
};

template<> struct maction< label >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LABEL : " << in.string() << std::endl;
    }
};

template<> struct maction< unknown >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "UNKNOWN : " << in.string() << std::endl;
    }
};

template<> struct maction< scopestart >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "SCOPE START" << std::endl;
    }
};

template<> struct maction< scope >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "SCOPE END" << std::endl;
    }
};

template<> struct maction< funcscope >
{
    template< typename Input > static void apply( const Input& in )
    {
//		std::cout << "FUNCSCOPE END" << std::endl;
    }
};

template<> struct maction< vartype >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "VARTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< functype >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "FUNCTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< staticarraysize >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "ARRAY SIZE : " << in.string() << std::endl;
    }
};

template<> struct maction< funcid >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "FUNCID : " << in.string() << std::endl;
    }
};

template<> struct maction< identifier >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "ID : " << in.string() << std::endl;
    }
};

template<> struct maction< funcdecl >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "FUNCDECL IS VALID" << std::endl;
    }
};


template<> struct maction< globalvardecl >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "GLOBALVARDECL    " << in.string() << std::endl;
    }
};

template<> struct maction< localvardecl >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "LOCALVARDECL    " << in.string() << std::endl;
    }
};

template<> struct maction< directive >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "DIRECTIVE    " << in.string();
    }
};

template<> struct maction< declaration >
{
    template< typename Input > static void apply( const Input& in )
    {
		//std::cout << in.string() << std::endl;
    }
};

template<> struct maction< program >
{
    template< typename Input > static void apply( const Input& in )
    {
		std::cout << "END OF PROGRAM.\n";
    }
};


template< typename Rule > struct mcontrol : normal< Rule > {};

template<> struct mcontrol< preprocess > : normal< preprocess >
{
    template< typename Input >
    static void start( Input& in, std::string & out)
    {
		std::cout << "START OF PREPROCESS.\n";
    }

    template< typename Input >
    static void success( Input& in, std::string & out)
    {
		std::cout << "END OF PREPROCESS.\n";
    }

    template< typename Input >
    static void failure( Input& in, std::string & out)
    {
		std::cout << "FAILURE OF PREPROCESS.\n";
    }

    template< typename Input >
    static void raise( const Input& in, std::string & out)
    {
        throw parse_error(  internal::demangle< program >(), in );
    }
};

template<> struct mcontrol< program > : normal< program >
{
    template< typename Input >
    static void start( Input& in)
    {
		std::cout << "START OF COMPILATION.\n";
    }

    template< typename Input >
    static void success( Input& in)
    {
		std::cout << "END OF COMPILATION.\n";
    }

    template< typename Input >
    static void failure( Input& in)
    {
		std::cout << "FAILURE OF COMPILATION.\n";
    }

    template< typename Input >
    static void raise( const Input& in)
    {
        throw parse_error(  internal::demangle< program >(), in );
    }
};

int main(const int argc, char* argv[])  // NOLINT(bugprone-exception-escape)
{
	if (argc > 1)
	{
		clock_t t = clock();

		std::string PreProcessedStr;
		file_input FileInput(argv[1]);
		parse<preprocess, maction, mcontrol>(FileInput, PreProcessedStr);

		std::cout << PreProcessedStr << std::endl;

		string_input in1(PreProcessedStr, "");
		parse<program, maction, mcontrol>(in1);

		printf("Compiled in %fs.\n", float(clock() - t) / CLOCKS_PER_SEC);
	}

	return 1;
}
