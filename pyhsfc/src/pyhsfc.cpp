

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
 * Support for python Game.
 *****************************************************************************************/
class PyState;
class PyGame : public Game
{
    friend class PyState;
public:
    /* docstrings */
    static const char* ds_class; 
    static const char* ds_players; 

    /* A constructor substitute to work with python keyword arguments */
    PyGame(const std::string& gdldescription, 
           const std::string& gdlfilename, 
           bool use_gadelac);

    /* Returns the list of players */
    py::list players();
};

const char* PyGame::ds_class = 
"Game class represents GDL game instance. This is a finite state machine with each state\n\
being a valid game state and joint moves the transitions between states.";

const char* PyGame::ds_players = "Returns a list of the Player objects";

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

py::list PyGame::players()
{
    py::list pylist;
    std::vector<Player> plyrs;
    Game::players(std::back_inserter(plyrs));
    BOOST_FOREACH(const Player& p, plyrs)
    {
        pylist.append(p);
    }
    return pylist;
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
    static const char* ds_is_terminal; 
    static const char* ds_legals; 
    static const char* ds_joints; 
    static const char* ds_play; 
    static const char* ds_playout; 
    static const char* ds_goals; 

    py::dict legals();
    py::list joints();
    py::dict goals();
    py::dict playout();
    void play(const boost::python::object& obj);

    PyState(PyGame& game);
    PyState(PyGame& game, const PortableState& ps);
	PyState(const PyState& other);
};

const char* PyState::ds_class = 
"State class represents a GDL game state. States are Game specific and are copyable.";

const char* PyState::ds_is_terminal = 
"Returns true if the state is a terminal game state.";

const char* PyState::ds_legals = 
"Returns the dict of legal moves matching players to the list of their moves for a non-terminal state.";

const char* PyState::ds_joints = 
"Return a list of joint moves, where each joint move is a dict from players to moves.";

const char* PyState::ds_play = 
"Execute a joint move of a move per player. Performs a transition to the next game state.";

const char* PyState::ds_playout = 
"Perform a random playout to termination and return a dict of the goal scores for each player for the terminal state.";

const char* PyState::ds_goals = "Returns the dict of the goal scores for each player for a terminal state.";


PyState::PyState(PyGame& game) : State(game)
{ }

PyState::PyState(PyGame& game, const PortableState& ps) : State(game, ps)
{ }

PyState::PyState(const PyState& other) : State(other)
{ }

py::dict PyState::legals()
{
    py::dict pydict;
    boost::unordered_map<Player, std::vector<Move> >lgls = State::legals();
    typedef std::pair<Player, std::vector<Move> > pmvs_t;
    BOOST_FOREACH(const pmvs_t& pmvs, lgls)
    {
        py::list pylist;
        BOOST_FOREACH(const Move& mv, pmvs.second)
        {
            pylist.append(mv);
        }
        pydict[pmvs.first] = pylist;        
    }
    return pydict;
}

py::list PyState::joints()
{
    py::list pylist;
    std::vector<JointMove> jts = State::joints();

    BOOST_FOREACH(const JointMove& jm, jts)
    {
        py::dict pydict;
        typedef std::pair<Player, Move> pm_t;
        BOOST_FOREACH(const pm_t& pm, jm)
        {
            pydict[pm.first] = pm.second;
        }
        pylist.append(pydict);
    }
    return pylist;
}


py::dict PyState::goals()
{
    py::dict pydict;
    boost::unordered_map<Player, unsigned int> pgs = State::goals();
    typedef std::pair<Player, unsigned int > pg_t;
    BOOST_FOREACH(const pg_t& pg, pgs)
    {
        pydict[pg.first] = pg.second;        
    }
    return pydict;
}

py::dict PyState::playout()
{
    py::dict pydict;
    boost::unordered_map<Player, unsigned int> pgs = State::playout();
    typedef std::pair<Player, unsigned int > pg_t;
    BOOST_FOREACH(const pg_t& pg, pgs)
    {
        pydict[pg.first] = pg.second;        
    }
    return pydict;
}

void PyState::play(const boost::python::object& obj)
{
    std::vector<std::pair<Player,Move> > pmvs;
    for (unsigned int i = 0; i < py::len(obj); ++i)
    {
        py::object pm_pair = obj[i];
        Player p = py::extract<Player>(pm_pair[0]);
        Move m = py::extract<Move>(pm_pair[1]);
        pmvs.push_back(std::make_pair(p, m));
    }
    State::play(pmvs.begin(), pmvs.end());
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

    py::class_<PyGame,boost::noncopyable>
        ("Game", PyGame::ds_class, 
         py::init<const std::string&, const std::string&, bool>(
             (py::arg("gdl")=std::string(), py::arg("file")=std::string(), py::arg("gadelac")=false)))
        .def("players", &PyGame::players, PyGame::ds_players)
        ;

    py::class_<PyState>("State", PyState::ds_class, py::init<const PyState&>())
        .def(py::init<PyGame&>())
        .def(py::init<PyGame&, PortableState&>())
        .def("is_terminal", &State::isTerminal, PyState::ds_is_terminal)
        .def("legals", &PyState::legals, PyState::ds_legals)
        .def("joints", &PyState::joints, PyState::ds_joints)
        .def("goals", &PyState::goals, PyState::ds_goals)
        .def("playout", &PyState::playout, PyState::ds_playout)
        .def("play", &PyState::play, PyState::ds_play)
        ;

    py::class_<PyPortableState>("PortableState", PyPortableState::ds_class,
                              py::init<const PyState&>())
        .def("__hash__", &PortableState::hash_value)
        .def("__eq__", &PortableState::operator==)
        .def("__ne__", &PortableState::operator!=)
        ;
}
