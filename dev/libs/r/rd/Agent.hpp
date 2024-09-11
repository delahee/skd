#pragma once

namespace rd {
	class Agent;
	class DelayedAgent;
	class Vars;

	class AgentList {
	public:
		Str					name;
		std::vector<Agent*> repo;
		std::vector<Agent*> garbage;
		bool				iterInProgress = false;
	public:
							AgentList() {}
		virtual				~AgentList();

		void				push_back(Agent * p);
		void				push_back(std::function<void(double)> proc);

		void				add(Agent * p);
		void				add(std::function<void(double)> proc);
		void				add(std::function<void(void)> proc);
		void				add(const char * name, std::function<void(double)> proc);
		void				add(const char * name, std::function<void(void)> proc);

		bool				has(const char* name);
		bool				has(Agent & ag);

		void				destroy(Agent * p);
		void				remove(Agent * p);
		void				dispose();
		
		void				update(double dt);
		void				delay(double ms, r::proc p);

		void				deleteAllChildren();
		rd::Agent*			getByName(const char* str);
		rd::Agent*			getByTag(const char* str);
		inline rd::Agent*	findByName(const char* str) { return getByName(str); };
		inline rd::Agent*	findByTag(const char* str) { return getByTag(str); };
		rd::Agent*			first();
	};

	class Agent {
		public:
			static	int _UID	;
					int _id		= _UID++;

			Str					name;
			Str					tags;
			AgentList *			list = nullptr;

			//do not use this without extreme care, most probably only for VFX, if you want a fire and forget thing, rather consider using an enabled on/off agent
			bool				deleteSelf = false;

								Agent(const char * name, AgentList* list = nullptr);
								Agent(AgentList* list = nullptr);
			virtual				~Agent();

			//sets a pseudo name with a uid glued, allows to setup family for most object but forbids snatching by name
			void				setName(const char* name);

			//sets the name allowing snatching by name
			void				setUniqueName(const char* name);
			virtual void		update(double dt) {};

			virtual void		onDispose();
			virtual void		dispose();
			virtual void		detach();
			virtual bool		im();

			//manages "destroy" during iteration schema
			void				safeDestruction();
	};

	class AnonAgent : public Agent {
		public:
			std::function<void(double)> cbk;
			rd::Vars*					vars{};

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

			virtual ~AnonAgent();
			virtual void onDispose() override;
			virtual void update(double dt) override {
				Agent::update(dt);
				if (cbk) cbk(dt);
			};
	};

	
	
}