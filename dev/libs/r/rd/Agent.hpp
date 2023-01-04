#pragma once

#include <string>
#include <vector>
#include <functional>

namespace rd {
	class Agent;

	class AgentList {
	public:

		std::string			name;
		std::vector<Agent*> repo;

	public:
						AgentList() {}
		virtual			~AgentList();

		void			push_back(Agent * p);
		void			add(Agent * p);
		bool			has(const char* name);
		bool			has(Agent & ag);

		void			remove(Agent * p);
		void			dispose();
		
		void			update(double dt);

		void			deleteAllChildren();
	};

	class Agent {
		public:

			static	int _UID	;
					int _id		= _UID++;

			std::string			name;
			AgentList *			list = nullptr;
			bool				deleteSelf = false;

								Agent(AgentList* list = nullptr);
			virtual				~Agent();
			virtual void		update(double dt) {};

			virtual void		dispose();
			virtual void		detach();
	};

	class AnonAgent : public Agent {
		public:
			std::function<void(double)> cbk;

			AnonAgent(AgentList* list = nullptr) : Agent(list) {

			};

			AnonAgent(std::function<void(void)> cbk, AgentList * list = nullptr) : Agent(list) {
				this->cbk = [=](double dt) { cbk(); };
			};

			AnonAgent(std::function<void(double)> cbk, AgentList * list = nullptr) : Agent(list) {
				this->cbk = cbk;
			};

			void set(std::function<void(void)> cbk, AgentList* list = nullptr){
				this->cbk = [=](double dt) { cbk(); };
				if(list)
					list->add(this);
			}

			virtual ~AnonAgent() {
				cbk = nullptr;
			};

			virtual void dispose() override;

			virtual void update(double dt) override {
				Agent::update(dt);
				if(cbk) cbk(dt);
			}
	};

}