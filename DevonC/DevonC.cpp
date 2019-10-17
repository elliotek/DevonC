#include <iostream>
#include <time.h>

#include "Compiler.h"
#include "../PEGTL-master/include/tao/pegtl.hpp"

namespace pegtl = TAO_PEGTL_NAMESPACE;
using namespace pegtl;
using namespace DevonC;

template< typename Rule > struct maction {};

struct pp_blank_line : until< eol, blank > {};
struct pp_comment : seq< one<'/'>, one<'/'>, until< eolf > > {};
struct pp_long_comment : seq< one<'/'>, one<'*'>, until < sor < eof, seq< one<'*'>, one<'/'> > >>> {};
struct pp_code : any {};
struct preprocess : star< sor<pp_comment, pp_long_comment, pp_code>> {};

template<> struct maction< pp_comment >
{
    template< typename Input > static void apply( const Input& in, std::string & out)
    {
		std::cout << "PP_COMMENT : "<< in.string() << std::endl;
		out += "\n";
    }
};

template<> struct maction< pp_long_comment >
{
    template< typename Input > static void apply( const Input& in, std::string & out)
    {
		std::string lc = in.string();
		std::cout << "PP_LONG_COMMENT : "<< lc << std::endl;
		size_t n = std::count(lc.begin(), lc.end(), '\n');
		for(int i = 0; i < n; i++)
			out += "\n";
    }
};

template<> struct maction< pp_code >
{
    template< typename Input > static void apply( const Input& in, std::string & out)
    {
		out += in.string();
    }
};

template<> struct maction< preprocess >
{
    template< typename Input > static void apply( const Input& in, std::string & out)
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
struct type_pointer : one<'*'> {};
struct typespecifier : seq< type_base, star< sblk, type_pointer> > {};

struct literalchar : seq< one<'\''>, seven, one<'\''>> {};
struct literalhexa : seq< one<'0'>, one<'x'>, must<plus<xdigit>> > {};
struct literaldecimal : seq< opt< one<'-'> >, plus<digit>> {};
struct literaltrue : TAO_PEGTL_STRING("true") {};
struct literalfalse : TAO_PEGTL_STRING("false") {};
struct literalnullptr : TAO_PEGTL_STRING("nullptr") {};
struct staticarraysize : sor< literalhexa, literaldecimal > {};
struct vardeclid : identifier {};
struct literalexp : sor<literaltrue, literalfalse, literalnullptr, literalchar, literalhexa, literaldecimal> {};
struct varinit : seq< sblk, one<'='>, sblk, literalexp> {};
struct vartype : typespecifier {};
struct vardecl : seq< vardeclid, star< sblk, one<'['>, sblk, staticarraysize, sblk, one<']'> >, opt<varinit> > {};
struct compvardecl : seq<sblk, vartype, pblk, list< vardecl, seq< sblk, one<','>, sblk > > > {};
struct globalvardecl : seq< compvardecl, one<';'> > {};
struct localvardecl : seq< compvardecl, one<';'> > {};
struct forvardecl : compvardecl {};

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
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
//		std::cout << "BLANK LINE" << std::endl;
    }
};

template<> struct maction< sblk >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
//		std::cout << "SBLK(" << in.string() << ")" << std::endl;
    }
};

template<> struct maction< funcargexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "FUNCARGEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< funccall >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "FUNCCALL : " << in.string() << std::endl;
	}
};

template<> struct maction< expressionerror >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "EXPRESSIONERROR : " << in.string() << std::endl;
	}
};

template<> struct maction< applyunaryexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "APPLYUNARYEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< productexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "PRODUCTEXPRESSION : " << in.string() << std::endl;
	}
};

template<> struct maction< sumexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "SUMEXPRESSION : " << in.string() << std::endl;
	}
};

template<ERelopType T> struct maction< relop<T> >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "RELOP : " << in.string() << std::endl;
	}
};

