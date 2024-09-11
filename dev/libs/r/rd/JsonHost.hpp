#pragma once

#include "Agent.hpp"

namespace rd {
	class JsonHost : public Agent {
	public:
		JSONResource			root;

		bool					debug = false;
		bool					init = false;
		bool					watchFile = true;

		Pasta::File *			file = nullptr;
		std::string				filePath;
		Pasta::u64				lastReadTime = 0;

		float					DELTA_CHECK = 0.1f;
		float					checkProgress = 0.0f;
		Sig						onAfterEval;

		double					getFloat(const char * label, const char * );
	public:
		JsonHost(std::string & _scriptPath, AgentList * al);
		void cleanRsc();
		virtual					~JsonHost();

		virtual void			update(double dt);
		virtual void			eval();

		std::unordered_map<std::string, Json::Value>	all;
	};
}//end namespace rd