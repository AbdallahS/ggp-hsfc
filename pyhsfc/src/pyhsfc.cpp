#include <sstream>
#include <list>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/stl_iterator.hpp>
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

void state_play1(State& state, const std::vector<PlayerMove>& jm)
{
	state.play(jm);
}

void state_play2(State& state, const boost::python::list& obj)
{
	py::stl_input_iterator<PlayerMove> begin(obj), end;
	state.play(begin, end);
}

void state_play3(State& state, const boost::python::tuple& obj)
{
	py::stl_input_iterator<PlayerMove> begin(obj), end;
	state.play(begin, end);
}

State init_state(Game& game)
{
	return game.initState();
}


/*****************************************************************************************
 * Setup the python module
 *****************************************************************************************/

BOOST_PYTHON_MODULE(pyhsfc)
{
	// Enable user-defined docstrings and python signatures, but disable
	// the C++ signatures.
	py::docstring_options local_docstring_options(true, true, false);

	py::class_<Player>("Player", 
"Player class represents individual GDL roles. Instances of the Player class\n\
are not created explicitly but are instead created through queries to Game\n\
and State objects.\n\n\
Use the str(object) function to return a printable name of the player.",
					   py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		.def("__repr__", &Player::tostring)
		.def("__hash__", &Player::hash_value)
		.def("__eq__", &Player::operator==)
		.def("__ne__", &Player::operator!=)
		;


	py::class_<Move>("Move",
"Move class represents an individual GDL action. Instances of the Move class are\n\
not created explicitly but are instead created through queries to State objects.\n\n\
Use the str(object) function to return a GDL formatted string of the action.",
					 py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		.def("__repr__", &Move::tostring)
		;

	py::class_<PlayerMove>("PlayerMove", 
"PlayerMove class represents an player-action pair. Each PlayerMove object is\n\
the result of a query to a State object. The object is mutable with read-only\n\
properties.",
						   py::no_init)
		.def("__repr__", &to_string<PlayerMove>)
		.def_readonly("player", &PlayerMove::first, "Read-only property of the Player object")
		.def_readonly("move", &PlayerMove::second, "Read-only property of the Move object")
		;

	py::class_<PlayerGoal>("PlayerGoal", 
"PlayerGoal class represents an player-goal pair that is the score for the player in some\n\
(terminal) game state. Each PlayerGoal object is the result of a query to a State object\n\
that represents a terminal game state.The object is mutable with read-only properties.",
						   py::no_init)
		.def("__repr__", &to_string<PlayerGoal>)
		.def_readonly("player", &PlayerGoal::first, "Read-only property of the Player object")
		.def_readonly("goal", &PlayerGoal::second, "Read-only property of the score (integer: 0 - 100)")
		;

	py::class_<std::vector<Player> >("PlayerVec")
		.def(py::vector_indexing_suite<std::vector<Player> >() );

	py::class_<std::vector<PlayerMove> >("PlayerMoveVec")
		.def(py::vector_indexing_suite<std::vector<PlayerMove> >() );

	py::class_<std::vector<PlayerGoal> >("PlayerGoalVec")
		.def(py::vector_indexing_suite<std::vector<PlayerGoal> >() );

	py::class_<Game, boost::noncopyable>("Game", 
"Game class represents GDL game instance. This is a finite state machine with each state\n\
being a valid game state and joint moves the transitions between states.", 
										 py::init<const std::string&>())
		.def("NumPlayers", &Game::numPlayers, "Returns the number of players/roles in the game")
		.def("Players", &game_players, "Returns a list of the Player objects")
		.def("InitState", &init_state, "Returns the initial state")
		;

	py::class_<State>("State", 
					  "State class represents a GDL game state. States are Game specific and are copyable.",
					  py::init<const State&>()
					  )
		.def("IsTerminal", &State::isTerminal, "Returns true if the state is a terminal game state.")
		.def("Legals", &state_legals, "Returns the list of legal PlayerMoves for a non-terminal state.")
		.def("Goals", &state_goals, "Returns the PlayerGoals for a terminal state.")
		.def("Playout", &state_playout, "Perform a random playout to termination and return the PlayerGoals for the terminal state.")
		.def("Play", &state_play1, "Execute a joint move of a move per player. Performs a transition to the next game state.")
		.def("Play", &state_play2)
		.def("Play", &state_play3)
		;
}
