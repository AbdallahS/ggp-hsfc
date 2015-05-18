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

// Count the number of moves that the player has in the state
Move get_move(const State& state, const Player& player, const std::string& move)
{
    typedef boost::unordered_map<Player, std::vector<Move> > pmvs_t;
    pmvs_t pmvs = state.legals();
    pmvs_t::const_iterator it = pmvs.find(player);
    BOOST_CHECK(it != pmvs.end());
    BOOST_FOREACH(const Move& m, it->second)
    {
        if (m.tostring() == move)
        {
            return m;
        }
    }
    BOOST_CHECK(false);
}



/****************************************************************
 * Tictactoe specific functions.
 ****************************************************************/

// Run a playout from any (non-terminal) tictactoe game state.
// Someone either wins or it is a draw.
void tictactoe_playout_check(const State &state)
{
    if (state.isTerminal()) return;
    State tmpstate(state);
    std::vector<PlayerGoal> results;
    tmpstate.playout(results);
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
 * Test of loading GDL from a string
 ****************************************************************/
const char* g_gdl = " \
;;;; RULES   \
 \
(role xplayer) \
(role oplayer) \
(init (cell 1 1 b)) \
(init (cell 1 2 b)) \
(init (cell 1 3 b)) \
(init (cell 2 1 b)) \
(init (cell 2 2 b)) \
(init (cell 2 3 b)) \
(init (cell 3 1 b)) \
(init (cell 3 2 b)) \
(init (cell 3 3 b)) \
(init (control xplayer)) \
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b))) \
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b))) \
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b)) \
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j)) \
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k)) \
(<= (next (control xplayer)) (true (control oplayer))) \
(<= (next (control oplayer)) (true (control xplayer))) \
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x))) \
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x))) \
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x))) \
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x))) \
(<= (line ?x) (row ?m ?x)) \
(<= (line ?x) (column ?m ?x)) \
(<= (line ?x) (diagonal ?x)) \
(<= open (true (cell ?m ?n b))) \
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w))) \
(<= (legal xplayer noop) (true (control oplayer))) \
(<= (legal oplayer noop) (true (control xplayer))) \
(<= (goal xplayer 100) (line x)) \
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open)) \
(<= (goal xplayer 0) (line o)) \
(<= (goal oplayer 100) (line o)) \
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open)) \
(<= (goal oplayer 0) (line x)) \
(<= terminal (line x)) \
(<= terminal (line o)) \
(<= terminal (not open)) \
 \
;;;; STRATS  \
 \
(strat does 0) \
(strat goal 1) \
(strat init 0) \
(strat legal 0) \
(strat next 0) \
(strat role 0) \
(strat terminal 1) \
(strat true 0) \
(strat cell 0) \
(strat column 0) \
(strat control 0) \
(strat diagonal 0) \
(strat line 0) \
(strat open 0) \
(strat row 0) \
 \
;;;; PATHS   \
 \
(arg does/2 0 oplayer/0) \
(arg does/2 0 xplayer/0) \
(arg does/2 1 mark/2 0 1/0) \
(arg does/2 1 mark/2 0 2/0) \
(arg does/2 1 mark/2 0 3/0) \
(arg does/2 1 mark/2 1 1/0) \
(arg does/2 1 mark/2 1 2/0) \
(arg does/2 1 mark/2 1 3/0) \
(arg does/2 1 noop/0) \
(arg goal/2 0 oplayer/0) \
(arg goal/2 0 xplayer/0) \
(arg goal/2 1 0/0) \
(arg goal/2 1 100/0) \
(arg goal/2 1 50/0) \
(arg init/1 0 cell/3 0 1/0) \
(arg init/1 0 cell/3 0 2/0) \
(arg init/1 0 cell/3 0 3/0) \
(arg init/1 0 cell/3 1 1/0) \
(arg init/1 0 cell/3 1 2/0) \
(arg init/1 0 cell/3 1 3/0) \
(arg init/1 0 cell/3 2 b/0) \
(arg init/1 0 control/1 0 xplayer/0) \
(arg legal/2 0 oplayer/0) \
(arg legal/2 0 xplayer/0) \
(arg legal/2 1 mark/2 0 1/0) \
(arg legal/2 1 mark/2 0 2/0) \
(arg legal/2 1 mark/2 0 3/0) \
(arg legal/2 1 mark/2 1 1/0) \
(arg legal/2 1 mark/2 1 2/0) \
(arg legal/2 1 mark/2 1 3/0) \
(arg legal/2 1 noop/0) \
(arg next/1 0 cell/3 0 1/0) \
(arg next/1 0 cell/3 0 2/0) \
(arg next/1 0 cell/3 0 3/0) \
(arg next/1 0 cell/3 1 1/0) \
(arg next/1 0 cell/3 1 2/0) \
(arg next/1 0 cell/3 1 3/0) \
(arg next/1 0 cell/3 2 b/0) \
(arg next/1 0 cell/3 2 o/0) \
(arg next/1 0 cell/3 2 x/0) \
(arg next/1 0 control/1 0 oplayer/0) \
(arg next/1 0 control/1 0 xplayer/0) \
(arg role/1 0 oplayer/0) \
(arg role/1 0 xplayer/0) \
(arg terminal/0) \
(arg true/1 0 cell/3 0 1/0) \
(arg true/1 0 cell/3 0 2/0) \
(arg true/1 0 cell/3 0 3/0) \
(arg true/1 0 cell/3 1 1/0) \
(arg true/1 0 cell/3 1 2/0) \
(arg true/1 0 cell/3 1 3/0) \
(arg true/1 0 cell/3 2 b/0) \
(arg true/1 0 cell/3 2 o/0) \
(arg true/1 0 cell/3 2 x/0) \
(arg true/1 0 control/1 0 oplayer/0) \
(arg true/1 0 control/1 0 xplayer/0) \
(arg column/2 0 1/0) \
(arg column/2 0 2/0) \
(arg column/2 0 3/0) \
(arg column/2 1 b/0) \
(arg column/2 1 o/0) \
(arg column/2 1 x/0) \
(arg diagonal/1 0 b/0) \
(arg diagonal/1 0 o/0) \
(arg diagonal/1 0 x/0) \
(arg line/1 0 b/0) \
(arg line/1 0 o/0) \
(arg line/1 0 x/0) \
(arg open/0) \
(arg row/2 0 1/0) \
(arg row/2 0 2/0) \
(arg row/2 0 3/0) \
(arg row/2 1 b/0) \
(arg row/2 1 o/0) \
(arg row/2 1 x/0) \
";


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
 * Test the Game constructor
 ****************************************************************/

BOOST_AUTO_TEST_CASE(game_construction)
{
    // Load tictactoe from text and from file
    Game game1(g_gdl);
    Game game2(boost::filesystem::path("./tictactoe.gdl"));

    // Basic test that the games are different
    BOOST_CHECK(game1 != game2);
    BOOST_CHECK(!(game1 == game2));

    boost::unordered_set<std::string> tmpset1;
    boost::unordered_set<std::string> tmpset2;

    // Check that the players for the two games are the same
    // (ie. have the same text strings)
    BOOST_CHECK(game1.numPlayers() > 0);
    BOOST_CHECK_EQUAL(game1.numPlayers(), game2.numPlayers());

    game1.players(boost::make_function_output_iterator(playername_loader(tmpset1)));
    game2.players(boost::make_function_output_iterator(playername_loader(tmpset2)));
    BOOST_CHECK(tmpset1 == tmpset2);

    // Now check the moves from the initial game states.
    tmpset1.clear();
    tmpset2.clear();
    State state1(game1);
    State state2(game2);

    state1.legals(boost::make_function_output_iterator(playermovename_loader(tmpset1)));
    state2.legals(boost::make_function_output_iterator(playermovename_loader(tmpset2)));
    BOOST_CHECK(tmpset1.size() > 0);
    BOOST_CHECK(tmpset1 == tmpset2);
}

/****************************************************************
 *
 ****************************************************************/

BOOST_AUTO_TEST_CASE(basic_game_tests)
{
    Game game(boost::filesystem::path("./tictactoe.gdl"));
    Game game2(boost::filesystem::path("./tictactoe.gdl"));

    BOOST_CHECK(game != game2);
    BOOST_CHECK(!(game == game2));

    boost::unordered_map<Player, std::string> playermap1;
    boost::unordered_map<Player, std::string> playermap2;
    std::vector<Player> players = game.players();

    BOOST_FOREACH(const Player& p, players)
    {
        playermap1[p] = p.tostring();
        playermap2[p] = p.tostring();
    }

    typedef std::pair<Player,std::string> pp_t;
    BOOST_FOREACH(const pp_t& pp, playermap1)
    {
        BOOST_CHECK_EQUAL(pp.first.tostring(), pp.second);
        BOOST_CHECK_EQUAL(playermap2[pp.first], pp.second);
    }
}

/****************************************************************
 * Test functions of the State class
 ****************************************************************/

BOOST_AUTO_TEST_CASE(state_functions)
{
    Game game(boost::filesystem::path("./tictactoe.gdl"));
    State state1 = game.initState();
    BOOST_CHECK(!state1.isTerminal());

    State state2(state1);
    BOOST_CHECK(!state2.isTerminal());

    /* Test legals */
    std::vector<PlayerMove> legs;
    state1.legals(std::back_inserter(legs));
    BOOST_CHECK_EQUAL(legs.size(), 10);

    /* Test legals() */
    boost::unordered_map<Player, std::vector<Move> > legmoves = state1.legals();
    unsigned mcount = 0;
    typedef std::pair<Player, std::vector<Move> > pms_t;
    BOOST_FOREACH(const pms_t& pms, legmoves)
    {
        mcount += pms.second.size();
    }
//    boost::shared_ptr<std::vector<PlayerMove> > legs_ptr;
//    legs_ptr = state1.legals();
//    BOOST_CHECK_EQUAL(legs_ptr->size(), 10);
    BOOST_CHECK_EQUAL(mcount, 10);

    // To test the PortableState we make a single move from the
    // initial state, then save this as a portable state. Run a
    // playout, reload the portable state and check that this is takes
    // us back to the correct state.

    // NOTE: There is no function to compare that two states are equal!
    // Best we can do is test that it is no-longer terminal. Need to
    // ask Michael to add some functions.

    std::vector<PlayerMove> does;
    does.push_back(pick_first(legs, "xplayer"));
    does.push_back(pick_first(legs, "oplayer"));
    BOOST_CHECK_EQUAL(does.size(), 2);
    state1.play(does);
    state2.play(does);
    BOOST_CHECK(!state1.isTerminal());
    BOOST_CHECK(!state2.isTerminal());
    boost::shared_ptr<PortableState> pstate(new PortableState(state2));

    std::vector<PlayerGoal> result;
    state2.playout(result);
    BOOST_CHECK(state2.isTerminal());

    State state3(game, *pstate);
    BOOST_CHECK(!state3.isTerminal());

    // Now test the serialisation of Portable state. Do the same test
    // as before but by taking the further step of (de-)serializing.
    std::ostringstream oserialstream;
    boost::archive::text_oarchive oa(oserialstream);
    oa << pstate;
    std::string serialised(oserialstream.str());
    BOOST_TEST_MESSAGE(serialised);

    std::istringstream iserialstream(serialised);
    boost::archive::text_iarchive ia(iserialstream);

    boost::shared_ptr<PortableState> pstate2;
    ia >> pstate2;

    // Test the State assigment operator
    state2 = State(game, *pstate2);
    BOOST_CHECK(!state2.isTerminal());
    state2.playout(result);
    BOOST_CHECK(state2.isTerminal());

    State state4(game, *pstate2);
    BOOST_CHECK(!state4.isTerminal());

}

/****************************************************************
 * Test that the move and player text generated for tictactoe
 * are correct.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(text_check)
{
    Game game(boost::filesystem::path("./tictactoe.gdl"));
    State state1 = game.initState();
    BOOST_CHECK(!state1.isTerminal());

    std::vector<PlayerMove> legs;
    state1.legals(std::back_inserter(legs));

    unsigned int count = 0;
    bool matchply = false;
    bool matchmv = false;
    BOOST_FOREACH(const PlayerMove& pm, legs)
    {
        std::string ply = pm.first.tostring();
        std::string mv = pm.second.tostring();
        BOOST_CHECK(ply == "oplayer" || ply == "xplayer");
        BOOST_CHECK(mv == "noop" || mv == "(mark 1 1)" ||
                    mv == "(mark 1 2)" || mv == "(mark 1 3)" ||
                    mv == "(mark 2 1)" || mv == "(mark 2 2)" ||
                    mv == "(mark 2 3)" || mv == "(mark 3 1)" ||
                    mv == "(mark 3 2)" || mv == "(mark 3 3)");
    }
}


// NOTE: WE CURRENTLY HAVE NO WAY OF PICKING A MOVE EXPLICITLY. SO THE BEST
// THAT WE CAN DO IS TO PICK THE FIRST FOR EACH PLAYER AND THEN TEST THAT
// IT IS NOT TERMINAL FOR THE FIRST 5 MOVES.
BOOST_AUTO_TEST_CASE(tictactoe)
{
//  BOOST_TEST_MESSAGE("Testing HSFC with tictactoe");
//  BOOST_CHECK_THROW(Game game1("./nonexistentfile"), HSFCException);

    Game game(boost::filesystem::path("./tictactoe.gdl"));
    BOOST_CHECK_EQUAL(game.numPlayers(), 2);
    State state = game.initState();
    BOOST_CHECK(!state.isTerminal());

//  BOOST_TEST_MESSAGE("Choosing legal joint moves until terminal intersperced with playouts.");
    // Tictactoe is turn taking with available moves counting
    // down from 9.
    unsigned int turn = 0;
    int step= 9;
    while (!state.isTerminal())
    {
        tictactoe_playout_check(state);
        BOOST_CHECK(!state.isTerminal());
        std::vector<PlayerMove> legs;
        state.legals(std::back_inserter(legs));
        if (turn == 0)
        {
            BOOST_CHECK_EQUAL(count_player_moves(legs, "xplayer"), step);
            BOOST_CHECK_EQUAL(count_player_moves(legs, "oplayer"), 1);
        }
        else
        {
            BOOST_CHECK_EQUAL(count_player_moves(legs, "xplayer"), 1);
            BOOST_CHECK_EQUAL(count_player_moves(legs, "oplayer"), step);
        }
        std::vector<PlayerMove> does;
        does.push_back(pick_first(legs, "xplayer"));
        does.push_back(pick_first(legs, "oplayer"));
        state.play(does);
        turn = (++turn) % 2;
        --step;
    }
    // Tictactoe will terminate early only if there is a winner
    // so test for this and also that a draw is 50/50.
    BOOST_CHECK(step < 5);
    std::vector<PlayerGoal> results;
    state.goals(std::back_inserter(results));
    BOOST_CHECK_EQUAL(results.size(), 2);

//    boost::shared_ptr<std::vector<PlayerGoal> > results_ptr;
//    results_ptr = state.goals();
//    BOOST_CHECK_EQUAL(results_ptr->size(), 2);

    JointGoal results2 = state.goals();
    BOOST_CHECK_EQUAL(results2.size(), 2);

    bool haswinner =
        (results[0].second == 100 && results[1].second == 0) ||
        (results[1].second == 100 && results[0].second == 0);
    if (step > 1) BOOST_CHECK(haswinner);
    if (!haswinner)
    {
        BOOST_CHECK(results[0].second == 50);
        BOOST_CHECK(results[1].second == 50);
    }
    BOOST_TEST_MESSAGE("Succesfully played tictactoe till termination.");
}


/****************************************************************
 * Test the state changes correctly even when we don't call
 * legals(). Deep in the HSFC a state is not valid until the
 * legal moves have been calculated by it. I hack around this by
 * calling legals() whenever a state is created/copied/assigned.
 ****************************************************************/

BOOST_AUTO_TEST_CASE(state_transition)
{
    Game game(boost::filesystem::path("./tictactoe.gdl"));
    State main_state(game);

    /* Use a sequence of two joint moves as my test-case: jm1, jm2 */
    std::vector<JointMove> tmp_jms = main_state.joints();
    BOOST_CHECK_EQUAL(tmp_jms.size(),9);
    JointMove jm1 = tmp_jms[0];
    main_state.play(jm1);
    tmp_jms = main_state.joints();
    BOOST_CHECK_EQUAL(tmp_jms.size(),8);
    JointMove jm2 = tmp_jms[0];

    /* Now we use jm1 and jm2 to test various state transitions
       for states that were copied and assigned without calculating
       the legals in between play() calls. */
    State state1(game);
    state1.play(jm1);
    state1.play(jm2);
    BOOST_CHECK_EQUAL(get_num_moves(state1, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state1, "oplayer"), 1);

    State state2(game);
    state2.play(jm1);
    State state3(state2);
    state2.play(jm2);
    state3.play(jm2);
    BOOST_CHECK_EQUAL(get_num_moves(state2, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state2, "oplayer"), 1);
    BOOST_CHECK_EQUAL(get_num_moves(state3, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state3, "oplayer"), 1);

    State state4(game);
    State state5(state4);
    state4.play(jm1);
    state5 = state4;
    state4.play(jm2);
    state5.play(jm2);
    BOOST_CHECK_EQUAL(get_num_moves(state4, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state4, "oplayer"), 1);
    BOOST_CHECK_EQUAL(get_num_moves(state5, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state5, "oplayer"), 1);

    State state6(game);
    State state7(game);
    state6.play(jm1);
    state7 = state6;
    state6.play(jm2);
    state7.play(jm2);
    BOOST_CHECK_EQUAL(get_num_moves(state6, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state6, "oplayer"), 1);
    BOOST_CHECK_EQUAL(get_num_moves(state7, "xplayer"), 7);
    BOOST_CHECK_EQUAL(get_num_moves(state7, "oplayer"), 1);
}

/****************************************************************
 * Related to the issue that calculating the legal moves does itself
 * modify the internal structure of a State, similarly testing
 * for termination also modifies the internal structure of the
 * State. As part of the fix for the legals moves we do test
 * for termination when creating/copying/assigning states as
 * well as updating the state after a joint move play(). However,
 * I wasn't testing for termination after a playout(). So
 * have now added a termination test that happens after playout()
 * is call to ensure that the state is valid.
 *
 ****************************************************************/

BOOST_AUTO_TEST_CASE(state_transition_from_playout)
{
    Game game(boost::filesystem::path("./tictactoe.gdl"));
    State state(game);

    /* Perform a playout from a state. Then make a portable copy
       before and after testing for termination. */

    state.playout();
    PortableState pstate1(state);
    BOOST_CHECK(state.isTerminal());

    PortableState pstate2(state);
    BOOST_CHECK(pstate1 == pstate2);

}


/*
 * Even though two joint moves are equivalent they can have different
 * ordering when iterated. This can be caused by a different hash
 * bucket size.
 */
bool same_order(const JointMove& jm1, const JointMove& jm2)
{
    BOOST_CHECK(jm1 == jm2);

    typedef JointMove::const_iterator tmp_t;
    tmp_t iter1 = jm1.begin();
    tmp_t iter2 = jm2.begin();
    while (iter1 != jm1.end())
    {
        if (*iter1++ != *iter2++) return false;
    }
    return true;
}

BOOST_AUTO_TEST_CASE(test_hashing)
{
    Game game(g_gdl);
    State state(game);
    Player xplayer = get_player(game, "xplayer");
    Player oplayer = get_player(game, "oplayer");
    BOOST_CHECK(xplayer.tostring() == "xplayer");
    BOOST_CHECK(oplayer.tostring() == "oplayer");

    Move xmark11 = get_move(state, xplayer, "(mark 1 1)");
    Move xmark12 = get_move(state, xplayer, "(mark 1 2)");
    Move xmark13 = get_move(state, xplayer, "(mark 1 3)");
    Move onoop = get_move(state, oplayer, "noop");
    BOOST_CHECK(xmark11.tostring() == "(mark 1 1)");
    BOOST_CHECK(xmark12.tostring() == "(mark 1 2)");
    BOOST_CHECK(xmark13.tostring() == "(mark 1 3)");
    BOOST_CHECK(onoop.tostring() == "noop");

    JointMove jm1 = JointMove(1);
    jm1.insert(std::make_pair(oplayer, onoop));
    jm1.insert(std::make_pair(xplayer, xmark11));

    unsigned int i = 10000;
    bool tested_jm_hashing = false;
    while (i < 10010)
    {
        JointMove jm2 = JointMove(i++);
        jm2.insert(std::make_pair(xplayer, xmark11));
        jm2.insert(std::make_pair(oplayer, onoop));
        BOOST_CHECK(jm1.bucket_count() != jm2.bucket_count());
        BOOST_CHECK(hash_value(jm1) == hash_value(jm2));
        if (!same_order(jm1, jm2)) tested_jm_hashing = true;
    }
    BOOST_CHECK_MESSAGE(tested_jm_hashing,
                       "Couldn't generate different iteration " <<
                       "ordering for JointMove to test hashing");

    JointMove jm3 = JointMove();
    jm3.insert(std::make_pair(xplayer, xmark11));
    jm3.insert(std::make_pair(oplayer, onoop));
    state.play(jm3);

    Move omark12 = get_move(state, oplayer, "(mark 1 2)");
    Move omark13 = get_move(state, oplayer, "(mark 1 3)");
    Move xnoop = get_move(state, xplayer, "noop");
    BOOST_CHECK(omark12.tostring() == "(mark 1 2)");
    BOOST_CHECK(omark13.tostring() == "(mark 1 3)");
    BOOST_CHECK(xnoop.tostring() == "noop");

    JointMove jm4 = JointMove();
    jm4.insert(std::make_pair(oplayer, omark12));
    jm4.insert(std::make_pair(xplayer, xnoop));
    state.play(jm4);

    Move xmark13_2 = get_move(state, xplayer, "(mark 1 3)");
    Move onoop_2 = get_move(state, oplayer, "noop");
    BOOST_CHECK(xmark13_2.tostring() == "(mark 1 3)");
    BOOST_CHECK(onoop_2.tostring() == "noop");

    BOOST_CHECK(xmark13 == xmark13_2);
    BOOST_CHECK(onoop == onoop_2);
    BOOST_CHECK(hash_value(xmark13) == hash_value(xmark13_2));
    BOOST_CHECK(hash_value(onoop) == hash_value(onoop_2));
    BOOST_CHECK(boost::hash<Move>()(xmark13) == boost::hash<Move>()(xmark13_2));
    BOOST_CHECK(boost::hash<Move>()(onoop) == boost::hash<Move>()(onoop_2));
}




/*

Note: there is no easy way to test that an assertion is
triggered. Would have to use Boost Assert and set an special handler
to throw an exception on assertion failure rather than the default
behaviour of calling std::abort(). Hence commenting out the code
below.
*/
/*
BOOST_AUTO_TEST_CASE(test_invalid_game_comparisions)
{
    Game game_a(g_gdl);
    Game game_b(g_gdl);

    std::vector<Player> players_a = game_a.players();
    std::vector<Player> players_b = game_b.players();

//    BOOST_CHECK(players_a == players_b, BoostAssertError);
    BOOST_CHECK(players_a == players_b);

}
*/