template<> struct maction< literaldecimal >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "LITERALDECIMAL : " << in.string() << std::endl;
		Compiler.SetCurLiteral(LiteralType::Numeric, std::stoi(in.string()));
	}
};
template<> struct maction< literalchar >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "LITERALCHAR : " << in.string() << std::endl;
		Compiler.SetCurLiteral(LiteralType::Numeric, std::string(in.string())[1]);
	}
};

template<> struct maction< literalhexa >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "LITERALHEXA : " << in.string() << std::endl;
		Compiler.SetCurLiteral(LiteralType::Numeric, std::stoi(in.string(), nullptr, 16));
	}
};

template<> struct maction< literaltrue >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "LITERALTRUE : " << std::endl;
		Compiler.SetCurLiteral(LiteralType::Boolean, 1);
	}
};

template<> struct maction< literalfalse >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "LITERALFALSE : " << std::endl;
		Compiler.SetCurLiteral(LiteralType::Boolean, 0);
	}
};

template<> struct maction< literalnullptr >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "LITERALNULLPTR : " <<  std::endl;
		Compiler.SetCurLiteral(LiteralType::Nullptr);
	}
};

template<> struct maction< applynotexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "! EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< relexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "REL EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< orexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "|| EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< andexpression >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "&& EXPR : " << in.string() << std::endl;
	}
};

template<> struct maction< literalexp >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
//		std::cout << "LITERALEXP : " << in.string() << std::endl;
	}
};

template<> struct maction< gotostatement >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "GOTOSTATEMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< ifcond >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "IFCOND : " << in.string() << std::endl;
	}
};

template<> struct maction< dowhilecond >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "DOWHILECOND : " << in.string() << std::endl;
    }
};

template<> struct maction< whilecond >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "WHILECOND : " << in.string() << std::endl;
	}
};

template<> struct maction< memberid >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "MEMBERID : " << in.string() << std::endl;
	}
};

template<> struct maction< arrayindex >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "ARRAYINDEX : " << in.string() << std::endl;
	}
};

template<> struct maction< arrayaccess >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "ARRAYACCESS : " << in.string() << std::endl;
	}
};

template<> struct maction< varid >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		//std::cout << "VARID : " << in.string() << std::endl;
	}
};

template<> struct maction< lvalue >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "LVALUE : " << in.string() << std::endl;
	}
};

template<> struct maction< assignment >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "ASSIGNMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< forstatement >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "FORSTATEMENT" << std::endl;
	}
};

template<> struct maction< forcond >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "FORCOND : " << in.string() << std::endl;
	}
};

template<> struct maction< nextstatement >
{
	template< typename Input > static void apply(const Input& in, Compiler & Compiler)
	{
		std::cout << "NEXTSTATEMENT : " << in.string() << std::endl;
	}
};

template<> struct maction< ifstatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "IFSTATEMENT" << std::endl;
    }
};

template<> struct maction< elsestatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "ELSESTATEMENT" << std::endl;
    }
};

template<> struct maction< dowhilestatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "DO WHILE STATEMENT : " << in.string() << std::endl;
    }
};
    
template<> struct maction< whilestatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "WHILE STATEMENT : " << in.string() << std::endl;
    }
    
	template< typename Input > static void failure( Input& in, Compiler & Compiler)
    {
		std::cout << "!!!! WHILE STATEMENT FAILURE: " << in.string() << std::endl;
    }
};

template<> struct maction< breakstatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "BREAK STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< unknownstatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "UNKNOWN STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< returnstatement >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "RETURN STATEMENT : " << in.string() << std::endl;
    }
};

template<> struct maction< funcparam >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "FUNCPARAM : " << in.string() << std::endl;
    }
};

template<> struct maction< paramtype >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "PARAMTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< paramid >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "PARAMID : " << in.string() << std::endl;
    }
};

template<> struct maction< localscope >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "LOCALSCOPE END : " << in.string() << std::endl;
    }
};

template<> struct maction< labelid >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "LABELID : " << in.string() << std::endl;
    }
};

