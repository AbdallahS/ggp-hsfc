#include <list>
#include <iterator>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <hsfc/hsfc.h>

using namespace HSFC;
namespace py = boost::python;

const char* create()
{
	Game("./tictactoe.gdl");
	return "OK!";
}

typedef std::vector<Player> PlayerList;

template<typename T>
py::list std_vector_to_py_list(const std::vector<T>& v)
{
	py::object get_iter = py::iterator<std::vector<T> >();
	py::object iter = get_iter(v);
	py::list l(iter);
	return l;
}

PlayerList game_players(const Game& game)
{
	PlayerList plyrs;
	game.players(std::back_inserter(plyrs));
	return plyrs;
}



BOOST_PYTHON_MODULE(pyhsfc)
{
	py::class_<Player>("Player", py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		;

	py::class_<Move>("Move", py::no_init)
		.def(py::self_ns::str(py::self_ns::self))
		;

	py::class_<PlayerList>("PlayerList")
		.def(py::vector_indexing_suite<PlayerList>() );

	py::class_<Game, boost::noncopyable>("Game", py::init<const std::string&>())
		.def("NumPlayers", &Game::numPlayers)
		.def("Players", &game_players)
		;

	py::def("game_players", &game_players);
}
