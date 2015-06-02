//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/function_output_iterator.hpp>
#include <hsfc/hsfc.h>
#include <hsfc/portable.h>

using namespace HSFC;

/****************************************************************
 * Use two GDLs defined at end of this file: tictactoe and
 * breakthrough. Note: rely on Gadelac to be present in order to
 * pre-process the files.
 ****************************************************************/
extern const char* g_tictactoe;
extern const char* g_breakthrough;


/****************************************************************
 * General support functions
 ****************************************************************/

// Count the number of moves that are legal for the given player
unsigned int count_player_moves(const std::vector<PlayerMove> moves, const std::string& player)
{
    unsigned int count = 0;
    BOOST_FOREACH(const PlayerMove& pm, moves)
    {
        if (pm.first.tostring() == player) ++count;
    }
    return count;
}

// Pick the first available move for the given player
PlayerMove pick_first(const std::vector<PlayerMove> moves, const std::string& player)
{
    BOOST_FOREACH(const PlayerMove& pm, moves)
    {
        if (pm.first.tostring() == player) return pm;
    }
    BOOST_CHECK(false);
    throw std::string("To prevent clang compiler warning");
}

// Return the player
Player get_player(const Game& game, const std::string& playername)
{
    std::vector<Player> players = game.players();
    BOOST_FOREACH(const Player& p, players)
    {
        if (p.tostring() == playername) return p;
    }
    BOOST_CHECK(false);
    throw std::string("To prevent clang compiler warning");
}

// Count the number of moves that the player has in the state
unsigned int get_num_moves(const State& state, const std::string& player)
{
    typedef boost::unordered_map<Player, std::vector<Move> > pmvs_t;
    pmvs_t pmvs = state.legals();
    BOOST_FOREACH(const pmvs_t::value_type& pm, pmvs)
    {
        if (pm.first.tostring() == player) return pm.second.size();
    }
    BOOST_CHECK(false);
    return 0;
}


/****************************************************************
 * Tictactoe specific functions.
 ****************************************************************/

// Run a playout from any (non-terminal) tictactoe game state.
// Someone either wins or it is a draw.
void tictactoe_playout_check(const State &state,
                             std::vector<PlayerGoal>& results)
{
    BOOST_CHECK(!state.isTerminal());
    State tmpstate(state);
    results.clear();
    tmpstate.playout(results);
    BOOST_CHECK_EQUAL(results.size(), 2);
    bool haswinner =
        (results[0].second == 100 && results[1].second == 0) ||
        (results[1].second == 100 && results[0].second == 0);
    if (!haswinner)
    {
        BOOST_CHECK(results[0].second == 50);
        BOOST_CHECK(results[1].second == 50);
    }
}

/****************************************************************
 * Breakthrough specific functions.
 ****************************************************************/

// Run a playout from any (non-terminal) tictactoe game state.
// Someone either wins or it is a draw.
void breakthrough_playout_check(const State &state,
                                std::vector<PlayerGoal>& results)
{
    BOOST_CHECK(!state.isTerminal());
    State tmpstate(state);
    results.clear();
    tmpstate.playout(results);
    BOOST_CHECK_EQUAL(results.size(), 2);
    BOOST_CHECK((results[0].second == 100 && results[1].second == 0) ||
                (results[1].second == 100 && results[0].second == 0));
}


/****************************************************************
 * Test of loading GDL from a string
 ****************************************************************/

// Load a unordered_set of playernames (using tostring())
struct playername_loader
{
    playername_loader(boost::unordered_set<std::string>& m) : m_(m){};
    void operator()(const Player& o) { m_.insert(o.tostring()); }
    boost::unordered_set<std::string>& m_;
};

// Load a unordered_set of move texts (using tostring())
struct playermovename_loader
{
    playermovename_loader(boost::unordered_set<std::string>& m) : m_(m){};
    void operator()(const PlayerMove& o) const
    {
        m_.insert(o.first.tostring() + o.second.tostring());
    }
    boost::unordered_set<std::string>& m_;
};

/****************************************************************
 * Test functions of the State class
 ****************************************************************/

