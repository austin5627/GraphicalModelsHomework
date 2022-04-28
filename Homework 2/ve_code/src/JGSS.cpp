#include "JG.h"
#include "myRandom.h"
void createFunction(Function& function, Function& out);
void JGNodeSS::initialize()
{
	for (int i = 0; i < functions.size(); i++) {
		if (functions[i])
			delete (functions[i]);
	}
	functions=vector<Function*>(original_functions.size());
	for(int i=0;i<original_functions.size();i++){
		functions[i]=new Function();
		createFunction(*original_functions[i],*functions[i]);
	}
	for(int i=0;i<edges_.size();i++)
	{
		edges_[i]->message1()=Function();
		edges_[i]->message2()=Function();
	}
}
void JGNodeSS::addFunction(Function& function)
{
	original_functions.push_back(&function);
}
void JGNodeSS::updateVariables()
{
	vector<Function*> all_functions;
	compileAllFunctions(all_functions);
	variables()=vector<Variable*>();
	for(int i=0;i<all_functions.size();i++)
		do_set_union(variables_,all_functions[i]->variables(),variables_,less_than_comparator_variable);
}
void JGNodeSS::compileAllFunctions(vector<Function*>& all_functions)
{
	all_functions=functions;
	for(int i=0;i<edges_.size();i++)
	{
		if(edges_[i]->node1()==this)
		{
			if(edges_[i]->message2().variables().empty())
				continue;
			all_functions.push_back(&(edges_[i]->message2()));
		}
		else
		{
			if(edges_[i]->message1().variables().empty())
				continue;
			all_functions.push_back(&(edges_[i]->message1()));
		}
	}
}
void JGNodeSS::getCF(vector<Variable*>& cond_variables_, Variable* marg_variable, CPT& cf)
{
	vector<Function*> all_functions;
	compileAllFunctions(all_functions);
	
	vector<Variable*> all_variables,wanted_variables,marg_vars,cond_variables;
	marg_vars.push_back(marg_variable);
	for(int i=0;i<all_functions.size();i++)
	{
		do_set_union(all_variables,all_functions[i]->variables(),all_variables,less_than_comparator_variable);
	}
	
	wanted_variables=cond_variables_;
	wanted_variables.push_back(marg_variable);
	sort(wanted_variables.begin(),wanted_variables.end(),less_than_comparator_variable);

	do_set_intersection(wanted_variables,all_variables,all_variables,less_than_comparator_variable);
	do_set_intersection(all_variables,cond_variables_,cond_variables,less_than_comparator_variable);
	do_set_intersection(all_variables,marg_vars,marg_vars,less_than_comparator_variable);
	assert(!marg_vars.empty());

	Function func;
	

	Function::multiplyAndMarginalize(all_variables,all_functions,func);
	cf=CPT(func,marg_variable,cond_variables);
	
	
}

void JGNodeSS::getMarginal(vector<Variable*>& marg_variables, Function& function)
{
	vector<Function*> all_functions;
	compileAllFunctions(all_functions);
	Function::multiplyAndMarginalize(marg_variables,all_functions,function);
}


JGEdgeSS::JGEdgeSS(JGNodeSS* ss_node1__, JGNodeSS* ss_node2__): JGEdge(),ss_node1_(ss_node1__),ss_node2_(ss_node2__)
{
	node1_=ss_node1_;
	node2_=ss_node2_;
	node1_to_node2_message_=new Function();
	node2_to_node1_message_=new Function();
}
void JGEdgeSS::initialize()
{
	message1()=Function();
	message2()=Function();
}

void JGEdgeSS::sendMessage1to2()
{
	vector<Function*> all_functions;
	all_functions=ss_node1_->functions;
	
	for(int i=0;i<ss_node1_->edges().size();i++)
	{
		JGEdge* curr_edge=ss_node1_->edges()[i];
		if(curr_edge==this)
		{
			//cout<<"Here .. ";
			continue;
		}
		if(curr_edge->node1()==ss_node1_)
		{
			if(curr_edge->message2().variables().empty())
				continue;
			all_functions.push_back(&curr_edge->message2());
		}
		else
		{
			if(curr_edge->message1().variables().empty())
				continue;
			all_functions.push_back(&curr_edge->message1());
		}
	}
	//cout<<"Calling multiply and marginalize ";
	//cout<<ss_node1_->id()<<" to "<<ss_node2_->id()<<endl;
	Function::multiplyAndMarginalize(this->variables(),all_functions,*node1_to_node2_message_);
	//Function::dummy_multiplyMarginalize(this->variables(),all_functions,this->message1());
}

void JGEdgeSS::sendMessage2to1()
{
	vector<Function*> all_functions;
	all_functions=ss_node2_->functions;
	for(int i=0;i<ss_node2_->edges().size();i++)
	{
		JGEdge* curr_edge=ss_node2_->edges()[i];
		if(curr_edge==this)
		{
			//cout<<"Here .. ";
			continue;
		}
		if(curr_edge->node1()==ss_node2_)
		{
			if(curr_edge->message2().variables().empty())
				continue;
			all_functions.push_back(&curr_edge->message2());
		}
		else
		{
			if(curr_edge->message1().variables().empty())
				continue;
			all_functions.push_back(&curr_edge->message1());
		}
	}
	//cout<<"Calling multiply and marginalize ";
	//cout<<ss_node2_->id()<<" to "<<ss_node1_->id()<<endl;
	Function::multiplyAndMarginalize(this->variables(),all_functions,*node2_to_node1_message_);
	//Function::dummy_multiplyMarginalize(this->variables(),all_functions,this->message2());
}
