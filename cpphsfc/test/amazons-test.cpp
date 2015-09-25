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

using namespace HSFC;

/****************************************************************
 ****************************************************************/
extern const char* g_amazons;


/****************************************************************
 * General support functions
 ****************************************************************/

// Count the number of moves that are legal for the given player
unsigned int count_player_moves(const std::vector<PlayerMove> moves,
                                const std::string& player)
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

const unsigned int NUM_AMAZONS_PLAYOUTS=1000;

BOOST_AUTO_TEST_CASE(amazons_test)
{
    std::string amazons(g_amazons);
    Game game(amazons);

    std::cerr << "Amazons loaded. Now to run the playout test" << std::endl;
    State state1(game);
    std::vector<PlayerGoal> result1;

    return;
    breakthrough_playout_check(state1, result1);
    BOOST_CHECK(!state1.isTerminal());

    boost::unordered_map<Player, std::vector<unsigned int> > results;
    boost::unordered_map<Player, float> average;
    for (int i = 0; i < NUM_AMAZONS_PLAYOUTS; ++i)
    {
        std::vector<PlayerGoal> result;
        State state2(game);
        breakthrough_playout_check(state2, result);
        for (std::vector<PlayerGoal>::const_iterator pgi = result.begin();
             pgi != result.end(); ++pgi)
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
        for(std::vector<unsigned int>::iterator i = scores.begin();
            i != scores.end(); ++i)
        {
            sum += *i;
        }
        ++curr;
        average[p] = (float)sum/(float)scores.size();
    }

    // Sum of the averages should equal 100.
    float sum_ave = 0.0;
    std::cout << "Amazons average scores from "
              << NUM_AMAZONS_PLAYOUTS << " playouts: ";
    for (boost::unordered_map<Player, float>::iterator i = average.begin();
         i != average.end(); ++i)
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

const char* g_amazons=" \n\
; Amazons 8x8 \n\
; \n\
; In this version of Amazons, moving and firing an arrow occur on separate turns. \n\
 \n\
(role white) \n\
(role black) \n\
 \n\
; Bases and inputs \n\
 \n\
(base (turn white move)) \n\
(base (turn white fire)) \n\
(base (turn black move)) \n\
(base (turn black fire)) \n\
 \n\
(<= (base (cell ?x ?y white)) \n\
    (index ?x) \n\
    (index ?y)) \n\
(<= (base (cell ?x ?y black)) \n\
    (index ?x) \n\
    (index ?y)) \n\
(<= (base (cell ?x ?y arrow)) \n\
    (index ?x) \n\
    (index ?y)) \n\
(<= (base (justMoved ?x ?y)) \n\
    (index ?x) \n\
    (index ?y)) \n\
 \n\
(<= (input ?player noop) \n\
    (role ?player)) \n\
(<= (input ?player (move ?x1 ?y1 ?x2 ?y2)) \n\
    (role ?player) \n\
    (queenMove ?x1 ?y1 ?x2 ?y2)) \n\
(<= (input ?player (fire ?x ?y)) \n\
    (role ?player) \n\
    (index ?x) \n\
    (index ?y)) \n\
 \n\
; Initial state \n\
 \n\
(init (cell 1 3 white)) \n\
(init (cell 3 1 white)) \n\
(init (cell 6 1 white)) \n\
(init (cell 8 3 white)) \n\
(init (cell 1 6 black)) \n\
(init (cell 3 8 black)) \n\
(init (cell 6 8 black)) \n\
(init (cell 8 6 black)) \n\
 \n\
(init (turn white move)) \n\
 \n\
; Legal moves \n\
 \n\
(<= (legal white noop) \n\
    (true (turn black ?any))) \n\
(<= (legal black noop) \n\
    (true (turn white ?any))) \n\
 \n\
(<= (legal ?player ?move) \n\
    (legalMove ?player ?move)) \n\
(<= (legalMove ?player (move ?x1 ?y1 ?x2 ?y2)) \n\
    (true (turn ?player move)) \n\
    (true (cell ?x1 ?y1 ?player)) \n\
    (dir ?dir) \n\
    (openPath ?x1 ?y1 ?x2 ?y2 ?dir)) \n\
(<= (legalMove ?player (fire ?x2 ?y2)) \n\
    (true (turn ?player fire)) \n\
    (true (justMoved ?x1 ?y1)) \n\
    (dir ?dir) \n\
    (openPath ?x1 ?y1 ?x2 ?y2 ?dir)) \n\
 \n\
(<= (openPath ?x1 ?y1 ?x2 ?y2 ?dir) \n\
    (oneInDir ?x1 ?y1 ?x2 ?y2 ?dir) \n\
    (not (occupied ?x2 ?y2))) \n\
; Ideal recursive call would probably look like this, but hard for provers to handle right... \n\
;(<= (openPath ?x1 ?y1 ?x3 ?y3 ?dir) \n\
;    (openPath ?x1 ?y1 ?x2 ?y2 ?dir) \n\
;    (oneInDir ?x2 ?y2 ?x3 ?y3 ?dir) \n\
;    (not (occupied ?x3 ?y3))) \n\
; This is in a sense \"quadratic\", because we also need to compute versions of openPath \n\
; that start with each intermediate coordinate  \n\
(<= (openPath ?x1 ?y1 ?x3 ?y3 ?dir) \n\
    (oneInDir ?x1 ?y1 ?x2 ?y2 ?dir) \n\
    (not (occupied ?x2 ?y2)) \n\
    (openPath ?x2 ?y2 ?x3 ?y3 ?dir)) \n\
 \n\
(<= (occupied ?x ?y) \n\
    (true (cell ?x ?y ?any))) \n\
 \n\
; Game dynamics \n\
 \n\
(<= (next (turn ?player fire)) \n\
    (true (turn ?player move))) \n\
(<= (next (turn ?opponent move)) \n\
    (true (turn ?player fire)) \n\
    (opponent ?player ?opponent)) \n\
 \n\
(<= (next (cell ?x2 ?y2 ?player)) \n\
    (does ?player (move ?x1 ?y1 ?x2 ?y2))) \n\
(<= (next (cell ?x ?y ?player)) \n\
    (true (cell ?x ?y ?player)) \n\
    (not (vacated ?x ?y))) \n\
(<= (vacated ?x1 ?y1) \n\
    (does ?player (move ?x1 ?y1 ?x2 ?y2))) \n\
(<= (next (justMoved ?x2 ?y2)) \n\
    (does ?player (move ?x1 ?y1 ?x2 ?y2))) \n\
 \n\
(<= (next (cell ?x ?y arrow)) \n\
    (does ?player (fire ?x ?y))) \n\
 \n\
; Game ending conditions \n\
(<= terminal \n\
    (true (turn ?player ?any)) \n\
    (not (anyLegalMove ?player))) \n\
(<= (anyLegalMove ?player) \n\
    (legal ?player ?move)) \n\
 \n\
(<= (goal ?player 0) \n\
    (true (turn ?player ?any))) \n\
(<= (goal ?player 100) \n\
    (true (turn ?opponent ?any)) \n\
    (opponent ?opponent ?player)) \n\
 \n\
; Constants \n\
 \n\
(index 1) \n\
(index 2) \n\
(index 3) \n\
(index 4) \n\
(index 5) \n\
(index 6) \n\
(index 7) \n\
(index 8) \n\
(nextIndex 1 2) \n\
(nextIndex 2 3) \n\
(nextIndex 3 4) \n\
(nextIndex 4 5) \n\
(nextIndex 5 6) \n\
(nextIndex 6 7) \n\
(nextIndex 7 8) \n\
 \n\
(opponent white black) \n\
(opponent black white) \n\
 \n\
(dir n) \n\
(dir ne) \n\
(dir e) \n\
(dir se) \n\
(dir s) \n\
(dir sw) \n\
(dir w) \n\
(dir nw) \n\
 \n\
(<= (oneInDir ?x ?y1 ?x ?y2 n) \n\
    (index ?x) \n\
    (nextIndex ?y1 ?y2)) \n\
(<= (oneInDir ?x1 ?y1 ?x2 ?y2 ne) \n\
    (nextIndex ?x1 ?x2) \n\
    (nextIndex ?y1 ?y2)) \n\
(<= (oneInDir ?x1 ?y ?x2 ?y e) \n\
    (nextIndex ?x1 ?x2) \n\
    (index ?y)) \n\
(<= (oneInDir ?x1 ?y2 ?x2 ?y1 se) \n\
    (nextIndex ?x1 ?x2) \n\
    (nextIndex ?y1 ?y2)) \n\
(<= (oneInDir ?x ?y2 ?x ?y1 s) \n\
    (index ?x) \n\
    (nextIndex ?y1 ?y2)) \n\
(<= (oneInDir ?x2 ?y2 ?x1 ?y1 sw) \n\
    (nextIndex ?x1 ?x2) \n\
    (nextIndex ?y1 ?y2)) \n\
(<= (oneInDir ?x2 ?y ?x1 ?y w) \n\
    (nextIndex ?x1 ?x2) \n\
    (index ?y)) \n\
(<= (oneInDir ?x2 ?y1 ?x1 ?y2 nw) \n\
    (nextIndex ?x1 ?x2) \n\
    (nextIndex ?y1 ?y2)) \n\
 \n\
(<= (queenMove ?x1 ?y1 ?x2 ?y2) \n\
    (dir ?dir) \n\
    (queenMoveDir ?x1 ?y1 ?x2 ?y2 ?dir)) \n\
(<= (queenMoveDir ?x1 ?y1 ?x2 ?y2 ?dir) \n\
    (oneInDir ?x1 ?y1 ?x2 ?y2 ?dir)) \n\
(<= (queenMoveDir ?x1 ?y1 ?x3 ?y3 ?dir) \n\
    (oneInDir ?x1 ?y1 ?x2 ?y2 ?dir) \n\
    (queenMoveDir ?x2 ?y2 ?x3 ?y3 ?dir)) \n\
 \n\
 \n\
";
