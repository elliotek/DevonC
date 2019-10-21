#pragma once

#include <string>
#include <vector>
#include <optional>
#include <iostream>

#include "../PEGTL-master/include/tao/pegtl.hpp"

#define DLOG printf

namespace DevonC
{
	enum class LiteralType : unsigned char
	{
		None,
		Numeric,
		Boolean,
		Nullptr,
	};

	enum class VarType : unsigned char
	{
		Unknown,
		Int,
		Char,
		Short,
		Void,
		Bool,
		Pointer,
	};

	struct Variable
	{
		std::string Identifier;
		VarType Type = VarType::Unknown;
		std::vector<int> ArraySizes;
		int PointerIndirection = 0;
		std::optional<int> StaticInit;

		Variable() {};
		Variable(const std::string& _Identifier) : Identifier(_Identifier) {};
		Variable(const std::string&& _Identifier) : Identifier(_Identifier) {};
	};

	enum class CodeBlockType : unsigned char
	{
		Unknown,
		Expression,
		Assignment,
		IfCond,
		IfTrue,
		IfFalse,
	};

	struct CodeBlockHandler
	{
		CodeBlockType Type;

	};

	struct Scope
	{
		std::vector<Variable>			Variables;
		std::vector<CodeBlockHandler>	CodeBlocks;

		//	std::string Name;
	};

	struct Function
	{
		Scope Scope;

		std::string Identifier;
		Function(const std::string& _Identifier) : Identifier(_Identifier) {};
	};

	enum class EErrorCode : unsigned char
	{
		VoidVarDecl,
		BadInitializerLiteralType,
	};

	class Compiler
	{
		std::vector<Variable>	GlobalVars;
		std::vector<Function>	Functions;
		std::vector<Scope>		ScopeStack;
		std::vector<Variable>	PendingVarDecls;
		int NbErrors = 0;

		int TypeSize(VarType _Type);
		void ErrorMessage(EErrorCode ErrorCode, std::string source, size_t line);

	public:
		Variable CurVarDecl;
		int CurLiteralValue = 0;
		LiteralType CurLiteralType = LiteralType::None;

