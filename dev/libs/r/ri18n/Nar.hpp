#pragma once

#include <string>
#include <functional>

namespace ri18n{
	enum class AstNodeType : u32 {
		Seq,//(s0:Ast, s1 : Ast);
		Sentence,//(l : String);
		Em,//(sub : Ast); //*
		Strong,//(sub : Ast); //**
		Event,//(str:String); // [ev]
		CondEvent,//(str:String); {?}
		UniqueEvent,//(str:String); {ev}
		CondUniqueEvent,//(str:String);[?ev]
		ImportantEvent,//(str:String);
		Tag,//(tag:String, sub : Ast);
		TagFrom,//(tag:String, isRestEmpty : Bool);
		Nop,
		Script//(str:String); // ::sc:: 
	};

	struct AstNode {
		static int		UID;

		int				uid = 0;
		AstNodeType		type = AstNodeType::Nop;
		AstNode*		a0Data = nullptr;
		AstNode*		a1Data = nullptr;
		std::string		strData;
		Str				repl;
		bool			isRestEmpty=false;

		explicit AstNode(AstNodeType t) {
			type = t;
			uid = UID++;
		}

		explicit AstNode(AstNodeType t,const std::string & str) {
			type = t;
			strData = str;
			uid = UID++;
		}
		
		explicit AstNode(AstNodeType t, AstNode * a0Data, AstNode * a1Data = nullptr ) {
			type = t;
			this->a0Data = a0Data;
			this->a1Data = a1Data;
			uid = UID++;
		}

		explicit AstNode(AstNodeType t, const std::string & str, AstNode * a0Data) {
			type = t;
			strData = str;
			this->a0Data = a0Data;
			uid = UID++;
		}

		explicit AstNode(AstNodeType t, const std::string & str, bool isRestEmpty) {
			type = t;
			strData = str;
			this->isRestEmpty = isRestEmpty;
			uid = UID++;
		}

		~AstNode() {
			if (a0Data) delete(a0Data); a0Data = nullptr;
			if (a1Data) delete(a1Data); a1Data = nullptr;
			type = AstNodeType::Nop;
		}

		int countType(AstNodeType _type);
		void traverseScripts(std::function<void(const std::string & literals)>);
		static std::string to_string(AstNodeType t);

		void im();
	};


	class Nar {
	public:
					Nar();
					~Nar();

		AstNode *	make(const std::string & str);
		std::string	stringify(AstNode*node);
	protected:
		eastl::vector<AstNode*> nodes;
	};
}
