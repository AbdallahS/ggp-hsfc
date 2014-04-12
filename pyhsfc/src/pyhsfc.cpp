

#include <sstream>
#include <list>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/args.hpp>
#include <boost/python/object.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

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
 * Support for python Player.
 *****************************************************************************************/

struct PyPlayer
{
    /* docstrings */
    static const char* ds_class; 
};

const char* PyPlayer::ds_class = 
"Player class represents individual GDL roles. Instances of the Player class\n\
are not created explicitly but are instead created through queries to Game\n\
and State objects.\n\n\
Use the str(object) function to return a printable name of the player.";

/*****************************************************************************************
 * Support for python Move.
 *****************************************************************************************/

struct PyMove
{
    /* docstrings */
    static const char* ds_class;     
};

const char* PyMove::ds_class = 
"Move class represents an individual GDL action. Instances of the Move class are\n\
not created explicitly but are instead created through queries to State objects.\n\n\
Use the str(object) function to return a GDL formatted string of the action.";


/*****************************************************************************************
 * Support for python PlayerMove.
 *****************************************************************************************/

struct PyPlayerMove
{
    /* docstrings */
    static const char* ds_class;     
    static const char* ds_player;     
    static const char* ds_move;     
};

const char* PyPlayerMove::ds_class = 
"PlayerMove class represents an player-action pair. Each PlayerMove object is\n\
the result of a query to a State object. The object is mutable with read-only\n\
properties.";

const char* PyPlayerMove::ds_player = "Read-only property of the Player object";

const char* PyPlayerMove::ds_move = "Read-only property of the Move object";

/*****************************************************************************************
 * Support for python PlayerGoal.
 *****************************************************************************************/

struct PyPlayerGoal
{
    /* docstrings */
    static const char* ds_class;     
    static const char* ds_player;     
    static const char* ds_goal;     
};

const char* PyPlayerGoal::ds_class = 
"PlayerGoal class represents an player-goal pair that is the score for the player in some\n\
(terminal) game state. Each PlayerGoal object is the result of a query to a State object\n\
that represents a terminal game state.The object is mutable with read-only properties.";

const char* PyPlayerGoal::ds_player = "Read-only property of the Player object";

const char* PyPlayerGoal::ds_goal = "Read-only property of the score (integer: 0 - 100)";


/*****************************************************************************************
 * Support for python Game.
 *****************************************************************************************/
class PyState;
class PyGame : public Game
{
    friend class PyState;
public:
    /* docstrings */
    static const char* ds_class; 
    static const char* ds_Players; 
    static const char* ds_NumPlayers; 
    static const char* ds_InitState; 

    /* A constructor substitute to work with python keyword arguments */
    PyGame(const std::string& gdldescription, 
           const std::string& gdlfilename, 
           bool use_gadelac);

    /* Returns the list of players */
    std::vector<Player> players();
};

const char* PyGame::ds_class = 
"Game class represents GDL game instance. This is a finite state machine with each state\n\
being a valid game state and joint moves the transitions between states.";

const char* PyGame::ds_Players = "Returns a list of the Player objects";

const char* PyGame::ds_NumPlayers = "Returns the number of players/roles in the game";

const char* PyGame::ds_InitState = "Returns the initial state";


PyGame::PyGame(const std::string& gdldescription, 
               const std::string& gdlfilename, 
               bool usegadelac)
{
    if (gdldescription.empty() && gdlfilename.empty())
        throw HSFCException() 
            << ErrorMsgInfo("No GDL file or description specified"); 
    if (!gdldescription.empty() && !gdlfilename.empty())
        throw HSFCException() 
            << ErrorMsgInfo("Cannot speficy both a GDL file and description");
    if (!gdldescription.empty())        
        Game::initialise(gdldescription, usegadelac);
    else
        Game::initialise(boost::filesystem::path(gdlfilename), usegadelac);
}

std::vector<Player> PyGame::players()
{
    std::vector<Player> plyrs;
    Game::players(std::back_inserter(plyrs));
    return plyrs;
}

/*****************************************************************************************
 * Support for python Game.
 *****************************************************************************************/
class PyState : public State
{
    friend class PyGame;
public:
    /* docstrings */
    static const char* ds_class; 
    static const char* ds_IsTerminal; 
    static const char* ds_Legals; 
    static const char* ds_Play; 
    static const char* ds_Playout; 
    static const char* ds_Goals; 

    std::vector<PlayerMove> legals();
    std::vector<PlayerGoal> goals();
    std::vector<PlayerGoal> playout();
    void play1(const std::vector<PlayerMove>& jm);
    void play2(const boost::python::list& obj);
    void play3(const boost::python::tuple& obj);

    PyState(PyGame& game);
    PyState(PyGame& game, const PortableState& ps);
	PyState(const PyState& other);
};

const char* PyState::ds_class = 
"State class represents a GDL game state. States are Game specific and are copyable.";

const char* PyState::ds_IsTerminal = 
"Returns true if the state is a terminal game state.";

const char* PyState::ds_Legals = 
"Returns the list of legal PlayerMoves for a non-terminal state.";

const char* PyState::ds_Play = 
"Execute a joint move of a move per player. Performs a transition to the next game state.";

const char* PyState::ds_Playout = 
"Perform a random playout to termination and return the PlayerGoals for the terminal state.";

const char* PyState::ds_Goals = "Returns the PlayerGoals for a terminal state.";


PyState::PyState(PyGame& game) : State(game)
{ }