BOOST_AUTO_TEST_CASE(tictactoe_test)
{
    Game game(std::string(g_tictactoe), true);
    State state1(game);
    BOOST_CHECK(!state1.isTerminal());

    boost::unordered_map<Player, std::vector<unsigned int> > results;
    boost::unordered_map<Player, float> average;
    for (int i = 0; i < 1000; ++i)
    {
        std::vector<PlayerGoal> result;
        State state2(game);
        tictactoe_playout_check(state2, result);
        for (std::vector<PlayerGoal>::const_iterator pgi = result.begin(); pgi != result.end(); ++pgi)
        {
            if (results.find(pgi->first) == results.end())
            {
                results[pgi->first] = std::vector<unsigned int>();
            }
            results[pgi->first].push_back(pgi->second);
        }
    }

    boost::unordered_map<Player, std::vector<unsigned int> >::iterator curr = results.begin();
    while (curr != results.end())
    {
        Player p = curr->first;
        std::vector<unsigned int>& scores = curr->second;
        unsigned int sum = 0;
        for(std::vector<unsigned int>::iterator i = scores.begin(); i != scores.end(); ++i)
        {
            sum += *i;
        }
        ++curr;
        average[p] = (float)sum/(float)scores.size();
    }

    // Sum of the averages should equal 100.
    float sum_ave = 0.0;
    std::cout << "Tictactoe average scores from 1000 playouts: ";
    for (boost::unordered_map<Player, float>::iterator i = average.begin(); i != average.end(); ++i)
    {
        std::cout << (i->first).tostring() << "=" << i->second << " ";
        sum_ave += i->second;
    }
    std::cout << std::endl;
    BOOST_CHECK_EQUAL(sum_ave, 100.0);
}

BOOST_AUTO_TEST_CASE(breakthrough_test)
{
    Game game(std::string(g_breakthrough), true);
    State state1(game);
    BOOST_CHECK(!state1.isTerminal());

    boost::unordered_map<Player, std::vector<unsigned int> > results;
    boost::unordered_map<Player, float> average;
    for (int i = 0; i < 1000; ++i)
    {
        std::vector<PlayerGoal> result;
        State state2(game);
        breakthrough_playout_check(state2, result);
        for (std::vector<PlayerGoal>::const_iterator pgi = result.begin(); pgi != result.end(); ++pgi)
        {
            if (results.find(pgi->first) == results.end())
            {
                results[pgi->first] = std::vector<unsigned int>();
            }
            results[pgi->first].push_back(pgi->second);
        }
    }

    boost::unordered_map<Player, std::vector<unsigned int> >::iterator curr = results.begin();
    while (curr != results.end())
    {
        Player p = curr->first;
        std::vector<unsigned int>& scores = curr->second;
        unsigned int sum = 0;
        for(std::vector<unsigned int>::iterator i = scores.begin(); i != scores.end(); ++i)
        {
            sum += *i;
        }
        ++curr;
        average[p] = (float)sum/(float)scores.size();
    }

    // Sum of the averages should equal 100.
    float sum_ave = 0.0;
    std::cout << "Breakthrough average scores from 1000 playouts: ";
    for (boost::unordered_map<Player, float>::iterator i = average.begin(); i != average.end(); ++i)
    {
        std::cout << (i->first).tostring() << "=" << i->second << " ";
        sum_ave += i->second;
    }
    std::cout << std::endl;
    BOOST_CHECK_EQUAL(sum_ave, 100.0);
}



/****************************************************************
 * The GDL test files
 ****************************************************************/

