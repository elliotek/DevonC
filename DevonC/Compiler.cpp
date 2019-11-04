#include "Compiler.h"

using namespace DevonC;

int Compiler::TypeSize(VarType _Type)
{
	switch (_Type)
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

void Compiler::ErrorMessage(EErrorCode ErrorCode, size_t line)
{
	std::cout << IncludeStack.top() << " Line " << line << " : ";

	switch (ErrorCode)
	{
	case EErrorCode::IncludeFileFail:
		std::cout << "#include failed : \"" << LastFilename << "\"";
		break;

	case EErrorCode::VoidVarDecl:
		std::cout << "\'void\' is not a valid variable type.";
		break;

	case EErrorCode::BadInitializerLiteralType:
		std::cout << "Initializer literal is incompatible with the type of the variable being declared.";
		break;
	}

	std::cout << std::endl;
	++NbErrors;
}

bool Compiler::Compile(const char * _Filename)
{
	IncludeStack.push(_Filename);
	
	bool Ret = true;

	try
	{
		std::string PreProcessedStr;
		file_input FileInput(_Filename);
		parse<preprocess, maction, mcontrol>(FileInput, PreProcessedStr);

		string_input PreProcessedStrInput(PreProcessedStr, "");
		parse<program, maction, mcontrol>(PreProcessedStrInput, *this);
	}
	catch(std::exception & err)
	{
		std::cout << err.what() << "\n";
		Ret = false;
	}

	IncludeStack.pop();

	return Ret;
}

void Compiler::SetCurLiteral(LiteralType _Type, int _Value)
{
	CurLiteralValue = _Value;
	CurLiteralType = _Type;
}

void Compiler::PushPendingVarDecl(const size_t line)
{
	if (CurVarDecl.Type == VarType::Void && CurVarDecl.PointerIndirection == 0)
	{
		ErrorMessage(EErrorCode::VoidVarDecl, line);
		CurVarDecl.Type = VarType::Int;
	}

	if (CurVarDecl.StaticInit.has_value()
		&& CurVarDecl.PointerIndirection > 0
		&& CurLiteralType != LiteralType::Nullptr
		)
		ErrorMessage(EErrorCode::BadInitializerLiteralType, line);

	PendingVarDecls.push_back(std::move(CurVarDecl));
}

void Compiler::ValidateGlobalVar()
{
	GlobalVars.insert(GlobalVars.end(), PendingVarDecls.begin(), PendingVarDecls.end());
	PendingVarDecls.clear();
}

void Compiler::DumpDebug()
{
	std::cout << "\nGlobals:\n";
	for (auto var : GlobalVars)
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
		for (int i = 0; i < var.PointerIndirection; i++)
			std::cout << "*";
		std::cout << " " << var.Identifier;

		for (auto arraySize : var.ArraySizes)
			std::cout << "[" << arraySize << "]";

		if (var.StaticInit.has_value())
			std::cout << " = " << var.StaticInit.value() << "\n";

		std::cout << "\n";
	}
}

