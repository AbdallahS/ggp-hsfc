//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HSFCTest

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/foreach.hpp>
#include <sexprtoflat.h>

using namespace HSFC;


BOOST_AUTO_TEST_CASE(sexprtoflat)
{
    // For s-expression there is a difference between X and (X).
    // But flat cannot represent (X).
	BOOST_CHECK_EQUAL(sexpr_to_flat("aa"), "aa");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa ab)"), "aa/1 ab");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa (ab ac) ad)"), "aa/2 ab/1 ac ad");
	BOOST_CHECK_EQUAL(sexpr_to_flat("(aa (ab ac (ad ae) af) ag)"), "aa/2 ab/3 ac ad/1 ae af ag");

	// This will fail
//	BOOST_CHECK_EQUAL(sexpr_to_flat("((aa ab) ac)"), "aa/1/1 ab ac");

}

BOOST_AUTO_TEST_CASE(flattosexpr)
{
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa"), "aa");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/1 ab"), "(aa ab)");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/2 ab/1 ac ad"), "(aa (ab ac) ad)");
	BOOST_CHECK_EQUAL(flat_to_sexpr("aa/2 ab/3 ac ad/1 ae af ag"), "(aa (ab ac (ad ae) af) ag)");
}

BOOST_AUTO_TEST_CASE(normalisegdl)
{
    // Test role init, legal, not, true
    const std::string gdl1 = "(ROLE BLACK) (INIT BLUE) (<= (LEGAL WHITE TEST) (NOT (TRUE (TRUE T))))";
    const std::string egdl1 = "(role BLACK) (init BLUE) (<= (legal WHITE TEST) (not (true (TRUE T))))";
    std::string ngdl1 = gdl_keywords_to_lowercase(gdl1);
    BOOST_CHECK_EQUAL(ngdl1, egdl1);

    // Test legal, true, and, or
    const std::string gdl2 = "(<= (LEGAL WHITE TEST) (AND (TRUE T) (OR B1 B2)))";
    const std::string egdl2 = "(<= (legal WHITE TEST) (and (true T) (or B1 B2)))";
    std::string ngdl2 = gdl_keywords_to_lowercase(gdl2);
    BOOST_CHECK_EQUAL(ngdl2, egdl2);

    // Test terminal, and, true, or, does
    const std::string gdl3 = "(<= TERMINAL (AND (TRUE T) (OR (DOES B 2) B2)))";
    const std::string egdl3 = "(<= terminal (and (true T) (or (does B 2) B2)))";
    std::string ngdl3 = gdl_keywords_to_lowercase(gdl3);
    BOOST_CHECK_EQUAL(ngdl3, egdl3);

    // Test multi-line
    const std::string gdl4 = "\n\
;; This is a test \n\
(<= TERMINAL ;; terminate now\n\
(AND (TRUE T) (OR (DOES B 2) B2)))";
    std::string ngdl4 = gdl_keywords_to_lowercase(gdl4);
    BOOST_CHECK_EQUAL(ngdl4, egdl3);

    // Test next, true, distinct
    const std::string gdl5  = "(<= (NEXT (SOME 1)) (TRUE (SOME 1)) (DISTINCT ?N (NOW)))";
    const std::string egdl5 = "(<= (next (SOME 1)) (true (SOME 1)) (distinct ?N (NOW)))";
    std::string ngdl5 = gdl_keywords_to_lowercase(gdl5);
    BOOST_CHECK_EQUAL(ngdl5, egdl5);

    // Test goal, does
    const std::string gdl6  = "(<= (GOAL R 1) (DOES R BREAK))";
    const std::string egdl6 = "(<= (goal R 1) (does R BREAK))";
    std::string ngdl6 = gdl_keywords_to_lowercase(gdl6);
    BOOST_CHECK_EQUAL(ngdl6, egdl6);

}