		void Compile(char* _Filename);
		void SetCurLiteral(LiteralType _Type, int _Value = 0);
		void PushPendingVarDecl(const std::string& source, const size_t line);
		void ValidateGlobalVar();
		int GetNbErrors() const { return NbErrors; }
		void DumpDebug();
	};

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
		template< typename Input > static void apply(const Input& in, std::string& out)
		{
			std::cout << "PP_COMMENT : " << in.string() << std::endl;
			out += "\n";
		}
	};

	template<> struct maction< pp_long_comment >
	{
		template< typename Input > static void apply(const Input& in, std::string& out)
		{
			std::string lc = in.string();
			std::cout << "PP_LONG_COMMENT : " << lc << std::endl;
			size_t n = std::count(lc.begin(), lc.end(), '\n');
			for (int i = 0; i < n; i++)
				out += "\n";
		}
	};

	template<> struct maction< pp_code >
	{
		template< typename Input > static void apply(const Input& in, std::string& out)
		{
			out += in.string();
		}
	};

	template<> struct maction< preprocess >
	{
		template< typename Input > static void apply(const Input& in, std::string& out)
		{
			std::cout << "PREPROCESS" << std::endl;
		}
	};


	struct blank_line : until< eol, blank > {};
	struct sblk : star<sor<blank, eol>> {};
	struct pblk : plus<sor<blank, eol>> {};
	struct directive : seq< sblk, one<'#'>, until< eol, any > > {};
	struct Id : seq< alpha, star<alnum> > {};

	struct type_int : TAO_PEGTL_STRING("int") {};
	struct type_char : TAO_PEGTL_STRING("char") {};
	struct type_short : TAO_PEGTL_STRING("short") {};
	struct type_void : TAO_PEGTL_STRING("void") {};
	struct type_bool : TAO_PEGTL_STRING("bool") {};
	struct type_base : sor< type_int, type_char, type_short, type_void, type_bool > {};
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
	struct arrayaccess : seq< one<'['>, sblk, arrayindex, sblk, one<']'> > {};
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
	template<> struct relop<LowerEq> : seq<one<'<'>, one<'='>> {};
	template<> struct relop<Lower> : one<'<'> {};
	template<> struct relop<GreaterEq> : seq<one<'>'>, one<'='>> {};
	template<> struct relop<Greater> : one<'>'> {};
	template<> struct relop<Equal> : two<'='> {};
	template<> struct relop<NotEqual> : seq<one<'!'>, one<'='>> {};

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
	struct returnstatement : seq<TAO_PEGTL_STRING("return"), opt< sblk, expression>, sblk, one<';'> > {};
	struct breakstatement : seq<TAO_PEGTL_STRING("break"), sblk, one<';'> > {};
	struct whilestatement : seq<TAO_PEGTL_STRING("while"), must< sblk, one<'('>, sblk, plus< whilecond, sblk>, one<')'>, sblk, statement > > {};
	struct nextstatement : expression {};
	struct forstatement : seq<TAO_PEGTL_STRING("for"), must< sblk, one<'('>, sblk, sor< forvardecl, expression >, sblk, one<';'>, sblk, forcond, sblk, one<';'>, sblk, nextstatement, sblk, one<')'>, sblk, statement > > {};
	struct dowhilestatement : seq<TAO_PEGTL_STRING("do"), must< sblk, statement, sblk, TAO_PEGTL_STRING("while"), sblk, one<'('>, sblk, plus< dowhilecond, sblk>, one<')'>, sblk, one<';'> > > {};
	struct elsestatement : seq<TAO_PEGTL_STRING("else"), sblk, statement > {};
	struct ifstatement : seq<TAO_PEGTL_STRING("if"), sblk, one<'('>, sblk, plus< ifcond, sblk>, one<')'>, sblk, statement, opt< sblk, elsestatement > > {};
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

	struct unknown : until< one<';'>, any > {};
	struct program : until< eof, sor<	blank_line,
		directive,
		declaration,
		unknown
	> > {};

	template<> struct maction< funcargexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCARGEXPRESSION : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< funccall >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCCALL : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< expressionerror >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("EXPRESSIONERROR : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< applyunaryexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("APPLYUNARYEXPRESSION : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< productexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("PRODUCTEXPRESSION : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< sumexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("SUMEXPRESSION : %s\n", in.string().c_str());
		}
	};

	template<ERelopType T> struct maction< relop<T> >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("RELOP : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< literaldecimal >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALDECIMAL : %s\n", in.string().c_str());
			Compiler.SetCurLiteral(LiteralType::Numeric, std::stoi(in.string()));
		}
	};
	template<> struct maction< literalchar >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALCHAR : %s\n", in.string().c_str());
			Compiler.SetCurLiteral(LiteralType::Numeric, std::string(in.string())[1]);
		}
	};

	template<> struct maction< literalhexa >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALHEXA : %s\n", in.string().c_str());
			Compiler.SetCurLiteral(LiteralType::Numeric, std::stoi(in.string(), nullptr, 16));
		}
	};

	template<> struct maction< literaltrue >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALTRUE\n");
			Compiler.SetCurLiteral(LiteralType::Boolean, 1);
		}
	};

	template<> struct maction< literalfalse >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALFALSE\n");
			Compiler.SetCurLiteral(LiteralType::Boolean, 0);
		}
	};

	template<> struct maction< literalnullptr >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LITERALNULLPTR\n");
			Compiler.SetCurLiteral(LiteralType::Nullptr);
		}
	};

	template<> struct maction< applynotexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("! EXPR : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< relexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("REL EXPR : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< orexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("|| EXPR : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< andexpression >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("&& EXPR : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< gotostatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("GOTOSTATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< ifcond >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("IFCOND : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< dowhilecond >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("DOWHILECOND : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< whilecond >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("WHILECOND : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< memberid >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("MEMBERID : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< arrayindex >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ARRAYINDEX : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< arrayaccess >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ARRAYACCESS : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< lvalue >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LVALUE : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< assignment >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ASSIGNMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< forstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FORSTATEMENT\n");
		}
	};

	template<> struct maction< forcond >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FORCOND : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< nextstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("NEXTSTATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< ifstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("IFSTATEMENT\n");
		}
	};

	template<> struct maction< elsestatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ELSESTATEMENT\n");
		}
	};

	template<> struct maction< dowhilestatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("DO WHILE STATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< whilestatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("WHILE STATEMENT : %s\n", in.string().c_str());
		}

		template< typename Input > static void failure(Input& in, Compiler& Compiler)
		{
			DLOG("!!!! WHILE STATEMENT FAILURE : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< breakstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("BREAK STATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< unknownstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("UNKNOWN STATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< returnstatement >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("RETURN STATEMENT : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< funcparam >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCPARAM : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< paramtype >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("PARAMTYPE : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< paramid >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("PARAMID : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< localscope >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LOCALSCOPE END : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< labelid >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LABELID : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< label >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LABEL : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< unknown >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("UNKNOWN : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< scopestart >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("SCOPE START : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< scope >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("SCOPE END : %s\n", in.string().c_str());
		}
	};


	template<> struct maction< vartype >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("VARTYPE : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< functype >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCTYPE : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< staticarraysize >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ARRAY SIZE : %s\n", in.string().c_str());
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
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCID : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< identifier >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("ID : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< funcdecl >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("FUNCDECL IS VALID\n");
		}
	};

	template<> struct maction< vardeclid >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			Compiler.CurVarDecl.Identifier = in.string();
			Compiler.CurVarDecl.StaticInit.reset();
		}
	};

	template<> struct maction< globalvardecl >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("GLOBALVARDECL : %s\n", in.string().c_str());
			Compiler.ValidateGlobalVar();
		}
	};

	template<> struct maction< localvardecl >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("LOCALVARDECL : %s\n", in.string().c_str());
		}
	};

	template<> struct maction< varinit >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("VARINIT : %s\n", in.string().c_str());
			Compiler.CurVarDecl.StaticInit = Compiler.CurLiteralValue;
		}
	};

	template<> struct maction< vardecl >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("VARDECL : %s\n", in.string().c_str());

			const auto pos = in.position();
			Compiler.PushPendingVarDecl(pos.source, pos.line);
		}
	};

	template<> struct maction< directive >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			DLOG("DIRECTIVE : %s\n", in.string().c_str());
		}
	};


	template<> struct maction< program >
	{
		template< typename Input > static void apply(const Input& in, Compiler& Compiler)
		{
			std::cout << "END OF PROGRAM.\n";
		}
	};


	template< typename Rule > struct mcontrol : normal< Rule > {};

	template<> struct mcontrol< preprocess > : normal< preprocess >
	{
		template< typename Input >
		static void start(Input& in, std::string& out)
		{
			std::cout << "START OF PREPROCESS.\n";
		}

		template< typename Input >
		static void success(Input& in, std::string& out)
		{
			std::cout << "END OF PREPROCESS.\n";
		}

		template< typename Input >
		static void failure(Input& in, std::string& out)
		{
			std::cout << "FAILURE OF PREPROCESS.\n";
		}

		template< typename Input >
		static void raise(const Input& in, std::string& out)
		{
			throw parse_error(internal::demangle< program >(), in);
		}
	};

	template<> struct mcontrol< program > : normal< program >
	{
		template< typename Input >
		static void start(Input& in, Compiler& Compiler)
		{
			std::cout << "START OF COMPILATION.\n";
		}

		template< typename Input >
		static void success(Input& in, Compiler& Compiler)
		{
			std::cout << "END OF COMPILATION.\n";
		}

		template< typename Input >
		static void failure(Input& in, Compiler& Compiler)
		{
			std::cout << "FAILURE OF COMPILATION.\n";
		}

		template< typename Input >
		static void raise(const Input& in, Compiler& Compiler)
		{
			throw parse_error(internal::demangle< program >(), in);
		}
	};
}