PyState::PyState(PyGame& game, const PortableState& ps) : State(game, ps)
{ }

PyState::PyState(const PyState& other) : State(other)
{ }


std::vector<PlayerMove> PyState::legals()
{
    std::vector<PlayerMove> pms;
    State::legals(std::back_inserter(pms));
    return pms;
}

std::vector<PlayerGoal> PyState::goals()
{
    std::vector<PlayerGoal> pgs;
    State::goals(std::back_inserter(pgs));
    return pgs;
}

std::vector<PlayerGoal> PyState::playout()
{
    std::vector<PlayerGoal> pgs;
    State::playout(std::back_inserter(pgs));
    return pgs;
}

void PyState::play1(const std::vector<PlayerMove>& jm)
{
    State::play(jm);
}

void PyState::play2(const boost::python::list& obj)
{
    py::stl_input_iterator<PlayerMove> begin(obj), end;
    State::play(begin, end);
}

void PyState::play3(const boost::python::tuple& obj)
{
    py::stl_input_iterator<PlayerMove> begin(obj), end;
    State::play(begin, end);
}

/*****************************************************************************************
 * Support for python PortableState.
 *****************************************************************************************/
class PyPortableState : public PortableState
{
public:
   /* docstrings */
    static const char* ds_class; 

    PyPortableState(const PyState& state);
};

const char* PyPortableState::ds_class = 
"PortableState is a immutable representation of a State. Because it is immutable it is\n\
hashable. Note: in C++ the PortableState is portable across multiple Game instances\n\
(provided the instances were loaded with the identical GDL). These instances may be\n\
running on different computers for example as part of a distributed MPI program.\n\
However, at the moment there is no way to get the data in and out from the python\n\
perspective. So at the moment for python it is only useful as a way of storing states\n\
in a hashable form.";

PyPortableState::PyPortableState(const PyState& state) : PortableState((const State&)state)
{ }

/*****************************************************************************************
 * Setup the python module
 *****************************************************************************************/


BOOST_PYTHON_MODULE(pyhsfc)
{
    // Enable user-defined docstrings and python signatures, but disable

    // the C++ signatures.
    py::docstring_options local_docstring_options(true, true, false);

    py::class_<Player>
        ("Player", PyPlayer::ds_class, py::no_init)
        .def(py::self_ns::str(py::self_ns::self))
        .def("__repr__", &Player::tostring)
        .def("__hash__", &Player::hash_value)
        .def("__eq__", &Player::operator==)
        .def("__ne__", &Player::operator!=)
        ;

    py::class_<Move>
        ("Move", PyMove::ds_class, py::no_init)
        .def(py::self_ns::str(py::self_ns::self))
        .def("__repr__", &Move::tostring)
        .def("__hash__", &Move::hash_value)
        .def("__eq__", &Move::operator==)
        .def("__ne__", &Move::operator!=)
        ;

    py::class_<PlayerMove>
        ("PlayerMove", PyPlayerMove::ds_class, py::no_init)
        .def("__repr__", &to_string<PlayerMove>)
        .def_readonly("player", &PlayerMove::first, PyPlayerMove::ds_player)
        .def_readonly("move", &PlayerMove::second, PyPlayerMove::ds_move)
        ;

    py::class_<PlayerGoal>
        ("PlayerGoal", PyPlayerGoal::ds_class, py::no_init)
        .def("__repr__", &to_string<PlayerGoal>)
        .def_readonly("player", &PlayerGoal::first, PyPlayerGoal::ds_player)
        .def_readonly("goal", &PlayerGoal::second, PyPlayerGoal::ds_goal)
        ;

    py::class_<std::vector<Player> >("PlayerVec")
        .def(py::vector_indexing_suite<std::vector<Player> >() );

    py::class_<std::vector<PlayerMove> >("PlayerMoveVec")
        .def(py::vector_indexing_suite<std::vector<PlayerMove> >() );

    py::class_<std::vector<PlayerGoal> >("PlayerGoalVec")
        .def(py::vector_indexing_suite<std::vector<PlayerGoal> >() );


    py::class_<PyGame,boost::noncopyable>
        ("Game", PyGame::ds_class, 
         py::init<const std::string&, const std::string&, bool>(
             (py::arg("gdl")=std::string(), py::arg("file")=std::string(), py::arg("gadelac")=false)))
        .def("NumPlayers", &Game::numPlayers, PyGame::ds_NumPlayers)
        .def("Players", &PyGame::players, PyGame::ds_Players)
        ;

    py::class_<PyState>("State", PyState::ds_class, py::init<const PyState&>())
        .def(py::init<PyGame&>())
        .def(py::init<PyGame&, PortableState&>())
        .def("IsTerminal", &State::isTerminal, PyState::ds_IsTerminal)
        .def("Legals", &PyState::legals, PyState::ds_Legals)
        .def("Goals", &PyState::goals, PyState::ds_Goals)
        .def("Playout", &PyState::playout, PyState::ds_Playout)
        .def("Play", &PyState::play1, PyState::ds_Play)
        .def("Play", &PyState::play2, PyState::ds_Play)
        .def("Play", &PyState::play3, PyState::ds_Play)
        ;


    py::class_<PyPortableState>("PortableState", PyPortableState::ds_class,
                              py::init<const PyState&>())
        .def("__hash__", &PortableState::hash_value)
        .def("__eq__", &PortableState::operator==)
        .def("__ne__", &PortableState::operator!=)
        ;

}
