#include "stdafx.h"
#include <regex>

#include "RandText.hpp"
#include "Nar.hpp"

using namespace ri18n;
ri18n::RandText::RandText() {
}


RandText* ri18n::RandText::parse(const char* src) {
	Nar n;
	RandText* res = new RandText();
	const char* head = src;
	const char* lookahead = head;
	const char* lastSpace = head;
	struct L {
		const char* start = 0;
		const char* content = 0;
		const char* rest = 0;
	};

	std::vector<const char*> starts;
	std::vector<const char*> litStart;
	std::vector<const char*> litEnd;
	auto isColon = rd::String::isColon;
	auto skipToDoubleColon = [](const char* c) {
		return strstr(c, "::");
	};

	while (lookahead && *lookahead && rd::String::isIdentifierCharacter(*lookahead)) {
		lookahead++;
		if (*lookahead == ':' && !isColon(lookahead + 1) && !(*(lookahead - 1) == ' ')) {
			starts.push_back(head);//scan full key
			head = lookahead + 1;
			lookahead = head;

			litStart.push_back(lookahead + 1);
			const char* literal = lookahead;
			while (!rd::String::isEmpty(literal)) {
				if (*literal == ' ')
					lastSpace = literal;
				if (isColon(literal)) {
					if (isColon(literal + 1)) {
						literal = skipToDoubleColon(literal + 2);
						if (!*literal)
							break;
						literal += 2;//skip scripting colon
					}
					else {
						litEnd.push_back(lastSpace);//get back to last space and let loop scan key
						head = lastSpace + 1;
						lookahead = head;
						break;
					}
				}
				literal++;
			}
		}
	}

	for (int i = 0; i < starts.size(); ++i) {
		int isLast = (i == starts.size() - 1);
		const char* kStart = starts[i];
		rd::String::ltrim(kStart);
		Str key = rd::String::sub(kStart, strstr(kStart, ":"));
		Str content = isLast ? litStart[i] : rd::String::sub(litStart[i], litEnd[i]);

		RandTextEntry& re = *(new RandTextEntry());
		re.idx = i;
		re.key = std::move(key);
		re.content = content;
		if (strstr(content.c_str(), "<br/>"))
			re.chunks = rd::String::split(content.c_str(), "<br/> ");
		else
			re.chunks = rd::String::split(content.c_str(), "\n");
		//for (auto& c : re.chunks) 
		//	re.asts.push_back(n.make(c.c_str()));
		res->index.push_back(&re);
	}
	for (auto r : res->index)
		res->lines[r->key] = r;
	return res;
}


std::string ri18n::RandText::gen(const char* key, rd::Rand* random) {
	if (!random)
		random = &rd::Rand::get();
	ri18n::RandTextEntry* e = rs::Std::get(lines, key);
	if (!e) 
		return {};
	
	std::regex ident("::[a-zA-Z0-9_]+::",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	int iter = 0;
	int replaced = false;
	int maxIter = 64;
	
	if (e->chunks.empty())
		return {};

	std::string root = e->randChunk(random);
	do {
		replaced = false;
		std::smatch matches;
		if (std::regex_search(root, matches, ident)) {
			for (auto& m : matches) {
				std::string str = m.str();
				auto st = rd::String::sub(str.c_str() + 2, str.c_str() + str.size()-2);
				if (!rs::Std::exists(lines, st.c_str())) continue;
				auto en = rs::Std::get(lines, st.c_str());
				if (en->chunks.size()) {
					auto ln = en->randChunk();
					root = rd::String::replace(root, m.str(), ln);
					replaced = true;
				}
				continue;
			}
		}
		else {
			replaced = false;
			break;
		}
		iter++;
	} while (replaced && iter < maxIter);
	int here = 0;
	return root;
}

static Str s_randText;
void ri18n::RandText::im(){
	using namespace ImGui;
	if (TreeNode("Content")) {
		for (auto& p : lines) {
			if (TreeNode(p.first)) {
				p.second->im();
				TreePop();
			}
		}
		TreePop();
	}
	if (Button("gen")) 
		s_randText = gen("run");
	if(!s_randText.empty())
		Text(s_randText.c_str());
}

std::string ri18n::RandTextEntry::randChunk(rd::Rand* random){
	if (chunks.empty()) return {};
	if (random == 0)
		random = &rd::Rand::get();
	//init 
	if (bag.isEmpty()) {
		auto idx = rs::Std::zero_iota<int>(chunks.size() - 1);
		bag.set(*random, idx);
	}
	return chunks[bag.next()];//if it crashes look at the bag
}

void ri18n::RandTextEntry::im(){
	using namespace ImGui;
	LabelText("key", key.c_str());
	LabelText("content", content.c_str());
	for (auto& c : chunks) 
		Text(c);
	//bag.im();
}