const char* g_tictactoe=" \n\
(role xplayer) \n\
(role oplayer) \n\
(init (cell 1 1 b)) \n\
(init (cell 1 2 b)) \n\
(init (cell 1 3 b)) \n\
(init (cell 2 1 b)) \n\
(init (cell 2 2 b)) \n\
(init (cell 2 3 b)) \n\
(init (cell 3 1 b)) \n\
(init (cell 3 2 b)) \n\
(init (cell 3 3 b)) \n\
(init (control xplayer)) \n\
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b))) \n\
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j)) \n\
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k)) \n\
(<= (next (control xplayer)) (true (control oplayer))) \n\
(<= (next (control oplayer)) (true (control xplayer))) \n\
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x))) \n\
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x))) \n\
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x))) \n\
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x))) \n\
(<= (line ?x) (row ?m ?x)) \n\
(<= (line ?x) (column ?m ?x)) \n\
(<= (line ?x) (diagonal ?x)) \n\
(<= open (true (cell ?m ?n b))) \n\
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w))) \n\
(<= (legal xplayer noop) (true (control oplayer))) \n\
(<= (legal oplayer noop) (true (control xplayer))) \n\
(<= (goal xplayer 100) (line x)) \n\
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal xplayer 0) (line o)) \n\
(<= (goal oplayer 100) (line o)) \n\
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open)) \n\
(<= (goal oplayer 0) (line x)) \n\
(<= terminal (line x)) \n\
(<= terminal (line o)) \n\
(<= terminal (not open)) \n\
";

const char* g_breakthrough=" \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; \n\
;;; Breakthrough \n\
;;; \n\
;;; This version of Breakthrough integrates MG's bugfix: \n\
;;;     There was no goal or terminal clause for the case \n\
;;;     that one player has lost all pieces! \n\
;;; \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; ROLE Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(role white) \n\
(role black) \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; BASE & INPUT Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(<= (base (cellHolds ?x ?y ?p)) (index ?x) (index ?y) (role ?p)) \n\
(<= (base (control ?p)) (role ?p)) \n\
(<= (input ?p noop) (role ?p)) \n\
(<= (input ?p (move ?x1 ?y1 ?x2 ?y2)) (role ?p) (index ?x1) (index ?x2) (index ?y1) (index ?y2)) \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; INIT Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(init (cellHolds 1 1 white)) \n\
(init (cellHolds 2 1 white)) \n\
(init (cellHolds 3 1 white)) \n\
(init (cellHolds 4 1 white)) \n\
(init (cellHolds 5 1 white)) \n\
;(init (cellHolds 6 1 white)) \n\
;(init (cellHolds 7 1 white)) \n\
;(init (cellHolds 8 1 white)) \n\
(init (cellHolds 1 2 white)) \n\
(init (cellHolds 2 2 white)) \n\
(init (cellHolds 3 2 white)) \n\
(init (cellHolds 4 2 white)) \n\
(init (cellHolds 5 2 white)) \n\
;(init (cellHolds 6 2 white)) \n\
;(init (cellHolds 7 2 white)) \n\
;(init (cellHolds 8 2 white)) \n\
 \n\
(init (cellHolds 1 4 black)) \n\
(init (cellHolds 2 4 black)) \n\
(init (cellHolds 3 4 black)) \n\
(init (cellHolds 4 4 black)) \n\
(init (cellHolds 5 4 black)) \n\
;(init (cellHolds 6 7 black)) \n\
;(init (cellHolds 7 7 black)) \n\
;(init (cellHolds 8 7 black)) \n\
(init (cellHolds 1 5 black)) \n\
(init (cellHolds 2 5 black)) \n\
(init (cellHolds 3 5 black)) \n\
(init (cellHolds 4 5 black)) \n\
(init (cellHolds 5 5 black)) \n\
;(init (cellHolds 6 8 black)) \n\
;(init (cellHolds 7 8 black)) \n\
;(init (cellHolds 8 8 black)) \n\
 \n\
(init (control white)) \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; LEGAL Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(<= (legal white (move ?x ?y1 ?x ?y2)) \n\
    (true (control white)) \n\
    (true (cellHolds ?x ?y1 white)) \n\
    (succ ?y1 ?y2) \n\
    (cellEmpty ?x ?y2)) \n\
(<= (legal white (move ?x1 ?y1 ?x2 ?y2)) \n\
    (true (control white)) \n\
    (true (cellHolds ?x1 ?y1 white)) \n\
    (succ ?y1 ?y2) \n\
    (succ ?x1 ?x2) \n\
    (not (true (cellHolds ?x2 ?y2 white)))) \n\
(<= (legal white (move ?x1 ?y1 ?x2 ?y2)) \n\
    (true (control white)) \n\
    (true (cellHolds ?x1 ?y1 white)) \n\
    (succ ?y1 ?y2) \n\
    (succ ?x2 ?x1) \n\
    (not (true (cellHolds ?x2 ?y2 white)))) \n\
 \n\
(<= (legal black (move ?x ?y1 ?x ?y2)) \n\
    (true (control black)) \n\
    (true (cellHolds ?x ?y1 black)) \n\
    (succ ?y2 ?y1) \n\
    (cellEmpty ?x ?y2)) \n\
(<= (legal black (move ?x1 ?y1 ?x2 ?y2)) \n\
    (true (control black)) \n\
    (true (cellHolds ?x1 ?y1 black)) \n\
    (succ ?y2 ?y1) \n\
    (succ ?x1 ?x2) \n\
    (not (true (cellHolds ?x2 ?y2 black)))) \n\
(<= (legal black (move ?x1 ?y1 ?x2 ?y2)) \n\
    (true (control black)) \n\
    (true (cellHolds ?x1 ?y1 black)) \n\
    (succ ?y2 ?y1) \n\
    (succ ?x2 ?x1) \n\
    (not (true (cellHolds ?x2 ?y2 black)))) \n\
     \n\
(<= (legal white noop) \n\
    (true (control black))) \n\
(<= (legal black noop) \n\
    (true (control white))) \n\
     \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; NEXT Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
     \n\
