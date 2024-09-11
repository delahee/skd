#pragma once

#include "rd/Agent.hpp"

namespace r2 {
	//helps track the node and delete it at the appropriate time
	class NodeAgent : public rd::Agent {
		typedef rd::Agent Super;
	public:
		std::string								type;
		rd::Vars								data;
		r2::Node*								node = 0;
		std::function<void(r2::Node*, double)>	cbk;

					NodeAgent(r2::Node* n = 0, rd::AgentList* parent = 0);
		virtual		~NodeAgent();
		virtual void onDispose() override{
			cbk = {};
		};

		NodeAgent(std::function<void(void)> cbk, rd::AgentList* list = nullptr) : Agent(list) {
			this->cbk = [=](r2::Node*, double) { cbk(); };
		};

		NodeAgent(std::function<void(r2::Node*, double)> cbk, rd::AgentList* list = nullptr) : Agent(list) {
			this->cbk = cbk;
		};

		virtual void onSerialized() {};
		virtual void onDeserialized() {};

		virtual void update(double dt) override;
		virtual void serialize(Pasta::JReflect& jr, const char* name = 0);

		virtual bool im() override;

		virtual void apply();
	};

	class NodeAgentList : public NodeAgent {
		typedef NodeAgent Super;
	public:
		rd::AgentList	agents;

						NodeAgentList(r2::Node* n = 0, rd::AgentList* parent = 0);
		virtual			~NodeAgentList();

		virtual void	update(double dt) override;
		virtual bool	im() override;

		void			serializeArray(Pasta::JReflect& jr, eastl::vector<r2::NodeAgent*>& arr, u32 size, const char* name);

		virtual void	serialize(Pasta::JReflect& jr, const char* name = 0) override;

		eastl::vector<NodeAgent*> getNodeAgents();
	};

	struct NodeAgentFactory {
		struct NodeAgentEntry {
			Str typeName;
			std::function<NodeAgent* ()> ctor;
		};
		static eastl::vector<NodeAgentEntry>					customCreates;

		static constexpr char									NA_LIST[] = "rd::na::list";

		static void												addCustomType(const char*, std::function<NodeAgent* ()>);
		static NodeAgent*										create(const char* nodeName);
		static bool												imCreate(const char * prefix, NodeAgent*& res);
	};
}
