#include <boost/python.hpp>
#include <hsfc/hsfc.h>

using namespace HSFC;
using namespace boost::python;

const char* create()
{
	Game("./tictactoe.gdl");
	return "OK!";
}


BOOST_PYTHON_MODULE(pyhsfc)
{
	class_<Player>("Player", no_init)
		.def(self_ns::str(self_ns::self))
		;

	class_<Move>("Move", no_init)
		.def(self_ns::str(self_ns::self))
		;

	class_<Game, boost::noncopyable>("Game", init<const std::string&>())
		.def("NumPlayers", &Game::numPlayers)
		.def("Players", &Game::players)
		;

//	def("create", create);
}
