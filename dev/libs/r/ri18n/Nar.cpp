#include "stdafx.h"

#include "Lex.hpp"
#include "Nar.hpp"
#include "Gram.hpp"

using namespace std;
using namespace ri18n;


Nar::Nar() {

}

Nar::~Nar()
{
}

AstNode * Nar::make(const std::string & str) {
	bool inspect = false;
#ifdef debug
	//if ( str.indexOf("stupides ou bien") >= 0 ){
		//trace("B:"+str);
		//haxe.Utf8.iter( str, function(code){
			//trace( code+" " + fromCharCode(code));
		//});
		//inspect = true;
	//}
#endif

	Lex l;
	vector<Lexem> lexs = l.parse(str);
	Gram g;

	//checking funcs
	//cout << Lex::dumpLexems(lexs) << endl;

	AstNode * n = g.parse(lexs);
#ifdef debug
	//if ( inspect )
		//trace(t);
#endif
	return n;
}

//if stack breaks here please make it terminal rec
string Nar::stringify(AstNode * node)
{
	string res;
	AstNode* cur = node;
	if (cur == nullptr) return string("");
	switch (cur->type){
		case AstNodeType::Nop: {
			res += "Nop ";
		}
		break;
		case AstNodeType::Script: {
			res += "Script("+ cur->strData+") ";
		}
		break;
		case AstNodeType::Sentence: {
			res += "Sentence(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::Event: {
			res += "Event(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::CondEvent: {
			res += "CondEvent(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::UniqueEvent: {
			res += "UniqueEvent(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::CondUniqueEvent: {
			res += "CondUniqueEvent(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::ImportantEvent: {
			res += "ImportantEvent(" + cur->strData + ") ";
		}
		break;
		case AstNodeType::Seq:{
			res += "Seq( " + stringify(cur->a0Data) +","+ stringify(cur->a1Data) +" ) ";
		}
		break;
		case AstNodeType::Em:{
			res += "Em( " + stringify(cur->a0Data) + " ) ";
		}
		break;
		case AstNodeType::Strong:{
			res += "Strong( " + stringify(cur->a0Data) + " ) ";
		}
		break;
		case AstNodeType::Tag:{
			res += "Tag( id:" +cur->strData+" content:"+ stringify(cur->a0Data) +" ) ";
		}
		break;
		case AstNodeType::TagFrom:{
			res += "TagFrom( id:" + cur->strData + " content:" + stringify(cur->a0Data) +" rst:"+ to_string(cur->isRestEmpty)+" ) ";
		}
		break;
		default:{
			res += "Error ";
		}
		break;
	}
	return res;
}

int AstNode::UID = 0;

int AstNode::countType(AstNodeType _type) {
	int base = (type == _type) ? 1 : 0;
	int l0 = (a0Data == nullptr) ? 0 : a0Data->countType(_type);
	int l1 = (a1Data == nullptr) ? 0 : a1Data->countType(_type);
	return base + l0 + l1;
}

void AstNode::traverseScripts(std::function<void(const string&literals)> f){
	switch (type){
		case ri18n::AstNodeType::Script:
			f(this->strData);
			break;
		default:
			break;
	}
	if (a0Data != nullptr) a0Data->traverseScripts(f);
	if (a1Data != nullptr) a1Data->traverseScripts(f);
}

string AstNode::to_string(AstNodeType t){
	switch (t) {
		case AstNodeType::Seq: return "Seq";  break;
		case AstNodeType::Sentence: return "Sentence";  break;
		case AstNodeType::Em: return "Em";  break;
		case AstNodeType::Strong: return "Strong";  break;
		case AstNodeType::Event: return "Event";  break;
		case AstNodeType::CondEvent: return "CondEvent";  break;
		case AstNodeType::UniqueEvent: return "UniqueEvent";  break;
		case AstNodeType::CondUniqueEvent: return "CondUniqueEvent";  break;
		case AstNodeType::ImportantEvent: return "ImportantEvent";  break;
		case AstNodeType::Tag: return "Tag";  break;
		case AstNodeType::TagFrom: return "TagFrom";  break;
		case AstNodeType::Nop: return "Nop";  break;
		case AstNodeType::Script: return "Script";  break;

	}
	return "";
}

static void imNar(ri18n::AstNode* ast) {
	if (!ast)
		return;
	using namespace ri18n;
	using namespace ImGui;

	switch (ast->type) {

	case AstNodeType::Seq:
		ImGui::Text("Seq");
		Indent();
		imNar(ast->a0Data);
		imNar(ast->a1Data);
		Unindent();
		break;
	case AstNodeType::Sentence:
		ImGui::Text("\""s + ast->strData + "\""); 
		break;
	case AstNodeType::Em://[[fallthrough]]
	case AstNodeType::Strong:
		Indent();
		imNar(ast->a0Data);
		Unindent();
		break;
	case AstNodeType::Script:
		ImGui::Text(Nar().stringify(ast));
		break;
	case AstNodeType::CondEvent:
	case AstNodeType::UniqueEvent:
	case AstNodeType::CondUniqueEvent:
	case AstNodeType::ImportantEvent:
	case AstNodeType::Event:
		ImGui::Text(Nar().stringify(ast));
		break;

	case AstNodeType::Tag:
		ImGui::Text("<"s + ast->strData + ">");
		Indent();
		imNar(ast->a0Data);
		Unindent();
		break;
	case AstNodeType::TagFrom:
		ImGui::Text("[>"s + ast->strData + "]");
		Indent();
		imNar(ast->a0Data);
		Unindent();
		break;
	}


}
void ri18n::AstNode::im(){
	using namespace ImGui;
	imNar(this);
}

