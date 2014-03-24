#include <sstream>
#include <list>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <hsfc/hsfc.h>

using namespace HSFC;
namespace py = boost::python;

/* FIXUP: NEED TO HANDLE EXCEPTIONS */


/*****************************************************************************************
 * Helper functions for PlayerGoal and PlayerMove
 *****************************************************************************************/

template<typename PairT>
std::string to_string(const PairT& obj)
{
	std::stringstream ss;
	ss << "(" << obj.first << ", " << obj.second << ")";
	return ss.str();
}

/*****************************************************************************************
 * Helper function to act as Game member function
 *****************************************************************************************/
std::vector<Player> game_players(const Game& game)
{
	std::vector<Player> plyrs;
	game.players(std::back_inserter(plyrs));
	return plyrs;
}

/*****************************************************************************************
 * Helper functions to act as State member function
 *****************************************************************************************/
std::vector<PlayerMove> state_legals(const State& state)
{
	std::vector<PlayerMove> pms;
	state.legals(std::back_inserter(pms));
	return pms;
}

std::vector<PlayerGoal> state_goals(const State& state)
{
	std::vector<PlayerGoal> pgs;
	state.goals(std::back_inserter(pgs));
	return pgs;
}

std::vector<PlayerGoal> state_playout(State& state)
{
	std::vector<PlayerGoal> pgs;
	state.playout(std::back_inserter(pgs));
	return pgs;
}

void state_play(State& state, const std::vector<PlayerMove>& jm)
{
	state.play(jm);
}


/*****************************************************************************************
 * Setup the python module
 *****************************************************************************************/

BOOST_PYTHON_MODULE(pyhsfc)
{
	py::class_<Player>("Player", py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		.def("__repr__", &Player::tostring)
		;

	py::class_<Move>("Move", py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		.def("__repr__", &Move::tostring)
		;

	py::class_<PlayerMove>("PlayerMove", py::no_init)
		.def("__repr__", &to_string<PlayerMove>)
		.def_readonly("player", &PlayerMove::first)
		.def_readonly("move", &PlayerMove::second)
		;

	py::class_<PlayerGoal>("PlayerGoal", py::no_init)
		.def("__repr__", &to_string<PlayerGoal>)
		.def_readonly("player", &PlayerGoal::first)
		.def_readonly("goal", &PlayerGoal::second)
		;

	py::class_<std::vector<Player> >("PlayerVec")
		.def(py::vector_indexing_suite<std::vector<Player> >() );

	py::class_<std::vector<PlayerMove> >("PlayerMoveVec")
		.def(py::vector_indexing_suite<std::vector<PlayerMove> >() );

	py::class_<std::vector<PlayerGoal> >("PlayerGoalVec")
		.def(py::vector_indexing_suite<std::vector<PlayerGoal> >() );

	py::class_<Game, boost::noncopyable>("Game", py::init<const std::string&>())
		.def("NumPlayers", &Game::numPlayers)
		.def("Players", &game_players)
		;

	py::class_<State>("State", py::init<Game&>())
		.def("IsTerminal", &State::isTerminal)
		.def("Legals", &state_legals)
		.def("Goals", &state_goals)
		.def("Playout", &state_playout)
		.def("Play", &state_play)
		;
}
