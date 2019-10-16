#pragma once

#include <string>
#include <vector>
#include <optional>

namespace DevonC
{
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

		union
		{
			int Int;
			char Char;
			short Short;
			bool Bool;
			int Pointer;
		} InitialValue;

		Variable() {};
		Variable(const std::string & _Identifier) : Identifier(_Identifier) {};
		Variable(const std::string && _Identifier) : Identifier(_Identifier) {};
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
		Function(const std::string & _Identifier) : Identifier(_Identifier) {};
	};

	class Compiler
	{
		std::vector<Variable>	GlobalVars;
		std::vector<Function>	Functions;
		std::vector<Scope>		ScopeStack;

		std::vector<Variable>	PendingVarDecls;

		int TypeSize(VarType _Type)
		{
			switch(_Type)
			{
			case VarType::Void:		return 0;
			case VarType::Char:		return 1;
			case VarType::Bool:		return 2;
			case VarType::Short:	return 2;
			case VarType::Pointer:	return 4;
			case VarType::Int:		return 4;
			default:				return -1;
			}
		}

	public:
		Compiler()
		{
		}

		~Compiler()
		{
			std::cout << "\nGlobals:\n";
			for(auto var : GlobalVars)
			{
				std::cout << "\t";
				switch (var.Type)
				{
				case VarType::Void:		std::cout << "Void";	break;
				case VarType::Char:		std::cout << "Char";	break;
				case VarType::Bool:		std::cout << "Bool";	break;
				case VarType::Short:	std::cout << "Short";	break;
				case VarType::Int:		std::cout << "Int";		break;
				default:				std::cout << "Unknown-Type";	break;
				}
				for(int i = 0; i < var.PointerIndirection; i++)
					std::cout << "*";
				std::cout << " " << var.Identifier;

				for(auto arraySize : var.ArraySizes)
					std::cout << "[" << arraySize << "]";

				std::cout << "\n";
			}
		}

		Variable CurVarDecl;
		int CurLiteralValue;

		void PushPendingVarDecl()
		{
			PendingVarDecls.push_back(std::move(CurVarDecl));
		}

		void ValidateGlobalVar()
		{
			GlobalVars.insert(GlobalVars.end(), PendingVarDecls.begin(), PendingVarDecls.end());
			PendingVarDecls.clear();
		}
	};
}