template<> struct maction< label >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "LABEL : " << in.string() << std::endl;
    }
};

template<> struct maction< unknown >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "UNKNOWN : " << in.string() << std::endl;
    }
};

template<> struct maction< scopestart >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "SCOPE START" << std::endl;
    }
};

template<> struct maction< scope >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "SCOPE END" << std::endl;
    }
};

template<> struct maction< funcscope >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
//		std::cout << "FUNCSCOPE END" << std::endl;
    }
};

template<> struct maction< vartype >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "VARTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< functype >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "FUNCTYPE : " << in.string() << std::endl;
    }
};

template<> struct maction< staticarraysize >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::cout << "ARRAY SIZE : " << in.string() << std::endl;
		Compiler.CurVarDecl.ArraySizes.push_back(Compiler.CurLiteralValue);
	}
};

template<> struct maction< type_pointer >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		Compiler.CurVarDecl.PointerIndirection++;
	}
};

template<> struct maction< type_base >
{
	template< typename Input > static void apply(const Input& in, Compiler& Compiler)
	{
		std::string t = in.string();
		switch (t[0])
		{
		case 'i':	Compiler.CurVarDecl.Type = VarType::Int;	break;
		case 'c':	Compiler.CurVarDecl.Type = VarType::Char;	break;
		case 's':	Compiler.CurVarDecl.Type = VarType::Short;	break;
		case 'v':	Compiler.CurVarDecl.Type = VarType::Void;	break;
		case 'b':	Compiler.CurVarDecl.Type = VarType::Bool;	break;
		}

		Compiler.CurVarDecl.PointerIndirection = 0;

	}
};

template<> struct maction< funcid >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "FUNCID : " << in.string() << std::endl;
    }
};

template<> struct maction< identifier >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "ID : " << in.string() << std::endl;
    }
};

template<> struct maction< funcdecl >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "FUNCDECL IS VALID" << std::endl;
    }
};

template<> struct maction< vardeclid >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		Compiler.CurVarDecl.Identifier = in.string();
		Compiler.CurVarDecl.StaticInit.reset();
    }
};

template<> struct maction< globalvardecl >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "GLOBALVARDECL    " << in.string() << std::endl;
		Compiler.ValidateGlobalVar();
    }
};

template<> struct maction< localvardecl >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "LOCALVARDECL    " << in.string() << std::endl;
    }
};

template<> struct maction< varinit >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "VARINIT    " << in.string() << std::endl;
		Compiler.CurVarDecl.StaticInit = Compiler.CurLiteralValue;
    }
};

template<> struct maction< vardecl >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "VARDECL    " << in.string() << std::endl;

		const auto pos = in.position();
		Compiler.PushPendingVarDecl(pos.source, pos.line);
    }
};

template<> struct maction< directive >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		std::cout << "DIRECTIVE    " << in.string();
    }
};

template<> struct maction< declaration >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
    {
		//std::cout << in.string() << std::endl;
    }
};

template<> struct maction< program >
{
    template< typename Input > static void apply( const Input& in, Compiler & Compiler )
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
    static void start( Input& in, Compiler & Compiler)
    {
		std::cout << "START OF COMPILATION.\n";
    }

    template< typename Input >
    static void success( Input& in, Compiler & Compiler)
    {
		std::cout << "END OF COMPILATION.\n";
    }

    template< typename Input >
    static void failure( Input& in, Compiler & Compiler)
    {
		std::cout << "FAILURE OF COMPILATION.\n";
    }

    template< typename Input >
    static void raise( const Input& in, Compiler & Compiler)
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

		Compiler Compiler;

		string_input PreProcessedStrInput(PreProcessedStr, "");
		parse<program, maction, mcontrol>(PreProcessedStrInput, Compiler);

		printf("Compiled in %fs.\n", float(clock() - t) / CLOCKS_PER_SEC);

		const int NbErr = Compiler.GetNbErrors();
		printf("%d error%s.", NbErr, NbErr>1?"s":"");
	}

	return 1;
}