(<= (next (cellHolds ?x2 ?y2 ?player)) \n\
    (role ?player) \n\
    (does ?player (move ?x1 ?y1 ?x2 ?y2))) \n\
(<= (next (cellHolds ?x3 ?y3 ?state)) \n\
    (true (cellHolds ?x3 ?y3 ?state)) \n\
    (role ?player) \n\
    (does ?player (move ?x1 ?y1 ?x2 ?y2)) \n\
    (distinctCell ?x1 ?y1 ?x3 ?y3) \n\
    (distinctCell ?x2 ?y2 ?x3 ?y3)) \n\
     \n\
(<= (next (control white)) \n\
    (true (control black))) \n\
(<= (next (control black)) \n\
    (true (control white))) \n\
     \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; TERMINAL Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(<= terminal  \n\
    whiteWin) \n\
(<= terminal \n\
    blackWin) \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; GOAL Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(<= (goal white 100) \n\
    whiteWin) \n\
(<= (goal white 0) \n\
    (not whiteWin)) \n\
     \n\
(<= (goal black 100) \n\
    blackWin) \n\
(<= (goal black 0) \n\
    (not blackWin)) \n\
 \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; View Definitions \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(<= (cell ?x ?y) \n\
    (index ?x) \n\
    (index ?y)) \n\
     \n\
(<= (cellEmpty ?x ?y) \n\
    (cell ?x ?y) \n\
    (not (true (cellHolds ?x ?y white))) \n\
    (not (true (cellHolds ?x ?y black)))) \n\
     \n\
(<= (distinctCell ?x1 ?y1 ?x2 ?y2) \n\
    (cell ?x1 ?y1) \n\
    (cell ?x2 ?y2) \n\
    (distinct ?x1 ?x2)) \n\
(<= (distinctCell ?x1 ?y1 ?x2 ?y2) \n\
    (cell ?x1 ?y1) \n\
    (cell ?x2 ?y2) \n\
    (distinct ?y1 ?y2)) \n\
     \n\
(<= whiteWin \n\
    (index ?x) \n\
    (true (cellHolds ?x 5 white))) \n\
(<= blackWin \n\
    (index ?x) \n\
    (true (cellHolds ?x 1 black))) \n\
 \n\
; MG's bugfix     \n\
(<= whiteWin \n\
	(not blackCell)) \n\
(<= blackWin \n\
	(not whiteCell)) \n\
(<= whiteCell \n\
	(cell ?x ?y) \n\
	(true (cellHolds ?x ?y white))) \n\
(<= blackCell \n\
	(cell ?x ?y) \n\
	(true (cellHolds ?x ?y black)))     \n\
     \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
;;; Static Relations \n\
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; \n\
 \n\
(index 1) (index 2) (index 3) (index 4) (index 5)  \n\
;(index 6) (index 7) (index 8) \n\
(succ 1 2)  (succ 2 3)  (succ 3 4)  (succ 4 5)   \n\
;(succ 5 6)  (succ 6 7)  (succ 7 8) \n\
";
