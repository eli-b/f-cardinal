﻿/* Copyright (C) Jiaoyang Li
* Unauthorized copying of this file, via any medium is strictly prohibited
* Confidential
* Written by Jiaoyang Li <jiaoyanl@usc.edu>, May 2019
*/

/*driver.cpp
* Solve a MAPF instance on 2D grids.
*/
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include "CBS.h"


int main(int argc, char** argv)
{
	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("map,m", po::value<string>()->required(), "input file for map")
		("agents,a", po::value<string>()->required(), "input file for agents")
		("output,o", po::value<string>(), "output file for schedule")
		("rows", po::value<int>()->default_value(0), "number of rows")
		("cols", po::value<int>()->default_value(0), "number of columns")
		("obs", po::value<int>()->default_value(0), "number of obstacles")
		("warehouseWidth", po::value<int>()->default_value(0), "width of working stations on both sides, for generating instacnes")
		("screen,s", po::value<int>()->default_value(0), "screen option (0: none; 1: results; 2:all)")
		("seed,d", po::value<int>()->default_value(0), "random seed")
		("agentNum,k", po::value<int>()->default_value(0), "number of agents")
		("cutoffTime,t", po::value<double>()->default_value(7200), "cutoff time (seconds)")

		("PC,p", po::value<bool>()->default_value(true), "conflict prioirtization")
		("bypass", po::value<bool>()->default_value(true), "Bypass1")
		("heuristics,h", po::value<string>()->default_value("NONE"), "heuristics for the high-level search (NONE, CG,DG, WDG)")
		("disjointSplitting", po::value<bool>()->default_value(true), "disjoint splitting")
		("rectangleReasoning", po::value<bool>()->default_value(false), "Using rectangle reasoning")
		("corridorReasoning", po::value<bool>()->default_value(false), "Using corridor reasoning")
		("targetReasoning", po::value<bool>()->default_value(false), "Using target reasoning")
		("restart", po::value<int>()->default_value(1), "number of restart times (at least 1)")
		("sipp", po::value<bool>()->default_value(true), "using sipp as the single agent solver")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	po::notify(vm);

	if (vm["sipp"].as<bool>() && vm["targetReasoning"].as<bool>())
	{
		cerr << "SIPP cannot work together with target reasoning!" << endl;
		return -1;
	}

	heuristics_type h;
	if (vm["heuristics"].as<string>() == "NONE")
		h = heuristics_type::NONE;
	else if (vm["heuristics"].as<string>() == "CG")
		h = heuristics_type::CG;
	else if (vm["heuristics"].as<string>() == "DG")
		h = heuristics_type::DG;
	else if (vm["heuristics"].as<string>() == "WDG")
		h = heuristics_type::WDG;
	else
	{
		cout << "WRONG HEURISTICS NAME!" << endl;
		return -1;
	}

	srand((int)time(0));

	// load the instance
	Instance instance(vm["map"].as<string>(), vm["agents"].as<string>(),
		vm["agentNum"].as<int>(),
		vm["rows"].as<int>(), vm["cols"].as<int>(), vm["obs"].as<int>(), vm["warehouseWidth"].as<int>());

	srand(vm["seed"].as<int>());

	int runs = vm["restart"].as<int>();
	
	// initialize the solver
	CBS cbs(instance, 1.0, h, vm["PC"].as<bool>(), vm["sipp"].as<bool>(), vm["screen"].as<int>());
	cbs.disjoint_splitting = vm["disjointSplitting"].as<bool>();
	cbs.bypass = vm["bypass"].as<bool>();
	cbs.rectangle_reasoning = vm["rectangleReasoning"].as<bool>();
	cbs.corridor_reasoning = vm["corridorReasoning"].as<bool>();
	cbs.target_reasoning = vm["targetReasoning"].as<bool>();


	double runtime = 0;
	int initial_h = 0;
	for (int i = 0; i < runs; i++)
	{
		cbs.clear();
		cbs.runICBSSearch(vm["cutoffTime"].as<double>(), initial_h);
		runtime += cbs.runtime;
		if (cbs.solution_found)
			break;
		initial_h = (int)cbs.min_f_val - cbs.dummy_start->g_val;
	}
	cbs.runtime = runtime;
	if (vm.count("output"))
		cbs.saveResults(vm["output"].as<string>(), vm["agents"].as<string>());
	cbs.clearSearchEngines();
	return 0;

}
